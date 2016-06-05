#pragma once

#include <pebble.h>

typedef void* EventHandle;

// app focus service

typedef void(*EventFocusHandler)(bool in_focus, void *context);
typedef struct EventFocusHandlers {
  EventFocusHandler will_focus;
  EventFocusHandler did_focus;
} EventFocusHandlers;

EventHandle events_app_focus_service_subscribe_handlers(AppFocusHandlers handlers);
EventHandle events_app_focus_service_subscribe_handlers_context(EventFocusHandlers handlers, void *context);
EventHandle events_app_focus_service_subscribe(AppFocusHandler handler);
EventHandle events_app_focus_service_subscribe_context(EventFocusHandler handler, void *context);
void events_app_focus_service_unsubscribe(EventHandle handle);

// battery state service

typedef void(*EventBatteryStateHandler)(BatteryChargeState state, void *context);
EventHandle events_battery_state_service_subscribe(BatteryStateHandler handler);
EventHandle events_battery_state_service_subscribe_context(EventBatteryStateHandler handler, void *context);
void events_battery_state_service_unsubscribe(EventHandle handle);

// Connection service

typedef void(*EventConnectionHandler)(bool connected, void *context);
typedef struct EventConnectionHandlers {
	EventConnectionHandler pebble_app_connection_handler;
	EventConnectionHandler pebblekit_connection_handler;
} EventConnectionHandlers;

EventHandle events_connection_service_subscribe(ConnectionHandlers conn_handlers);
EventHandle events_connection_service_subscribe_context(EventConnectionHandlers conn_handlers, void *context);
void events_connection_service_unsubscribe(EventHandle handle);

// Health service

EventHandle events_health_service_events_subscribe(HealthEventHandler handler, void *context);
void events_health_service_events_unsubscribe(EventHandle handle);

// Tick timer service

typedef void(*EventTickHandler)(struct tm *tick_time, TimeUnits units_changed, void *context);
EventHandle events_tick_timer_service_subscribe(TimeUnits units, TickHandler handler);
EventHandle events_tick_timer_service_subscribe_context(TimeUnits units, EventTickHandler handler, void *context);
void events_tick_timer_service_unsubscribe(EventHandle handle);
