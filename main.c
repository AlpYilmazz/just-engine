
// typedef     unsigned char           uint8;
// typedef     unsigned short          uint16;
// typedef     unsigned int            uint32;
// typedef     unsigned long long      uint64;

// typedef     char                    int8;
// typedef     short                   int16;
// typedef     int                     int32;
// typedef     long long               int64;

// typedef     float                   float32;
// typedef     double                  float64;

// typedef     uint8                   bool;


// #define DECL_EVENT_SYSTEM_BASE(TEvent, t_event)                                                                          \
//     typedef struct {                                                                                                \
//         uint32 count;                                                                                               \
//         uint32 capacity;                                                                                            \
//         TEvent* items;                                                                                              \
//     } EventBuffer_##TEvent;                                                                                       \
//                                                                                                                     \
//     void event_buffer_##t_event##_push_back(EventBuffer_##TEvent* buffer, TEvent item);                           \
//     void event_buffer_##t_event##_push_back_batch(EventBuffer_##TEvent* buffer, TEvent* items, uint32 count);     \
//     void event_buffer_##t_event##_clear(EventBuffer_##TEvent* buffer);                                            \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         EventBuffer_##TEvent event_buffers[2];                                                                    \
//         uint8 this_frame_ind;                                                                                       \
//     } Events_##TEvent;                                                                                            \
//                                                                                                                     \
//     uint32 just_engine_events_##t_event##_this_frame_buffer_count(Events_##TEvent* events);                           \
//     void just_engine_events_##t_event##_send_single(Events_##TEvent* events, TEvent event);                           \
//     void just_engine_events_##t_event##_send_batch(Events_##TEvent* events, TEvent* event_list, uint32 count);        \
//     void just_engine_events_##t_event##_swap_buffers(Events_##TEvent* events);                                        \
//                                                                                                                     \
//     typedef struct {                                                                                                \
//         uint32 index;                                                                                               \
//         Events_##TEvent* events;                                                                                  \
//     } EventsIter_##TEvent;                                                                                        \
//                                                                                                                     \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new(Events_##TEvent* events, uint32 offset);        \
//     EventsIter_##TEvent just_engine_events_iter_##t_event##_new_zero(Events_##TEvent* events);                  \
//     bool just_engine_events_iter_##t_event##_has_next(EventsIter_##TEvent* iter);                                 \
//     TEvent just_engine_events_iter_##t_event##_read_next(EventsIter_##TEvent* iter);                              \

// #define DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TEvent, t_event)                                                       \
//     TEvent just_engine_events_iter_##t_event##_consume_next(EventsIter_##TEvent* iter);                           


// typedef struct {
//     uint32 id;
//     bool consumed;
// } TestEvent;

// DECL_EVENT_SYSTEM_BASE(TestEvent, test_event)
// DECL_EVENT_SYSTEM_EXT_CONSUMABLE(TestEvent, test_event)

#include <stdio.h>

#include "justengine.h"

const unsigned char __DEFAULT_IMAGE_DATA[4] = {255, 0, 255, 255};

int main() {

    Events_TextureAssetEvent events = {0};
    printf("Count: %d\n", just_engine_events_texture_asset_event_this_frame_buffer_count(&events));

    TextureAssets texture_assets = {0};

    TextureHandle handle = just_engine_texture_assets_reserve_texture_slot(&texture_assets);
    Image image = {
        .data = (void*)&__DEFAULT_IMAGE_DATA,
        .width = 1,
        .height = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    just_engine_texture_assets_put_image(&texture_assets, handle, image);

    Image* image_get = just_engine_texture_assets_get_image_unchecked(&texture_assets, handle);
    printf("Image: %d %d\n", image_get->width, image_get->height);

    just_engine_texture_assets_unload_image(&texture_assets, handle);

    return 0;
}