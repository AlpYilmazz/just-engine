
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

static uint32 LOCAL_offset = 0;
void handle_image_loaded_events(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
) {
    EventsIter_TextureAssetEvent events_iter = just_engine_events_iter_texture_asset_events_new(RES_texture_asset_events, LOCAL_offset);

    while (just_engine_events_iter_texture_asset_events_has_next(&events_iter)) {
        TextureAssetEvent event = just_engine_events_iter_texture_asset_events_read_next(&events_iter);
        just_engine_texture_assets_load_texture_uncheched(RES_texture_assets, event.handle);
    }

    LOCAL_offset = just_engine_events_texture_asset_event_this_frame_buffer_count(RES_texture_asset_events);
}

int test() {



    return 0;
}