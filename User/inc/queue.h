#pragma once
#include "stm32f4xx.h"
#include <stdbool.h>

#define ARR_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

#define RX_QUEUE_ID 0
#define TX_QUEUE_ID 1
#define QUEUES_COUNT 2

typedef struct
{
    void* storage;
    void* first;
    void* next;
    uint8_t capacity;
    uint8_t elementSize;
} Queue_t;

void queueInit(Queue_t* queue, void* storage, uint8_t capacity, uint8_t elementSize);
Queue_t* queueGetId(uint8_t id);
bool queueEnqueue(Queue_t* queue, void * element);
bool queueDequeue(Queue_t* queue, void * element);
uint8_t queueSize(Queue_t * queue);
bool queuePeek(Queue_t* queue, void * element);
bool queueIsFull(Queue_t * queue);
bool queueIsEmpty(Queue_t * queue);
