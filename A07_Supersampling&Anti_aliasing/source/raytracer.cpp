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
	//���Grid��intersect������Ҳ��Ҫ����RT���֣�
	//������ߺ������ཻ
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
				Hit hit_shadows(dist); //��ʼ������Զ
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
			// ������tmin��Ҳ�����ӵ㸽�����������ֱ��
			RayTree::SetMainSegment(ray, tmin, hit.getT());
		}
		// Initialize.
		Vec3f pixelColor(0.f, 0.f, 0.f);
		Vec3f hitPoint = hit.getIntersectionPoint();
		Material *material = hit.getMaterial();
		// ������*���������ɫ
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
					//�����Դ��hitpoint֮��û���ϰ�����ô������ɫ
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
		Vec3f attenuation;// ˥���ʣ�ʵ�����ǻ������ϵķ����ʣ�Խ����Խ�ߣ�
		// �������������Ӧ����С��1�ģ����ÿ�η������weight������˥��

		// �����������С����󷴵������ҹ���weight������ֵweight
		// reflect�����м��㷴�����
		if (bounces<max_bounces && weight>cutoff_weight && material->reflect(ray, hit, attenuation, lightScattered)) {
			// ����Զ�Ĳο�Hit
			Hit hit_ref(INFINITY);
			// �ݹ�
			pixelColor += attenuation * RayCast(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
			RayTree::AddReflectedSegment(lightScattered, tmin, hit_ref.getT());
		}
		// <----------Reflactive---------->
		if (bounces < max_bounces && weight>cutoff_weight && material->refract(ray, hit, attenuation, lightScattered)) {
			Hit hit_ref(INFINITY);
			// �ݹ�
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

