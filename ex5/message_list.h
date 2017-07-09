#ifndef MESSAGE_LIST_
#define MESSAGE_LIST_

#include "message_slot.h"

// Consider spinlocking the list
typedef struct message_list Mlist;

typedef struct message_node Mnode;

// Creates a new message list
Mlist *mlist_create(void);

// Appends a new message to the end of the list (Use mlist_find to make sure
// no item with the same id exists). Returns 0 on success, -E otherwise.
int mlist_append(Mlist *list, int id, Smessage *message);

// Searches the list for an item with ida and returns it. NULL is returned if
// no such item exists.
Smessage *mlist_find(Mlist *list, int id);

// Removes item with id.
Smessage *mlist_remove(Mlist *list, int id);

// Frees all memory associated with the list, including messages contained in
// the list.
void mlist_destroy(Mlist *list);

#endif /* !MESSAGE_LIST_ */