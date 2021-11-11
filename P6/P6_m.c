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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
struct msgbuf
{
    long mtype;     /* message type, must be > 0 */
    int prim; /* message data */
};
//int queue; // Buzón de mensajes
int queue_wait;
typedef struct node
{
    int data;
    struct node *ptr;
} node;
void free_list(node *head)
{
    node *prev = head;
    node *cur = head;
    while (cur)
    {
        prev = cur;
        cur = prev->ptr;
        free(prev);
    }
}
node *head, *p;
node *insert(node *head, int num)
{
    node *temp, *prev, *next;
    temp = (node *)malloc(sizeof(node));
    temp->data = num;
    temp->ptr = NULL;
    if (!head)
    {
        head = temp;
    }
    else
    {
        prev = NULL;
        next = head;
        while (next && next->data <= num)
        {
            prev = next;
            next = next->ptr;
        }
        if (!next)
        {
            prev->ptr = temp;
        }
        else
        {
            if (prev)
            {
                temp->ptr = prev->ptr;
                prev->ptr = temp;
            }
            else
            {
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
void reciever()
{
    struct msgbuf mensaje;
    //struct msgbuf mensaje_prim;
    int cont = 0;
    while (cont != 4)
    {
        msgrcv(queue_wait, &mensaje, sizeof(struct msgbuf), 2, 0);
        if (mensaje.prim == -1)
        {
            cont++;
        }
        else
        {
            int a = mensaje.prim;
            head = insert(head, a);
        }
    }
    p = head;
    while (p)
    {
        printf("%d\n", p->data);
        p = p->ptr;
    }
    free_list(head);
    exit(0);
}
void finder(int i)
{
    struct msgbuf mensaje;
    struct msgbuf mensaje_prim;
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
        if ((cont == 2 || i == 2) && i != 1)
        {
            mensaje_prim.mtype = 2;   
            mensaje_prim.prim = i;                                             
            msgsnd(queue_wait, &mensaje_prim, sizeof(struct msgbuf), 0); //Enviamos el primo
    

        }
    }
    mensaje_prim.prim = -1;
    msgsnd(queue_wait, &mensaje_prim, sizeof(struct msgbuf), 0);
    exit(0);
}

int main(int argc, char *argv[])
{
    min = atoi(argv[1]);
    max = atoi(argv[2]);
    queue_wait = msgget(0x1230, 0666 | IPC_CREAT);
    if (queue_wait == -1)
    {
        fprintf(stderr, "No se pudo crear el buzón\n");
        exit(1);
    }
    head = NULL;
    if (fork() == 0)
    {
        reciever();
        exit(0);
    }
    for (int i = 0; i < 4; i++)
    {
        if (fork() == 0)
        {
            finder(i);
            exit(0);
        }
    }
    
    for (int i = 0; i < 5; i++)
    {
        wait(NULL);
    }
    msgctl(queue_wait, IPC_RMID, NULL);
}