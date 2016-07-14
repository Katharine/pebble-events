#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct BatteryHandler {
  bool has_context;
  union {
    EventBatteryStateHandler ctx_handler;
    BatteryStateHandler handler;
  };
  void *context;
} BatteryHandler;

static bool prv_each_state_change(void *object, void *ctx) {
  BatteryChargeState *state = ctx;
  BatteryHandler *handlers = object;
  if (handlers->has_context) {
    handlers->ctx_handler(*state, handlers->context);
  } else {
    handlers->handler(*state);
  }
  return true;
}

static void prv_handle_state_change(BatteryChargeState state) {
  linked_list_foreach(s_handler_list, prv_each_state_change, &state);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    battery_state_service_subscribe(prv_handle_state_change);
  }
}

EventHandle events_battery_state_service_subscribe(BatteryStateHandler handler) {
  prv_init();

  BatteryHandler *our_handler = malloc(sizeof(BatteryHandler));
  our_handler->has_context = false;
  our_handler->handler = handler;
  our_handler->context = NULL;
  linked_list_append(s_handler_list, our_handler);
  return our_handler;
}

EventHandle events_battery_state_service_subscribe_context(EventBatteryStateHandler handler, void *context) {
  prv_init();

  BatteryHandler *our_handler = malloc(sizeof(BatteryHandler));
  our_handler->has_context = true;
  our_handler->ctx_handler = handler;
  our_handler->context = context;
  linked_list_append(s_handler_list, our_handler);
  return our_handler;
}

void events_battery_state_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    battery_state_service_unsubscribe();
    free(s_handler_list);
    s_handler_list = NULL;
  }
}


