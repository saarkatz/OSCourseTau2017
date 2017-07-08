#include "message_list.h"
#include "definitions.h"

typedef struct message_node {
  int id;
  Smessage *message;

  Mnode *prev;
  Mnode *next;
} Mnode;

typedef struct message_list {
  int count;
  Mnode Sentinal;
} Mlist;

Mlist *mlist_create(void) {
  Mlist *list;

  PRINTK_D("%s", "Createing list\n");

  list = (Mlist*)kmalloc(sizeof(*list), GFP_KERNEL);
  if (NULL == list) {
    // According to
    // https://stackoverflow.com/questions/37897767/error-handling-checking-in-the-kernel-realm
    // kmalloc does not return any indication of error, it just returns NULL
    // on failure. I will return ENOMEM in this case.
    PRINTK_I("%s", ERR_CREATE_MLIST);
    return NULL;
  }

  memset(list, 0, sizeof(*list));
  list->Sentinal.next = &list->Sentinal;
  list->Sentinal.prev = &list->Sentinal;

  return list;
}

// Private add method.
// Connects node after target. No argument checks are done!
// The list should be circular.
int mlist_add_after(Mnode *target, Mnode *node) {
  node->next = target->next;
  target->next = node;
  node->prev = target;
  node->next->prev = node;
  return 0;
}

// Private remove method.
// Disconnects the node. No argument checks are done!
int mlist_remove_at(Mnode *node) {
  node->next->prev = node->prev;
  node->prev->next = node->next;
  node->next = node;
  node->prev = node;
  return 0;
}

int mlist_append(Mlist *list, int id, Smessage *message) {
  Mnode *node;

  PRINTK_D("Appending to list (id:%d)\n", id);

  node = (Mnode*)kmalloc(sizeof(*node), GFP_KERNEL);
  if (node <= 0) {
    PRINTK_I("%s", ERR_CREATE_MNODE);
    return -ENOMEM;
  }

  memset(node, 0, sizeof(*node));
  node->id = id;
  node->message = message;

  // Append node to list
  mlist_add_after(&list->Sentinal, node);
  list->count += 1;
  return 0;
}

Smessage *mlist_find(Mlist *list, int id) {
  Mnode *iterator = &list->Sentinal;

  PRINTK_D("Searching for item (id:%d)\n", id);

  while (iterator != list->Sentinal.prev) {
    iterator = iterator->next;
    if (id == iterator->id) {
      return iterator->message;
    }
  }
  return NULL;
}

Smessage *mlist_remove(Mlist *list, int id) {
  Mnode *iterator = &list->Sentinal;
  Smessage *message;

  PRINTK_D("Removing item from list (id:%d)\n", id);

  while (iterator != list->Sentinal.prev) {
    iterator = iterator->next;
    if (id == iterator->id) {
      mlist_remove_at(iterator);
      message = iterator->message;
      kfree(iterator);
      list->count -= 1;

      PRINTK_D("%s", "Item removed\n");

      return message;
    }
  }

  PRINTK_D("No item with id %d was found\n", id);

  return NULL;
}

void mlist_destroy(Mlist *list) {
  Mnode *iterator = &list->Sentinal;

  PRINTK_D("%s", "Destroying list\n");

  while (&list->Sentinal != list->Sentinal.next) {
    iterator = list->Sentinal.next;
    mlist_remove_at(iterator);
    kfree(iterator->message);
    kfree(iterator);
  }
  kfree(list);
}
