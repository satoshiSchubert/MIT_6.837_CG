## A4. Shadows, Reflection & Refraction
---

### 记录点

0. 补充：类成员函数中const的使用
   
   参考：https://www.jianshu.com/p/35c8a245d62d

   一般放在函数体后，形如：`void fun() const;`

    如果一个成员函数的不会修改数据成员，那么最好将其声明为const，因为const成员函数中不允许对数据成员进行修改，如果修改，编译器将报错，这将大大提高程序的健壮性。
   

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

2.  Bounding Box

   如何对RT算法进行加速?

   cost ≈ height * width * num primitives * intersection cost * size of recursive ray tree * num shadow rays *num supersamples *num glossy rays * num temporal samples *num focal samples * ...

   那么其实，可以从减少num primitives做起。

   这就可以使用Bounding Box了。每个primitives都有一个最小的BBOX，通过计算筛去绝对不会和视野相交的BBOX及其所包含的Primitives，就可以实现减少计算量。

   复习一下如何判断一个Box和视线相交：

   ![](../pics/bbox.jpg)

   那么如何计算每一种primitive的bbox呢?实际上很简单，只需要注意求最大最小值就行：

   球的BBOX：

   ![](../pics/bbox1.jpg)

   平面的BBOX（通常来说是无穷大）：

   ![](../pics/bbox2.jpg)

   对应transform的BBOX：

   ![](../pics/bbox3.jpg)

   三角形则比较特殊，他的BBOX如下：

   ![](../pics/bbox4.jpg)

   而对于变换后的三角形，BBOX如下：

   ![](../pics/bbox5.jpg)

3. 父类中若需要用到子类，则需要在前面提前声明
   
   在object3d.h中，父类Object3D中用到了他的一个派生子类Grid,如果不在其之前声明则会报错。

   ```cpp
   //这里需要提前声明一下Grid
    class Grid;
    // Object3D
    class Object3D {
        ...
    }
    ```

4. Grid加速的总体框架
    
    
    1. 首先先创建一个Grid，并且确定分辨率（nx，ny），注意每个格子的单位长宽不能相等（为什么？）

    ![](../pics/3ddda1.jpg)

    2. 将Objects插入Grid

    ![](../pics/grid1.jpg)

    3. 对于某一条射线经过的所有cell，进行判断

    当前经过的cell是否有objects插入？如有，则返回最近的插入，如没有，则继续

    ![](../pics/grid2.jpg)

    4. 避免重复计算（这步不是很清楚，如果有效交点在后面的cell怎么办？）

    光线碰到一个object时，先mark一下；之后如果再碰到这个mark过的object，就不重复计算交点了。
    
    ![](../pics/grid3.jpg)

    5. 忽略超出Grid范围的intersect(这步也不是很清楚，为什么maybe sth closer？)

    ![](../pics/grid4.jpg)

    6. 那么对于某条光线，如何得到它路径上所经过的cell呢？这就需要用到3DDDA了

5. 3D DDA(Three Dimensional Digital Difference Analyzer)

    参考：https://zhuanlan.zhihu.com/p/415869768

    在求光线和voxel的交点时，更方便的方法是初始阶段先手动计算光线和第一个voxel焦点，之后再使用3D DDA方法不断扩展。

    3D DDA方法：<br>

    1. 注意到x轴和y轴中的间隔是均匀的

    这里的dirx等同于角度的cos，diry是角度的sine

    ![](../pics/3ddda1.jpg)

    2. 不断步进

    我们维护一个距离，而每个交点由于在坐标轴上，而网格是均匀分割的，因而同样平行线上 的所有交点（如上图中所有绿点）与起点距离可以轻松的累加得到。我们可以发现这个类似于DDA的算法，由于我们每次可能的交点都会出现在三个轴平面上，我们分别寻找接下来分别在三个轴平面上的三个交点，获取最近的那个做步进，显然我们根据交点所在的轴得知下一个cell的方位，比如我们交于xy平面，那肯定就是z轴步进1了。并且这样的做法一定是保守的。
    
    ![](../pics/3ddda2.jpg)

