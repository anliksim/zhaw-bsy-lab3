//******************************************************************************
// File:    queues.c
// Purpose: implementation of queues for scheduler
// Author:  M. Thaler, 2012, (based on former work by J. Zeman and M. Thaler)
// Changes: tham, 2/2015
//          tham, 1/2019 (queState -> reset by mqDelete)
// Version: v.fs19
//******************************************************************************

#include <stdlib.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdio.h>

#include "mlist.h"
#include "mthread.h"
#include "queues.h"

//******************************************************************************
// local function prototypes

unsigned int mqGetTime(void);

//******************************************************************************
// Static data of queueing system

mlist_t* readyQueues[NUM_PRIO_QUEUES];  // the ready queue
mlist_t* waitQueue;

//******************************************************************************
//
// Queueing functions
//
//==============================================================================
// Function:    initQueues
// Purpose:     initialize queueing system
//              behaves like a singleton

static int queueState = 0;

void mqInit(void) {    
    if (queueState == 0) {
        for (int i = 0; i < NUM_PRIO_QUEUES; i++){
            readyQueues[i] = mlNewList();
        }
        waitQueue = mlNewList();
        mqGetTime();                                // register start time
        queueState = 1;                             // now we are initialized 
    }
}

//==============================================================================
// Function:    delQueues
// Purpose:     clean up dynamically allocated data and reset queue state
// Hint:        will be called by scheduler before termination

void mqDelete(void) {
    for (int i = 0; i < NUM_PRIO_QUEUES; i++){
        mlDelList(readyQueues[i]);
    }
    mlDelList(waitQueue);
    queueState = 0;
    printf("\n*** cleaning queues ***\n");
}

//==============================================================================
// Function:    getNextThread
// Purpose:     returns thread (tcb) with highest priority in the ready queues
//              if there is no thread, return value is NULL

mthread_t* mqGetNextThread(void) {

    mthread_t* iter;
    while ((iter = mlReadFirst(waitQueue)) &&
           (mtGetReadyTime(iter) <= mqGetTime())) {
        mlDequeue(waitQueue);
        mlEnqueue(readyQueues[mtGetPrio(iter)], iter);
    }

    for (int i = 0; i < NUM_PRIO_QUEUES; i++){
        mthread_t* next = mlDequeue(readyQueues[i]);
        if (next != NULL) {
            return next;
        }
    }

    if (iter != NULL) {
        while (mtGetReadyTime(iter) > mqGetTime()) { }
        return mqGetNextThread();
    } else {
        return NULL;
    }
}

//==============================================================================
// Function:    add to queue
// Purpose:     initialize queueing system

void mqAddToQueue(mthread_t *tcb, int sleepTime) {
    if (sleepTime > 0) {
        mtSetReadyTime(tcb, mqGetTime() + sleepTime);
        mlSortIn(waitQueue, tcb);
    } else {
        mlEnqueue(readyQueues[mtGetPrio(tcb)], tcb);
    }
}

//==============================================================================
// Function:    printReadyQueueStatus
// Purpose:     prints the number of threads in the ready queues at mqGetTime()

void lqPrintReadyQueueStatus(mlist_t *queue) {
    printf("\t\tqueue, %d entries at time %d\n", 
                                        mlGetNumNodes(queue), mqGetTime());
}

//******************************************************************************
// Function:    getTime, local function
// Purpose:     returns wall clock time in 1ms resolution since program start,
//              works like singleton to register start time of module             

unsigned int mqGetTime(void) {
    static int    firstCall = 1;
    static struct timeval startTime;

    struct timeval currentTime;
    unsigned int time;

    if (firstCall) {
        gettimeofday(&startTime, NULL);
        firstCall = 0;
    }

    gettimeofday(&currentTime, NULL);
    time = (unsigned int)((currentTime.tv_sec  - startTime.tv_sec)*1000 + 
                         (currentTime.tv_usec - startTime.tv_usec)/1000);
    return time;
}

//******************************************************************************

