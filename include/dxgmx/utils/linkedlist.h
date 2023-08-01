/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_UTILS_LINKEDLIST_H
#define _DXGMX_UTILS_LINKEDLIST_H

typedef struct S_LinkedListNode
{
    void* data;
    struct S_LinkedListNode* next;
} LinkedListNode;

typedef struct S_LinkedList
{
    LinkedListNode* root;

    LinkedListNode* end;
} LinkedList;

/**
 * Initialize a linked list.
 *
 * No null pointers should be passed to this function.
 *
 * 'll' Target linked list.
 *
 * Returns:
 * 0 on sucess.
 */
int linkedlist_init(LinkedList* ll);

/**
 * Add an element to the end of a linked list.
 *
 * 'data' Data to store.
 * 'll' Non-NULL target linked list.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 */
int linkedlist_add(void* data, LinkedList* ll);

/**
 * Remove an element from a linked list by it's data. If there are multiple
 * elements in the list with the same 'data', then only the first
 * encountered one is removed.
 *
 * 'data' Data to remove by.
 * 'll' Non-NULL target linked list.
 *
 * Returns:
 * 0 on success.
 * -ENOENT if no element has it's data field set to 'data'.
 */
int linkedlist_remove_by_data(void* data, LinkedList* ll);

/**
 * Remove an element from a linked list by it's position.
 *
 * 'pos' 0 based position to remove.
 * 'll' Non-NULL target linked list.
 *
 * Returns:
 * 0 on success.
 * -ENOENT if 'pos' is bigger than the list size.
 */
int linkedlist_remove_by_position(size_t pos, LinkedList* ll);

/* This whole thing is scuffed as hell, i love C */
static LinkedListNode _g_break_node;

/* Convenience macro for walking a linked list. */
#define FOR_EACH_ENTRY_IN_LL(_ll, _type, _name)                                \
    for (LinkedListNode* _node_##_name = _ll.root; _node_##_name;              \
         _node_##_name = _node_##_name->next)                                  \
        for (_type _name = _node_##_name->data; _name; _name = NULL)

#define BREAK_LL(_name)                                                        \
    {                                                                          \
        _node_##_name = &_g_break_node;                                        \
        break;                                                                 \
    }

#endif // !_DXGMX_UTILS_LINKEDLIST_H
