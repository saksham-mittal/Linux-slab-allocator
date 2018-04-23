#include "libmymem.hpp"

/*-- addSlab() returns a pointer to the slab according to the bucket size --*/
struct slab* addSlab(int bucketSize) {
    int fd;
    struct slab* ptr;

    if ((fd = open("./1mfile", O_RDWR)) < 0) {
        perror("open");
        exit(1);
    }

    /*-- mmap() allocates the required memory and returns the pointer to the first slab --*/
    if ((ptr = (struct slab* )mmap(NULL, SZ, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        /*-- If mmap() fails, we return exit code 1 --*/
        cout << "Memory can't be allocated." << endl;
        exit(1);
    }

    /*-- objSize stores the size of an individual object --*/
    int objSize = (bucketSize + sizeof(struct slab*));
    /*-- index is the index in the bucket array --*/
    int index = log2(bucketSize) - 2;
    ptr->slabHeader.buck = &bcktTable[index];

    ptr->slabHeader.totobj = (SZ - sizeof(header)) / objSize;

    /*-- Intially all objects are free, so free objects = total objects --*/
    ptr->slabHeader.freeobj = (SZ - sizeof(header)) / objSize;
    /*-- The pointer to the next slab is made NULL --*/
    ptr->slabHeader.nxtSlab = NULL;

    /*-- The slab pointer is returned --*/
    return ptr;
}

/*-- mymalloc() returns the pointer to the address where the memory is allocated --*/
void* mymalloc(int size) {
    /*  Psuedo code :-

        add slab *
        find the bucket to put
        check the first slab for free objects
            update the bitmap and
            then return the address of the first free object available
        else go to the next slab
        if no slab has free objects then create a new slab and then return the free space
        and update the bitmap

    */
    int i;
    for(int index=0; index<12; index++) {
        if(bcktTable[index].objSize >= size) {
            i = index;
            break;
        }
    }

    /*-- lock the current bucket so that no other thread could access --*/
    bcktTable[i].buckMutex.lock();

    /*-- 'i' is the index of the bucket array where we need to find the required slab --*/
    if(bcktTable[i].firstSlab == NULL) {
        /*-- If no slab is there, then a slab is added to the beggining --*/
        bcktTable[i].firstSlab = addSlab(bcktTable[i].objSize);

        /*-- The bitmap is modified as the object is allocated --*/
        bcktTable[i].firstSlab->slabHeader.bitmap[0] = 1;

        /*-- The number of free objects is decreased by 1 --*/
        bcktTable[i].firstSlab->slabHeader.freeobj--;

        /*-- ret_mem is pointing to the first object --*/
        void* ret_mem = (void *)((char *)bcktTable[i].firstSlab + sizeof(header));

        /*-- This way we are writing the struct slab* to the memory --*/
        struct object* alloca_mem = (struct object*) ret_mem;
        alloca_mem->slbPtr = bcktTable[i].firstSlab;

        /*-- unlock the current bucket so that other threads could access --*/
        bcktTable[i].buckMutex.unlock();
        return (void*)((char *)ret_mem + sizeof(struct slab*));

    } else {
        /*-- Else we need to traverse the slab linked list to find the required slab --*/
        struct slab* traverse = bcktTable[i].firstSlab;
        while(traverse->slabHeader.nxtSlab && traverse->slabHeader.freeobj == 0) {
             traverse = traverse->slabHeader.nxtSlab;
        }

        /*-- 'traverse' is the current slab --*/
        if(!traverse->slabHeader.nxtSlab) {
            /*-- Check if any free objects are available --*/
            if(traverse->slabHeader.freeobj) {
                /*-- Return the required address using bitmap --*/
                int idx = 0;
                for(int j=0; j<traverse->slabHeader.totobj; j++) {
                    if(traverse->slabHeader.bitmap[j] == 0) {
                        idx = j;
                        break;
                    }
                }
                /*-- 'idx' is the index of the btimap where we need to allocate the object --*/
                traverse->slabHeader.bitmap[idx] = 1;
                traverse->slabHeader.freeobj--;

                /*-- ret_mem is pointing to the first object --*/
                void* ret_mem = (void *)((char*)traverse + sizeof(header) + idx * (bcktTable[i].objSize + sizeof(struct slab*))) ;
                struct object* alloca_mem = (struct object*) ret_mem;
                alloca_mem->slbPtr = traverse;

                /*-- unlock the current bucket so that other threads could access --*/
                bcktTable[i].buckMutex.unlock();
                return (void *)((char*)ret_mem + sizeof(struct slab*));

            } else {
                /*-- Add a new slab at the end of the slab list --*/
                traverse->slabHeader.nxtSlab = addSlab( bcktTable[i].objSize);
                /*-- The bitmap is modified at the 1st index --*/
                traverse->slabHeader.bitmap[0] = 1;
                /*-- The size of the free objects is reduced by 1 --*/
                traverse->slabHeader.freeobj--;

                /*-- ret_mem is pointing to the first memory --*/
                void* ret_mem = (void *)((char *)traverse->slabHeader.nxtSlab + sizeof(header)) ;
                struct object* alloca_mem = (struct object*) ret_mem;
                alloca_mem->slbPtr = traverse;

                /*-- unlock the current bucket so that other threads could access --*/
                bcktTable[i].buckMutex.unlock();
                return (void *)((char *)ret_mem + sizeof(struct slab*));
            }
        } else {
            /*-- return the required address from the slab --*/
            int idx = 0;
            for(int j=0; j<traverse->slabHeader.totobj; j++) {
                if(traverse->slabHeader.bitmap[j] == 0) {
                    idx = j;
                    break;
                }
            }
            /*-- 'idx' is the index of the btimap where we need to allocate the object --*/
            traverse->slabHeader.bitmap[idx] = 1;
            traverse->slabHeader.freeobj--;

            /*-- ret_mem is pointing to the first object --*/
            void* ret_mem = (void *)((char *)traverse + sizeof(header) + idx * (bcktTable[i].objSize + sizeof(struct slab*))) ;
            struct object* alloca_mem = (struct object*) ret_mem;
            alloca_mem->slbPtr = traverse;

            /*-- unlock the current bucket so that other threads could access --*/
            bcktTable[i].buckMutex.unlock();
            return (void *)((char *)ret_mem + sizeof(struct slab*));
        }
    }
    bcktTable[i].buckMutex.unlock();
}

/*-- myfree() funciton frees the memory at the address pointed by the pointer 'ptr' --*/
void myfree(void *ptr) {
    /* Psuedo code :-

        go to the ptr and use the slab* to go to the slab and update the bitmap
        increment freeobj
        if freeobj=totobj, free the slab
            free using munmap(pointer to header,SZ)

    */
    struct object* hdr = (struct object*)((char *)ptr - sizeof(struct slab*));

    /*-- parentSlab is the pointer to the slab where the object is a part of --*/
    struct slab* parentSlab = hdr->slbPtr;

    /*-- 'x' is the size of the ending of header to the beggining of the required object --*/
    long int x = ((char *)ptr - sizeof(struct slab*) - (char *)(void *)parentSlab + sizeof(header));
    struct bucket* parentBuck = parentSlab->slabHeader.buck;

    /*-- lock the current bucket so that no other thread could access --*/
    parentBuck->buckMutex.lock();

    /*-- 'idx' is index of bitmap to be modified --*/
    int idx = x / (parentSlab->slabHeader.buck->objSize + sizeof(struct slab*));

    /*-- The bitmap is modified at the 'idx' position --*/
    parentSlab->slabHeader.bitmap[idx]=0;

    /*-- The number of free objects is reduced by 1 --*/
    parentSlab->slabHeader.freeobj++;

    /*-- The data is unintialised to '0' --*/
    memset(ptr, '0', parentBuck->objSize);
    
    if(parentSlab->slabHeader.freeobj == parentSlab->slabHeader.totobj){
        /*-- If the number of free objects are equal to the total objects, then free the whole slab --*/
        struct slab* iter = parentBuck->firstSlab;

        if(iter->slabHeader.nxtSlab == NULL) {
            /*-- If the slab to be deleted is first slab, then bucket pointer is intialised to NULL --*/
            parentBuck->firstSlab = NULL;
        } else {
            /*-- Else we find the previous slab pointer and then point to the next of the current slab --*/
            while(iter->slabHeader.nxtSlab != parentSlab){
                iter = iter->slabHeader.nxtSlab;
            }
            iter->slabHeader.nxtSlab = parentSlab->slabHeader.nxtSlab;
        }

        /*-- This unmaps the memory allocated --*/
        munmap(parentSlab, SZ);
    }
        /*-- unlock the current bucket so that other threads could access --*/
        parentBuck->buckMutex.unlock();
        return;
}
