#ifndef __SLIST_H__
#define __SLIST_H__

#ifndef NULL
    #define NULL           (0)
#endif

#define ES_SLIST_OBJECT_INIT(object) { NULL }

struct SList_Node
{
    struct SList_Node *next;                         /**< point to next node. */
};
typedef struct SList_Node SList_t;                /**< Type for single list. */


#define slist_for_each(pos, head) \
    for (pos = (head)->next; pos != NULL; pos = pos->next)

void     SList_init(SList_t *l);
void     SList_append(SList_t *l, SList_t *n);
void     SList_insert(SList_t *l, SList_t *n);
unsigned int SList_len(const SList_t *l);
SList_t *SList_remove(SList_t *l, SList_t *n);
SList_t *SList_first(SList_t *l);
SList_t *SList_tail(SList_t *l);
SList_t *SList_next(SList_t *n);
int      SList_isempty(SList_t *l);

#endif /* __SLIST_H__ */
