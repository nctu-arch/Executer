#include <stdio.h>
#include <stdlib.h>
#define HS_SIG 888

static void hs_sig_handler(void){
	printf("in hs_sig_handler\n");	
}

static void init_sig(void){
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = (void (*))hs_sig_handler;
    sigaction(SIGUSR1, &action, NULL);
}
