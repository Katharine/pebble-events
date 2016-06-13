#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;
static uint16_t s_requested_outbox_size;
static uint16_t s_requested_inbox_size;

typedef struct AppMessageHandlerState {
  EventAppMessageHandlers handlers;
  void *context;
} AppMessageHandlerState;

// Handle outbox_sent
static bool prv_each_outbox_sent(void *object, void *ctx) {
  DictionaryIterator *iterator = ctx;
  AppMessageHandlerState *state = object;
  if (state->handlers.sent) {
    state->handlers.sent(iterator, state->context);
  }
  return true;
}

static void prv_handle_outbox_sent(DictionaryIterator *iterator, void *context) {
  linked_list_foreach(s_handler_list, prv_each_outbox_sent, iterator);
}

// Handle outbox_failed
struct failed {
  DictionaryIterator *iterator;
  AppMessageResult reason;
};

static bool prv_each_outbox_failed(void *object, void *ctx) {
  struct failed *args = ctx;
  AppMessageHandlerState *state = object;
  if (state->handlers.failed) {
    state->handlers.failed(args->iterator, args->reason, state->context);
  }
  return true;
}

static void prv_handle_outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  struct failed args = {
    .iterator = iterator,
    .reason = reason,
  };
  linked_list_foreach(s_handler_list, prv_each_outbox_failed, &args);
}

// Handle inbox_received
static bool prv_each_inbox_received(void *object, void *ctx) {
  DictionaryIterator *iterator = ctx;
  AppMessageHandlerState *state = object;
  if (state->handlers.received) {
    state->handlers.received(iterator, state->context);
  }
  return true;
}

static void prv_handle_inbox_received(DictionaryIterator *iterator, void *context) {
  linked_list_foreach(s_handler_list, prv_each_inbox_received, iterator);
}

// Handle inbox_dropped
static bool prv_each_inbox_dropped(void *object, void *ctx) {
  AppMessageHandlerState *state = object;
  if (state->handlers.dropped) {
    state->handlers.dropped((AppMessageResult)ctx, state->context);
  }
  return true;
}

static void prv_handle_inbox_dropped(AppMessageResult reason, void *context) {
  linked_list_foreach(s_handler_list, prv_each_inbox_dropped, (void *)reason);
}

static bool prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    app_message_register_outbox_sent(prv_handle_outbox_sent);
    app_message_register_outbox_failed(prv_handle_outbox_failed);
    app_message_register_inbox_received(prv_handle_inbox_received);
    app_message_register_inbox_dropped(prv_handle_inbox_dropped);
  }
  return true;
}

void events_app_message_request_inbox_size(uint32_t size) {
  if (size > s_requested_inbox_size) {
    s_requested_inbox_size = size;
  }
}

void events_app_message_request_outbox_size(uint32_t size) {
  if (size > s_requested_outbox_size) {
    s_requested_outbox_size = size;
  }
}

AppMessageResult events_app_message_open(void) {
  if (s_requested_outbox_size < 16) {
    s_requested_outbox_size = 16;
  }
  if (s_requested_inbox_size < 16) {
    s_requested_inbox_size = 16;
  }

  return app_message_open(s_requested_inbox_size, s_requested_outbox_size);
}

EventHandle events_app_message_subscribe_handlers(EventAppMessageHandlers handlers, void *context) {
  prv_init();

  AppMessageHandlerState *state = malloc(sizeof(AppMessageHandlerState));
  state->handlers = handlers;
  state->context = context;
  linked_list_append(s_handler_list, state);
  return state;
}

void events_app_message_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    app_message_deregister_callbacks();
    free(s_handler_list);
    s_handler_list = NULL;
  }
}

EventHandle events_app_message_register_outbox_sent(AppMessageOutboxSent sent_callback, void *context) {
  return events_app_message_subscribe_handlers((EventAppMessageHandlers) {
    .sent = sent_callback,
  }, context);
}

EventHandle events_app_message_register_outbox_failed(AppMessageOutboxFailed failed_callback, void *context) {
  return events_app_message_subscribe_handlers((EventAppMessageHandlers) {
    .failed = failed_callback,
  }, context);
}

EventHandle events_app_message_register_inbox_received(AppMessageInboxReceived received_callback, void *context) {
  return events_app_message_subscribe_handlers((EventAppMessageHandlers) {
    .received = received_callback,
  }, context);
}

EventHandle events_app_message_register_inbox_dropped(AppMessageInboxDropped dropped_callback, void *context) {
  return events_app_message_subscribe_handlers((EventAppMessageHandlers) {
    .dropped = dropped_callback,
  }, context);
}
