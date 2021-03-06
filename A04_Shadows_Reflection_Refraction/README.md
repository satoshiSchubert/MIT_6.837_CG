## A4. Shadows, Reflection & Refraction
---

### 记录点

1. Ray Casting 和 Rendering Pipeline的区别
   
   我的理解是，这是两种计算模式？

   - Ray Casting：
     ```python
     for each pixel:
         for each object:
     ```
   - Rendering Pipeline:
     ```python
     for each triangle:
         for each pixel:
     ```

    ![](../pics/raycastingrenderpipeline.jpg)

    ![](../pics/rcandrp2.jpg)

2. Shadows
   
   先考虑引入了Phong Shading的Ray Casting：
   
   ```python
   #当你已经找到了最近的hit point时
   color = ambient*hit->getMaterial()->getDiffuseColor()
   for every light
        #对于每个光源，都分别计算并累加Shade颜色
        color += hit->getMaterial()->Shade(ray,hit,direction,lightColor)
    return color
   ```
   复杂度：$O(n*m*l)$,其中l=num of lights

   那么如何添加阴影呢？

    ```python
   #当你已经找到了最近的hit point时
   color = ambient*hit->getMaterial()->getDiffuseColor()
   # 对于每个光源
   for every light
        // ========CHANGE========
        #初始化一个从Hit点到光源的射线
        Ray ray2(hitPoint,directionToLight)
        #初始化一个t等于hitpoint到光源的距离的Hit
        Hit hit2(distanceToLight, NULL, NULL)
        #对于在场的所有物体（实际上只关注ray2光路上的物体）
        for every object:
            #迭代intersect
            object->intersect(ray2,hit2,0)
        #如果迭代之后t仍然还等于distanceToLight
        #也就是ray2光路上没有其他的阻碍物
        #这时候才更新Shade，否则就只留有ambient
        #（因为从光源到这个hitpoint之间有东西挡着）
        if(hit2->getT()=distanceToLight):
        // ========CHANGE========
            color += hit->getMaterial()->Shade(ray,hit,direction,lightColor)
    return color
   ```
   
   配图：注意橙色箭头和太阳之间有个物体阻挡。

   ![](../pics/shadow1.jpg)

   问题：自阴影（self-shadowing）--需要在计算intersect的时候添加一个epsilon（估计是增加容错率？）

   **这个epsilon现在还没搞懂是啥**

   ![](../pics/shadow2.jpg)

   **Shadow Optimization**如何提高shadow的计算效率呢？在检测ray2光路的交点时，只需要知道是否有其他交点，而不需要知道哪个交点最近

   这就需要写一个新的routine：``Object3D::intersectShadowRay()``
   and it stops at first intersection.

3. Reflection
   
   最基本的：入射角等于反射角。
   $$
   R=V-2(V·N)N
   $$

   ![](../pics/reflection1.jpg)

   在这次assignment中，用的是传统的ray tracing，反射量是个常量，但是如果考虑更加真实的情况的话，应该要考虑fresnel reflection term

   material->reflect函数的具体实现:

   ```cpp
   bool PhongMaterial::reflect(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &reflected) {
        // 这个函数用来计算反射光线并传递给参数ray
        // reflectiveColor是材料的“反射率”，如果太小则认为这个点无法产生反射，返回false
        // 这个材料的“反射率”作为参数传给attenuation，以用来衰减光线的weight
        if (_reflectiveColor.Length() < 0.001)
            return false;
        // 计算反射光线
        // R=V-2(V·N)N | R:Reflected, V:View, N:Normal
        Vec3f ray_in = ray.getDirection();
        Vec3f normal = hit.getNormal();
        Vec3f ray_out = ray_in - 2.f*normal.Dot3(ray_in)*normal;
        ray_out.Normalize();
        reflected = Ray(hit.getIntersectionPoint(), ray_out);
        attenuation = _reflectiveColor;
        return true;
    }
   ```

4. Refration
   
   经过（复杂的）推导，得到折射公式如下：

   $$
   \left[\eta_{r}(\mathbf{N} \cdot \mathbf{I})-\sqrt{1-\eta_{r}{ }^{2}\left(1-(\mathbf{N} \cdot \mathbf{I})^{2}\right)}\right] \mathbf{N}-\eta_{r} \mathbf{I}
   $$

   ![](../pics/refraction1.jpg)

   具体实现：

   ```cpp
   bool PhongMaterial::refract(const Ray &ray, const Hit &hit, Vec3f &attenuation, Ray &refracted) {
        if (_transparentColor.Length() < 0.001)
            return false;
        Vec3f ray_in = ray.getDirection();
        Vec3f normal = hit.getNormal();
        // Snell-Descartes Law:nr = (sinT)/(sinI) = ni/nt
        float nr;
        if (ray_in.Dot3(normal) > 0) {
            normal = -1 * normal;
            nr = _indexOfRefraction;
        }
        else {
            // 正常情况
            nr = 1.f / _indexOfRefraction;// 空气是1
        }
        Vec3f I = ray_in * -1; // 参考示意图！
        float N_I = normal.Dot3(I);
        float t = 1 - pow(nr, 2)*(1 - pow(N_I, 2));
        // t should be greater than 0
        if (t > 0) {
            Vec3f T = (N_I * nr - sqrtf(t)) * normal - nr * I;
            T.Normalize();
            refracted = Ray(hit.getIntersectionPoint(), T);
            // 向外传递参数
            attenuation = _transparentColor;
            return true;
        }
        else {
            return false;
        }
    }
    ```

5. RayTracer 框架
   
   ![](../pics/rtframework1.jpg)

   不断递归后的效果：

   ![](../pics/recusion.jpg)

   Ray Tree:

   ![](../pics/recursion2.jpg)

   代码的具体实现(参考fuzhanzhan大佬)：

   ```cpp
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
            Material *material = hit.getMaterial();
            // 环境光*物体材料颜色
            pixelColor += scene->getAmbientLight() * material->getDiffuseColor();
            // For each light souce:
            for (int k = 1; k < scene->getNumLights(); k++) {
                Light *light = scene->getLight(k);
                Vec3f lightDir, lightColor;
                float distance;
                light->getIllumination(hitPoint, lightDir, lightColor, distance);
                // <----------Shadows---------->
                Ray ray_shadows(hitPoint, lightDir);
                Hit hit_shadows(distance);
                if (shadows) {
                    if (!group->intersectShadowRay(ray_shadows, hit_shadows, tmin))
                        pixelColor += material->Shade(ray, hit, lightDir, lightColor);
                }
                else
                    // Routine without shadow  
                    pixelColor += material->Shade(ray, hit, lightDir, lightColor);
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
            return pixelColor;
        }
        else {
            return scene->getBackgroundColor();
        }
    }
   ```


6. Plane的intersect()部分DEBUG

    画第一个图的时候，我画出来的阴影总是不对，如下所示

    正常：

    ![](../pics/errorplane1.jpg)

    而我画出来的:

    ![](../pics/errorplane2.jpg)

    乍一看，（抛开球体没有上色不谈）阴影的投影好像没有什么大问题，但是和示例比起来可以发现球体的底部和平面有切割，且阴影也偏扁平，直观上看起来**似乎平面和球的距离变小了**。

    但是经过长时间的排查，排除了camera，light等等的原因，最后把问题定位在plane的intersect()函数上。经过对比公式，发现计算t的式子出现了一点错误(但是之前两次作业竟然一直都没发现！)，直到这次计算投影时才发现：

    原代码：

    ```cpp 
    float t = (_d - _v_norm.Dot3(Rd)) / denom;
    ```

    实际上正确的代码：

    ```cpp 
    float t = (_d - _v_norm.Dot3(Ro)) / denom;
    ```

    对应的计算公式：

    ![](../pics/errorplane3.jpg)



7. Sphere的intersection()部分DEBUG

画阴影的时候，球体的颜色无法正常显示，也被当成阴影处理了，因此显示出来的颜色和底色一样（如上图）。刚开始考虑是没有添加epsilon，以致球面和自己相交，被误判定为阴影，后来通过不断debug，发现问题出在基础的object3d模块上，Sphere类的intersect()函数有问题：

第一个问题在于intersect()中更新hit的操作。之前更新hit是直接令`h = Hit(t, _material, v_norm);`，并且也没有出现什么问题，前两次作业的结果都是OK的（因此这次作业也很难想到是这些基础模块出问题）。实际上，这里必须要用`h.set(t, _material, v_norm, r)`，不能新创建一个Hit并赋值，因为set函数中会根据ray内部设置intersectionPoint = ray.pointAtParameter(t)，而这个intersectionPoint又会在raytracer.cpp `Vec3f hitPoint = hit.getIntersectionPoint()` 中被调用，因此，如果没有设置，则无法正常调用并生成hitPoint，后面的ray_shadows也就会是错误的。虽然问题出在sphere的intersect()函数中，但是因为画shadow时在一个像素routine中会循环调用多次intersect()，所以很难打印t来发现问题。

第二个问题则是出在intersect()本身的if else结构上。<br>
之前的结构如下：

```cpp
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
	...
	if (delta < 0)
		// No hit.
		return false;
	float t = INFINITY ; //t的初始值应该是无限远！
	...
	if (t < h.getT() && t >= 0) {//closer当前交点，更新
		Vec3f v_norm = Vec3f(r.pointAtParameter(t), _center);
		v_norm.Normalize();
		h = Hit(t, _material, v_norm);
	}
	return true;
```
这种结构下，只要是delta>0的，不论是否`t < h.getT()`，最后都会return true，但是实际上会出现delta>0(有成功相交)但是并没有更新交点hit的情况，这种情况也应该是return false的。

修改后：

```cpp
bool Sphere::intersect(const Ray &r, Hit &h, float tmin) {
	...
	if (delta >= 0) {
		...
		if (t < h.getT()) {//closer当前交点，更新
			Vec3f v_norm = Ro + t * Rd;
			v_norm.Normalize();
			h.set(t, _material, v_norm, r);
			return true;
		}
	}
	return false;
```
这样，delta>0但是却没有更新交点的情况也会被判为false。

