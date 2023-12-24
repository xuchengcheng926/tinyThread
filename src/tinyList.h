#ifndef __TINY_LIST_H__
#define __TINY_LIST_H__

typedef struct tagtinyListItem
{
    struct tagtinyListItem* next;
} tinyListItem;

typedef tinyListItem tinyListHead;

#define tinyListPtr(a) ((tinyListItem*)(&a))
#define tinyListTypePtr(a) ((tinyListItem*)(a))

#define tinyList_head(head) (head.next)
#define tinyList_next(ptr) ((ptr && tinyListTypePtr(ptr)->next)? tinyListTypePtr(ptr)->next: NULL)

#define tinyList_foreach(head) for (tinyListItem* p = tinyList_head(head); p != NULL; p = tinyList_next(p))

#define tinyList_init(head) {head.next = NULL;}

#define tinyList_destroy(head) tinyList_init(head)

#define tinyList_destroy_free(head, _myfree)         \
do{                                         \
    tinyListItem* pre = NULL;               \
    tinyList_foreach(head)                  \
    {                                       \
        if(pre) _myfree(pre);                  \
        pre = p;                            \
    }                                       \
    _myfree(pre);                              \
    head.next = NULL;                       \
}while(0);

#define tinyList_pushback(head, item)       \
do{                                         \
    tinyListItem* ptr = (tinyListPtr(head));\
    while(ptr->next)                        \
    {                                       \
        ptr = ptr->next;                    \
    }                                       \
    ptr->next = tinyListTypePtr(item);      \
    (tinyListTypePtr(item))->next = NULL;   \
}while(0);

#define tinyList_size(head)                 \
({                                          \
    tinyListItem* ptr = &head;              \
    int cnt = 0;                            \
    while(ptr->next)                        \
    {                                       \
        ptr = ptr->next;                    \
        cnt++;                              \
    }                                       \
    cnt;                                    \
})

#define tinyList_remove(head, item)                  \
do{                                                  \
    tinyListItem* ptr = &head;                       \
    tinyListItem* pre = &head;                       \
    while((ptr = ptr->next) != NULL)                 \
    {                                                \
        if(ptr == tinyListTypePtr(item))             \
            pre->next = tinyListTypePtr(item)->next; \
        pre = ptr;                                   \
    }                                                \
}while(0);


#endif // !__TINY_LIST_H__
