#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>

typedef struct node {
    void *data;
    struct node *next;
} Node;

typedef struct list {
    int size;
    Node *head;
} List;

List *list_create()
{
    List *new_list = (List*)malloc(sizeof(List));
    new_list->size = 0;
    new_list->head = NULL;
    return new_list;
}

void list_append(List *list, void *data)
{
    Node *new_head = (Node*)malloc(sizeof(Node));
    new_head->data = data;
    new_head->next = list->head;
    list->head = new_head;
    list->size++;
}

void *list_pop(List *list)
{
    if (list->size == 0) {
        return NULL;
    }
    Node *pop_node = list->head;
    void *pop_data = pop_node->data;
    list->head = pop_node->next;
    free(pop_node);
    list->size--;
    return pop_data;
}

void *list_at(List *list, int index)
{
    if (index >= list->size || index < 0) {
        return 0;
    }
    Node *i_node = list->head;
    for (int i = 1; i < (list->size-index); ++i) {
        i_node = i_node->next;
    }
    return i_node->data;
}

int list_remove_obj(List *l1, void* obj)
{
    if (!l1->size) return 1;
    
    Node *list_iter = l1->head; 
    if ((void*)list_iter->data == obj) {
        l1->head = list_iter->next; 
        free(list_iter->data);
        free(list_iter);
        l1->size--;
        return 0;
    }

    for (int i = 0; i < l1->size - 1; ++i) {
        if ((void*)list_iter->next->data == obj) {
            Node *tmp = list_iter->next;
            list_iter->next = list_iter->next->next;
            free(tmp->data);
            free(tmp);
            l1->size--;
            return 0;
        }
        list_iter = list_iter->next;
    }

    return 1;
}

void list_free(List *l1)
{
    while (l1->size) {
        list_pop(l1);
    }
    free(l1);
}

void list_full_free(List *l1)
{
    while (l1->size) {
        free(list_pop(l1));
    }
    free(l1);
}

/* Concatenate two lists */
int list_cat(List *l1, List *l2)
{
    for (int i = 0; i < l2->size; ++i) {
        void *code = list_at(l2, i);
        list_append(l1, code);
    }
    list_free(l2);

    return 0;
}

#endif