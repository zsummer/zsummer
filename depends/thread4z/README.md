Welcome to the Thread4z wiki!  
# Thread4z
Thread4z 是一款开源的C++轻量级跨平台线程库.  
Thread4z is an open source C++ lightweight cross-platform thread library.  

# Introduction  
1. 封装了线程的创建销毁管理类 CThread , 此包装屏蔽了linux/windows的接口差异部分, 只需要继承该类即可轻松享受并发编程[package of thread creation, destruction and management CThread, this packaging shielding interface between linux/windows parts, needs only to extend this class can easily enjoy concurrent programming].
2. 封装了性能极佳的用户态线程锁CLock, 使用linux的mutex与windows的临界区实现并消除系统带来的差异部分[package of userland threads have the excellent performance of the lock CLock, mutex and windows critical region using Linux implementation and eliminate difference between system parts].
3. 使用CLock类进一步包装了智能的CAutoLock, 在需要加锁的地方直接创建该类便可实现自动上锁和退出语句块自动退锁[The 3 uses the CLock class to further packaging intelligent CAutoLock, create the class directly to realize the automatic locking and exit statement block automatic back to lock in the need for locks in place].
4. 封装了匿名信号量(进程内)和具名信号量(可跨进程使用) CSem, 满足进程间互斥, 进程间和进程内事件通知, 资源计数共享等特殊同步需求[packaging anonymous signal (process) and named semaphores (the process) CSem, meet the mutex between processes, the course of events and process resource sharing and other special notice, count synchronization requirements].
5. 封装了部分原子操作AtomicAdd,AtomicInc,AtomicDec, 对于简单的计数等操作可通过此接口进行操作,可实现比用户态线程锁更极速的操作性能[packaging part of atomic operations AtomicAdd, AtomicInc, AtomicDec, for simple counting operation can be operated through this interface, can realize the lock operation performance is more extreme than the user mode threads].


#Auther: 张亚伟 YaweiZhang.
Web Site: www.zsummer.net  
mail: yawei_zhang@foxmail.com  
github: https://github.com/zsummer  
QQ Group: 19811947  
