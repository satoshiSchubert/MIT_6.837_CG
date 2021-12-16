# MIT_6.837_CG
Assignment repo for Course MIT_6.837

## Assignment 2

碰到的一个最大的bug：

大概在180行：<br/>
这里的Hit类 h 不能是新建的h，而应该是直接引用的h。<br/>
为什么呢？之前新建Hit hit也能跑通啊？<br/>
因为之前每一个obj都只由一个primitive构成，每个primitive之间可以解耦，所以primitive内的h不需一定是继承的<br/>
但是碰到一个obj由多个primitive构成的情况时，在一类的primitive里需要进行多次比较，就需要用到历史的hit数据，这时hit不是解耦的历史的hit是有意义的（比如兔子mesh前面的三角形挡住了后面的三角形）<br/>
因此在transform内的hit是不能新建的。