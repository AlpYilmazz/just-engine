#pragma once

#include "core.h"
#include "thread/threadsync.h"
#include "assets/asset.h"

typedef enum {
    AssetEvent_ImageLoaded,
    AssetEvent_ImageChanged,
    AssetEvent_ImageUnloaded,
    AssetEvent_TextureLoaded,
    AssetEvent_TextureChanged,
    AssetEvent_TextureUnloaded,
} TextureAssetEventType;

typedef struct {
    TextureHandle handle;
    TextureAssetEventType type;
    bool consumed;
} TextureAssetEvent;

typedef struct {
    usize count;
    usize capacity;
    TextureAssetEvent* items;
} EventBuffer_TextureAssetEvent;

typedef struct {
    SRWLock* rw_lock;
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

Events_TextureAssetEvent TextureAssetEvent__events_create();
void TextureAssetEvent__events_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void TextureAssetEvent__events_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, usize count);
void TextureAssetEvent__events_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    usize index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter(Events_TextureAssetEvent* events, usize offset);
/**
 * SHOULD ALWAYS BE PAIRED WITH A `events_iter_end` CALL TO RELEASE THE LOCK
 */
EventsIter_TextureAssetEvent TextureAssetEvent__events_begin_iter_all(Events_TextureAssetEvent* events);
/**
 * 
 * @return offset for next frame
 */
usize TextureAssetEvent__events_iter_end(EventsIter_TextureAssetEvent* iter);
bool TextureAssetEvent__events_iter_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_consume_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent TextureAssetEvent__events_iter_maybe_consume_next(EventsIter_TextureAssetEvent* iter, bool** set_consumed);