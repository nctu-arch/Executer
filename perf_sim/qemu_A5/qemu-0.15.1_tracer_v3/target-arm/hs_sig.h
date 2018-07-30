#include <stdio.h>
#include <stdlib.h>

void send_sig(){
	char line[10];
	FILE *cmd = popen("pidof qemu-system-arm", "r");
	fgets(line, 10, cmd);
	pid_t pid = strtoul(line, NULL, 10);
	pclose(cmd);
	printf("pid = %d\n", pid);
	kill(pid,SIGUSR1);
}
void send_SSH(){
	system("/home/yhchiang/arm_qemu/run_A.sh hello.o");	
}
