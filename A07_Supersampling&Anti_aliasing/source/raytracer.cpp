#include "raytracer.h"
#include "raytracing_stats.h"

Vec3f RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit) {
	if (nx != 0)
		return RayTracer::RayCastFast(ray, tmin, bounces, weight, hit);
	else
		return RayTracer::RayCast(ray, tmin, bounces, weight, hit);
}

Vec3f RayTracer::RayCastFast(Ray &ray, float tmin, int bounces, float weight, Hit &hit) {
	// Grid Accelerated Shader
	//因此Grid的intersect函数中也需要包含RT部分？
	//如果光线和网格相交
	if (grid->intersect(ray, hit, tmin)) {
		Vec3f color(0.f, 0.f, 0.f);
		Vec3f pos = hit.getIntersectionPoint();
		Material *material = hit.getMaterial();
		color += scene->getAmbientLight()*hit.getMaterial()->getDiffuseColor();
		for (int k = 0; k < scene->getNumLights(); k++) {
			Light *light = scene->getLight(k);
			Vec3f l, lightColor;
			float dist;
			light->getIllumination(pos, l, lightColor, dist);
			//color += hit.getMaterial()->Shade(ray, hit, l, lightColor);

			if (shadows) {
				Ray ray_shadows(pos, l); //Direction to the light
				Hit hit_shadows(dist); //初始是无限远
				RayTracingStats::IncrementNumShadowRays();
				if (!grid->intersectShadowRay(ray_shadows, hit_shadows, tmin))
					color += material->Shade(ray, hit, l, lightColor);
			}
			else {
				color += material->Shade(ray, hit, l, lightColor);
			}
		}
		//reflective
		Ray scattered;
		Vec3f attenuation;
		if (bounces < max_bounces && weight > cutoff_weight && material->reflect(ray, hit, attenuation, scattered)) {
			Hit hit_ref(INFINITY);
			color += attenuation * RayCastFast(scattered, tmin, bounces + 1, weight * attenuation.Length(), hit_ref);
			RayTree::AddReflectedSegment(scattered, tmin, hit_ref.getT());
		}
		//refraction transparent
		if (bounces < max_bounces && weight > cutoff_weight && material->refract(ray, hit, attenuation, scattered)) {
			Hit hit_ref(INFINITY);
			color += attenuation * RayCastFast(scattered, tmin, bounces + 1, weight * attenuation.Length(), hit_ref);
			RayTree::AddTransmittedSegment(scattered, tmin, hit_ref.getT());
		}


	
		return color;
	}
	else {
		return scene->getBackgroundColor();
	}
}

Vec3f RayTracer::RayCast(Ray &ray, float tmin, int bounces, float weight, Hit &hit) {
	// RT Shader
	Group *group = scene->getGroup();
	if (group->intersect(ray, hit, tmin)) {
		// Original ray without any bounces.
		if (bounces == 0) {
			// 画出从tmin（也就是视点附近）到交点的直线
			RayTree::SetMainSegment(ray, tmin, hit.getT());
		}
		// Initialize.
		Vec3f pixelColor(0.f, 0.f, 0.f);
		Vec3f hitPoint = hit.getIntersectionPoint();
		Material *material = hit.getMaterial();
		// 环境光*物体材料颜色
		pixelColor += scene->getAmbientLight() * material->getDiffuseColor();
		// For each light souce:
		for (int k = 0; k < scene->getNumLights(); k++) {
			Light *light = scene->getLight(k);
			Vec3f lightDir, lightColor;
			float distance;
			light->getIllumination(hitPoint, lightDir, lightColor, distance);
			// <----------Shadows---------->
			Ray ray_shadows(hitPoint + 0.1*lightDir, lightDir);
			Hit hit_shadows(distance);
			if (shadows) {
				RayTracingStats::IncrementNumShadowRays();
				if (!group->intersectShadowRay(ray_shadows, hit_shadows, tmin)) {
					//如果光源到hitpoint之间没有障碍，那么可以着色
					pixelColor += material->Shade(ray, hit, lightDir, lightColor);
				}
			}
			else {
				// Routine without shadow  
				pixelColor += material->Shade(ray, hit, lightDir, lightColor);
			}
			RayTree::AddShadowSegment(ray_shadows, tmin, hit_shadows.getT());
		}
		// <----------Reflective---------->
		Ray lightScattered;
		Vec3f attenuation;// 衰减率（实际上是击打点材料的反射率，越镜面越高）
		// 但是这个反射率应该是小于1的，因此每次反射光线weight都会有衰减

		// 如果反弹次数小于最大反弹次数且光线weight大于阈值weight
		// reflect函数中计算反射光线
		if (bounces<max_bounces && weight>cutoff_weight && material->reflect(ray, hit, attenuation, lightScattered)) {
			// 无穷远的参考Hit
			Hit hit_ref(INFINITY);
			// 递归
			pixelColor += attenuation * RayCast(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
			RayTree::AddReflectedSegment(lightScattered, tmin, hit_ref.getT());
		}
		// <----------Reflactive---------->
		if (bounces < max_bounces && weight>cutoff_weight && material->refract(ray, hit, attenuation, lightScattered)) {
			Hit hit_ref(INFINITY);
			// 递归
			pixelColor += attenuation * RayCast(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
			RayTree::AddTransmittedSegment(lightScattered, tmin, hit_ref.getT());
		}
		//DEBUG
		//pixelColor.show();
		return pixelColor;
	}
	else {
		return scene->getBackgroundColor();
	}
}

