/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/linkedlist.h>

int linkedlist_init(LinkedList* ll)
{
    memset(ll, 0, sizeof(LinkedList));
    return 0;
}

int linkedlist_add(void* data, LinkedList* ll)
{
    LinkedListNode* node = ll->end;
    if (node)
    {
        /* Allocate next node */
        node->next = kmalloc(sizeof(LinkedListNode));
        if (!node->next)
            return -ENOMEM;

        node = node->next;
    }
    else
    {
        ll->root = kmalloc(sizeof(LinkedListNode));
        if (!ll->root)
            return -ENOMEM;

        node = ll->root;
    }

    node->data = data;
    node->next = NULL;

    ll->end = node;

    return 0;
}

int linkedlist_remove_by_data(void* data, LinkedList* ll)
{
    LinkedListNode* prev_node = NULL;
    LinkedListNode* n = ll->root;

    while (n)
    {
        if (n->data == data)
        {
            if (ll->end == n)
                ll->end = prev_node;

            if (prev_node)
                prev_node->next = n->next;
            else
                ll->root = n->next;

            kfree(n);
            return 0;
        }

        prev_node = n;
        n = n->next;
    }

    return -ENOENT;
}

int linkedlist_remove_by_position(size_t pos, LinkedList* ll)
{
    LinkedListNode* prev_node = NULL;
    LinkedListNode* n = ll->root;

    size_t running_count;
    for (running_count = 0; running_count < pos && n; ++running_count)
    {
        prev_node = n;
        n = n->next;
    }

    if (n)
    {
        if (ll->end == n)
            ll->end = prev_node;

        if (prev_node)
            prev_node->next = n->next;
        else
            ll->root = n->next;

        kfree(n);
        return 0;
    }

    return -ENOENT;
}
