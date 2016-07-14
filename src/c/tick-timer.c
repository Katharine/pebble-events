#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;
static TimeUnits s_current_subscription;

typedef struct TickHandlerState {
  bool has_context;
  TimeUnits tick_units;
  union {
    EventTickHandler ctx_handler;
    TickHandler handler;
  };
  void *context;
} TickHandlerState;

typedef struct TickArgs {
  struct tm *tick_time;
  TimeUnits units_changed;
} TickArgs;

static bool prv_each_tick(void *object, void *ctx) {
  TickArgs *args = ctx;
  TickHandlerState *handler = object;
  if ((args->units_changed & handler->tick_units) != 0) {
    if (handler->has_context) {
      handler->ctx_handler(args->tick_time, args->units_changed, handler->context);
    } else {
      handler->handler(args->tick_time, args->units_changed);
    }
  }
  return true;
}

static void prv_tick(struct tm *tick_time, TimeUnits units_changed) {
  TickArgs args = {
    .tick_time = tick_time,
    .units_changed = units_changed,
  };
  linked_list_foreach(s_handler_list, prv_each_tick, &args);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
    s_current_subscription = 0;
  }
}

static void prv_update_subscription(TimeUnits tick_units) {
  if ((s_current_subscription | tick_units) == s_current_subscription) {
    return;
  }
  s_current_subscription |= tick_units;
  tick_timer_service_subscribe(s_current_subscription, prv_tick);
}

EventHandle events_tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler) {
  prv_init();

  TickHandlerState *state = malloc(sizeof(TickHandlerState));
  state->has_context = false;
  state->tick_units = tick_units;
  state->handler = handler;
  state->context = NULL;
  linked_list_append(s_handler_list, state);
  prv_update_subscription(tick_units);
  return state;
}

EventHandle events_tick_timer_service_subscribe_context(TimeUnits tick_units, EventTickHandler handler, void *context) {
  prv_init();

  TickHandlerState *state = malloc(sizeof(TickHandlerState));
  state->has_context = true;
  state->tick_units = tick_units;
  state->ctx_handler = handler;
  state->context = context;
  linked_list_append(s_handler_list, state);
  prv_update_subscription(tick_units);
  return state;
}

static bool prv_add_to_subscription(void *object, void *context) {
  TickHandlerState *state = object;
  s_current_subscription |= state->tick_units;
  return true;
}

void events_tick_timer_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  s_current_subscription = 0;
  if (linked_list_count(s_handler_list) == 0) {
    tick_timer_service_unsubscribe();
    free(s_handler_list);
    s_handler_list = NULL;
  } else {
    linked_list_foreach(s_handler_list, prv_add_to_subscription, NULL);
    tick_timer_service_subscribe(s_current_subscription, prv_tick);
  }
}
