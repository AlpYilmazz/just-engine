#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base.h"
#include "assets/asset.h"

#include "events.h"

// AssetEventType
// TextureAssetEvent
// TextureAssetEventBuffer

void event_buffer_texture_asset_event_push_back(EventBuffer_TextureAssetEvent* buffer, TextureAssetEvent item) {
    #define INITIAL_CAPACITY 32
    #define GROWTH_FACTOR 2

    if (buffer->capacity == 0) {
        buffer->capacity = INITIAL_CAPACITY;
        buffer->items = malloc(buffer->capacity * sizeof(*&item));
    }
    else if (buffer->count == buffer->capacity) {
        buffer->capacity = GROWTH_FACTOR * buffer->capacity;
        buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*&item));
    }

    buffer->items[buffer->count] = item;
    buffer->count++;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR
}

void event_buffer_texture_asset_event_push_back_batch(EventBuffer_TextureAssetEvent* buffer, TextureAssetEvent* items, uint32 count) {
    #define INITIAL_CAPACITY 32
    #define GROWTH_FACTOR 2

    if (buffer->capacity == 0) {
        buffer->capacity = __max(INITIAL_CAPACITY, count);
        buffer->items = malloc(buffer->capacity * sizeof(TextureAssetEvent));
    }
    else if (buffer->count + count > buffer->capacity) {
        buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);
        buffer->items = realloc(buffer->items, buffer->capacity * sizeof(TextureAssetEvent));
    }

    memcpy(buffer->items + buffer->count, items, count * sizeof(TextureAssetEvent));
    buffer->count += count;

    #undef INITIAL_CAPACITY
    #undef GROWTH_FACTOR
}

void event_buffer_texture_asset_event_clear(EventBuffer_TextureAssetEvent* buffer) {
    buffer->count = 0;
}

// TextureAssetEvents

uint32 just_engine_events_texture_asset_event_this_frame_buffer_count(Events_TextureAssetEvent* events) {
    return events->event_buffers[events->this_frame_ind].count;
}

void just_engine_events_texture_asset_event_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event) {
    event_buffer_texture_asset_event_push_back(&events->event_buffers[events->this_frame_ind], event);
}

void just_engine_events_texture_asset_event_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, uint32 count) {
    event_buffer_texture_asset_event_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);
}

void just_engine_events_texture_asset_event_swap_buffers(Events_TextureAssetEvent* events) {
    events->this_frame_ind = !events->this_frame_ind;
    event_buffer_texture_asset_event_clear(&events->event_buffers[events->this_frame_ind]);
}

// TextureAssetEventsIter

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new(Events_TextureAssetEvent* events, uint32 offset) {
    return (EventsIter_TextureAssetEvent) {
        .index = offset,
        .events = events,
    };
}

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new_from_start(Events_TextureAssetEvent* events) {
    return just_engine_events_iter_texture_asset_events_new(events, 0);
}

bool just_engine_events_iter_texture_asset_events_has_next(EventsIter_TextureAssetEvent* iter) {
    return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;
}

TextureAssetEvent just_engine_events_iter_texture_asset_events_read_next(EventsIter_TextureAssetEvent* iter) {
    EventBuffer_TextureAssetEvent events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];
    EventBuffer_TextureAssetEvent events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];

    uint32 index = iter->index;
    iter->index++;

    if (index < events_this_frame.count) {
        return events_this_frame.items[index];
    }
    return events_last_frame.items[index - events_this_frame.count];
}

TextureAssetEvent just_engine_events_iter_texture_asset_events_consume_next(EventsIter_TextureAssetEvent* iter) {
    EventBuffer_TextureAssetEvent events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];
    EventBuffer_TextureAssetEvent events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];

    uint32 index = iter->index;
    iter->index++;

    TextureAssetEvent* event;
    if (index < events_this_frame.count) {
        event = &events_this_frame.items[index];
    }
    else {
        event = &events_last_frame.items[index - events_this_frame.count];
    }

    event->consumed = true;
    return *event;
}


// TEST SYSTEM
#if 0

static uint32 LOCAL_offset = 0;
void test_read_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_new(RES_texture_asset_events, LOCAL_offset);
    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_read_next(&events_iter);
        printf("Event:\n -- Handle: %d\n -- Type: %s\n -- Consumed: %d"
                        , event.handle.id, event.type, event.consumed);
    }
    LOCAL_offset = just_engine_texture_asset_events_this_frame_buffer_count(RES_texture_asset_events);
}

static uint32 LOCAL_offset_2 = 0;
void test_consume_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_new(RES_texture_asset_events, LOCAL_offset_2);
    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_consume_next(&events_iter);
        printf("Event:\n -- Handle: %d\n -- Type: %s\n -- Consumed: %d"
                        , event.handle.id, event.type, event.consumed);
    }
    LOCAL_offset_2 = just_engine_texture_asset_events_this_frame_buffer_count(RES_texture_asset_events);
}

void test_write_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    #define EVENT_COUNT 5
    TextureAssetEvent events[EVENT_COUNT] = {0};
    just_engine_texture_asset_events_send_single(RES_texture_asset_events, events[0]);
    just_engine_texture_asset_events_send_batch(RES_texture_asset_events, &events[1], EVENT_COUNT - 1);
    #undef EVENT_COUNT
}

#endif
// ---