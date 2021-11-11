#include <sys/sem.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 

#define SEM_ID int

void semwait(int semid)
{
	struct sembuf s;
	s.sem_num=0; // número de semáforo en el arreglo
	s.sem_op=-1;
	s.sem_flg=0;
	
	semop(semid,&s,1);
	return;
}
	
void semsignal(int semid)
{
	struct sembuf s;
	s.sem_num=0; // número de semáforo en el arreglo
	s.sem_op=1;
	s.sem_flg=0;
	
	semop(semid,&s,1);
	return;
}
int createsem(int key, int value)
{
    int semid;
    semid = semget(key,1,0666|IPC_CREAT);
    semctl(semid,0,SETVAL,value);
    return(semid);
}
int createsemarray(int key, int nsems)
{
	int semid;			   // 110 110 110   RW-RW-RW-
	semid=semget(key,nsems,0666|IPC_CREAT); // Solicitar un arreglo de semáforos al SO
	return(semid);
}

void initsem(int semid,int semnum,int value)
{
	semctl(semid,semnum,SETVAL,value);	
}

void erasesem(int semid)
{
	semctl(semid,0,IPC_RMID,0);	// Eliminar el arreglo de semaforos
	return;
}