#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/shm.h>
#include <signal.h>
#include <sched.h>
#include <malloc.h>
#include <sys/types.h>
#include "Semaphores.h"

typedef struct node{
    int data;
    struct node *ptr;
} node;

node* head, *p;
node* insert(node* head, int num) {
    node *temp, *prev, *next;
    temp = (node*)malloc(sizeof(node));
    temp->data = num;
    temp->ptr = NULL;
    if(!head){
        head=temp;
    } else{
        prev = NULL;
        next = head;
        while(next && next->data<=num){
            prev = next;
            next = next->ptr;
        }
        if(!next){
            prev->ptr = temp;
        } else{
            if(prev) {
                temp->ptr = prev->ptr;
                prev-> ptr = temp;
            } else {
                temp->ptr = head;
                head = temp;
            }            
        }   
    }
    return head;
}

int min;
int max;
int procs = 4;
int nprimos = 0;
//int cexit;
int sem;
int sem_linked;
int sem_forwait;
int *prim;

void free_list(node *head) {
    node *prev = head;
    node *cur = head;
    while(cur) {
        prev = cur;
        cur = prev->ptr;
        free(prev);
    }       
}
void reciever()
{
    while (prim[1] != 4)
    {
        semwait(sem_linked);
        if (prim[1] != 4)
        {
            int a = prim[0];
            head = insert(head, a);
        }

        semsignal(sem_forwait);
    }
    p = head;
    while (p)
    {
        printf("%d\n",p->data);
        p = p->ptr;
    }
    free_list(head);
    exit(0);
}
void finder(int i)
{
    int tnum = i;
    int inicio;
    if (tnum == 0)
    {
        inicio = min;
    }
    else
    {
        inicio = tnum * (max / procs);
    }
    int fin = inicio + (max / procs);
    int nprimos1 = 0;

    for (int i = inicio; i <= fin; i++)
    {
        int cont = 1;
        for (int j = 1; j <= ((i / 2) + 1); j++)
        {
            if (i % 2 == 0 && i != 2)
            {
                break;
            }

            if (i % j == 0)
            {
                cont++;
            }
        }
        if ((cont == 2 || i==2) && i!=1)
        {
            nprimos1++;
            semwait(sem); //Ccomienza sección crítica
            prim[0] = i;
            semsignal(sem_linked);

            semwait(sem_forwait);
            semsignal(sem); //Sale de la sección crítica
        }
    }
    semwait(sem);
    prim[1]++;
    semsignal(sem);
    exit(0);
}


int main(int argc, char *argv[])
{
    min = atoi(argv[1]);
    max = atoi(argv[2]);
    sem = createsem(0x1234, 1);        //para que solo un semaforo pase
    sem_linked = createsem(0x1235, 0); //Semaforo para indicar que puede tomar el valor
    sem_forwait = createsem(0x1233, 0);//Semaforo para esperar
    int status;
    head = NULL;
    int shmid;
    shmid = shmget(0x1237, 2 * sizeof(int), IPC_CREAT | 0666); //P[0] contiene el n# primo encontrado, P[1] contiene contador para terminar procesos
    if (shmid < 0)
    {
        fprintf(stderr, "Error al obtener memoria compartida\n");
        exit(1);
    }
    prim = shmat(shmid, NULL, 0);
    prim[1] = 0;
    if (fork() == 0)
    {
        reciever();
        exit(1);
    }
    for (int i = 0; i < 4; i++)
    {
        if (fork() == 0)
        {
            finder(i);
            exit(1);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        wait(NULL);
    }
    erasesem(sem);
    erasesem(sem_linked);
    erasesem(sem_forwait);
}