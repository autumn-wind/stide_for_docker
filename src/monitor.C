#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>      // for struct user_regs_struct
#include <sys/reg.h>
#include <sys/syscall.h>   /* For SYS_write etc */

#include <pthread.h>
#include <semaphore.h>

#include "common.h"

extern void* stide_for_docker(void *);

#define STACK_SIZE (1024 * 1024)

#define MAX_PROCESS_IN_CON 50

using namespace std;

struct pro_con_bfr {
	pid_t pid;
	int syscall_nr;
} bfr;

//mutex lock used by tracer for writing file or buffer
pthread_mutex_t lock;

//sem used by tracer(producer) and stide(consumer) thread
sem_t empty_buffer, full_buffer;

//syscall log file
ofstream flog;

void* trace_container_process(void *arg) {
	pid_t tracee = *(pid_t *)arg;
	int status;

	//the first time ptrace_syscall is active, the tracee is about to exit a system call
	bool syscall_enter = false;

	//trace's regs
	struct user_regs_struct regs;

	//seize the tracee and make it stop, this is better than PTRACE_ATTACH
	ptrace(PTRACE_SEIZE, tracee, 0, PTRACE_O_TRACESYSGOOD);
	ptrace(PTRACE_INTERRUPT, tracee, 0, 0);

	while (true) {

		//make tracee run, stop when system call happens
		ptrace(PTRACE_SYSCALL, tracee, 0, 0);
		waitpid(tracee, &status, 0);

		//the tracee exits or being terminated
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			printf("The tracee has exited or been terminated.\n");
			return (void *)1;
		}

		//the tracee issue a system call
		else if (WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0X80)) {

			//get tracee's regs
			ptrace(PTRACE_GETREGS, tracee, 0, &regs);

			//the tracee is stopped both when entering and exiting system call
			if (syscall_enter == false) {
				syscall_enter = true;
			}
			else if (syscall_enter == true) {

				//printf("%d %lld\n", tracee, regs.orig_rax);

				//pthread_mutex_lock(&lock);
				//flog << tracee << " " << regs.orig_rax << endl;
				//pthread_mutex_unlock(&lock);

				sem_wait(&empty_buffer);
				bfr.pid = tracee;
				bfr.syscall_nr = regs.orig_rax;
				flog << tracee << " " << regs.orig_rax << endl;
				sem_post(&full_buffer);

				syscall_enter = false;
			}
			else {
				err_EXIT("can not reach here");
			}
		}
	}

	return (void *)0;
}

int main(int argc, char *argv[]) {
	
	//wait for container init totally
	sleep(2);

	//form the path of container tasks file
	string path_part1("/sys/fs/cgroup/devices/docker/");
	string path_part2(argv[1]);
	string path_part3("/tasks");
	string tasks_file_path = path_part1 + path_part2 + path_part3;

	//tasks file
	ifstream ftasks;
	ftasks.open(tasks_file_path.c_str());
	if (!ftasks.is_open())
		err_EXIT("open");

	//read pids in tasks file
	vector<pid_t> pids_in_con;
	pid_t pid;
	while (ftasks >> pid) {
		pids_in_con.push_back(pid);
	}
	ftasks.close();

	//print the number of process in the container
	cout << "total container process number: " << pids_in_con.size() << endl;

	//we assume there are at most 50 processes in one container
	if (pids_in_con.size() > MAX_PROCESS_IN_CON) 
		err_EXIT("too many process in container");

	//open syscall log file
	flog.open("syscall_log.txt", ofstream::out);
	if (!flog.is_open())
		err_EXIT("syscall log file open failed");

	//init the mutex
	if (pthread_mutex_init(&lock, NULL) != 0)
		err_EXIT("mutex init failed");

	//init the sem
	if (sem_init(&empty_buffer, 0, 1) != 0)
		err_EXIT("sem init failed");
	if (sem_init(&full_buffer, 0, 0) != 0)
		err_EXIT("sem init failed");

	//record stack for all tracers
	char **ppstack = (char **)malloc(pids_in_con.size() * sizeof(char *));
	if(ppstack == NULL) {
		err_EXIT("malloc");
	}

	//thread id of every tracer
	vector<pthread_t> ptids;

	//create a new thread(tracer) for every process in container(tracee)
	for(int i = 0; i < pids_in_con.size(); ++i) {
		
		//allocate stack for the tracer
		if ((ppstack[i] = (char *)malloc(STACK_SIZE)) == NULL)
			err_EXIT("malloc");

		//create a new tracer thread
		pthread_t ptid;
		if (pthread_create(&ptid, NULL, trace_container_process, &pids_in_con[i]) != 0)
			err_EXIT("pthread_create");
		ptids.push_back(ptid);
	}

	//create stide thread
	pthread_t stide_ptid;
	if (pthread_create(&stide_ptid, NULL, stide_for_docker, NULL) != 0)
		err_EXIT("pthread_create");

	//wait for all the tracer exit
	//for(int i = 0; i < pids_in_con.size(); ++i) {
		//pthread_join(ptids[i], NULL);
	//}
	
	//wait for stide thread exit
	pthread_join(stide_ptid, NULL);

	//free resources
	pthread_mutex_destroy(&lock);
	sem_destroy(&empty_buffer);
	sem_destroy(&full_buffer);
	flog.close();

	return 0;
}
