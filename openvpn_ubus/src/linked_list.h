#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <string.h>
#include <stdlib.h>

#include "ubus.h"

void push_client(struct Clients **client, struct Clients *new);
struct Clients *create_client(struct Clients temp_client);
void delete_list();
void delete_node(struct Clients** head_ref, char *ip);

#endif
