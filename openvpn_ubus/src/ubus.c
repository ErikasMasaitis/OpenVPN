#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <unistd.h>
#include "main.h"
#include "ubus.h"
#include "helpers.h"

/**
 * get information about connected clients
 */
static int status_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);
/**
 * disconnect client
 */
static int set_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

static struct uloop_timeout event_timer;
static struct ubus_context *ctx;
static void set_event(void);
static void event_handler(struct uloop_timeout *timeout);
static void end_ubus(void);
/**
 * The enumaration array is used to specifie how much arguments will our 
 * methods accepted. Also to say trough which index which argument will 
 * be reacheble.
 */

enum {
	COUNTER_VALUE,
	__COUNTER_MAX
};

/**
 * This policy structure is used to determine the type of the arguments
 * that can be passed to some kind of method. 
 * This structure will be used in another structure applying this policy
 * to our selected method.
 */

static const struct blobmsg_policy kill_policy[] = {
	[COUNTER_VALUE] = { .name = "kill", .type = BLOBMSG_TYPE_STRING },
};

/**
 * This structure is used to register available methods.
 * If a method accepts arguments, the method should have a policy.
 */

static const struct ubus_method openvpn_methods[] = {
    UBUS_METHOD("dis_client", set_kill, kill_policy),
    UBUS_METHOD_NOARG("clients", status_get)
};

/**
 * This structure is used to define the type of our object with methods.
 */

static struct ubus_object_type openvpn_object_type =
	UBUS_OBJECT_TYPE(server_settings.name, openvpn_methods);

/**
 * This structure is used to register our program as an ubus object
 * with our methods and other neccessary data. 
 */
static struct ubus_object openvpn_object = {
	.name = server_settings.name,
	.type = &openvpn_object_type,
	.methods = openvpn_methods,
	.n_methods = ARRAY_SIZE(openvpn_methods),
};

/**
 * This method is used as a callback function to return the status
 * First we send command (status) to server
 * When we receive response we parse message to use it with ubus
 * @return 0 - on success; 1 - allocation problems
 */
static int status_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_buf b = {};

    int count = 0; //!< variable to tell if connected clients count are more than 0

    struct Clients *point_to_first = NULL;
    point_to_first = clients;
    while(point_to_first != NULL) {
            count = 1;
        	blob_buf_init(&b, 0);
            blobmsg_add_string(&b, "Common name", point_to_first->name);
            blobmsg_add_string(&b, "Real address", point_to_first->address);
            blobmsg_add_string(&b, "Bytes received", point_to_first->bytes_received);
            blobmsg_add_string(&b, "Bytes sent", point_to_first->bytes_sent);
            blobmsg_add_string(&b, "Connected since", point_to_first->connected);
            ubus_send_reply(ctx, req, b.head);
            blob_buf_free(&b);
            point_to_first = point_to_first->next;
    }

    if (count == 0) {
            blob_buf_init(&b, 0);
            blobmsg_add_string(&b, "information", "No clients are connected this moment");
            ubus_send_reply(ctx, req, b.head);
            blob_buf_free(&b);
            return 0;
    }

	return 0;
}

/**
 * This method is used as a callback function to return disconnected client information
 * First we send command (kill) to server
 * When we receive response we parse message to use it with ubus
 * @return 0 - on success; 1 - allocation problems
 */
static int set_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__COUNTER_MAX];
	struct blob_buf b = {};

    char *received_message;
    char argument[10]; //!< user input

	blobmsg_parse(kill_policy, __COUNTER_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[COUNTER_VALUE])
		    return UBUS_STATUS_INVALID_ARGUMENT;

	strcpy(argument, blobmsg_get_string(tb[COUNTER_VALUE]));
    received_message = parse_kill(argument);
    if (received_message == NULL)
            return 1;
    
    blob_buf_init(&b, 0);
    blobmsg_add_string(&b, "kill", received_message);
	ubus_send_reply(ctx, req, b.head);

    free(received_message);
    blob_buf_free(&b);

	return 0;
} 

/**
 * initiate ubus loop for process communication
 */
int process_ubus(void)
{
	uloop_init();
    set_event();

	ctx = ubus_connect(NULL);
	if (!ctx) {
            fprintf(stderr, "Failed to connect to ubus\n");
            return -1;
	}

	ubus_add_uloop(ctx);
	ubus_add_object(ctx, &openvpn_object);
	uloop_run();

    end_ubus();

	return 0;
}
/**
 * clear clients structure and end program
 */
static void end_ubus(void)
{
    if (clients != NULL) {
            free(clients);
            clients = NULL;
    }
	ubus_free(ctx);
	uloop_done();
    exit(1);
}
/**
 * event handler callback method.
 * checking if connection is still active
 * then gather information to fill status structure every 1s
 */
static void event_handler(struct uloop_timeout *timeout)
{
    if (is_socket_alive() != 0)
            end_ubus();
    gather_status();
    uloop_timeout_set(&event_timer, 400);
}

/**
 * setting event after 1s
 */
static void set_event(void)
{
    event_timer.cb = event_handler;
    uloop_timeout_set(&event_timer, 0);
}

