#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include "semaphore.h"

union semun { int val; }sem1;

int lockSemaphore(){
    openlog("ofproto.c", LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "%d try locked \n", getpid());
	closelog();
    int status = 0;
	int semid = semget(UBUS_SEMANUM, 1, IPC_CREAT | IPC_EXCL | 0600);
	if (semid == -1) {
		if (errno == EEXIST) {
			semid = semget(UBUS_SEMANUM, 1, 0);
		}
	}else {
		sem1.val = 1;
		status = semctl(semid, 0, SETVAL, sem1);
	}
    // printf("lock semid %d\n", semid);
	struct sembuf sembuf1;
    sembuf1.sem_num = 0;
    sembuf1.sem_op = -1;
    sembuf1.sem_flg = SEM_UNDO;
    if (semop(semid, &sembuf1, 1) == -1){
        openlog("ofproto.c", LOG_CONS, LOG_USER);
	    syslog(LOG_INFO, "%d lock fail \n", getpid());
	    closelog();
    }
    return semid;
}
void unlockSemaphore(int semid){
    openlog("ofproto.c", LOG_CONS, LOG_USER);
	syslog(LOG_INFO, "%d try unlock \n", getpid());
	closelog();
	if (semid == -1) {
        openlog("ofproto.c", LOG_CONS, LOG_USER);
	    syslog(LOG_INFO, "no semaphore \n");
	    closelog();
        // printf("\n");
        return;
	}
	struct sembuf sembuf1;
    sembuf1.sem_num = 0;
    sembuf1.sem_op = 1;
    sembuf1.sem_flg = SEM_UNDO;
    semop(semid, &sembuf1, 1);
    semctl(semid, 0, IPC_RMID, sem1);
}