#include <queue.h>
#include <stdint.h>
#include <stdbool.h>

uint16_t push_idx = 0;
uint8_t peekStr[73];

void push(queue_t *queue, uint8_t item){
	if(isFull(queue)){
		return;
	}
	queue->array[queue->rear][push_idx] = item;
	push_idx++;
}

uint8_t* peek(queue_t *queue, uint8_t length){
	uint8_t i = 4;
	uint8_t j = 0;
	while(i < 4+length){
		peekStr[j] = queue->array[queue->front][i];
		i++;
		j++;
	}
	peekStr[j++] = 10;
	peekStr[j] = 0;
	return peekStr;
}

void enqueue(queue_t *queue){
	if(isFull(queue)){
		return;
	}
	queue->array[queue->rear][push_idx] = '\0';
	queue->rear = (queue->rear + 1) % queue->capacity;
//	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
	push_idx = 0;
}

void dequeue(queue_t *queue){
	if(isEmpty(queue)){
		return;
	}
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
}

bool isFull(queue_t *queue){
	return(queue->size == queue->capacity);
}

bool isEmpty(queue_t *queue){
	return(queue->size == 0);
}
