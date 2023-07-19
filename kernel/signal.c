#include "include/types.h"
#include "include/signal.h"
#include "include/proc.h"

int set_sigaction(int signum, sigaction const *act, sigaction *oldact)
{
    
    struct proc * p = myproc();
    if(p->sigaction[signum].__sigaction_handler.sa_handler != NULL && oldact != NULL){
        *oldact = p->sigaction[signum];
    }
    if(act != NULL){
        p->sigaction[signum] = *act;
    }
    return 0;
}

int sigprocmask(int how, __sigset_t *set, __sigset_t *oldset){
	struct proc *p = myproc();

	for (int i = 0; i < SIGSET_LEN; i++) {
		if (oldset != NULL) {
			oldset->__val[i] = p->sig_set.__val[i];
		}
		if(set == NULL) 
			continue;
		switch (how) {
			case SIG_BLOCK: 
				p->sig_set.__val[i] |= set->__val[i];
				break;
			case SIG_UNBLOCK: 
				p->sig_set.__val[i] &= ~(set->__val[i]);
				break;
			case SIG_SETMASK: 
				p->sig_set.__val[i] = set->__val[i];
				break;
			default: 
                break;
				// panic("invalid how\n");
		}
	}
	// SIGTERM SIGKILL SIGSTOP cannot be masked 
	p->sig_set.__val[0] &= 1ul << SIGTERM | 1ul << SIGKILL | 1ul << SIGSTOP;
	return 0;
}

void sighandle(void)
{
	exit(-1);
}