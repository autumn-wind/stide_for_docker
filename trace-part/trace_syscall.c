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

int main(int argc, char **argv)
{   
	if (argc < 2) {
		fprintf(stderr, "Expected a program name as argument\n");
		return -1;
	}

	pid_t child = atoi(argv[1]);

	int status;

	/*the first time ptrace_syscall is in effect, the tracee is about to exit a system call*/
	bool syscall_enter = false;

	struct user_regs_struct regs;

	/*ptrace(PTRACE_ATTACH, child, 0, 0);*/
	/*waitpid(child, &status, 0);*/
	/*ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD);*/

	ptrace(PTRACE_SEIZE, child, 0, PTRACE_O_TRACESYSGOOD);
	ptrace(PTRACE_INTERRUPT, child, 0, 0);

	while(1) {
		ptrace(PTRACE_SYSCALL, child, 0, 0);
		waitpid(child, &status, 0);
		if(WIFEXITED(status) || WIFSIGNALED(status)) {
			printf("The tracee has exited or been terminated.\n");
			return 1;
		}
		else if(WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0X80)) {
			ptrace(PTRACE_GETREGS, child, 0, &regs);
			if(syscall_enter == false) {
				syscall_enter = true;
			}
			else if(syscall_enter == true) {
				/*printf("The child made a system call %lld\n", regs.orig_rax);*/
				printf("%d %lld\n", child, regs.orig_rax);
				syscall_enter = false;
			}
			else {
				assert(0);
			}
		}
	}
	return 0;
}
