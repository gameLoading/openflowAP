#ifndef QUEUE_DATATYPE_H
#define QUEUE_DATATYPE_H

typedef char* DataType;

#endif //QUEUE_DATATYPE_H

#ifndef QUEUE_QUEUENODE_H
#define QUEUE_QUEUENODE_H

typedef struct Node Node;

Node* createNode(DataType data);
DataType getData(Node* node);
void setLink(Node* node, Node* nextNode);
Node* getLink(Node* node);

#endif //QUEUE_QUEUENODE_H

#include <stdlib.h>

typedef struct Node {
    DataType data;
    Node* link;
} Node;

#ifndef QUEUE_QUEUE_H
#define QUEUE_QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Queue Queue;

Queue* createQueue();
void enqueue(Queue* q, DataType data);
DataType dequeue(Queue* q);
DataType peek(Queue* q);
void printQ(Queue* q);
void freeQueue(Queue* q);
bool isEmpty(Queue* q);
int copy_queue();

#endif //QUEUE_QUEUE_H

typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;