#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <limits.h>

#define threads 6
#define SIZE 20

sem_t mutex,empty,full,wrt;

int count=0;
int number;
int fullflag=0;

struct queue
{
    int front, rear, length;
    unsigned capacity;
    int* array;
};


struct queue* q;


struct queue* createq(unsigned capacity)
{
    struct queue* q = (struct queue*)malloc(sizeof(struct queue));
    q->capacity = capacity;
    q->front = q->length = 0;

    q->rear = capacity - 1;
    q->array = (int*)malloc(q->capacity * sizeof(int));
    return q;
}


int isFull(struct queue* q)
{
    return (q->length == q->capacity);
}

int isEmpty(struct queue* q)
{
    return (q->length == 0);
}


void enq(struct queue* q, int item)
{
    if (isFull(q))
    {
        printf("\nMonitor thread: Buffer full!!\n");
        fullflag=1;
    }
    else
    {
        q->rear = (q->rear + 1)% q->capacity;
        q->array[q->rear] = item;
        q->length = q->length + 1;
    }
}


void deq(struct queue* q)
{
    if (isEmpty(q))
    {
        printf("\nCollector thread: nothing is in the buffer!\n");
        fullflag=0;
    }
    else
    {
    q->front = (q->front + 1)% q->capacity;
    q->length = q->length - 1;
    fullflag=0;
    }
}

void counter(void *vargp)
{

    while(1)
    {


        printf("\nCounter thread %d received msg\n",*(int*)vargp);
        sleep(rand()%4);



        sem_wait(&wrt);

        printf("\nCounter thread %d waiting\n",*(int*)vargp);
        sleep(rand()%4);

        count++;

        sem_post(&wrt);

        printf("\nCounter thread %d: now adding to counter,counter value=%d\n",*(int*)vargp,count);
        sleep(rand()%4);

    }

}


void monitor(void  *vargp)
{
    while(1)
    {
        if(count!=0 && fullflag==0)
        {


           sem_wait(&empty);
           sem_wait(&mutex);



            printf("\nMonitor thread: waiting to read counter\n");


            sem_wait(&wrt);

            number=count;
            count=0;

            sem_post(&wrt);


            printf("\nMonitor thread: reading a count value of %d\n",number);


            enq(q,number);

            printf("\nMonitor thread: writing to buffer at position %d\n",q->rear);


        sem_post(&mutex);
        sem_post(&full);
        }
        else if(fullflag==1)
           {
               printf("\nMonitor thread: Buffer full!!\n");
           }

        sleep(rand()%4);

    }
}

void collector(void  *vargp)
{

    while(1)
    {

        sem_wait(&full);
        sem_wait(&mutex);


        printf("\nCollector thread: reading from the buffer at position %d\n",q->front);

        deq(q);



        sem_post(&mutex);
        sem_post(&empty);

        sleep(rand()%4);

    }

}


int main()
{

    int i;

    q = createq(SIZE);



    srand(time(0));

    pthread_t t1;
    pthread_t t2;
    pthread_t t3[threads];

    sem_init(&wrt, 0, 1);
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, SIZE);





    pthread_create(&t1,NULL,(void*)monitor,NULL);

    pthread_create(&t2,NULL,(void*)collector,NULL);


    for(i=0; i<threads; i++)
    {
        int *a=malloc(sizeof(int));
        *a=i;
        pthread_create(&t3[i],NULL,(void*)counter,a);
    }



    pthread_join(t1,NULL);

    pthread_join(t2,NULL);



    for(i=0; i<threads; i++)
    {
        pthread_join(t3[i],NULL);
    }


    return 0;

}
