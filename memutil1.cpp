#include "libmymem.hpp"

/*-- threadMain() function creates a object List which has the size of the objects from 4 to 8192 B.
It selects a random element and allocates that element, write to the memory, goes to sleep,
and then frees the memory allocated. Each thread doing so 'niterations' number of times --*/
void threadMain(int threadId, int niterations) {
    int objList[12];
    for(int i=0; i<12; i++) objList[i] = pow(2, i + 2);

    srand(time(NULL));
    for(int i=0; i<niterations; i++) {
        int size = objList[rand() % 12];

        void* ptr = mymalloc(size);

        /*-- This writes '1' to the memory for 'size' number of bytes  --*/
        memset(ptr, '1', size);

        // Sleep for small random time
        int sleepTime = rand()%1000 + 1;
        this_thread::sleep_for(chrono::milliseconds(sleepTime));

        myfree(ptr);
    }

}

int main(int argc, char const *argv[])
{
    if(argc != 5) {
        cout << "Input Error" << endl;
        exit(1);
    }

    /*-- 3rd argument is the number of threads --*/
    int nthreads = atoi(argv[4]);
    /*-- 5th argument is the number of iterations --*/
    int niterations = atoi(argv[2]);

    /*-- We declare an array of threads of size 'nthreads' --*/
    thread th[nthreads];

    for(int i=0; i<nthreads; i++) {
        /*-- Each of the thread calls the threadMain() function --*/
        th[i] = std::thread(threadMain, i, niterations);
    }

    /*-- We join the threads after all threads have ran --*/
    for(int i=0; i<nthreads; i++) {
        th[i].join();
    }
    return 0;
}
