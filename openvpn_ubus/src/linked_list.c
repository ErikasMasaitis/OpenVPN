#include "linked_list.h"


/**
 * delete linked list
 */
void delete_list()
{
    struct Clients *temp = clients;
    struct Clients *next;
    while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
    }
}

/**
 * push client to linked list
 */
void push_client(struct Clients **client, struct Clients *new)
{
    if (*client != NULL) {
            new->next = *client;
    }
    *client = new;
}

void delete_node(struct Clients** head_ref, char *string)
{
    struct Clients *temp = *head_ref, *prev;

    if (temp != NULL && strcmp(temp->address, string) == 0 || strcmp(temp->name, string) == 0) {
        *head_ref = temp->next;
        free(temp);
        return;
    }
    
    while (temp != NULL && strcmp(temp->address, string) != 0 && strcmp(temp->name, string) != 0) {
        prev = temp;
        temp = temp->next;
    }
 
    if (temp == NULL)
        return;
 
    prev->next = temp->next;
 
    free(temp);
}

/**
 * allocate memory for linked list
 */
struct Clients *create_client(struct Clients temp_client)
{
    struct Clients *client;
    client = (struct Clients *) malloc(sizeof(struct Clients));
    if (client == NULL)
            return client;
    
    strcpy(client->name, temp_client.name);
    strcpy(client->address, temp_client.address);
    strcpy(client->bytes_received, temp_client.bytes_received);
    strcpy(client->bytes_sent, temp_client.bytes_sent);
    strcpy(client->connected, temp_client.connected);
    client->next = NULL;

    return client;
}