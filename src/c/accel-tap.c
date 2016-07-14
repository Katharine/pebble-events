#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct TapHandler {
  bool has_context;
  union {
    EventAccelTapHandler ctx_handler;
    AccelTapHandler handler;
  };
  void *context;
} TapHandler;

typedef struct TapArgs {
  AccelAxisType axis;
  int32_t direction;
} TapArgs;

static bool prv_each_handle_accel_tap(void *object, void *ctx) {
  TapArgs *args = ctx;
  TapHandler *handler = object;
  if (handler->has_context) {
    handler->ctx_handler(args->axis, args->direction, handler->context);
  } else {
    handler->handler(args->axis, args->direction);
  }
  return true;
}

static void prv_handle_accel_tap(AccelAxisType axis, int32_t direction) {
  TapArgs args = {
    .axis = axis,
    .direction = direction,
  };
  linked_list_foreach(s_handler_list, prv_each_handle_accel_tap, &args);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    accel_tap_service_subscribe(prv_handle_accel_tap);
  }
}

EventHandle events_accel_tap_service_subscribe(AccelTapHandler handler) {
  prv_init();

  TapHandler *our_handler = malloc(sizeof(TapHandler));
  our_handler->has_context = false;
  our_handler->handler = handler;
  our_handler->context = NULL;
  linked_list_append(s_handler_list, our_handler);
  return our_handler;
}

EventHandle events_accel_tap_service_subscribe_context(EventAccelTapHandler handler, void *context) {
  prv_init();

  TapHandler *our_handler = malloc(sizeof(TapHandler));
  our_handler->has_context = true;
  our_handler->ctx_handler = handler;
  our_handler->context = context;
  linked_list_append(s_handler_list, our_handler);
  return our_handler;
}

void events_accel_tap_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    tick_timer_service_unsubscribe();
    free(s_handler_list);
    s_handler_list = NULL;
  }
}
