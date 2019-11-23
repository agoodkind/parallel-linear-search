/**
 multitest_thread.c - program for threaded searching
 Rutgers CS 01:198:214 Systems Programming
 Professor John-Austen Francisco
 Authors: Anthony Siluk (ars373) & Alexander Goodkind (amg540)
 Due: 11/22/2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/types.h>
#include "multitest.h"
#include <sys/time.h>
#include <sys/resource.h>

struct thread_info {
    int start_index;
    int end_index;
    int target;
    int *array;
    pthread_t numThread;
    struct rusage usage;
};

void linearThread(void *args)
{
    
    struct thread_info *currentThread = args;
    int *list = &(currentThread->array[currentThread->start_index]);
    int size = currentThread->end_index - currentThread->start_index;
    int found = linear_search(list, size, currentThread->target);
    getrusage(RUSAGE_SELF, &(currentThread->usage));
    
    pthread_exit(found);
}

int threadSearchMain(int *list, int arraySize, int goal, int maxThreads, int size_to_search_for_each, struct perfdata* threadperfdata) {
    int found = -1;

    struct thread_info *threads;

    threads = (struct thread_info*)malloc(maxThreads*sizeof(struct thread_info));

    for (int i = 0; i < maxThreads; i++) {
        threads[i].target = goal;
        threads[i].start_index = i * size_to_search_for_each;
        threads[i].numThread = i + 1;
        threads[i].array = list;
        
        if (i == maxThreads - 1) {
            threads[i].end_index = arraySize;
        } else {
            threads[i].end_index = (i + 1) * size_to_search_for_each;
        }

        //create threads to then linearly search the array
        pthread_create(&threads[i].numThread, NULL, &linearThread, &threads[i]);
    }

    for (int i = 0; i < maxThreads; i++) {
        //join terminated threads together
        int res_found;
        struct timespec contextswitchstart, contextswitchend;

        clock_gettime(CLOCK_REALTIME, &contextswitchstart); 
        pthread_join(threads[i].numThread, &res_found);
        clock_gettime(CLOCK_REALTIME, &contextswitchend);


        ullint contexttime = BILLION * (contextswitchend.tv_sec - contextswitchstart.tv_sec) + contextswitchend.tv_nsec - contextswitchstart.tv_nsec;
        threadperfdata->contexttimeavg += contexttime;
        if (threadperfdata->contexttimeavg > contexttime) {
            threadperfdata->contexttimeavg /= 2;
        }
        threadperfdata->numcontextswitch += (ullint) threads[i].usage.ru_nvcsw;
        threadperfdata->numcontextswitch += (ullint)  threads[i].usage.ru_nivcsw;

        // printf("time spent in context switch: %lluns\n", BILLION * (contextswitchend.tv_sec - contextswitchstart.tv_sec) + contextswitchend.tv_nsec - contextswitchstart.tv_nsec);
        // printf("number of voluntary context switches: %ld \n", threads[i].usage.ru_nvcsw);
        // printf("number of involuntary context switches: %ld \n", threads[i].usage.ru_nivcsw);

        if (res_found != -1) {
            found = (i * size_to_search_for_each) + res_found;
        }
    }

    free(threads);

    return found;
}
