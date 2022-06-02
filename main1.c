#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "Queue.c"

#define MAX_LENGTH 1024
#define PORT "3491"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//Creating 3 Queues for the ActiveObjects
struct Queue* firstActiveO;
struct Queue* secondActiveO;
struct Queue* thirdActiveO;

//This function gets a word and do caesar code by 1 
void *caesarCode(void* word){
    for(int i=0; i < strlen(word);i++){
        //check if char is z if so turn into a 
        if(((char*)word)[i] == 122){
            ((char*)word)[i] = 97;
        }
        else if (((char*)word)[i] == 90) // check if char is Z so turn into A
        {
            ((char*)word)[i]=65;
        }
        else{ // all other chars i can update by 1
            ((char*)word)[i]+=1;
        }
    }
    return word;
}

//This function turn any lowerCase letter into upperCase and upper to lower
void *changeCaps(void* word){
    for(int i=0;i<strlen(word);i++){
        if(((char*)word)[i] > 96 && ((char*)word)[i] < 123){// lower case letter
            ((char*)word)[i] -=32; // 32 is the diff between upper and lower case letters
        }
        else if (((char*)word)[i] < 91 && ((char*)word)[i] > 64){
            ((char*)word)[i]+=32; // 32 is the diff between upper and lower case letters
        }
    }
    return word;
}

//Function to send to new data to the client
void *sendAnswer(int sock, char* word){
    int check = send(sock,word,strlen(word),0);
    if(check == -1){// send the new word to the sock
        printf("Error in sending! %d\n",check);
    } 
}

//Thirs ActiveObject
void AO_3(int sock){
    while(thirdActiveO->front !=NULL){// While queue is not empty
        void* rawData= deQ(thirdActiveO); // Get the top word
        sendAnswer(sock,rawData);
    }
}

//Second ACtiveObject
void AO_2(int sock){
    while (secondActiveO->front != NULL){// While queue is not empty
        void* rawData = deQ(secondActiveO); // Get the top word
        void* afterChange = changeCaps(rawData); // Change upper to lower and lower to upper
        enQ(thirdActiveO,afterChange);//Enquing the new word to the thirs ActiveObject Queue
        AO_3(sock);
    }
}


//First activeObject 
void AO_1(int sock,struct Queue* q){
    // deQ from the firstActiveO and send the to the ceaser function
    while ((q->front) != NULL){// While queue is not empty
        void* rawData = deQ(q); // Get the top word
        void* afterCaesar = caesarCode(rawData); // Calling caesarCode function
        enQ(secondActiveO,afterCaesar); // Enquing the new word to the the second activeObject Queue
        AO_2(sock); // calling next ActiveObject
    }
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void getDataFromClients(void* newfd1) {
    long int newfdTemp = (long int) newfd1;
    while (1) {
        //Setting what i need to get data from user
        char user_input[MAX_LENGTH];
        char rest[MAX_LENGTH];
        memset(user_input, 0, MAX_LENGTH);
        memset(rest, 0, MAX_LENGTH);
        recv(newfdTemp, user_input, MAX_LENGTH, 0);
        printf("Got data from client : %s",user_input);

        //Take the client data and add it to firstAO Queue
        enQ(firstActiveO,(void*)user_input);
        AO_1(newfdTemp,firstActiveO); // call the first ActiveObject

    }
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(){

    firstActiveO = createQ();
    secondActiveO = createQ();
    thirdActiveO = createQ();

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    int couneter = 0;

    while(1) {  // main accept() loop
        couneter++;
        sin_size = sizeof their_addr;

        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        printf("My FD is : %d\n",new_fd);
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);
        pthread_t myThread;
        pthread_create(&myThread, NULL, getDataFromClients, (void*)new_fd);
        if(couneter>10){
            break;
        }
    }
    return 0;
}