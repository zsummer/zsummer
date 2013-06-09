  
# Thread4z
thread4z 是一款开源的C++轻量级跨平台线程库.  

# Introduction  
1. 封装了线程的创建销毁管理类 CThread , 此包装屏蔽了linux/windows的接口差异部分, 只需要继承该类即可轻松享受并发编程.
2. 封装了性能极佳的用户态线程锁CLock, 使用linux的mutex与windows的临界区实现并消除系统带来的差异部分.
3. 使用CLock类进一步包装了智能的CAutoLock, 在需要加锁的地方直接创建该类便可实现自动上锁和退出语句块自动退锁.
4. 封装了匿名信号量(进程内)和具名信号量(可跨进程使用) CSem, 满足进程间互斥, 进程间和进程内事件通知, 资源计数共享等特殊同步需求.
5. 封装了部分原子操作AtomicAdd,AtomicInc,AtomicDec, 对于简单的计数等操作可通过此接口进行操作,可实现比用户态线程锁更极速的操作性能.


#Auther: 张亚伟 
Web Site: www.zsummer.net  
mail: yawei_zhang@foxmail.com  
github: https://github.com/zsummer  
QQ Group: 19811947  
