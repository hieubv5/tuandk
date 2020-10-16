#include "../include.h"

void SList_init(SList_t *l)
{
    l->next = NULL;
}

void SList_append(SList_t *l, SList_t *n)
{
    SList_t *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = NULL;
}

void SList_insert(SList_t *l, SList_t *n)
{
    n->next = l->next;
    l->next = n;
}

unsigned int SList_len(const SList_t *l)
{
    unsigned int len = 0;
    const SList_t *list = l->next;
    while (list != NULL)
    {
        list = list->next;
        len ++;
    }

    return len;
}

SList_t *SList_remove(SList_t *l, SList_t *n)
{
    /* remove slist head */
    SList_t *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (SList_t *)0) node->next = node->next->next;

    return l;
}

SList_t *SList_first(SList_t *l)
{
    return l->next;
}

SList_t *SList_tail(SList_t *l)
{
    while (l->next) l = l->next;

    return l;
}

SList_t *SList_next(SList_t *n)
{
    return n->next;
}

int SList_isempty(SList_t *l)
{
    return l->next == NULL;
}
