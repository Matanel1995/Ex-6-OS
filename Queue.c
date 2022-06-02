#include <stdio.h>
#include <pthread.h>
#include "Node.c"

//Got from GeeksForGeeks made my own changes
// A utility function to create an empty queue

struct Queue* createQ()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)); //Allocate mem for lock
    q->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t)); //Allocate mem for cond
    pthread_mutex_init(q->lock,NULL); // init my lock
    pthread_cond_init(q->cond,NULL); // init my cond
    return q;
}

void destroyQ(struct Queue* q){
    while(q->front != NULL){
        struct QNode* temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    pthread_cond_destroy(q->cond);
    pthread_mutex_destroy(q->lock);
    free(q);
    return;
}
 
// The function to add a key k to q
void enQ(struct Queue* q, void* data)
{
    // Create a new LL node
    struct QNode* temp = newNode((char*)data);
    pthread_mutex_lock(q->lock); // locking my lock make it thread safe
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        pthread_mutex_unlock(q->lock); // unlocking my lock
        return;
    }
 
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
    pthread_mutex_unlock(q->lock); // unlocking my lock
}
 
// Function to remove a key from given queue q
void* deQ(struct Queue* q)
{
    pthread_mutex_lock(q->lock); // locking my lock make it thread safe
    // If queue is empty, return NULL.
    if (q->front == NULL){
        pthread_mutex_unlock(q->lock); // unlocking my lock
        return NULL;
    }
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    
    pthread_mutex_unlock(q->lock); // unlocking my lock
    return(temp->data);
}
 
// Driver Program to test above functions
// int main()
// {
//     struct Queue* q = createQ();
//     enQueue(q, "10");
//     enQueue(q, "20");
//     deQueue(q);
//     deQueue(q);
//     enQueue(q, "30");
//     enQueue(q, "40");
//     enQueue(q, "50");
//     deQueue(q);
//     printf("Queue Front : %s \n", q->front->data);
//     printf("Queue Rear : %s \n", q->rear->data);
//     destroyQ(q);
//     printf("destroyed Queue! \n");
//     return 0;
// }