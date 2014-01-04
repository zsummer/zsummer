Welcome to the zsummer wiki!  
  
# introduction:  
ZSUMMER是一款C++ 跨平台的 高性能的 轻量级的网络底层库, 支持TCP, UDP, 拥有完善的定时器机制与日志系统..  
ZSUMMER is a cross-platform C++ high performance lightweight network library.  
# feature:   
MIT开源 代码可以任意使用在任何场合.  
MIT source code can be used in any occasions.  
  
纯原生 不依赖任何第三方库!  
does not rely on any third party libraries  
  
所有代码全手工打造, 代码简洁清爽 每一句都经过仔细斟酌, 结构清晰顺畅, 命名规范易读, 性能稳定强大 .  
All of the code is all hand-made, code simple and refreshing every word carefully, structure is clear and smooth, easy name specification, performance is stable and powerful.  
  
使用EPOLL/IOCP纯手工实现,采用优雅简洁的设计模式, 跨LINUX/WINDOWS 32/64平台而几乎不因此损耗任何性能.  
Realize the use of EPOLL/IOCP manual, use design pattern elegant, across the LINUX/WINDOWS 32/64 platform and almost no loss of any property.  
  
上层接口采用规范的proactor一致的接口设计, 轻松应对绝大多数服务端的使用场景. 
        包括高并发高吞吐要求的前端 以及后端DB代理服务 逻辑业务服务等等.  
The upper interface with interface design specification of Proactor consistent, easy to deal with the vast majority of server usage scenarios.  
        High throughput requirements including high concurrent front and rear DB agent service logic service and so on.  
  
每个IOSERVER都可以无限制挂靠的connecter角色, accepter角色与clienter角色 充分发挥多核性能.  
Each IOSERVER can limit the link the role of connecter, accepter and clienter role into full play the role of multi-core performance.  

完全异步的接口设计 排除业务逻辑中可能存在的底层重入问题  
Interface design fully asynchronous rule out the possibility of the underlying problem of re-entry business logic.   
  
# 压测数据 stress report    
![ping-pong stress](https://raw.github.com/zsummer/wiki-pic/master/stress_report/ping_pong_stress.png)

![100k_stress](https://raw.github.com/zsummer/wiki-pic/master/stress_report/100k_stress.png)
 
详细见报告页:[stress-report](https://github.com/zsummer/zsummer/wiki/stress-report)   
For details see the report page: [stress-report] ( https://github.com/zsummer/zsummer/wiki/stress-report )


# build server & client  
in linux:   
cd zsummer/example  
cmake .  
make  

in windows:  
cd zsummer/example  
use vs2005 open  zsummer_11x.sln  


# auther: 张亚伟 YaweiZhang   
Web Site: www.zsummer.net  
mail: yawei_zhang@foxmail.com  
github: https://github.com/zsummer  
