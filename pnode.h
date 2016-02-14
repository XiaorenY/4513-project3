#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct node
{
    char *name;
    struct node* next;
}listNode;

typedef listNode* pNode;

void initList(pNode* head, pNode* tail){
    *head = *tail = NULL;
    return;
}
void addNode(pNode* head, pNode* tail, char* filename){
    pNode newNode = malloc(sizeof(listNode));
    if(newNode == NULL){
        printf("malloc failled.\n");
        exit(-1);
    }
    newNode->name = (char*)malloc(strlen(filename)*sizeof(char));
    if(newNode->name == NULL){
        printf("malloc failled.\n");
        exit(-1);
    }
    strcpy(newNode->name, filename);
    newNode->next = NULL;
    if(*head == NULL){
        *head = newNode;
        *tail = *head;
        return;
    }
    (*tail)->next = newNode;
    *tail = (*tail)->next;
    return;
}
void travList(pNode head){
    while(head != NULL){
        printf("%s", head->name);
        head = head->next;
    }
    return;
}
/* return 1 if found
    return 0 if not found */
int findMovie(pNode head, char* moviename) {
    while(head != NULL){
        if(strcmp(head->name, moviename) == 0){
            return 1;
        }
        head = head->next;
    }
    return 0;
}
