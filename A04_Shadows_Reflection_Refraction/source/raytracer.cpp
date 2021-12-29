#include "raytracer.h"

Vec3f RayTracer::traceRay(Ray &ray, float tmin, int bounces, float weight, Hit &hit) {

	Group *group = scene->getGroup();

	// Shader
	if (group->intersect(ray, hit, tmin)) {
		// Original ray without any bounces.
		if (bounces == 0) {
			// ������tmin��Ҳ�����ӵ㸽�����������ֱ��
			RayTree::SetMainSegment(ray, tmin, hit.getT());
		}
		// Initialize.
		Vec3f pixelColor(0.f, 0.f, 0.f);
		Vec3f hitPoint = hit.getIntersectionPoint();
		//DEBUG
		//hitPoint.show();
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
		Vec3f attenuation;// ˥���ʣ�ʵ�����ǻ������ϵķ����ʣ�Խ����Խ�ߣ�
		// �������������Ӧ����С��1�ģ����ÿ�η������weight������˥��

		// �����������С����󷴵������ҹ���weight������ֵweight
		// reflect�����м��㷴�����
		if (bounces<max_bounces && weight>cutoff_weight && material->reflect(ray, hit, attenuation, lightScattered)) {
			// ����Զ�Ĳο�Hit
			Hit hit_ref(INFINITY);
			// �ݹ�
			pixelColor += attenuation * traceRay(lightScattered, tmin, bounces + 1, weight*attenuation.Length(), hit_ref);
			RayTree::AddReflectedSegment(lightScattered, tmin, hit_ref.getT());
		}
		// <----------Reflactive---------->
		if (bounces < max_bounces && weight>cutoff_weight && material->refract(ray, hit, attenuation, lightScattered)) {
			Hit hit_ref(INFINITY);
			// �ݹ�
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