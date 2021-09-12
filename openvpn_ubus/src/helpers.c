#include "helpers.h"
#include "main.h"
#include "linked_list.h"

static void status_split_on_newline(char *string, int mode);
static void line_split_into_parts(char *string, int mode);
static void delete_if_not_exists(char *string);
static int parse_status(char *message);
static int count_lines(char *string);
static char *parse_message(char *message, char symbol);
static char *message_copy(char *text, int *len);
static void remove_char(char *s);

/**
 * get status information from server
 * parse message and fill structure with information
 * @return 0 - success; 1 - client count is 0; 2 - strstr problems; 3 || 4 - allocation problems
 */
int gather_status()
{
    char *send_message;
    char *received_message;
    int len = 0;
    int rc = 0; //return code

    recv_all(1); //!< receive unnecessary messages (example - new client connect)

    send_message = message_copy("status\n", &len);
    if (send_message == NULL)
            return 3;

    send_all(send_message, &len);
    received_message = recv_all(0);

    if (received_message == NULL) {
            rc = 4;
            goto cleanup_1;
    }

    rc = parse_status(received_message);

    free(received_message);
    cleanup_1:
            free(send_message);
    
    return rc;
}

/**
 * parse received message and remove unnecessary characters
 * @return 0 - success; 1 - client count is zero; 2 - allocation problems; 3 - strstr problems
 */
static int parse_status(char *message)
{
    char *message_start = NULL;
    char *message_end = NULL;
    char *mystring = NULL;
    int clients_count = 0;
    static int old_clients_count = 0;

    clients_count = count_lines(message);

    if (clients_count == 0) {
            if (clients != NULL) {
                    delete_list();
                    clients = NULL;
            }
            old_clients_count = 0;
            return 1;
    }

    message_start = strstr(message, "Since\r\n")+7;
    message_end = strstr(message, "ROUTING TABLE\r\n");

    if(message_start == NULL || message_end == NULL)
            return 2;

    mystring = (char *) malloc((sizeof(char) * 100) * (clients_count));

    if(mystring == NULL)
            return 3;

    memmove(mystring, message_start, message_end - message_start-1);
    mystring[message_end - message_start-1] = '\0';

    if(old_clients_count == clients_count) 
            status_split_on_newline(mystring, 1);
    else if (old_clients_count < clients_count)
            status_split_on_newline(mystring, 0);
    else if (old_clients_count > clients_count)
            status_split_on_newline(mystring, 2);

    free(mystring);

    old_clients_count = clients_count;

    return 0;
}

int update_clients(struct Clients temp_client)
{
    struct Clients *point_to_first = NULL; //!< temporary node, helps us always start loop from first element
    point_to_first = clients;
    while (point_to_first != NULL) {
        if (strcmp(temp_client.address, point_to_first->address) == 0) {
                strcpy(point_to_first->bytes_received, temp_client.bytes_received);
                strcpy(point_to_first->bytes_sent, temp_client.bytes_sent);
                break;
        }
        point_to_first = point_to_first->next;
    }
}

/**
 * split message when \n occur and split tokens into parts
 * @mode:
 * 0 - find clients that disconnect and delete them from linked list
 * 1 - only update clients informaion
 */
static void status_split_on_newline(char *string, int mode)
{
    char *token;

    while((token = strtok_r(string, "\n", &string))) {
            line_split_into_parts(token, mode);
    }
}

/**
 * split line into parts
 * @mode:
 * 0 - find clients that disconnect and delete them from linked list
 * 1 - only update clients informaion
 */
static void line_split_into_parts(char *string, int mode)
{
    char temp_string[80];
    char *token = NULL;
    char *rest = NULL;
    int counter = 0;

    struct Clients *new_client;
    struct Clients temp_client;
    strcpy(temp_string, string);

    for (token = strtok_r(temp_string, ",", &rest);
            token != NULL;
            token = strtok_r(NULL, ",", &rest)) {
            remove_char(token);
            if (counter == 0) 
                    strcpy(temp_client.name, token);
            else if (counter == 1)
                    strcpy(temp_client.address, token);
            else if (counter == 2)
                    strcpy(temp_client.bytes_received, token);
            else if (counter == 3)
                    strcpy(temp_client.bytes_sent, token);
            else if (counter == 4)
                    strcpy(temp_client.connected, token); 
            counter++;  
    }
    if (mode == 0) {
            if (check_if_not_exist(temp_client) != 0) {
                    new_client = create_client(temp_client);
                    push_client(&clients, new_client);
            }
    } else {
            update_clients(temp_client);
    }
}

/**
 * deletes node if occurence is found
 */
static void delete_if_not_exists(char *string)
{
    struct Clients *point_to_first = NULL; //!< temporary node, helps us always start loop from first element
    point_to_first = clients;
    while (point_to_first != NULL) {
        if (strcmp(string, point_to_first->address) == 0 || strcmp(string, point_to_first->name) == 0) {
                delete_node(&clients, string);
                break;
        }
        point_to_first = point_to_first->next;
    }
}


/**
 * checks if given temp client does not exist
 * @return: 0 - exists; 1 - does not exist
 */
int check_if_not_exist(struct Clients temp_client)
{
    struct Clients *point_to_first = NULL; //!< temporary node, helps us always start loop from first element
    point_to_first = clients;
    while (point_to_first != NULL) {
        if (strcmp(temp_client.address, point_to_first->address) == 0) {
                return 0;
        }
        point_to_first = point_to_first->next;
    }
    return 1;
}

/**
 * 
 */
char *parse_kill(char *argument)
{
    char *received_message = NULL;
    char *send_message = NULL;
    char command[50]; //!< command that will be send to server
    int len = 0;

    recv_all(1); //!< receive unnecessary messages (example - new client connect)

    sprintf(command, "kill %s\n", argument);
 
    send_message = message_copy(command, &len);
    if (send_message == NULL)
            return send_message;

    send_all(send_message, &len);
    received_message = recv_all(0); 

    if (strstr(received_message, "SUCCESS:") != NULL)
            delete_if_not_exists(argument);

    if (received_message == NULL)
            goto cleanup;

    remove_char(received_message);

    cleanup:
            free(send_message);

    return received_message;
}

/**
 * count lines of given string
 * @return:
 * 8 is default number of lines if zero clients are connected.
 * If client connects +2 lines are added to message. Formula: (count-8)/2
 */
static int count_lines(char *string)
{
    int count = 0;
    string = strchr(string, '\n');

    while (string != NULL) {
            string = strchr(string+1, '\n');
            count++;
    }

    return (count-8)/2;
}

/**
 * malloc and copy given text to return_message 
 * set len pointer the value
 * @return malloced message or NULL if failed
 */
static char *message_copy(char *text, int *len)
{
    char *return_message;
    return_message = (char *) malloc(sizeof(text));
    if (return_message == NULL) return return_message;

	strcpy(return_message, text);
    *len = strlen(return_message);

    return return_message;
}

/**
 * remove \n and \r from string for ubus printing
 */
static void remove_char(char *s)
{
    int writer = 0, reader = 0;

    while (s[reader])
    {
            if (s[reader]!= '\n' && s[reader] != '\r') 
            {   
                    s[writer++] = s[reader];
            }

            reader++;    
    }
    s[writer]=0;
}

/**
 * if parsing is only needed for removing unnecessary symbol from received message, we use this method
 * @return parsed message on success, NULL if something failed 
 */
static char *parse_message(char *message, char symbol)
{
    char *parsed_message;
    parsed_message = strchr(message, symbol);

    if (parsed_message == NULL)
            return parsed_message;

    parsed_message++;
    remove_char(parsed_message);

    return parsed_message;
}