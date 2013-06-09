Welcome to the zsummer wiki!  
  
# ZSUMMER简介:  
ZSUMMER是一款C++ 跨平台的 高性能的 轻量级的网络底层库.  
# ZSUMMER优点:   
MIT开源 代码可以任意使用在任何场合.  

纯原生 不依赖任何第三方库!  

所有代码全手工打造, 代码简洁清爽 每一句都经过仔细斟酌, 结构清晰顺畅, 命名规范易读, 性能稳定强大 .  

使用EPOLL/IOCP纯手工实现,采用优雅简洁的设计模式, 跨LINUX/WINDOWS 32/64平台而几乎不因此损耗任何性能.  

上层接口采用规范的proactor一致的接口设计, 轻松应对绝大多数服务端的使用场景. 
        包括高并发高吞吐要求的前端 以及后端DB代理服务 逻辑业务服务等等.  

每个IOSERVER都可以无限制挂靠的connecter角色, accepter角色与clienter角色 充分发挥多核性能.  

完全异步的接口设计 排除业务逻辑中可能存在的底层重入问题  

  
# 压测数据  
WINDOWS: 60K并发 吞吐量120M 服务端每秒120,000次的消息包处理与收发 CPU总占用小于1%.  
LINUX, 因虚拟机性能问题 数据比windows差点: 40K并发 吞吐量80M 服务端每秒80,000次的消息包处理与收发, CPU总占用小于30%. 
详细见报告页:[stress-report](https://github.com/zsummer/zsummer/wiki/stress-report)   


# build server test  
in linux:   
cd test/zsummer_server  
cmake .  
make  

in windows:  
cd test/zsummer_server/vc8  
or  
cd test/zsummer_server/vc10  

use vs2005 or vs2010 open  zsummer.sln  

# build stress client test  
cd test/asio_client  

use vs2010 open asio_client.sln  
It need the boost asio support   

# auther: 张亚伟 
QQ Group: 19811947  
Web Site: www.zsummer.net  
mail: yawei_zhang@foxmail.com  
github: https://github.com/zsummer  
