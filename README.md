#说明

这是一个可以用来监测docker容器内部进程系统调用的工具，可以工作在如下两种模式：

1. 建立正常行为库——根据stide算法建立容器内部进程系统调用正常行为库。
2. 异常行为检测——根据stide算法与已经建立好的正常行为库进行对比，对整个容器环境进行入侵检测。

#环境

Ubuntu 16.04  
g++ 5.4.0

#编译

make

#使用

##建立正常行为库

1. ./add-to-db.sh
2. 运行docker容器（使用docker start/restart）
3. 运行完毕，生成default.db文件。

##异常行为检测

1. ./cmp-to-db.sh
2. 运行docker容器（使用docker start/restart）
3. 检测出异常，docker容器被停止

#执行流程

1. add-to-db.sh或cmp-to-db.sh脚本执行后，首先运行“*docker events --filter 'event=start'*”命令，一旦系统中有docker容器运行，“*docker events*”命令会输出相应容器的信息，这些信息通过管道传递给daemon进程。
2. daemon进程通过从“*docker events*”的输出信息得到刚刚运行的容器信息，包括容器的容器号，等待一段时间后（15秒），从这个docker容器对应cgroup的tasks文件中读取容器内所有已经运行的进程的PID。
3. 执行daemon程序时传递“*-a*”参数，表示之后将对容器内进程进行正常行为库的建立；否则，将对容器内进程进行异常行为检测。
4. daemon进程将获取到的所有容器内进程的PID传递给monitor进程。
5. monitor进程对应每一个容器内进程（tracee）创建一个线程（tracer），各个tracer使用ptrace系统调用对tracee进行系统调用的监控，并将拦截得到的系统调用信息全部写入一个缓冲区中。
6. monitor进程还会创建一个stide线程，从缓冲区中取出所有系统调用信息进行分析，根据之前传入daemon进程的参数决定是进行正常行为库的构建，还是进行异常行为检测。
7. 正常行为库构建完毕后（需花费2个小时），得到的树状正常行为库会存储在default.db文件，也可配置存入其他文件名对应的文件中（详见src/config.C），存储结构及解释详见UserDoc。
8. 若进行异常检测时发现系统调用短序列局部不匹配率超过设定的阈值（10%），则会发出警报，并停止整个沙箱。

#stide算法

详见UserDoc

#注意

1. “*docker events*”打印的信息格式可能会随着版本发生变化，且使用不同的命令使容器运行时，其打印的信息也不尽相同。而本程序只实现了对应于“*docker start*”和“*docker restart*”命令使容器运行后，对“*docker events*”命令输出信息的解析（见src/daemon.C），且实验时相应的docker版本号为1.13.1。
2. 容器开始运行后，程序中写死了等待15秒后开始收集容器内进程信息（见src/monitor.C），无法监控15秒以后容器内新创建的进程。
3. 程序中写死了用2个小时进行正常行为库的构建（详见src/stream.C）。
4. 程序中写死了每得到256条系统调用短序列便进行一次异常检测，并且也死了局部不匹配率的阈值为10%（详见src/stream.C）。
