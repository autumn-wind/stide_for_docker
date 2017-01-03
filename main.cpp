#include <iostream>
#include <fstream>
#include <vector>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>      // for struct user_regs_struct
#include <sys/reg.h>
#include <sys/syscall.h>   /* For SYS_write etc */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define STACK_SIZE (1024 * 1024)
#define err_EXIT(msg) do { perror(msg); exit(EXIT_FAILURE); \
					  } while(0)

using namespace std;

int trace_container_process(void *arg) {
	pid_t tracee = *(pid_t *)arg;
	int status;

	/*the first time ptrace_syscall is in effect, the tracee is about to exit a system call*/
	bool syscall_enter = false;

	struct user_regs_struct regs;

	/*ptrace(PTRACE_ATTACH, tracee, 0, 0);*/
	/*waitpid(tracee, &status, 0);*/
	/*ptrace(PTRACE_SETOPTIONS, tracee, 0, PTRACE_O_TRACESYSGOOD);*/

	ptrace(PTRACE_SEIZE, tracee, 0, PTRACE_O_TRACESYSGOOD);
	ptrace(PTRACE_INTERRUPT, tracee, 0, 0);

	while(1) {
		ptrace(PTRACE_SYSCALL, tracee, 0, 0);
		waitpid(tracee, &status, 0);
		if(WIFEXITED(status) || WIFSIGNALED(status)) {
			printf("The tracee has exited or been terminated.\n");
			return 1;
		}
		else if(WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0X80)) {
			ptrace(PTRACE_GETREGS, tracee, 0, &regs);
			if(syscall_enter == false) {
				syscall_enter = true;
			}
			else if(syscall_enter == true) {
				/*printf("The tracee made a system call %lld\n", regs.orig_rax);*/
				printf("%d %lld\n", tracee, regs.orig_rax);
				syscall_enter = false;
			}
			else {
				assert(0);
			}
		}
	}
	return 0;
}

int main( )
{
	while(true)
	{
		char time[50], container[20], start[20], cid[100], image[20], name[20];
		cin >> time >> container >> start >> cid >> image >> name;
		sleep(1);
		cout << cid << endl;
		string path_part_1("/sys/fs/cgroup/devices/docker/");
		string path_part_2(cid);
		string path_part_3("/tasks");
		string whole_path = path_part_1 + path_part_2 + path_part_3;
		ifstream tasks_file;
		tasks_file.open(whole_path);
		if(!tasks_file.is_open()) {
			cout << "open file failed!" << endl;
			continue;
		}
		vector<pid_t> pids;
		pid_t pid;
		while(tasks_file >> pid) {
			pids.push_back(pid);
		}
		tasks_file.close();
		cout << "total container process number: " << pids.size() << endl;
		if(pids.size() > 50) {
			cout << "container contains too many processs, refuse to trace!" << endl;
			continue;
		}
		char *stack_top = (char *)malloc(pids.size() * STACK_SIZE);
		if(stack_top == NULL) {
			err_EXIT("malloc");
		}
		for(int i = 0; i < pids.size(); ++i) {
			//cout << pids[i] << endl;
			int child_pid = clone(trace_container_process, stack_top + (i + 1) * STACK_SIZE, SIGCHLD, &pids[i]);
			if(child_pid == -1) {
				err_EXIT("clone");
			}
			sleep(1);    // give the child time to attach to the container process
		}
	}
	return 0;
}
