#include <pebble.h>
#include <@smallstoneapps/linked-list/linked-list.h>

#if PBL_API_EXISTS(unobstructed_area_service_subscribe)

#include "pebble-events.h"

static LinkedRoot *s_handler_list;

typedef struct AreaHandlers {
  UnobstructedAreaHandlers handlers;
  void *context;
} AreaHandlers;

static bool prv_each_will_change(void *object, void *ctx) {
  GRect *final_unobstructed_screen_area = ctx;
  AreaHandlers *handlers = object;
  if(handlers->handlers.will_change) {
    handlers->handlers.will_change(*final_unobstructed_screen_area, handlers->context);
  }
  return true;
}

static bool prv_each_change(void *object, void *ctx) {
  AnimationProgress progress = (AnimationProgress)ctx;
  AreaHandlers *handlers = object;
  if(handlers->handlers.change) {
    handlers->handlers.change(progress, handlers->context);
  }
  return true;
}

static bool prv_each_did_change(void *object, void *ctx) {
  AreaHandlers *handlers = object;
  if(handlers->handlers.did_change) {
    handlers->handlers.did_change(handlers->context);
  }
  return true;
}

static void prv_handle_will_change(GRect final_unobstructed_screen_area, void *context) {
  linked_list_foreach(s_handler_list, prv_each_will_change, &final_unobstructed_screen_area);
}

static void prv_handle_change(AnimationProgress progress, void *context) {
  linked_list_foreach(s_handler_list, prv_each_change, (void *)progress);
}

static void prv_handle_did_change(void *context) {
  linked_list_foreach(s_handler_list, prv_each_did_change, NULL);
}

static void prv_init(void) {
  if (!s_handler_list) {
    s_handler_list = linked_list_create_root();
  }
  if (linked_list_count(s_handler_list) == 0) {
    UnobstructedAreaHandlers handlers = {
      .will_change = prv_handle_will_change,
      .change = prv_handle_change,
      .did_change = prv_handle_did_change
    };
    unobstructed_area_service_subscribe(handlers, NULL);
  }
}

EventHandle events_unobstructed_area_service_subscribe(UnobstructedAreaHandlers handlers, void *context) {
  prv_init();

  AreaHandlers *our_handlers = malloc(sizeof(AreaHandlers));
  our_handlers->handlers = handlers;
  our_handlers->context = context;
  linked_list_append(s_handler_list, our_handlers);
  return our_handlers;
}

void events_unobstructed_area_service_unsubscribe(EventHandle handle) {
  int16_t index = linked_list_find(s_handler_list, handle);
  if (index == -1) {
    return;
  }
  free(linked_list_get(s_handler_list, index));
  linked_list_remove(s_handler_list, index);
  if (linked_list_count(s_handler_list) == 0) {
    unobstructed_area_service_unsubscribe();
  }
}

#endif // PBL_API_EXISTS(unobstructed_area_service_subscribe)
