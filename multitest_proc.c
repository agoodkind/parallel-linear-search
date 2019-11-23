/**
 multitest_proc.c - program for multiprocessing searching
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>

/**
 * take in the id of the proc (relative index)
 * and the size of each bucket
 * then we do a linear search on just this bucket
 * if found exit(status); status = relative index
 * if not found exit(INDEX_NOT_FOUND)
 */
void search_part_of_array(int *array, int array_size, int goal, int id, int number_of_forks, int size_to_search_per_fork)
{
    int start_index = id * size_to_search_per_fork;
    int end_index;

    if (id == number_of_forks - 1) {
        end_index = array_size;
    } else {
        end_index = (id + 1) * size_to_search_per_fork;
    }

    int relative_found_index = linear_search(&array[start_index], end_index - start_index, goal);

    if (relative_found_index == -1) {
        exit(INDEX_NOT_FOUND);
    } else {
        exit(relative_found_index);
    }
}

/**
 * this function will fork the number of times specified by each fork
 * then based on their respective index we tell them to search that relative part of the array
 * we then clean up & use the exit status to find the relative size of the index then we multiply it to find the final index
 */
int fork_search_main(int *array, int array_size, int goal, int number_of_forks, int size_to_search_per_fork, struct perfdata* procperfdata)
{
    // the value we found
    // -1 means not found
    int final_result = -1;

    // pid tracking
    pid_t shut_down[number_of_forks];
    pid_t pid = -1;

    // create our fork
    for (int i = 0; i < number_of_forks; i++) {
        // we need to flush std out so we dont get weird results
        fflush(stdout);
        pid = fork();

        if (pid < 0) {
            // error in creating  afork
            return -2;
        }

        if (pid == 0) {
            // in child
            // search the subset of the array
            search_part_of_array(array, array_size, goal, i, number_of_forks, size_to_search_per_fork);
            break;
        } else {
            // in parent
            shut_down[i] = pid;
        }
    }

    // here all child procs & parent run this code
    // when we are in the parent we wait for each process to exit
    // we then check the exit status and if there is INDEX_NOT_FOUND set we return that
    int exit_result = 254;
    int status = 0;

    if (pid) {
        // in parent
        for (int i = 0; i < number_of_forks; i++) {

            struct rusage usage;
            struct timespec contextswitchstart, contextswitchend;

            clock_gettime(CLOCK_REALTIME, &contextswitchstart);
            wait4(shut_down[i], &status, 0, &usage);
            clock_gettime(CLOCK_REALTIME, &contextswitchend);

            // printf("time spent in context switch: %lluns\n", BILLION * (contextswitchend.tv_sec - contextswitchstart.tv_sec) + contextswitchend.tv_nsec - contextswitchstart.tv_nsec);
            ullint contexttime = BILLION * (contextswitchend.tv_sec - contextswitchstart.tv_sec) + contextswitchend.tv_nsec - contextswitchstart.tv_nsec;
            procperfdata->contexttimeavg += contexttime;
            if (procperfdata->contexttimeavg > contexttime) {
                procperfdata->contexttimeavg /= 2;
            }
            procperfdata->numcontextswitch += usage.ru_nvcsw;
            procperfdata->numcontextswitch += usage.ru_nivcsw;
            // printf("number of voluntary context switches: %ld \n", usage.ru_nvcsw);
            // printf("number of involuntary context switches: %ld \n", usage.ru_nivcsw);

            exit_result = WEXITSTATUS(status);

            if (exit_result != INDEX_NOT_FOUND) {
                // exit status is a relative index, so we convert back to an absolute index
                int absolute_index = (i * size_to_search_per_fork) + exit_result;
                final_result = absolute_index;
                // printf("i: %d, status: %d, exit_result: %d, size_to_search_per_fork: %d, final_result: %d\n", i, status, exit_result, size_to_search_per_fork, final_result);
            }
        }
    }

    return final_result;
}
