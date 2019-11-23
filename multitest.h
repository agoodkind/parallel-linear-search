/**
 multitest.h - header for entire program
 Rutgers CS 01:198:214 Systems Programming
 Professor John-Austen Francisco
 Authors: Anthony Siluk (ars373) & Alexander Goodkind (amg540)
 Due: 11/22/2019
 */

#ifndef _multitest_h
#define _multitest_h

#define BILLION 1000000000L

// we define our index as not being found 251 because we set our max bucket size to 250
#define INDEX_NOT_FOUND       254
// the upper limit of how many elements each worker searches
#define MAX_BUCKET_SIZE       250
// we don't want to go above ulimit same as ULIMIT
#define MAX_WORKERS           2000
// MAX_BUCKET_SIZE * MAX_WORKERS
#define MAX_LIST_SIZE         (MAX_WORKERS * MAX_BUCKET_SIZE)
//#define MAX_LIST_SIZE

// for our testing
#define TEST_SEARCH_INNER_MAX 100

typedef long long int ullint;

// context profiling

struct perfdata {
    ullint contexttimeavg;
    ullint numcontextswitch;
};

// printers
void printstats(ullint *times, int size);
void printintArray(int *array, int listSize);
void printullintArray(ullint *array, int listSize);
void printdoublensArray(double *array, int listSize);
void printArray(ullint *array, int listSize);

// math functions
ullint max(ullint *array, int size);
ullint min(ullint *array, int size);
ullint sum(ullint *array, int size);
ullint mean(ullint *array, int size);
ullint stddev(ullint *array, int size);

// test case helpers
void dotest(ullint *times, int size, int numtimes, struct perfdata* perfdata);
void swapOneIndex(int *array, int indexToBeSwapped, int listSize);
int (*testsearchptr)(int *, int, int, int, int);

// business logic
void getSearchSettings(int listSize, int maxBucketCount, int *number_of_workers, int *size_to_search_for_each);
void generateRandomArray(int *array, int listSize);
int linear_search(int *list, int arraySize, int goal);

// main functionality
int threadSearchMain(int *list, int arraySize, int goal, int maxThreads, int size_to_search_for_each, struct perfdata* threadperfdata);
int fork_search_main(int *array, int array_size, int goal, int number_of_forks, int size_to_search_per_fork, struct perfdata* procperfdata);

// macro replacement
#if defined THREAD

#define search(array, listSize, target, number_of_workers, size_to_search_for_each, perf)  threadSearchMain(array, listSize, target, number_of_workers, size_to_search_for_each, perf)

#elif defined PROC

#define search(array, listSize, target, number_of_workers, size_to_search_for_each, perf)  fork_search_main(array, listSize, target, number_of_workers, size_to_search_for_each, perf)

#elif defined LINEAR

#define search(array, listSize, target, number_of_workers, size_to_search_for_each, perf)  linear_search(array, listSize, target)

#endif
#endif
