/**
 searchtest.c - driver for project2 searching
 Rutgers CS 01:198:214 Systems Programming
 Professor John-Austen Francisco
 Authors: Anthony Siluk (ars373) & Alexander Goodkind (amg540)
 Due: 11/22/2019
 */

#include "multitest.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ulimit.h>
#include <sys/types.h>
#include <math.h>

int manualnumworkers;
int manualsizeperworker;
int manualsearchsettings;
struct perfdata perf;

int main(int argc, char *argv[])
{

    perf.contexttimeavg = 0;
    perf.numcontextswitch = 0;

    char *searchtype;

#if defined PROC
    searchtype = "proc";

#elif defined THREAD
    searchtype = "thread";

#elif defined LINEAR
    searchtype = "linear";

#endif

    srand(time(0));
    
    printf("You are running in *%s* mode\n\n", searchtype);

    if (argc == 3) {
        int listSize = atoi(argv[1]);
        int target = atoi(argv[2]);

        if (listSize > MAX_LIST_SIZE) {
            printf("error: The max %s count is %d, and max bucket size is %d.\nTherefore list size must be no greater than %d.\n", searchtype, MAX_WORKERS, MAX_BUCKET_SIZE, MAX_BUCKET_SIZE * MAX_WORKERS);
            return 1;
        }

#ifdef PROC
        // set ulimit
        ulimit(UL_SETFSIZE, MAX_WORKERS);
#endif

        printf("Basic run with inputs array size %d, target %d\n", listSize, target);

        int *array = (int *)malloc(listSize * sizeof(int));

        generateRandomArray(array, listSize);

        int number_of_workers = 0;
        int size_to_search_for_each = 0;
        int bucketSize = MAX_BUCKET_SIZE;

        getSearchSettings(listSize, bucketSize, &number_of_workers, &size_to_search_for_each);

        int foundIndex = search(array, listSize, target, number_of_workers, size_to_search_for_each, &perf);

        printf("number of workers: %d\n", number_of_workers);

        printf("Found the number %d at index %d\n", target, foundIndex);
        printf("array[%d] = %d\n", foundIndex, array[foundIndex]);

        free(array);
    }

    /**
     * run the test cases
     * Similar to memgrid, write your own suite of tests to investigate the runtimes of Proceses and threads. You should compare Process and thread runtimes on different amounts of data. Make sure your steps in data size are significant based on what you want to test. You should determine:
     - a general trend of: time vs. size of list to search for Processes as well as time vs. size of list to search for threads
     - a tradeoff point for Processes vs threads
     i.e. how long a list would cause threads to perform at the same rate as a Process
     e.g. perhaps, if you create a new thread/Proc for every 250 integers, then searching a list of 5000 integers using threads (requiring 20 threads) is as fast as searching a list of 250 integers using a single Process
     - a tradeoff point for parallelism for Processes and threads
     i.e. at what point does splitting the work over more Processes/threads make the task take longer than not doing so?
     e.g. perhaps sorting a list of 250 elements, but splitting it up in to lists of size 10 (requiring 25 threads) is slower than splitting it up in to lists of size 11 (requiring 22 threads)

     When run, your code should print out:
     - which multi-mode it is in (-process or -thread)
     - which test from your testplan document is being run
     - the parameters of the test
     - which iteration is being run
     - the results (time) of each test
     - the aggregate results of a test batch once completed (min, max, average, standard deviation)
     **/

#define TEST_A_MAX_LIST_SIZE 35000
#define TEST_A_STEP_SIZE MAX_BUCKET_SIZE
#define TEST_A_TIMES_SIZE (TEST_A_MAX_LIST_SIZE / TEST_A_STEP_SIZE)
#define TEST_A_INNER_TIMES 1

    int start_size = 250;
    // - a general trend of: time vs. size of list to search for Processes as well as time vs. size of list to search for threads


    printf("\n\nTest case A: a general trend of time vs. size of list to search for %s.\n", searchtype);
    printf("This will vary the list size from %d ... %d, in steps of %d\n", start_size, TEST_A_MAX_LIST_SIZE, TEST_A_STEP_SIZE);

    printf("......\n");
        
    printf("testing %s perfomance\n", searchtype);
    
    ullint timesA[TEST_A_INNER_TIMES];
    

    int sizes[TEST_A_TIMES_SIZE];
    ullint avg_times[TEST_A_TIMES_SIZE];

    int counter = 0;

    for (int i = TEST_A_STEP_SIZE; i <= TEST_A_MAX_LIST_SIZE; i += TEST_A_STEP_SIZE) {
        dotest(timesA, TEST_A_MAX_LIST_SIZE, TEST_A_INNER_TIMES, &perf);
        avg_times[counter] = mean(timesA, TEST_A_INNER_TIMES);
        sizes[counter] = i;
        printf("\r%.2f%% ", (float) i / TEST_A_MAX_LIST_SIZE * 100);
        counter++;
    }

    printf("\nThe respective sizes for each avg time: \n"); 
    printintArray(sizes, TEST_A_TIMES_SIZE);

    printf("\nStats for Test case A: \n");
    printstats(avg_times, TEST_A_MAX_LIST_SIZE / MAX_BUCKET_SIZE);
    
    printf("\nTest case A done.\n");


#define SIZE_1 100
#define SIZE_2 1000
#define SIZE_3 20000

    ullint times[TEST_SEARCH_INNER_MAX];

    printf("\n\nTest case B, runs a series of searches of arrays of a fixed size to see if threads and processes ever share a similar average run time\n");
    printf("Sizes of arrays are, %d, %d, and %d\n", SIZE_1, SIZE_2, SIZE_3);


    dotest(times, SIZE_1, TEST_SEARCH_INNER_MAX, &perf);
    printf("\nStats for %s where array size is %d :\n", searchtype, SIZE_1);
    printstats(times, TEST_SEARCH_INNER_MAX);

    dotest(times, SIZE_2, TEST_SEARCH_INNER_MAX, &perf);
    printf("\nStats for %s where array size is %d :\n", searchtype, SIZE_2);
    printstats(times, TEST_SEARCH_INNER_MAX);

    dotest(times, SIZE_3, TEST_SEARCH_INNER_MAX, &perf);
    printf("\nStats for %s where array size is %d :\n", searchtype, SIZE_3);
    printstats(times, TEST_SEARCH_INNER_MAX);

    printf("Test case B is complete!\n");

#define TEST_C_SIZE          250

#define TEST_C_BUCKET_SIZE_1 1
#define TEST_C_BUCKET_SIZE_2 10
#define TEST_C_BUCKET_SIZE_3 25
#define TEST_C_BUCKET_SIZE_4 50
#define TEST_C_BUCKET_SIZE_5 100
#define TEST_C_BUCKET_SIZE_6 150
#define TEST_C_BUCKET_SIZE_7 250

    printf("\n\nTest Case C : Fixed array size of %d, checking to see if it is faster to search with 1 %s of %d, or by dividing it up to %d times\n", TEST_C_SIZE, searchtype, TEST_C_SIZE, TEST_C_SIZE);

    int bucket_sizes[7] = { TEST_C_BUCKET_SIZE_1, TEST_C_BUCKET_SIZE_2, TEST_C_BUCKET_SIZE_3, TEST_C_BUCKET_SIZE_4, TEST_C_BUCKET_SIZE_5, TEST_C_BUCKET_SIZE_6, TEST_C_BUCKET_SIZE_7 };

    manualsearchsettings = 1;
    for (int i = 0; i < 7; i++) {
        
        getSearchSettings(TEST_C_SIZE, bucket_sizes[i], &manualnumworkers, &manualsizeperworker);
        
        printf("\nUsing a bucketsize of %d resulting in %d workers\n", bucket_sizes[i], manualnumworkers);
        dotest(times, TEST_C_SIZE, TEST_SEARCH_INNER_MAX, &perf);
        printf("Stats for %s:\n", searchtype);
        printstats(times, TEST_SEARCH_INNER_MAX);
    }

    printf("Test case C is complete!\n");
    manualsearchsettings = 0;

    printf("\n\n*****Testing Complete for %s********\n\nTotal number of voluntary and involuntary context switches: %llu\nAverage time per context switch: %lluns\n", searchtype, perf.numcontextswitch,  perf.contexttimeavg);

    return 0;
}

/**
 * calcuate the needed search settings 
 * including: number of workers (aka threads or procs)
 * size to search per each worker
 */

void getSearchSettings(int listSize, int bucketSize, int *number_of_workers,
                       int *size_to_search_for_each)
{
    if (bucketSize > MAX_BUCKET_SIZE) {
        bucketSize = MAX_BUCKET_SIZE;
    }

    if (listSize % bucketSize == 0) {
        *number_of_workers = listSize / bucketSize;
    } else {
        *number_of_workers = (listSize / bucketSize) + 1;
    }

    *size_to_search_for_each = bucketSize;

    // printf("size_to_search_for_each: %d, listSize: %d, number_of_workers: %d, bucketSize: %d\n", *size_to_search_for_each, listSize, *number_of_workers, bucketSize);
}

/** 
 * given an array and size, fill it with random values between 0 and listSize, all unique
 * */

void generateRandomArray(int *array, int listSize)
{
    for (int i = 0; i < listSize; i++) {
        array[i] = i + 1;
    }

    int randIndex1;
    int randomizingIterations = 0;

    if (listSize >= 100) {
        randomizingIterations = (listSize / 4) * 3;
    } else {
        randomizingIterations = listSize;
    }

    //    printf("iterations = %d\n", randomizingIterations);
    for (int i = 0; i < randomizingIterations; i++) {
        
        if (listSize < 2) {
            randIndex1 = 0;
        } else {
            randIndex1 = (rand() % (listSize - 1));
        }

        swapOneIndex(array, randIndex1, listSize);
    }
}

/** 
 * swap a single index with a random one
 * */

void swapOneIndex(int *array, int indexToBeSwapped, int listSize)
{
    int tempVal;
    int randomIndex = (rand() % (listSize - 1));
    // printf("indextobeswapped %d, listsize %d, randomindex %d\n", indexToBeSwapped, listSize, randomIndex);
    tempVal = array[indexToBeSwapped];
    array[indexToBeSwapped] = array[randomIndex];
    array[randomIndex] = tempVal;
}

/** 
 * linear search is used by itself and also throughout the multitest functions
 * */

int linear_search(int *list, int arraySize, int goal)
{
    for (int i = 0; i < arraySize; i++) {
        if (list[i] == goal) {
            return i;
        }
    }

    return -1;
}


// ********* do  tests *********
/**
 * this function performs a variety of tests based on given global variable flgas combined with function inputs
 * it also times it
 * 
 * */
void dotest(ullint times[], int size, int numtimes, struct perfdata* perf)
{
    int bucketsize = MAX_BUCKET_SIZE;
    int numworkers = 0;
    int sizeperworker = 0;
    int indexfound;
    int randomtarget;

    struct timespec teststart, testend;

    int *current_list = (int *)malloc(size * sizeof(int));

    generateRandomArray(current_list, size);
    if (manualsearchsettings) {
        numworkers = manualnumworkers;
        sizeperworker = manualsizeperworker;
    } else {
        getSearchSettings(size, bucketsize, &numworkers, &sizeperworker);
    }

    randomtarget = (rand() % size) + 1;

    for (int k = 0; k < numtimes; k++) {
        clock_gettime(CLOCK_REALTIME, &teststart);
        indexfound = search(current_list, size, randomtarget, numworkers, sizeperworker, perf);
        clock_gettime(CLOCK_REALTIME, &testend);

        if (indexfound == -1) {
            //            printf("error couldn't find %d\n", randomtarget);
        } else {
            //            printf("Found the number %d at index %d; where array[%d] = %d\n", randomtarget, indexfound, indexfound, current_list[indexfound]);
            swapOneIndex(current_list, indexfound, size);
        }

        times[k] = BILLION * (testend.tv_sec - teststart.tv_sec) + testend.tv_nsec - teststart.tv_nsec;
        //times[j - startsize] += testend.tv_nsec - teststart.tv_nsec;
    }

    free(current_list);
}
/******* math helps for generating output *****/
ullint max(ullint *array, int size)
{
    int i;
    ullint maximum = array[0];
    for (i = 1; i < size; i++) {
        if (array[i] > maximum) {
            maximum = array[i];
        }
    }
    return maximum;
}

ullint min(ullint *array, int size)
{
    int i;
    ullint minimum = array[0];
    for (i = 1; i < size; i++) {
        if (array[i] < minimum) {
            minimum = array[i];
        }
    }
    return minimum;
}

ullint sum(ullint *array, int size)
{
    ullint sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }

    return sum;
}

ullint stddev(ullint *array, int size)
{
    ullint avg = mean(array, size);
    ullint stddevres = 0;

    for (int i = 0; i < size; i++) {
        ullint current = array[i];
        ullint base = llabs(current - avg);
        stddevres += pow(base, 2);
    }

    // (from i = 0 to k; k = MAX_LIST_SIZE; sum ((data[i] - avg)^2))^(1/2)

    return sqrt(stddevres / size);
}

ullint mean(ullint *times, int size)
{
    ullint ret = sum(times, size) / size;
    return ret;
}

/** print helpers**/

void printintArray(int *array, int listSize)
{
    int i;
    printf("[");
    for (i = 0; i < listSize - 1; i++) {
        printf("%d, ", array[i]);
    }
    printf("%d]", array[listSize - 1]);
    printf("\n");
}

void printullintArray(ullint *array, int listSize)
{
    int i;
    printf("[");
    for (i = 0; i < listSize - 1; i++) {
        printf("%llu, ", array[i]);
    }
    printf("%llu]", array[listSize - 1]);
    printf("\n");
}

void printdoublensArray(double *array, int listSize)
{
    int i;
    printf("[");
    for (i = 0; i < listSize - 1; i++) {
        printf("%gns, ", (double)array[i]);
    }
    printf("%gns]", (double)array[listSize - 1]);
    printf("\n");
}

void printstats(ullint *times, int size)
{
    printf("\nIndividual times:\n");
    printullintArray(times, TEST_SEARCH_INNER_MAX);
    printf("\n\nMinimum:            %lluns\n", min(times, size));
    printf("Maximum:            %lluns\n", max(times, size));
    printf("Standard Deviation: %lluns\n", stddev(times, size));
    printf("Average:            %lluns\n", mean(times, size));

}