#include <iostream>
#include <pthread.h>

using namespace std;

pthread_mutex_t lock; // lock for GetInstance
pthread_mutex_t lock2; // lock for Constructor and Destroy function

template<typename T> // make it with template so its generic singelton!
class Singleton{
    private:
        // Variable to the instance!
        static Singleton* instance;
        T val;

        // Private constructor.
        Singleton(T val);
    public:
        //requested function
        static Singleton* getInstance(T val);
        void destroy();
};

template<typename T>
Singleton<T> *Singleton<T>::instance = 0;

//Private constructor to singelton
// make it prive so only i can create new singelton if can be done at that moment
template<typename T>
Singleton<T>::Singleton(T val){
    lock= PTHREAD_MUTEX_INITIALIZER;
    lock2 = PTHREAD_MUTEX_INITIALIZER;
    this->val = val;
    pthread_mutex_lock(&lock2);
}

//Public methods:

//Function to return pointer to singelton
template<typename T>
Singleton<T> *Singleton<T>::getInstance(T val){
    if (instance == 0){ // if allready exist return its instance, if not creating new singelton and returning the new instance
        pthread_mutex_lock(&lock); // lock and unlock to make sure cant create 2 singeltons at the same time
        instance = new Singleton<T>(val);
        pthread_mutex_unlock(&lock);
    }
    return instance;
}


//Function to "Destroy" the singelton (make it aviable to create a new one!)
template<typename T>
void Singleton<T>::destroy(){
    instance =0; // change the instance back to 0 so we can create new singelton
    pthread_mutex_unlock(&lock2); // release the lock!
}

//Done with public methods!

int main(){
    //Testing my Singelton!
    int* temp;
    Singleton<int *>* myInt1 = Singleton<int *>::getInstance(temp); // Ok
    Singleton<int *>* myInt2 = Singleton<int *>::getInstance(temp);

//    /* The addresses will be the same. */
    if(myInt1 == myInt2){
        cout << "Both point at the same value! Singelton is working!" << endl;
    }
    else{
        cout << "Created 2 instances of a Signelton! fix your code" << endl;
    }
}