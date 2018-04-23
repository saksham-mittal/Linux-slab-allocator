# define pragma once

#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

/*-- The size of a slab is fixed to 64KB --*/
# define SZ 64 * 1024

/*-- This struct is for the header of the slab --*/
struct header {
    int totobj;
    int freeobj;
    bitset<16000> bitmap;
    struct bucket* buck;
    struct slab* nxtSlab;
};

/*-- This struct is to store the objects in the slab --*/
struct object{
    struct slab* slbPtr;
    void *data;
};

/*-- This is the struct for the slab --*/
struct slab {
    struct header slabHeader;
};

/*-- This is the struct for the bucket(hash table) --*/
struct bucket {
    int objSize;
    struct slab* firstSlab;
    mutex buckMutex;
};


/*-- Declaration and intitialisation for the bucket array --*/
bucket bcktTable[12] = {
    {4, NULL},
    {8, NULL},
    {16, NULL},
    {32, NULL},
    {64, NULL},
    {128, NULL},
    {256, NULL},
    {512, NULL},
    {1024, NULL},
    {2048, NULL},
    {4096, NULL},
    {8192, NULL}
};

/*-- Declaration for addSlab function --*/
struct slab* addSlab(int bucketSize);

/*-- Declaration for mymalloc function --*/
void* mymalloc(int size);

void myfree(void *ptr);
