#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct FocusHandlers {
  bool has_context;
  union {
    EventFocusHandlers ctx_handlers;
    AppFocusHandlers handlers;
  };
  void *context;
} FocusHandlers;

static bool prv_each_did_focus(void *object, void *ctx) {
  bool in_focus = (bool)ctx;
  FocusHandlers *handlers = object;
  if (handlers->has_context) {
    if (handlers->ctx_handlers.did_focus) {
      handlers->ctx_handlers.did_focus(in_focus, handlers->context);
    }
  } else {
    if (handlers->handlers.did_focus) {
      handlers->handlers.did_focus(in_focus);
    }
  }
  return true;
}

static bool prv_each_will_focus(void *object, void *ctx) {
  bool in_focus = (bool)ctx;
  FocusHandlers *handlers = object;
  if (handlers->has_context) {
    if (handlers->ctx_handlers.will_focus) {
      handlers->ctx_handlers.will_focus(in_focus, handlers->context);
    }
  } else {
    if (handlers->handlers.will_focus) {
      handlers->handlers.will_focus(in_focus);
    }
  }
  return true;
}

static void prv_handle_will_focus(bool in_focus) {
  linked_list_foreach(s_handler_list, prv_each_will_focus, (void *)in_focus);
}

static void prv_handle_did_focus(bool did_focus) {
  linked_list_foreach(s_handler_list, prv_each_did_focus, (void *)did_focus);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    app_focus_service_subscribe_handlers((AppFocusHandlers) {
      .will_focus = prv_handle_will_focus,
      .did_focus = prv_handle_did_focus,
    });
  }
}

EventHandle events_app_focus_service_subscribe_handlers(AppFocusHandlers handlers) {
  prv_init();

  FocusHandlers *our_handlers = malloc(sizeof(FocusHandlers));
  our_handlers->has_context = false;
  our_handlers->handlers = handlers;
  our_handlers->context = NULL;
  linked_list_append(s_handler_list, our_handlers);
  return our_handlers;
}

EventHandle events_app_focus_service_subscribe_handlers_context(EventFocusHandlers handlers, void *context) {
  prv_init();

  FocusHandlers *our_handlers = malloc(sizeof(FocusHandlers));
  our_handlers->has_context = true;
  our_handlers->ctx_handlers = handlers;
  our_handlers->context = context;
  linked_list_append(s_handler_list, our_handlers);
  return our_handlers;
}

EventHandle events_app_focus_service_subscribe(AppFocusHandler handler) {
  return events_app_focus_service_subscribe_handlers((AppFocusHandlers) {
    .will_focus = handler,
  });
}

EventHandle events_app_focus_service_subscribe_context(EventFocusHandler handler, void *context) {
  return events_app_focus_service_subscribe_handlers_context((EventFocusHandlers) {
    .will_focus = handler,
  }, context);
}

void events_app_focus_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    app_focus_service_unsubscribe();
    free(s_handler_list);
    s_handler_list = NULL;
  }
}


