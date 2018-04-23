#include "libmymem.hpp"

int main(int argc, char* argv[]) {
    int count = 0;

    if(argc != 3) {
        /*-- We pass 3 arguments to the main --*/
        cout << "Input Error" << endl;
        exit(1);
    }

    /*-- The 3rd one is the number of iterations --*/
    int limit = atoi(argv[2]);

    /*-- This is the seed for the random numbers --*/
    srand(time(NULL));
    while(count < limit) {
        /*-- We run the loop for 'limit' number of times --*/
        int size = rand() % 8192 + 1;

        /*-- The mymalloc() function is called and the pointer to the struct object is stored --*/
        void* ptr = mymalloc(size);

        /*-- This writes '1' to the memory for 'size' number of bytes  --*/
        memset(ptr, '1', size);

        int sleepTime = rand()%1000 + 1;
        /*-- The preogram sleeps for a random amount of time --*/
        usleep(sleepTime*100);

        /*-- Then, myfree() is called on the memory location of user's data --*/
        myfree(ptr);
        count++;
    }
    return 0;

}
