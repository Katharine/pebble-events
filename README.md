pebble-events
=============

pebble-events fixes the Pebble SDK's event system, supporting multiple
independent subscriptions to a single event, and providing a context parameter
to those that didn't already have one.

Everyone should use it!

**Note**: This library depends on Pebble SDK 3.14 or greater for the `PBL_API_EXISTS`
macro.

Installation
------------

```
pebble package install pebble-events
```

You must be using a new-style project; install the latest pebble tool and SDK
and run `pebble convert-project` on your app if you aren't.

Usage
-----

Usage mirrors the Pebble SDK, but with `events_` prefixing all the function
names. Subscription functions have the same signature as the SDK functions of
the same name, except that they return an `EventHandle` that can be passed to
the corresponding unsubscribe function.

To support usage where some data needs to be passed to the callback (for
instance, a layer component that needs to subscribe to the tick service might
need to pass a pointer to itself to that callback), you can use the `_context`
variants. These have a signature similar to the non-context versions, but with
a final `void *context` argument at the end.

### App Focus Service

Wrapping the SDK [AppFocusService](https://developer.pebble.com/docs/c/Foundation/Event_Service/AppFocusService/),
with the same behaviour.

```c
EventHandle events_app_focus_service_subscribe_handlers(AppFocusHandlers handlers);
EventHandle events_app_focus_service_subscribe_handlers_context(EventFocusHandlers handlers, void *context);
EventHandle events_app_focus_service_subscribe(AppFocusHandler handler);
EventHandle events_app_focus_service_subscribe_context(EventFocusHandler handler, void *context);
void events_app_focus_service_unsubscribe(EventHandle handle);
```

### Battery State Service

Wrapping the SDK [BatteryStateService](https://developer.pebble.com/docs/c/Foundation/Event_Service/BatteryStateService/),
with the same behaviour.

```c
EventHandle events_battery_state_service_subscribe(BatteryStateHandler handler);
EventHandle events_battery_state_service_subscribe_context(EventBatteryStateHandler handler, void *context);
void events_battery_state_service_unsubscribe(EventHandle handle);
```

### Connection Service

Wrapping the SDK [ConnectionService](https://developer.pebble.com/docs/c/Foundation/Event_Service/ConnectionService/),
with the same behaviour. Note that the deprecated BluetoothConnectionService is not implemented.

```c
EventHandle events_connection_service_subscribe(ConnectionHandlers conn_handlers);
EventHandle events_connection_service_subscribe_context(EventConnectionHandlers conn_handlers, void *context);
void events_connection_service_unsubscribe(EventHandle handle);
```

### Health Service

Wrapping the SDK [HealthService](https://developer.pebble.com/docs/c/Foundation/Event_Service/HealthService/),
with the same behaviour. Note that no `_context` variant is provided because the
native version already has a `context` parameter.

```c
EventHandle events_health_service_events_subscribe(HealthEventHandler handler, void *context);
bool events_health_service_events_unsubscribe(EventHandle handle);
```

### Tick Timer Service

Wrapping the SDK [TickTimerService](https://developer.pebble.com/docs/c/Foundation/Event_Service/TickTimerService/),
with similar behaviour. Unlike the `TickTimerService`, however, we will not
necessarily call your handler immediately after subscription. We also may
spuriously call your handler as a result of internal subscription management.
However, in most cases we will call your handlers only on the requested time
units. We will subscribe to the lowest-resolution time unit that permits this.

```c
EventHandle events_tick_timer_service_subscribe(TimeUnits units, TickHandler handler);
EventHandle events_tick_timer_service_subscribe_context(TimeUnits units, EventTickHandler handler, void *context);
void events_tick_timer_service_unsubscribe(EventHandle handle);
```

### Appmessage Service

The appmessage service is more complex than other services because once we open
appmessage, we cannot change the inbox size. Therefore, to use the appmessage
service, you must call `events_app_message_request_inbox_size` with your
desired inbox size and `events_app_message_request_outbox_size` with your
desired outbox size (if you don't send messages in one direction you don't need)
to call it in that direction. The consumer app, and _only_ the app, should call
`events_app_message_open` once all libraries have been initialised. After this
point, the two `request` functions will no longer work. If you are building a
library, this may require you to add a `library_prepare()` method or similar to
be called at app initialisation.

To subscribe to more than one appmessage callback at once, you can use
`events_app_message_subscribe_handlers`. Otherwise, analogues to the SDK's
standard appmessage functions are also available.

Keep in mind that you will receive all appmessages, even if you did not expect
them. Check for a key that your code sends to ensure it is intended for you.
If you could have multiple instances that should not all process a given
message, include some means of determining which instance should handle it (e.g. if
it's a request/response, a pointer cast to a uint32 could work).

```c
typedef struct EventAppMessageHandlers {
	AppMessageOutboxSent sent;
	AppMessageOutboxFailed failed;
	AppMessageInboxReceived received;
	AppMessageInboxDropped dropped;
} EventAppMessageHandlers;
void events_app_message_request_inbox_size(uint32_t size);
void events_app_message_request_outbox_size(uint32_t size);
AppMessageResult events_app_message_open(void);
EventHandle events_app_message_subscribe_handlers(EventAppMessageHandlers handlers, void *context);
void events_app_message_unsubscribe(EventHandle handle);

// For consistency with the SDK.
EventHandle events_app_message_register_outbox_sent(AppMessageOutboxSent sent_callback, void *context);
EventHandle events_app_message_register_outbox_failed(AppMessageOutboxFailed failed_callback, void *context);
EventHandle events_app_message_register_inbox_received(AppMessageInboxReceived received_callback, void *context);
EventHandle events_app_message_register_inbox_dropped(AppMessageInboxDropped dropped_callback, void *context);
```

### Accelerometer Tap Service

Wrapping the _Tap_ part of the SDK [AccelerometerService](https://developer.pebble.com/docs/c/Foundation/Event_Service/AccelerometerService/),
with the same behaviour.

```c
EventHandle events_accel_tap_service_subscribe(AccelTapHandler handler);
EventHandle events_accel_tap_service_subscribe_context(EventAccelTapHandler handler, void *context);
void events_accel_tap_service_unsubscribe(EventHandle handle);
```

### Unobstructed Area Service

Wrapping the SDK [UnobstructedAreaService](https://developer.pebble.com/docs/c/User_Interface/UnobstructedArea/),
with the same behaviour. Note that no `_context` variant is provided because the
native version already has a `context` parameter.

```c
EventHandle events_unobstructed_area_service_subscribe(UnobstructedAreaHandlers handlers, void *context);
void events_unobstructed_area_service_unsubscribe(EventHandle handle);
```
