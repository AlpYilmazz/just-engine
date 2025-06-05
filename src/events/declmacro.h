#pragma once

/**
 * Use matching versions of DECLARE and DEFINE_IMPL macros
 * Both ACCESS_SINGLE_THREADED or ACCESS_MULTI_THREADED
 * 
 * Event types have these constraints:
 * - Should define field "bool consumed"
 */

#define EventBuffer(TYPE_EVENT) EventBuffer_##TYPE_EVENT
#define Events(TYPE_EVENT) Events_##TYPE_EVENT
#define EventsIter(TYPE_EVENT) EventsIter_##TYPE_EVENT

#define events_create(TYPE_EVENT) TYPE_EVENT##__events_create
#define events_send_single(TYPE_EVENT) TYPE_EVENT##__events_send_single
#define events_send_batch(TYPE_EVENT) TYPE_EVENT##__events_send_batch
#define events_swap_buffers(TYPE_EVENT) TYPE_EVENT##__events_swap_buffers

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
#define events_begin_iter(TYPE_EVENT) TYPE_EVENT##__events_begin_iter
/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
#define events_begin_iter_all(TYPE_EVENT) TYPE_EVENT##__events_begin_iter_all
/**
 * 
 * @return offset for next frame
 * 
 */
#define events_iter_end(TYPE_EVENT) TYPE_EVENT##__events_iter_end
#define events_iter_has_next(TYPE_EVENT) TYPE_EVENT##__events_iter_has_next
#define events_iter_read_next(TYPE_EVENT) TYPE_EVENT##__events_iter_read_next
#define events_iter_consume_next(TYPE_EVENT) TYPE_EVENT##__events_iter_consume_next

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__EVENT_SYSTEM__ACCESS_SINGLE_THREADED(TYPE_EVENT) \
\
    typedef struct {\
        usize count;\
        usize capacity;\
        TYPE_EVENT* items;\
    } EventBuffer_##TYPE_EVENT;\
\
    typedef struct {\
        EventBuffer_##TYPE_EVENT event_buffers[2];\
        uint8 this_frame_ind;\
    } Events_##TYPE_EVENT;\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create();\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event);\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count);\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events);\
\
    typedef struct {\
        usize index;\
        Events_##TYPE_EVENT* events;\
    } EventsIter_##TYPE_EVENT;\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset);\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events);\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter);\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter);\

// -------------------------------------------------------------------------------------------------------------------

#define __DECLARE__EVENT_SYSTEM__ACCESS_MULTI_THREADED(TYPE_EVENT) \
\
    typedef struct {\
        usize count;\
        usize capacity;\
        TYPE_EVENT* items;\
    } EventBuffer_##TYPE_EVENT;\
\
    typedef struct {\
        SRWLock* rw_lock;\
        EventBuffer_##TYPE_EVENT event_buffers[2];\
        uint8 this_frame_ind;\
    } Events_##TYPE_EVENT;\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create();\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event);\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count);\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events);\
\
    typedef struct {\
        usize index;\
        Events_##TYPE_EVENT* events;\
    } EventsIter_##TYPE_EVENT;\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset);\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events);\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter);\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter);\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter);\

// -------------------------------------------------------------------------------------------------------------------

#define _IMPL_____EVENT_SYSTEM__ACCESS_SINGLE_THREADED(TYPE_EVENT) \
\
    void TYPE_EVENT##__event_buffer_push_back(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT item) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = INITIAL_CAPACITY;\
            buffer->items = malloc(buffer->capacity * sizeof(*&item));\
        }\
        else if (buffer->count == buffer->capacity) {\
            buffer->capacity = GROWTH_FACTOR * buffer->capacity;\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*&item));\
        }\
\
        buffer->items[buffer->count] = item;\
        buffer->count++;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_push_back_batch(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT* items, usize count) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = __max(INITIAL_CAPACITY, count);\
            buffer->items = malloc(buffer->capacity * sizeof(*items));\
        }\
        else if (buffer->count + count > buffer->capacity) {\
            buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*items));\
        }\
\
        memcpy(buffer->items + buffer->count, items, count * sizeof(*items));\
        buffer->count += count;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_clear(EventBuffer_##TYPE_EVENT* buffer) {\
        buffer->count = 0;\
    }\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create() {\
        Events_##TYPE_EVENT events = {0};\
        return events;\
    }\
\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event) {\
        TYPE_EVENT##__event_buffer_push_back(&events->event_buffers[events->this_frame_ind], event);\
    }\
\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count) {\
        TYPE_EVENT##__event_buffer_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);\
    }\
\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events) {\
        events->this_frame_ind = !events->this_frame_ind;\
        TYPE_EVENT##__event_buffer_clear(&events->event_buffers[events->this_frame_ind]);\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset) {\
        return (EventsIter_##TYPE_EVENT) {\
            .index = offset,\
            .events = events,\
        };\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events) {\
        return TYPE_EVENT##__events_begin_iter(events, 0);\
    }\
\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter) {\
        usize count = iter->events->event_buffers[iter->events->this_frame_ind].count;\
        return count;\
    }\
\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter) {\
            return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        if (index < events_this_frame.count) {\
            return events_this_frame.items[index];\
        }\
        return events_last_frame.items[index - events_this_frame.count];\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        event->consumed = true;\
        return *event;\
    }\

// -------------------------------------------------------------------------------------------------------------------

#define _IMPL_____EVENT_SYSTEM__ACCESS_MULTI_THREADED(TYPE_EVENT) \
\
    void TYPE_EVENT##__event_buffer_push_back(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT item) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = INITIAL_CAPACITY;\
            buffer->items = malloc(buffer->capacity * sizeof(*&item));\
        }\
        else if (buffer->count == buffer->capacity) {\
            buffer->capacity = GROWTH_FACTOR * buffer->capacity;\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*&item));\
        }\
\
        buffer->items[buffer->count] = item;\
        buffer->count++;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_push_back_batch(EventBuffer_##TYPE_EVENT* buffer, TYPE_EVENT* items, usize count) {\
        const uint32 INITIAL_CAPACITY = 32;\
        const uint32 GROWTH_FACTOR = 2;\
\
        if (buffer->capacity == 0) {\
            buffer->capacity = __max(INITIAL_CAPACITY, count);\
            buffer->items = malloc(buffer->capacity * sizeof(*items));\
        }\
        else if (buffer->count + count > buffer->capacity) {\
            buffer->capacity = __max(GROWTH_FACTOR * buffer->capacity, buffer->count + count);\
            buffer->items = realloc(buffer->items, buffer->capacity * sizeof(*items));\
        }\
\
        memcpy(buffer->items + buffer->count, items, count * sizeof(*items));\
        buffer->count += count;\
\
    }\
\
    void TYPE_EVENT##__event_buffer_clear(EventBuffer_##TYPE_EVENT* buffer) {\
        buffer->count = 0;\
    }\
\
    Events_##TYPE_EVENT TYPE_EVENT##__events_create() {\
        Events_##TYPE_EVENT events = {0};\
        events.rw_lock = alloc_create_srw_lock();\
        return events;\
    }\
\
    void TYPE_EVENT##__events_send_single(Events_##TYPE_EVENT* events, TYPE_EVENT event) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            TYPE_EVENT##__event_buffer_push_back(&events->event_buffers[events->this_frame_ind], event);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    void TYPE_EVENT##__events_send_batch(Events_##TYPE_EVENT* events, TYPE_EVENT* event_list, usize count) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            TYPE_EVENT##__event_buffer_push_back_batch(&events->event_buffers[events->this_frame_ind], event_list, count);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    void TYPE_EVENT##__events_swap_buffers(Events_##TYPE_EVENT* events) {\
        srw_lock_acquire_exclusive(events->rw_lock);\
            events->this_frame_ind = !events->this_frame_ind;\
            TYPE_EVENT##__event_buffer_clear(&events->event_buffers[events->this_frame_ind]);\
        srw_lock_release_exclusive(events->rw_lock);\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter(Events_##TYPE_EVENT* events, usize offset) {\
        srw_lock_acquire_shared(events->rw_lock);\
        return (EventsIter_##TYPE_EVENT) {\
            .index = offset,\
            .events = events,\
        };\
    }\
\
    EventsIter_##TYPE_EVENT TYPE_EVENT##__events_begin_iter_all(Events_##TYPE_EVENT* events) {\
        return TYPE_EVENT##__events_begin_iter(events, 0);\
    }\
\
    usize TYPE_EVENT##__events_iter_end(EventsIter_##TYPE_EVENT* iter) {\
        usize count = iter->events->event_buffers[iter->events->this_frame_ind].count;\
        srw_lock_release_shared(iter->events->rw_lock);\
        return count;\
    }\
\
    bool TYPE_EVENT##__events_iter_has_next(EventsIter_##TYPE_EVENT* iter) {\
            return iter->index < iter->events->event_buffers[0].count + iter->events->event_buffers[1].count;\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_read_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        if (index < events_this_frame.count) {\
            return events_this_frame.items[index];\
        }\
        return events_last_frame.items[index - events_this_frame.count];\
    }\
\
    TYPE_EVENT TYPE_EVENT##__events_iter_consume_next(EventsIter_##TYPE_EVENT* iter) {\
        EventBuffer_##TYPE_EVENT events_this_frame = iter->events->event_buffers[iter->events->this_frame_ind];\
        EventBuffer_##TYPE_EVENT events_last_frame = iter->events->event_buffers[!iter->events->this_frame_ind];\
\
        usize index = iter->index;\
        iter->index++;\
\
        TYPE_EVENT* event;\
        if (index < events_this_frame.count) {\
            event = &events_this_frame.items[index];\
        }\
        else {\
            event = &events_last_frame.items[index - events_this_frame.count];\
        }\
\
        event->consumed = true;\
        return *event;\
    }\

