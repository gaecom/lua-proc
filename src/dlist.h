#ifndef DLIST_H
#define DLIST_H

/**
 * A list entry
 **/
typedef struct dlist
{
    struct dlist    *next;
    struct dlist    *prev;
} dlist;

#define DLIST_DECLARE(name) \
    struct dlist name = { &(name), &(name) }

#define DLIST_INIT(ptr) \
    do \
    { \
        (ptr)->next = (ptr); \
        (ptr)->prev = (ptr); \
    } while (0)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know the prev/next
 * entries already!
 */
static inline void
dlist_add__(dlist *new, dlist *prev, dlist *next)
{
    next->prev = new; 
    new->next = next; 
    new->prev = prev; 
    prev->next = new;
}

/**
 * Add a new entry
 *
 * Insert a new entry after the specified entry.  This is good for implementing
 * stacks.
 *
 * @param new new entry to be added
 * @param head entry to add it after
 **/
static inline void
dlist_add(dlist *new, dlist *head)
{
    dlist_add__(new, head, head->next);
}

/**
 * Add a new entry
 *
 * Insert a new entry before the specified entry.  This is useful for
 * implementing queues.
 *
 * @param new new entry to be added
 * @param head entry to add it before
 **/
static inline void
dlist_add_tail(dlist   *new, dlist   *head)
{
    dlist_add__(new, head->prev, head);
}


/*
 * Delete a list entry by making the prev/next entries point to each other
 *
 * This is only for internal list manipulation where we know the prev/next
 * entries already!
 */
static inline void
dlist_delete__(dlist *prev, dlist *next)
{
    next->prev = prev; 
    prev->next = next; 
}

/**
 * Delete an entry from the list
 *
 * @note Dlist_isEmpty() on entry does not return true after this; the entry is
 * in an undefined state.
 *
 * @param entry The element to delete from the list.
 **/
static inline void 
dlist_delete(dlist   *entry)
{
    dlist_delete__(entry->prev, entry->next); 
    entry->next = (void *) 0; 
    entry->prev = (void *) 0;
}


/**
 * Delete entry from list and reinitialize it.
 *
 * @param entry The element to delete form the list.
 **/
static inline void
dlist_delete_and_init(dlist *entry)
{
    dlist_delete__(entry->prev, entry->next);
    DLIST_INIT(entry);
}

/**
 * Delte from one list and add as another's head
 *
 * @param list The entry to move
 * @param head The head that will preced our entry
 **/
static inline void
dlist_move(dlist *list, dlist *head)
{
    dlist_delete__(list->prev, list->next); 
    dlist_add(list, head);
}


/**
 * Delete from one list and add as another's tail
 *
 * @param list The entry to move
 * @param head The head that will preced our entry
 **/

static inline void
dlist_move_tail(dlist *list, dlist *head)
{
    dlist_delete__(list->prev, list->next); 
    dlist_add_tail(list, head);
}


/**
 * Test whether a list is empty
 *
 * @param head
 *      The list to test 
 **/
static inline int
dlist_is_empty(dlist *head)
{
    return head->next == head;
}

static inline void
dlist_splice__(dlist *list, dlist *head)
{
    dlist   *first = list->next;
    dlist   *last = list->prev;
    dlist   *at = head->next;

    first->prev = head;
    head->next = first;
    last->next = at;
    at->prev = last;
}


/**
 * Join two lists
 *
 * @param list The new list to add
 * @param head The place to add it in the first list
 **/
static inline void
dlist_splice(dlist *list, dlist *head)
{
    if (!dlist_is_empty(list))
    {
        dlist_splice__(list, head);
    }
}


/**
 * Join two lists and reinitialize the emptied list.
 *
 * @param list The new list to add
 * @param head The place to add it in the first list
 *
 * @note The list at list is reinitialized.
 **/
static inline void
dlist_splice_and_init(
    dlist   *list,
    dlist   *head
    )
{
    if (!dlist_isEmpty(list))
    {
        dlist_splice__(
            list,
            head);

        DLIST_INIT(
            list);
    }
}


/**
 * Get the struct for this entry
 *
 * @param ptr The &struct list_head pointer
 * @param type The type of the struct this is embedded in.
 * @param member The name of the list_struct within the struct
 **/
#define dlist_get_entry(ptr, type, member) \
    ((type *) ((char *) (ptr) - (unsigned long)(&((type *)0)->member)))


/**
 * Iterate over a list
 *
 * @param pos
 *      The &struct list_head to use as a loop counter.
 *
 * @param head
 *      The head for your list
 **/
#define dlist_foreach(pos, head) \
    for ( \
        pos = (head)->next; \
        pos != (head); \
        pos = pos->next)

/**
 * Iterate over a list safe against removal of list entry
 *
 * @param pos
 *      The &struct list_head to use as a loop counter.
 * @param n
 *      Another &struct list_head to use as tempory storage
 * @param head
 *      The head for your list
 **/
#define dlist_foreach_safe(pos, n, head) \
    for ( \
        pos = (head)->next, n = pos->next; \
        pos != (head); \
        pos = n, n = pos->next)

#endif /* ! DLIST_H */
