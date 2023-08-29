#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 80
#define QUEUE_SIZE 4

typedef struct
{
	uint8_t front, rear, size;
	uint8_t capacity;
	uint8_t array[QUEUE_SIZE][BUFFER_SIZE];
} queue_t;

void push(queue_t *queue, uint8_t item);
uint8_t* peek(queue_t *queue, uint8_t length);
void enqueue(queue_t *queue);
void dequeue(queue_t *queue);
bool isFull(queue_t *queue);
bool isEmpty(queue_t *queue);


#endif


