#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core.h"
#include "thread/threadsync.h"
#include "assets/asset.h"

#include "events.h"

// AssetEventType
// TextureAssetEvent
// TextureAssetEventBuffer

void TextureAssetEvent__event_buffer_push_back(EventBuffer_TextureAssetEvent* buffer, TextureAssetEvent item) {
    const uint32 INITIAL_CAPACITY = 32;
    const uint32 GROWTH_FACTOR = 2;

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
}

void TextureAssetEvent__event_buffer_push_back_batch(EventBuffer_TextureAssetEvent* buffer, TextureAssetEvent* items, usize count) {
    const uint32 INITIAL_CAPACITY = 32;
    const uint32 GROWTH_FACTOR = 2;

    if (buffer->capacity == 0) {
        buffer->capacity = __max(INITIAL_CAPACITY, count);
        buffer->items = malloc(buffer->capacity * sizeof(*items));
    }
    else if (buffer->count + count > buffer->capacity) {
        buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);
        buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*items));
    }

    memcpy(buffer->items + buffer->count, items, count * sizeof(*items));
    buffer->count += count;
}

void TextureAssetEvent__event_buffer_clear(EventBuffer_TextureAssetEvent* buffer) {
    buffer->count = 0;
}

// TextureAssetEvents

Events_TextureAssetEvent TextureAssetEvent__events_create() {
    Events_TextureAssetEvent events = {0};
    events.rw_lock = alloc_create_srw_lock();
    return events;
}

void TextureAssetEvent__events_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event) {
    srw_lock_acquire_exclusive(events->rw_lock);
        TextureAssetEvent__event_buffer_push_back(&events->event_buffers[events->this_frame_ind], event);
    srw_lock_release_exclusive(events->rw_lock);
}

void TextureAssetEvent__events_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, usize count) {
    srw_lock_acquire_exclusive(events->rw_lock);
        TextureAssetEvent__event_buffer_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);
    srw_lock_release_exclusive(events->rw_lock);
}

void TextureAssetEvent__events_swap_buffers(Events_TextureAssetEvent* events) {
    srw_lock_acquire_exclusive(events->rw_lock);
        events->this_frame_ind = !events->this_frame_ind;
        TextureAssetEvent__event_buffer_clear(&events->event_buffers[events->this_frame_ind]);
    srw_lock_release_exclusive(events->rw_lock);
}

// TextureAssetEventsIter

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter(Events_TextureAssetEvent* events, usize offset) {
    srw_lock_acquire_shared(events->rw_lock);
    return (EventsIter_TextureAssetEvent) {
        .index = offset,
        .events = events,
    };
}

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter_all(Events_TextureAssetEvent* events) {
    return TextureAssetEvent__events_begin_iter(events, 0);
}

/**
 * 
 * @return offset for next frame
 */
usize TextureAssetEvent__events_iter_end(EventsIter_TextureAssetEvent* iter) {
    usize count = iter->events->event_buffers[iter->events->this_frame_ind].count;
    srw_lock_release_shared(iter->events->rw_lock);
    return count;
}

bool TextureAssetEvent__events_iter_has_next(EventsIter_TextureAssetEvent* iter) {
        return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;
}

TextureAssetEvent TextureAssetEvent__events_iter_read_next(EventsIter_TextureAssetEvent* iter) {
    EventBuffer_TextureAssetEvent events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];
    EventBuffer_TextureAssetEvent events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];

    usize index = iter->index;
    iter->index++;

    if (index < events_this_frame.count) {
        return events_this_frame.items[index];
    }
    return events_last_frame.items[index - events_this_frame.count];
}

TextureAssetEvent TextureAssetEvent__events_iter_consume_next(EventsIter_TextureAssetEvent* iter) {
    EventBuffer_TextureAssetEvent events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];
    EventBuffer_TextureAssetEvent events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];

    usize index = iter->index;
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

TextureAssetEvent TextureAssetEvent__events_iter_maybe_consume_next(EventsIter_TextureAssetEvent* iter, bool** set_consumed) {
    EventBuffer_TextureAssetEvent events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];
    EventBuffer_TextureAssetEvent events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];

    usize index = iter->index;
    iter->index++;

    TextureAssetEvent* event;
    if (index < events_this_frame.count) {
        event = &events_this_frame.items[index];
    }
    else {
        event = &events_last_frame.items[index - events_this_frame.count];
    }

    *set_consumed = &event->consumed;
    return *event;
}


// TEST SYSTEM
#if 0

static uint32 LOCAL_offset = 0;
void test_read_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    EventsIter_TextureAssetEvent events_iter = TextureAssetEvent__events_iter_begin_iter(RES_texture_asset_events, LOCAL_offset);
    while (TextureAssetEvent__events_iter_has_next(&events_iter)) {
        TextureAssetEvent event = TextureAssetEvent__events_iter_read_next(&events_iter);
        printf("Event:\n -- Handle: %d\n -- Type: %s\n -- Consumed: %d"
                        , event.handle.id, event.type, event.consumed);
    }
    LOCAL_offset = TextureAssetEvent__events_iter_end_iter(&events_iter);
}

static uint32 LOCAL_offset_2 = 0;
void test_consume_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    EventsIter_TextureAssetEvent events_iter = TextureAssetEvent__events_iter_begin_iter(RES_texture_asset_events, LOCAL_offset_2);
    while (TextureAssetEvent__events_iter_has_next(&events_iter)) {
        TextureAssetEvent event = TextureAssetEvent__events_iter_consume_next(&events_iter);
        printf("Event:\n -- Handle: %d\n -- Type: %s\n -- Consumed: %d"
                        , event.handle.id, event.type, event.consumed);
    }
    LOCAL_offset_2 = TextureAssetEvent__events_iter_end_iter(&events_iter);
}

void test_write_events_system(Events_TextureAssetEvent* RES_texture_asset_events) {
    #define EVENT_COUNT 5
    TextureAssetEvent events[EVENT_COUNT] = {0};
    TextureAssetEvent__events_send_single(RES_texture_asset_events, events[0]);
    TextureAssetEvent__events_send_batch(RES_texture_asset_events, &events[1], EVENT_COUNT - 1);
    #undef EVENT_COUNT
}

#endif
// ---