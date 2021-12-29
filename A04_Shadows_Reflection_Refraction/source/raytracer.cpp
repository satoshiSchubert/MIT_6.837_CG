#include "raytracer.h"

Vec3f RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit) {

	Group *group = scene->getGroup();

	// Shader
	if (group->intersect(ray, hit, tmin)) {
		// Original ray without any bounces.
		if (bounces == 0) {
			// 画出从tmin（也就是视点附近）到交点的直线
			RayTree::SetMainSegment(ray, tmin, hit.getT());
		}
		// Initialize.
		Vec3f pixelColor(0.f, 0.f, 0.f);
		Vec3f hitPoint = hit.getIntersectionPoint();
		//DEBUG
		//hitPoint.show();
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
			Ray ray_shadows(hitPoint+0.1*lightDir, lightDir);
			Hit hit_shadows(distance);
			if (shadows) {
				if (!group->intersectShadowRay(ray_shadows, hit_shadows, tmin)) {
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
			pixelColor += attenuation * traceRay(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
			RayTree::AddReflectedSegment(lightScattered, tmin, hit_ref.getT());
		}
		// <----------Reflactive---------->
		if (bounces < max_bounces && weight>cutoff_weight && material->refract(ray, hit, attenuation, lightScattered)) {
			Hit hit_ref(INFINITY);
			// 递归
			pixelColor += attenuation * traceRay(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
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