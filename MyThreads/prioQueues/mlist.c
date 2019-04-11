//******************************************************************************
// File:    mlist.c
// Purpose: implementation mlist (a single linked list with header dummy)
// Author:  M. Thaler, 2012, (based on former work by J. Zeman and M. Thaler)
// Version: v.fs19
//******************************************************************************

#include <stdlib.h>

#include "commondefs.h"
#include "mthread.h"
#include "mlist.h"

//******************************************************************************
// macro to allocate new tnode_t

#define mlNewTNode() (tnode_t *)malloc(sizeof(tnode_t))
#define mlNewTList() (mlist_t *)malloc(sizeof(mlist_t))

//******************************************************************************

mlist_t *mlNewList() {


    tnode_t *head = mlNewTNode();
    // TODO if head == NULL return ERROR
    head->next = NULL;
    head->tcb = NULL;

    mlist_t *list = mlNewTList();
    list->head = head;
    list->tail = head;
    list->iter = NULL;

    return list;

}

void mlDelList(mlist_t *list) {

    tnode_t *tmp;
    tnode_t *node = list->head;

    while (node != NULL) {
        tmp = node;
        node = node->next;
        free(tmp->tcb);
        free(tmp);
    }
    free(list);
}

void mlEnqueue(mlist_t *list, mthread_t *tcb) {

    tnode_t *new_node = mlNewTNode();
    new_node->next = NULL;
    new_node->tcb = tcb;

    list->tail->next = new_node;
    list->tail = new_node;
}

mthread_t *mlDequeue(mlist_t *list) {

    tnode_t *fist_node = list->head->next;
    if (fist_node == NULL) {
        return NULL;
    }

    list->head->next = fist_node->next;
    if (list->head->next == NULL) {
        list->tail = list->head;
    }

    mthread_t *thread = fist_node->tcb;
    free(fist_node);

    return thread;
}

void mlSortIn(mlist_t *list, mthread_t *tcb) {

    if (list->head->next == NULL) {
        mlEnqueue(list, tcb);
    } else {

        unsigned int tcb_time = mtGetReadyTime(tcb);
        for (tnode_t *node = list->head; node->next != NULL; node = node->next) {

            if (tcb_time < mtGetReadyTime(node->next->tcb)) {

                tnode_t *new_node = mlNewTNode();
                new_node->next = node->next;
                new_node->tcb = tcb;

                node->next = new_node;
                break;
            } else if(node->next->next == NULL) {
                mlEnqueue(list, tcb);
                break;
            }

        }
    }
}

mthread_t *mlReadFirst(mlist_t *list) {
    if (list->head->next == NULL) {
        return NULL;
    }
    return list->head->next->tcb;
}

unsigned int mlGetNumNodes(mlist_t *list) {
    unsigned int count = 0;
    for (tnode_t *node = list->head->next; node != NULL; node = node->next) {
        ++count;
    }
    return count;
}

void mlSetPtrFirst(mlist_t *list) {
    list->iter = list->head->next;
}

void mlSetPtrNext(mlist_t *list) {
    if (list->iter == NULL) {
        list->iter = list->head;
    } else {
        list->iter = list->iter->next;
    }
}

mthread_t *mlReadCurrent(mlist_t *list) {
    if (list->iter == NULL) {
        return NULL;
    }
    return list->iter->tcb;
}

//******************************************************************************

