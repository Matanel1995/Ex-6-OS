#ifndef NODE
#define NODE
#include <stdlib.h>
#include <pthread.h>

#define MAX_LENGTH 1024


// got from GeeksForGeeks
struct QNode {
    void* data;
    struct QNode* next;
}Node;
 
// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
    struct QNode *front, *rear;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
};
 
// A utility function to create a new linked list node.
struct QNode* newNode(char* data)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->data = (char*)malloc(MAX_LENGTH);
    //Loop to copy the data into the Node
    for(int i=0;i<MAX_LENGTH;i++){
        ((char*)temp->data)[i] = data[i];
    }
    temp->next = NULL;
    return temp;
}
#endif