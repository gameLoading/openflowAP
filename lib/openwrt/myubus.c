/*
 * Copyright (C) 2011 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <unistd.h>
#include <string.h>
#include "openwrt/myubus.h"
#include <syslog.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include "tools/mysemaphore.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

static struct blob_buf b;
static int listen_timeout;
static int timeout = 30;
static bool simple_output = false;
static int verbose = 0;
static int monitor_dir = -1;
static uint32_t monitor_mask;
static const char * const monitor_types[] = {
	[UBUS_MSG_HELLO] = "hello",
	[UBUS_MSG_STATUS] = "status",
	[UBUS_MSG_DATA] = "data",
	[UBUS_MSG_PING] = "ping",
	[UBUS_MSG_LOOKUP] = "lookup",
	[UBUS_MSG_INVOKE] = "invoke",
	[UBUS_MSG_ADD_OBJECT] = "add_object",
	[UBUS_MSG_REMOVE_OBJECT] = "remove_object",
	[UBUS_MSG_SUBSCRIBE] = "subscribe",
	[UBUS_MSG_UNSUBSCRIBE] = "unsubscribe",
	[UBUS_MSG_NOTIFY] = "notify",
};

static const char *format_type(void *priv, struct blob_attr *attr)
{
	static const char * const attr_types[] = {
		[BLOBMSG_TYPE_INT8] = "\"Boolean\"",
		[BLOBMSG_TYPE_INT32] = "\"Integer\"",
		[BLOBMSG_TYPE_STRING] = "\"String\"",
		[BLOBMSG_TYPE_ARRAY] = "\"Array\"",
		[BLOBMSG_TYPE_TABLE] = "\"Table\"",
	};
	const char *type = NULL;
	size_t typeid;

	if (blob_id(attr) != BLOBMSG_TYPE_INT32)
		return NULL;

	typeid = blobmsg_get_u32(attr);
	if (typeid < ARRAY_SIZE(attr_types))
		type = attr_types[typeid];
	if (!type)
		type = "\"(unknown)\"";

	return type;
}

static void receive_list_result(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	struct blob_attr *cur;
	char *s;
	size_t rem;

	if (simple_output || !verbose) {
		printf("%s\n", obj->path);
		return;
	}

	printf("'%s' @%08x\n", obj->path, obj->id);

	if (!obj->signature)
		return;

	blob_for_each_attr(cur, obj->signature, rem) {
		s = blobmsg_format_json_with_cb(cur, false, format_type, NULL, -1);
		printf("\t%s\n", s);
		free(s);
	}
}

char result[MAX_JSON_SIZE];
static void receive_call_result_data(struct ubus_request *req, int type, struct blob_attr *msg)
{
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " receive_call_result_data() called\n");
	closelog();
	if (!msg){
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		syslog(LOG_INFO, " not message\n");
		closelog();
		return;
	}

	char* str = blobmsg_format_json(msg, true);
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " test result : %s\n", str);
	closelog();
    strncpy(result, str, MAX_JSON_SIZE-1);
    result[MAX_JSON_SIZE-1] = '\0';
	// printf("%s---1", result);
	free(str);
}

static void print_event(const char *type, struct blob_attr *msg)
{
	char *str;

	str = blobmsg_format_json(msg, true);
	printf("{ \"%s\": %s }\n", type, str);
	fflush(stdout);
	free(str);
}

static int receive_request(struct ubus_context *ctx, struct ubus_object *obj,
			    struct ubus_request_data *req,
			    const char *method, struct blob_attr *msg)
{
	print_event(method, msg);
	return 0;
}

static void receive_event(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
	print_event(type, msg);
}

static int ubus_cli_list(struct ubus_context *ctx, int argc, char **argv)
{
	const char *path = NULL;

	if (argc > 1)
		return -2;

	if (argc == 1)
		path = argv[0];

	return ubus_lookup(ctx, path, receive_list_result, NULL);
}

static int ubus_cli_call(struct ubus_context *ctx, int argc, char **argv)
{
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " ubus_cli_call() called\n");
	closelog();

	uint32_t id;
	int ret;

	if (argc < 2 || argc > 3)	{
		return -2;
	}

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " blob_buf_init(&b, 0); call\n");
	closelog();

	blob_buf_init(&b, 0);
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " blob_buf_init(&b, 0); called\n");
	closelog();

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " argv[0] : %s \n", argv[0]);
	closelog();

		openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " argv[1] : %s \n", argv[1]);
	closelog();

	if (argc == 3) {
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, " argv[2] : %s \n", argv[2]);
		closelog();

		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, " blobmsg_add_json_from_string(&b, argv[2]); call\n");
		closelog();

		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, " %s parse try\n", argv[2]);
		closelog();
		if (!blobmsg_add_json_from_string(&b, argv[2])){
			openlog("ofproto.c", LOG_CONS, LOG_USER);
			syslog(LOG_INFO, " %s parse failed\n", argv[2]);
			closelog();
		};
	}

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " ret = ubus_lookup_id(ctx, argv[0], &id); called\n");
	closelog();
	ret = ubus_lookup_id(ctx, argv[0], &id);
	if (ret){
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, " ubus_lookup_id(ctx, argv[0], &id) error\n");
		closelog();
		return ret;
	}

	
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " ubus_invoke(ctx, id, argv[1], b.head, receive_call_result_data, NULL, timeout * 1000) called\n");
	closelog();
	ret = ubus_invoke(ctx, id, argv[1], b.head, receive_call_result_data, NULL, timeout * 1000);
	blob_buf_free(&b);
	return ret;
}

struct cli_listen_data {
	struct uloop_timeout timeout;
	bool timed_out;
};

static void ubus_cli_listen_timeout(struct uloop_timeout *timeout)
{
	struct cli_listen_data *data = container_of(timeout, struct cli_listen_data, timeout);
	data->timed_out = true;
	uloop_end();
}

static void do_listen(struct ubus_context *ctx, struct cli_listen_data *data)
{
	memset(data, 0, sizeof(*data));
	data->timeout.cb = ubus_cli_listen_timeout;
	uloop_init();
	ubus_add_uloop(ctx);
	if (listen_timeout)
		uloop_timeout_set(&data->timeout, listen_timeout * 1000);
	uloop_run();
	uloop_done();
}

static int ubus_cli_listen(struct ubus_context *ctx, int argc, char **argv)
{
	struct ubus_event_handler ev = {
		.cb = receive_event,
	};
	struct cli_listen_data data;
	const char *event;
	int ret = 0;

	if (argc > 0) {
		event = argv[0];
	} else {
		event = "*";
		argc = 1;
	}

	do {
		ret = ubus_register_event_handler(ctx, &ev, event);
		if (ret)
			break;

		argv++;
		argc--;
		if (argc <= 0)
			break;

		event = argv[0];
	} while (1);

	if (ret) {
		if (!simple_output)
			fprintf(stderr, "Error while registering for event '%s': %s\n",
				event, ubus_strerror(ret));
		return -1;
	}

	do_listen(ctx, &data);

	return 0;
}

static int ubus_cli_subscribe(struct ubus_context *ctx, int argc, char **argv)
{
	struct ubus_subscriber sub = {
		.cb = receive_request,
	};
	struct cli_listen_data data;
	const char *event;
	int ret = 0;

	if (argc > 0) {
		event = argv[0];
	} else {
		if (!simple_output)
			fprintf(stderr, "You need to specify an object to subscribe to\n");
		return -1;
	}

	ret = ubus_register_subscriber(ctx, &sub);
	for (; !ret && argc > 0; argc--, argv++) {
		uint32_t id;

		ret = ubus_lookup_id(ctx, argv[0], &id);
		if (ret)
			break;

		ret = ubus_subscribe(ctx, &sub, id);
	}

	if (ret) {
		if (!simple_output)
			fprintf(stderr, "Error while registering for event '%s': %s\n",
				event, ubus_strerror(ret));
		return -1;
	}

	do_listen(ctx, &data);

	return 0;
}


static int ubus_cli_send(struct ubus_context *ctx, int argc, char **argv)
{
	if (argc < 1 || argc > 2)
		return -2;

	blob_buf_init(&b, 0);

	if (argc == 2 && !blobmsg_add_json_from_string(&b, argv[1])) {
		if (!simple_output)
			fprintf(stderr, "Failed to parse message data\n");
		return -1;
	}

	return ubus_send_event(ctx, argv[0], b.head);
}

struct cli_wait_data {
	struct uloop_timeout timeout;
	struct ubus_event_handler ev;
	char **pending;
	int n_pending;
};

static void wait_check_object(struct cli_wait_data *data, const char *path)
{
	int i;

	for (i = 0; i < data->n_pending; i++) {
		if (strcmp(path, data->pending[i]) != 0)
			continue;

		data->n_pending--;
		if (i == data->n_pending)
			break;

		memmove(&data->pending[i], &data->pending[i + 1],
			(data->n_pending - i) * sizeof(*data->pending));
		i--;
	}

	if (!data->n_pending)
		uloop_end();
}

static void wait_event_cb(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
	static const struct blobmsg_policy policy = {
		"path", BLOBMSG_TYPE_STRING
	};
	struct cli_wait_data *data = container_of(ev, struct cli_wait_data, ev);
	struct blob_attr *attr;
	const char *path;

	if (strcmp(type, "ubus.object.add") != 0)
		return;

	blobmsg_parse(&policy, 1, &attr, blob_data(msg), blob_len(msg));
	if (!attr)
		return;

	path = blobmsg_data(attr);
	wait_check_object(data, path);
}

static void wait_list_cb(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	struct cli_wait_data *data = priv;

	wait_check_object(data, obj->path);
}


static void wait_timeout(struct uloop_timeout *timeout)
{
	uloop_end();
}

static int ubus_cli_wait_for(struct ubus_context *ctx, int argc, char **argv)
{
	struct cli_wait_data data = {
		.timeout.cb = wait_timeout,
		.ev.cb = wait_event_cb,
		.pending = argv,
		.n_pending = argc,
	};
	int ret;

	if (argc < 1)
		return -2;

	uloop_init();
	ubus_add_uloop(ctx);

	ret = ubus_register_event_handler(ctx, &data.ev, "ubus.object.add");
	if (ret)
		return ret;

	if (!data.n_pending)
		return ret;

	ret = ubus_lookup(ctx, NULL, wait_list_cb, &data);
	if (ret)
		return ret;

	if (!data.n_pending)
		return ret;

	uloop_timeout_set(&data.timeout, timeout * 1000);
	uloop_run();
	uloop_done();

	if (data.n_pending)
		return UBUS_STATUS_TIMEOUT;

	return ret;
}

static const char *
ubus_cli_msg_type(uint32_t type)
{
	const char *ret = NULL;
	static char unk_type[16];


	if (type < ARRAY_SIZE(monitor_types))
		ret = monitor_types[type];

	if (!ret) {
		snprintf(unk_type, sizeof(unk_type), "%d", type);
		ret = unk_type;
	}

	return ret;
}

static char *
ubus_cli_get_monitor_data(struct blob_attr *data)
{
	static const struct blob_attr_info policy[UBUS_ATTR_MAX] = {
		[UBUS_ATTR_STATUS] = { .type = BLOB_ATTR_INT32 },
		[UBUS_ATTR_OBJPATH] = { .type = BLOB_ATTR_STRING },
		[UBUS_ATTR_OBJID] = { .type = BLOB_ATTR_INT32 },
		[UBUS_ATTR_METHOD] = { .type = BLOB_ATTR_STRING },
		[UBUS_ATTR_OBJTYPE] = { .type = BLOB_ATTR_INT32 },
		[UBUS_ATTR_SIGNATURE] = { .type = BLOB_ATTR_NESTED },
		[UBUS_ATTR_DATA] = { .type = BLOB_ATTR_NESTED },
		[UBUS_ATTR_ACTIVE] = { .type = BLOB_ATTR_INT8 },
		[UBUS_ATTR_NO_REPLY] = { .type = BLOB_ATTR_INT8 },
		[UBUS_ATTR_USER] = { .type = BLOB_ATTR_STRING },
		[UBUS_ATTR_GROUP] = { .type = BLOB_ATTR_STRING },
	};
	static const char * const names[UBUS_ATTR_MAX] = {
		[UBUS_ATTR_STATUS] = "status",
		[UBUS_ATTR_OBJPATH] = "objpath",
		[UBUS_ATTR_OBJID] = "objid",
		[UBUS_ATTR_METHOD] = "method",
		[UBUS_ATTR_OBJTYPE] = "objtype",
		[UBUS_ATTR_SIGNATURE] = "signature",
		[UBUS_ATTR_DATA] = "data",
		[UBUS_ATTR_ACTIVE] = "active",
		[UBUS_ATTR_NO_REPLY] = "no_reply",
		[UBUS_ATTR_USER] = "user",
		[UBUS_ATTR_GROUP] = "group",
	};
	struct blob_attr *tb[UBUS_ATTR_MAX];
	int i;

	blob_buf_init(&b, 0);
	blob_parse(data, tb, policy, UBUS_ATTR_MAX);

	for (i = 0; i < UBUS_ATTR_MAX; i++) {
		const char *n = names[i];
		struct blob_attr *v = tb[i];

		if (!tb[i] || !n)
			continue;

		switch(policy[i].type) {
		case BLOB_ATTR_INT32:
			blobmsg_add_u32(&b, n, blob_get_int32(v));
			break;
		case BLOB_ATTR_STRING:
			blobmsg_add_string(&b, n, blob_data(v));
			break;
		case BLOB_ATTR_INT8:
			blobmsg_add_u8(&b, n, !!blob_get_int8(v));
			break;
		case BLOB_ATTR_NESTED:
			blobmsg_add_field(&b, BLOBMSG_TYPE_TABLE, n, blobmsg_data(v), blobmsg_data_len(v));
			break;
		}
	}

	return blobmsg_format_json(b.head, true);
}

static void
ubus_cli_monitor_cb(struct ubus_context *ctx, uint32_t seq, struct blob_attr *msg)
{
	static const struct blob_attr_info policy[UBUS_MONITOR_MAX] = {
		[UBUS_MONITOR_CLIENT] = { .type = BLOB_ATTR_INT32 },
		[UBUS_MONITOR_PEER] = { .type = BLOB_ATTR_INT32 },
		[UBUS_MONITOR_SEND] = { .type = BLOB_ATTR_INT8 },
		[UBUS_MONITOR_TYPE] = { .type = BLOB_ATTR_INT32 },
		[UBUS_MONITOR_DATA] = { .type = BLOB_ATTR_NESTED },
	};
	struct blob_attr *tb[UBUS_MONITOR_MAX];
	uint32_t client, peer, type;
	bool send;
	char *data;

	blob_parse_untrusted(msg, blob_raw_len(msg), tb, policy, UBUS_MONITOR_MAX);

	if (!tb[UBUS_MONITOR_CLIENT] ||
	    !tb[UBUS_MONITOR_PEER] ||
	    !tb[UBUS_MONITOR_SEND] ||
	    !tb[UBUS_MONITOR_TYPE] ||
	    !tb[UBUS_MONITOR_DATA]) {
		printf("Invalid monitor msg\n");
		return;
	}

	send = blob_get_int32(tb[UBUS_MONITOR_SEND]);
	client = blob_get_int32(tb[UBUS_MONITOR_CLIENT]);
	peer = blob_get_int32(tb[UBUS_MONITOR_PEER]);
	type = blob_get_int32(tb[UBUS_MONITOR_TYPE]);

	if (monitor_mask && type < 32 && !(monitor_mask & (1 << type)))
		return;

	if (monitor_dir >= 0 && send != monitor_dir)
		return;

	data = ubus_cli_get_monitor_data(tb[UBUS_MONITOR_DATA]);
	printf("%s %08x #%08x %14s: %s\n", send ? "->" : "<-", client, peer, ubus_cli_msg_type(type), data);
	free(data);
	fflush(stdout);
}

static int ubus_cli_monitor(struct ubus_context *ctx, int argc, char **argv)
{
	int ret;

	uloop_init();
	ubus_add_uloop(ctx);
	ctx->monitor_cb = ubus_cli_monitor_cb;
	ret = ubus_monitor_start(ctx);
	if (ret)
		return ret;

	uloop_run();
	uloop_done();

	ubus_monitor_stop(ctx);
	return 0;
}

static struct {
	const char *name;
	int (*cb)(struct ubus_context *ctx, int argc, char **argv);
} commands[] = {
	{ "list", ubus_cli_list },
	{ "call", ubus_cli_call },
	{ "listen", ubus_cli_listen },
	{ "subscribe", ubus_cli_subscribe },
	{ "send", ubus_cli_send },
	{ "wait_for", ubus_cli_wait_for },
	{ "monitor", ubus_cli_monitor },
};

int ubus_cmd_call(char* path, char* item, char* buffer, char* arg){
	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " ubus_cmd_call() call\n");
	closelog();
	int semid = lockSemaphore();

	const char *progname, *ubus_socket = NULL;
	struct ubus_context *ctx;
	int ret = 0;
	char *cmd;
	size_t i;
	int ch;

	int argc = arg == NULL ? 3 : 4;
	char* values[] = { 
		"call",
		path,
		item,
		arg
	};
	char **argv = values;

	progname = argv[0];
	cmd = argv[0];

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " ubus_connect(ubus_socket); call\n");
	closelog();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		if (!simple_output)

			fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	argv++;
	argc--;

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	// syslog(LOG_INFO, " callback of commands call\n");
	closelog();

	if (argc == 3){
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, " arg:%p argv:%s\n", argv[2], argv[2]);
		closelog();
	}

	ret = -2;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {	
		openlog("ofproto.c", LOG_CONS, LOG_USER);
		// syslog(LOG_INFO, "%s vs %s\n", commands[i].name, cmd);
		closelog();
		if (strcmp(commands[i].name, cmd) != 0)
			continue;

		ret = commands[i].cb(ctx, argc, argv);
		break;
	}

	openlog("ofproto.c", LOG_CONS, LOG_USER);
	syslog(LOG_INFO, " waiting for data receive\n");
	closelog();

    while (true)
    {
        if (result[0] != '\0') {
            strncpy(buffer, result, MAX_JSON_SIZE-1);
            buffer[MAX_JSON_SIZE-1] = '\0';
			openlog("ofproto.c", LOG_CONS, LOG_USER);
			// syslog(LOG_INFO, " data receive : %s\n", buffer);
			closelog();
            break;
        }
		usleep(100000);
    }
    
	if (ret > 0 && !simple_output)
		fprintf(stderr, "Command failed: %s\n", ubus_strerror(ret));

	ubus_free(ctx);
	unlockSemaphore(semid);
	return ret;
}

int ubus_cmd_just_call(char* path, char* item){
	int semid = lockSemaphore();
	const char *progname, *ubus_socket = NULL;
	struct ubus_context *ctx;
	int ret = 0;
	char *cmd;
	size_t i;
	int ch;

	int argc = 3;
	char* values[] = { 
		"call",
		path,
		item
	};
	char **argv = values;

	progname = argv[0];
	cmd = argv[0];

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		if (!simple_output)
			fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	argv++;
	argc--;

	ret = -2;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(commands[i].name, cmd) != 0)
			continue;

		ret = commands[i].cb(ctx, argc, argv);
		break;
	}
	if (ret > 0 && !simple_output)
		fprintf(stderr, "Command failed: %s\n", ubus_strerror(ret));

	ubus_free(ctx);
	unlockSemaphore(semid);
	return ret;
}