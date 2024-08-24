#pragma once

#include "base.h"
#include "assets/asset.h"

typedef enum {
    AssetEvent_Loaded,
    AssetEvent_Changed,
    AssetEvent_Unloaded,
} AssetEventType;

typedef struct {
    TextureHandle handle;
    AssetEventType type;
    bool consumed;
} TextureAssetEvent;

typedef struct {
    uint32 count;
    uint32 capacity;
    TextureAssetEvent* items;
} EventBuffer_TextureAssetEvent;

typedef struct {
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

uint32 just_engine_events_texture_asset_event_this_frame_buffer_count(Events_TextureAssetEvent* events);
void just_engine_events_texture_asset_event_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void just_engine_events_texture_asset_event_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, uint32 count);
void just_engine_events_texture_asset_event_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    uint32 index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new(Events_TextureAssetEvent* events, uint32 offset);
EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_new_from_start(Events_TextureAssetEvent* events);
bool just_engine_events_iter_texture_asset_events_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_consume_next(EventsIter_TextureAssetEvent* iter);

