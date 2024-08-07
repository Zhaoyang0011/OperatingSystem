#ifndef _LIST_H
#define _LIST_H

#include <type.h>

typedef struct list {
    struct list *prev, *next;
} list_t;

KLINE void list_init(list_t *list) {
    list->prev = list;
    list->next = list;
    return;
}

KLINE void __list_del(list_t *prev, list_t *next) {
    next->prev = prev;
    prev->next = next;
    return;
}

KLINE void __list_add(list_t *node, list_t *prev, list_t *next) {
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
    return;
}

KLINE void list_add(list_t *node, list_t *head) {
    __list_add(node, head, head->next);
    return;
}

KLINE void list_add_tail(list_t *node, list_t *head) {
    __list_add(node, head->prev, head);
    return;
}

KLINE void __list_del_entry(list_t *entry) {
    __list_del(entry->prev, entry->next);
    return;
}

KLINE void list_del(list_t *entry) {
    __list_del(entry->prev, entry->next);
    list_init(entry);
    return;
}

KLINE void list_move(list_t *list, list_t *head) {
    list_del(list);
    list_add(list, head);
    return;
}

KLINE void list_move_tail(list_t *list, list_t *head) {
    list_del(list);
    list_add_tail(list, head);
    return;
}

KLINE bool_t list_is_empty(const list_t *head) {
    if (head->next == head) {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t list_is_empty_careful(const list_t *head) {
    list_t *next = head->next;
    if (next == head && next == head->prev) {
        return TRUE;
    }
    return FALSE;
}

KLINE bool_t list_is_last(const list_t *list, const list_t *head) {
    if (list->next == head) {
        return TRUE;
    }
    return FALSE;
}

#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
/* 	一直for，直到到达head 
	keep doing for loop until head is reached */

#define list_for_each_head_dell(pos, head) for (pos = (head)->next; pos != (head); pos = (head)->next)
/* 	一直for，直到head的下一个等于head（在循环的过程中需要改变head，否则死循环） 
	keep doing for loop until "next" of head equals to head (head need to be changed during the loop, otherwise that will be a infinite loop) */

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/* list_t的指针减去它在整个结构体中的位置(&((type *)0)->member) (地址为0的type型数据中的member成员，就是member成员在type型数据中的偏移量），等于该结构体的开始
   The pointer of list_t minus its in the structure (&((type *)0)->member) (The member "member" in the data of type "type" at address 0, equals to the offset of member "member" in type "type"), you will get the start address of that structure*/

#define list_next_entry(pos, type, member) \
    list_entry((pos)->member.next, type, member)
#endif