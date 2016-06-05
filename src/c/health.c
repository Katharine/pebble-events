#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct HealthHandler {
  HealthEventHandler handler;
  void *context;
} HealthHandler;

static bool prv_each_handler(void *object, void *ctx) {
  HealthEventType *event = ctx;
  HealthHandler *handler = object;
  handler->handler(*event, handler->context);
  return true;
}

static void prv_handle_health_event(HealthEventType event, void *context) {
  linked_list_foreach(s_handler_list, prv_each_handler, &event);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    health_service_events_subscribe(prv_handle_health_event, NULL);
  }
}

EventHandle events_health_service_events_subscribe(HealthEventHandler handler, void *context) {
  prv_init();

  HealthHandler *our_handler = malloc(sizeof(HealthHandler));
  our_handler->handler = handler;
  our_handler->context = context;
  linked_list_append(s_handler_list, our_handler);
  return our_handler;
}

void events_health_service_events_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    health_service_events_unsubscribe();
  }
}
