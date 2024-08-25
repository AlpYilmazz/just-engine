
#include "raylib.h"

#include "base.h"
#include "assets/asset.h"
#include "events/events.h"


// TODO:
//  - Game Base Thread
//      - runs game logic
//  - Render Base Thread (Main thread)
//      - runs render logic
//      - gpu bus io
//      - draw calls

static uint32 LOCAL_image_loaded_events_offset = 0;

void system_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_new(RES_texture_asset_events, LOCAL_image_loaded_events_offset);

    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_read_next(&events_iter);
        switch (event.type) {
        case AssetEvent_Changed:
            just_engine_texture_assets_unload_texture(RES_texture_assets, event.handle);
            // fallthrough
        case AssetEvent_Loaded:
            just_engine_texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
        }
    }

    LOCAL_image_loaded_events_offset = just_engine_events_texture_asset_event_this_frame_buffer_count(RES_texture_asset_events);
}

int test() {



    return 0;
}