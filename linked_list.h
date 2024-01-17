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

void list_free(List *list)
{
    while (list->size) {
        list_pop(list);
    }
    free(list);
}