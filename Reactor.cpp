#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef void *(*myReactorFunction)(void *arg); // my function will send pointer to it!

//Defining my Reactor struct including my File descriptor, my function , and the thread
typedef struct Reactor{
    int fileID;
    pthread_t threadID;
    myReactorFunction func;
} reactor, *preactor;

//Defining my request struct
typedef struct ReacReqests{
    int fileID;
    preactor reac;
} reqests, *preqests;

//Constructor return void* to new reactor
void* newReactor(){
    preactor reac = (preactor)(malloc(sizeof(reactor))); //allocating memory for my reactor and return void*
    return (void*)reac;
}

//For each request i will init the reactor, function to use, and the file desciptor
//and will start the thread on this request
void InstallHandler(preactor reac, myReactorFunction FuncToDo, int FD){
    reac->func = FuncToDo;
    reac->fileID = FD;
    preqests req = (preqests)(malloc(sizeof(reqests)));
    req->fileID = FD;
    req->reac = reac;
    pthread_create(&reac->threadID, NULL, FuncToDo, req);
}
// remove the task
void RemoveHandler(preactor reac, int fd_free){
    pthread_join(reac->threadID, NULL);
    reac->fileID = -1;
    reac->func = NULL;
}