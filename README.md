个人对GAMES101课程作业的实现的记录 -w-

## 项目配置

在 Win10 上配置的环境

Eigen 3.4.0

OpenCV 4.5.4

https://zhuanlan.zhihu.com/p/259208999

基本是按这个来的，需要注意 Opencv 版本不要太高

对于每个Assignment中的CmakeLists.txt，需要把 include_directories 改成自己Eigen库的目录，同时将代码中的

```
#include<eigen3/Eigen/Eigen>
```

 改为

```
#include<Eigen/Eigen>
```

