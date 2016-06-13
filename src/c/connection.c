#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct ConnectionHandlerState {
  bool has_context;
  union {
    EventConnectionHandlers ctx_handlers;
    ConnectionHandlers handlers;
  };
  void *context;
} ConnectionHandlerState;

static bool prv_each_app_connection(void *object, void *ctx) {
  bool connected = (bool)ctx;
  ConnectionHandlerState *handlers = object;
  if (handlers->has_context) {
    if (handlers->ctx_handlers.pebble_app_connection_handler) {
      handlers->ctx_handlers.pebble_app_connection_handler(connected, handlers->context);
    }
  } else {
    if (handlers->handlers.pebble_app_connection_handler) {
      handlers->handlers.pebble_app_connection_handler(connected);
    }
  }
  return true;
}

static bool prv_each_pebblekit_connection(void *object, void *ctx) {
  bool connected = (bool)ctx;
  ConnectionHandlerState *handlers = object;
  if (handlers->has_context) {
    if (handlers->ctx_handlers.pebblekit_connection_handler) {
      handlers->ctx_handlers.pebblekit_connection_handler(connected, handlers->context);
    }
  } else {
    if (handlers->handlers.pebblekit_connection_handler) {
      handlers->handlers.pebblekit_connection_handler(connected);
    }
  }
  return true;
}

static void prv_handle_pebblekit_connection(bool connected) {
  linked_list_foreach(s_handler_list, prv_each_pebblekit_connection, (void *)connected);
}

static void prv_handle_app_connection(bool connected) {
  linked_list_foreach(s_handler_list, prv_each_app_connection, (void *)connected);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    connection_service_subscribe((ConnectionHandlers) {
      .pebble_app_connection_handler = prv_handle_app_connection,
      .pebblekit_connection_handler = prv_handle_pebblekit_connection,
    });
  }
}

EventHandle events_connection_service_subscribe(ConnectionHandlers handlers) {
  prv_init();

  ConnectionHandlerState *our_handlers = malloc(sizeof(ConnectionHandlerState));
  our_handlers->has_context = false;
  our_handlers->handlers = handlers;
  our_handlers->context = NULL;
  linked_list_append(s_handler_list, our_handlers);
  return our_handlers;
}

EventHandle events_connection_service_subscribe_context(EventConnectionHandlers handlers, void *context) {
  prv_init();

  ConnectionHandlerState *our_handlers = malloc(sizeof(ConnectionHandlerState));
  our_handlers->has_context = true;
  our_handlers->ctx_handlers = handlers;
  our_handlers->context = context;
  linked_list_append(s_handler_list, our_handlers);
  return our_handlers;
}

void events_connection_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    connection_service_unsubscribe();
    free(s_handler_list);
    s_handler_list = NULL;
  }
}


