#include "tools/myqueue.h"

Node* createNode(DataType data);
DataType getData(Node* node);
void setLink(Node* node, Node* nextNode);
Node* getLink(Node* node);


#include <stdlib.h>

Node* createNode(DataType data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->link = NULL;

    return newNode;
}

DataType getData(Node* node) {
    return node->data;
}

void setLink(Node* node, Node* nextNode) {
    node->link = nextNode;
}

Node* getLink(Node* node) {
    return node->link;
}

#include <stdbool.h>
#include <stdio.h>

typedef struct Queue Queue;

Queue* createQueue();
void enqueue(Queue* q, DataType data);
DataType dequeue(Queue* q);
DataType peek(Queue* q);
void printQ(Queue* q);
void freeQueue(Queue* q);
bool isEmpty(Queue* q);

Queue* createQueue() {
    Queue* newQ = (Queue*)malloc(sizeof(Queue));
    newQ->front = NULL;
    newQ->rear = NULL;

    return newQ;
}

void enqueue(Queue* q, DataType data) {
    Node* newNode = createNode(data);
    if(q->front == NULL) {
        q->front = newNode;
        q->rear = newNode;
    }
    else {
        setLink(q->rear, newNode);
        q->rear = newNode;
    }
}

DataType dequeue(Queue* q) {
    if(isEmpty(q)) {
        printf("Queue is Empty!\n");
        return 0;
    }
    else {
        Node* tempNode = q->front;
        DataType data = getData(q->front);

        if(tempNode == q->rear) {
            q->front = NULL;
            q->rear = NULL;
        }
        else {
            q->front = getLink(tempNode);
        }
        free(tempNode);
        return data;
    }
}

DataType peek(Queue* q) { //queue에 front에 해당하는 데이터 반환
    if(isEmpty(q)) {
        printf("Queue is Empty!\n");
        return 0;
    }
    else {
        return getData(q->front);
    }
}

void printQ(Queue* q) {
    Node* temp = q->front;

    printf("Queue : ");
    while(temp != NULL) {
        printf("%s ", getData(temp));
        temp = getLink(temp);
    }
    printf("\n\n");
}

void freeQueue(Queue* q) {
    if(!isEmpty(q)) {
        Node *target = q->front;
        Node *newFront = getLink(target);

        while (target != NULL) {
            newFront = getLink(target);
            free(target->data);
            free(target);
            target = newFront;
        }
        free(newFront->data);
        free(newFront);
    }
    free(q);
}

bool isEmpty(Queue* q) {
    return q->front == NULL;
}