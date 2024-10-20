#pragma once

#include "stdlib.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define __HEADER_LOGGING
#ifdef __HEADER_LOGGING

typedef enum {
    LOG_LEVEL_ALL = 0,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE,
} LogLevel;

void SET_LOG_LEVEL(LogLevel log_level);

void JUST_LOG_TRACE(const char* format, ...);
void JUST_LOG_DEBUG(const char* format, ...);
void JUST_LOG_INFO(const char* format, ...);
void JUST_LOG_WARN(const char* format, ...);
void JUST_LOG_ERROR(const char* format, ...);
void JUST_LOG_PANIC(const char* format, ...);

#endif // __HEAEDER_LOGGING

#define __HEADER_BASE
#ifdef __HEADER_BASE

typedef     unsigned char           uint8;
typedef     unsigned short          uint16;
typedef     unsigned int            uint32;
typedef     unsigned long long      uint64;

typedef     char                    int8;
typedef     short                   int16;
typedef     int                     int32;
typedef     long long               int64;

typedef     float                   float32;
typedef     double                  float64;

typedef     uint64                  usize;
typedef     unsigned char           byte;
// typedef     uint8                   bool;

#define Option(Type) DeclType_Option_##Type
#define Option_None {0}
#define Option_Some(val) { .is_some = true, .value = val, }
#define DECLARE__Option(Type) typedef struct { bool is_some; Type value; } Option(Type);

DECLARE__Option(uint8);
DECLARE__Option(uint16);
DECLARE__Option(uint32);
DECLARE__Option(uint64);
DECLARE__Option(int8);
DECLARE__Option(int16);
DECLARE__Option(int32);
DECLARE__Option(int64);
DECLARE__Option(float32);
DECLARE__Option(float64);
DECLARE__Option(usize);
DECLARE__Option(byte);
DECLARE__Option(char);

#define PANIC(...) { JUST_LOG_PANIC(__VA_ARGS__); exit(EXIT_FAILURE); }

#define STRUCT_ZERO_INIT {0}
#define LAZY_INIT {0}
#define LATER_INIT {0}
#define UNINIT {0}

#define MAX(a, b) ((a >= b) ? a : b)
#define MIN(a, b) ((a <= b) ? a : b)

#define SIGNOF(a) ( (x == 0) ? 0 : ( (x > 0) ? 1 : -1 ) )

static const uint32 ALL_SET_32 = 0b11111111111111111111111111111111;

#define RECTANGLE_NICHE ( \
    (Rectangle) {   \
        .x = *(float32*) &ALL_SET_32, \
        .y = *(float32*) &ALL_SET_32, \
        .width = *(float32*) &ALL_SET_32, \
        .height = *(float32*) &ALL_SET_32, \
    }   \
)

#define BYTEWISE_EQUALS(e1, e2, type)   \
    ( bytewise_equals((byte*)e1, (byte*)e2, sizeof(type)) )

static inline bool bytewise_equals(byte* e1, byte* e2, uint32 count) {
    bool is_eq = 1;
    for (uint32 i = 0; i < count; i++) {
        is_eq &= (e1[i] == e2[2]);
    }
    return is_eq;
}

typedef enum {
    Anchor_Top_Left = 0,    // DEFAULT
    Anchor_Top_Right,
    Anchor_Bottom_Left,
    Anchor_Bottom_Right,

    Anchor_Top_Mid,
    Anchor_Bottom_Mid,
    Anchor_Left_Mid,
    Anchor_Right_Mid,

    Anchor_Center,
} AnchorType;

typedef struct {
    Vector2 origin;         // uv coordinate: [0.0, 1.0]
} Anchor;                   // {0} -> origin = {0, 0} -> Anchor_Top_Left

static inline Anchor make_anchor(AnchorType type) {
    switch (type) {
    default:
    case Anchor_Top_Left:
        return (Anchor) { .origin = {0, 0} };
    case Anchor_Top_Right:
        return (Anchor) { .origin = {1, 0} };
    case Anchor_Bottom_Left:
        return (Anchor) { .origin = {0, 1} };
    case Anchor_Bottom_Right:
        return (Anchor) { .origin = {1, 1} };

    case Anchor_Top_Mid:
        return (Anchor) { .origin = {0.5, 0} };
    case Anchor_Bottom_Mid:
        return (Anchor) { .origin = {0.5, 1} };
    case Anchor_Left_Mid:
        return (Anchor) { .origin = {0, 0.5} };
    case Anchor_Right_Mid:
        return (Anchor) { .origin = {1, 0.5} };

    case Anchor_Center:
        return (Anchor) { .origin = {0.5, 0.5} };
    }
    PANIC("Incorrect AnchorType!\n");
}

static inline Anchor make_custom_anchor(Vector2 origin) {
    return (Anchor) { .origin = origin };
}

typedef struct {
    union {
        struct {
            float32 width;
            float32 height;
        };
        Vector2 as_vec;
    };
} RectSize;

typedef struct {
    uint32 width;
    uint32 height;
} URectSize;

static inline Rectangle into_rectangle(Vector2 position, RectSize size) {
    return (Rectangle) {
        .x = position.x,
        .y = position.y,
        .width = size.width,
        .height = size.height,
    };
}

static inline Vector2 find_rectangle_top_left(Anchor anchor, Vector2 position, RectSize size) {
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_top_left_rect(Anchor anchor, Rectangle rect) {
    Vector2 position = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Subtract(
        position,
        Vector2Multiply(anchor.origin, size)
    );
}

static inline Vector2 find_rectangle_position(Anchor anchor, Vector2 top_left, RectSize size) {
    return Vector2Add(
        top_left,
        Vector2Multiply(anchor.origin, size.as_vec)
    );
}

static inline Vector2 find_rectangle_position_rect(Anchor anchor, Rectangle rect) {
    Vector2 top_left = {rect.x, rect.y};
    Vector2 size = {rect.width, rect.height};
    return Vector2Add(
        top_left,
        Vector2Multiply(anchor.origin, size)
    );
}

typedef uint32 BitMask32;

static inline BitMask32 set_bit(BitMask32 mask, uint32 bit) {
    return mask | (1 << bit);
}

static inline BitMask32 unset_bit(BitMask32 mask, uint32 bit) {
    return mask & ((1 << bit) ^ 0xFFFFFFFF);
}

static inline bool check_bit(BitMask32 mask, uint32 bit) {
    return mask & (1 << bit);
}

static inline bool check_overlap(BitMask32 mask1, BitMask32 mask2) {
    return mask1 & mask2;
}

#define PRIMARY_LAYER 1

typedef struct {
    BitMask32 mask;
} Layers;

static inline void set_layer(Layers* layers, uint32 layer) {
    layers->mask = set_bit(layers->mask, layer);
}

static inline void unset_layer(Layers* layers, uint32 layer) {
    layers->mask = unset_bit(layers->mask, layer);
}

static inline bool check_layer(Layers layers, uint32 layer) {
    return check_bit(layers.mask, layer);
}

static inline bool check_layer_overlap(Layers ls1, Layers ls2) {
    return check_overlap(ls1.mask, ls2.mask);
}

static inline Layers on_single_layer(uint32 layer) {
    Layers layers = {0};
    set_layer(&layers, layer);
    return layers;
}

static inline Layers on_primary_layer() {
    return on_single_layer(PRIMARY_LAYER);
}

typedef struct {
    uint32 id;
    uint32 generation;
} ComponentId;

static inline ComponentId new_component_id(uint32 id, uint32 generation) {
    return (ComponentId) { id, generation };
}

#define Vector2_From(val) ((Vector2) {val, val})
#define Vector2_Ones ((Vector2) {1.0, 1.0})
#define Vector2_Unit_X ((Vector2) {1.0, 0.0})
#define Vector2_Unit_Y ((Vector2) {0.0, 1.0})
#define Vector2_Neg_Unit_X ((Vector2) {-1.0, 0.0})
#define Vector2_Neg_Unit_Y ((Vector2) {0.0, -1.0})
#define Vector2_On_X(val) ((Vector2) {val, 0.0})
#define Vector2_On_Y(val) ((Vector2) {0.0, val})
#define Vector2_XX(vec) ((Vector2) {vec.x, vec.x})
#define Vector2_YY(vec) ((Vector2) {vec.y, vec.y})
#define Vector2_YX(vec) ((Vector2) {vec.y, vec.x})

static inline Vector2 vector2_from(float32 val) {
    return (Vector2) {val, val};
}

static inline Vector2 vector2_ones() {
    return (Vector2) {1.0, 1.0};
}

static inline Vector2 vector2_unit_x() {
    return (Vector2) {1.0, 0.0};
}

static inline Vector2 vector2_unit_y() {
    return (Vector2) {0.0, 1.0};
}

static inline Vector2 vector2_neg_unit_x() {
    return (Vector2) {-1.0, 0.0};
}

static inline Vector2 vector2_neg_unit_y() {
    return (Vector2) {0.0, -1.0};
}

static inline Vector2 vector2_on_x(float32 val) {
    return (Vector2) {val, 0.0};
}

static inline Vector2 vector2_on_y(float32 val) {
    return (Vector2) {0.0, val};
}

static inline Vector2 vector2_xx(Vector2 vec) {
    return (Vector2) {vec.x, vec.x};
}

static inline Vector2 vector2_yy(Vector2 vec) {
    return (Vector2) {vec.y, vec.y};
}

static inline Vector2 vector2_yx(Vector2 vec) {
    return (Vector2) {vec.y, vec.x};
}

#endif // __HEADER_BASE

#define __HEADER_MEMORY_MEMORY
#ifdef __HEADER_MEMORY_MEMORY

typedef struct {
    usize size;
    usize alignment;
} MemoryLayout;

#define layoutof(TYPE) ((MemoryLayout) { .size = sizeof(TYPE), .alignment = _Alignof(TYPE)})

usize addr_align_up(usize addr, usize align);
void* ptr_align_up(void* ptr, usize align);

#define BUMP_ALLOCATOR_DEFAULT_SIZE (10 * 1024)

typedef struct {
    byte* base;
    byte* cursor;
    usize total_size;
} BumpAllocator;

typedef BumpAllocator TemporaryStorage;

BumpAllocator make_bump_allocator_with_size(usize size);
BumpAllocator make_bump_allocator();
void free_bump_allocator(BumpAllocator* bump_allocator);
void reset_bump_allocator(BumpAllocator* bump_allocator);
void* bump_alloc(BumpAllocator* bump_allocator, usize size);
void* bump_alloc_aligned(BumpAllocator* bump_allocator, MemoryLayout layout);

#define ARENA_ALLOCATOR_DEFAULT_REGION_SIZE (10 * 1024)

typedef struct ArenaRegion {
    struct ArenaRegion* next_region;
    usize total_size;
    usize free_size;
    byte* cursor;
    byte base[];
} ArenaRegion;

typedef struct {
    usize region_size;
    ArenaRegion* head_region;
} ArenaAllocator;

ArenaAllocator make_arena_allocator_with_region_size(usize region_size);
ArenaAllocator make_arena_allocator();
void free_arena_allocator(ArenaAllocator* arena_allocator);
void reset_arena_allocator(ArenaAllocator* arena_allocator);
void* arena_alloc(ArenaAllocator* arena_allocator, usize size);
void* arena_alloc_aligned(ArenaAllocator* arena_allocator, MemoryLayout layout);

#endif // __HEADER_MEMORY_MEMORY

#define __HEADER_THREAD_TASK
#ifdef __HEADER_THREAD_TASK

typedef void TaskArgVoid;

typedef struct {
    unsigned int (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} ThreadEntry;

typedef struct {
    void (*handler)(TaskArgVoid*);
    TaskArgVoid* arg;
} Task;

#endif // __HEADER_THREAD_TASK

#define __HEADER_THREAD_THREAD
#ifdef __HEADER_THREAD_THREAD

typedef struct {
    unsigned int id;
    uintptr_t handle;
} Thread;

Thread thread_spawn(ThreadEntry entry);
void end_thread(unsigned int return_code);
void thread_join(Thread thread);

#endif // __HEADER_THREAD_THREAD

#define __HEADER_THREAD_THREADSYNC
#ifdef __HEADER_THREAD_THREADSYNC

// Sync objects are opeque
// because windows.h and raylib.h
// has name collisions

typedef void SRWLock;

SRWLock* alloc_create_srw_lock();
void free_srw_lock(SRWLock* lock);
void srw_lock_acquire_exclusive(SRWLock* lock);
void srw_lock_acquire_shared(SRWLock* lock);
void srw_lock_release_exclusive(SRWLock* lock);
void srw_lock_release_shared(SRWLock* lock);

#endif // __HEADER_THREAD_THREADSYNC

#define __HEADER_THREAD_THREADPOOL
#ifdef __HEADER_THREAD_THREADPOOL

typedef enum {
    // NO_SHUTDOWN = 0
    THREADPOOL_IMMEDIATE_SHUTDOWN = 1,
    THREADPOOL_GRACEFULL_SHUTDOWN = 2,
} ThreadPoolShutdown;

// ThreadPool is opeque
// because windows.h and raylib.h
// has name collisions
// and ThreadPool includes windows structs
typedef void ThreadPool;

ThreadPool* thread_pool_create(unsigned int n_threads, unsigned int task_queue_capacity);
bool thread_pool_add_task(ThreadPool* pool, Task task);
void thread_pool_shutdown(ThreadPool* pool, ThreadPoolShutdown shutdown);

void example_async_task_print_int_arg(TaskArgVoid* arg);

#endif // __HEADER_THREAD_THREADPOOL

#define __HEADER_ASSET_ASSET
#ifdef __HEADER_ASSET_ASSET

#define TEXTURE_SLOTS 100
#define DEFAULT_TEXTURE_HANDLE_ID   0
#define BLANK_TEXTURE_HANDLE_ID     1
#define WHITE_TEXTURE_HANDLE_ID     2

typedef struct {
    uint32 id;
} TextureHandle;

TextureHandle new_texture_handle(uint32 id);

typedef struct {
    bool exists;
    Image* texture;
} ImageResponse;

typedef struct {
    bool exists;
    Texture* texture;
} TextureResponse;

typedef struct {
    uint32 next_slot_available_bump;
    bool slots[TEXTURE_SLOTS];
    bool image_ready[TEXTURE_SLOTS];
    bool texture_ready[TEXTURE_SLOTS];
    bool image_changed[TEXTURE_SLOTS];
    Image images[TEXTURE_SLOTS];
    Texture textures[TEXTURE_SLOTS];
} TextureAssets;

TextureAssets just_engine_new_texture_assets();

TextureHandle just_engine_texture_assets_reserve_texture_slot(TextureAssets* assets);

void just_engine_texture_assets_put_image(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_image_unchecked(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_load_texture_uncheched(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_update_texture_unchecked(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_update_texture_rec_unchecked(TextureAssets* assets, TextureHandle handle, Rectangle rec);
void just_engine_texture_assets_put_image_and_load_texture(TextureAssets* assets, TextureHandle handle, Image image);
void just_engine_texture_assets_load_texture_then_unload_image(TextureAssets* assets, TextureHandle handle, Image image);

ImageResponse just_engine_texture_assets_get_image(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_or_default(TextureAssets* assets, TextureHandle handle);
Image* just_engine_texture_assets_get_image_unchecked(TextureAssets* assets, TextureHandle handle);

TextureResponse just_engine_texture_assets_get_texture(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_or_default(TextureAssets* assets, TextureHandle handle);
Texture* just_engine_texture_assets_get_texture_unchecked(TextureAssets* assets, TextureHandle handle);

void just_engine_texture_assets_unload_image(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_texture(TextureAssets* assets, TextureHandle handle);
void just_engine_texture_assets_unload_slot(TextureAssets* assets, TextureHandle handle);

#endif // __HEADER_ASSET_ASSET

#define __HEADER_EVENTS_EVENTS
#ifdef __HEADER_EVENTS_EVENTS

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
    SRWLock* rw_lock;
    EventBuffer_TextureAssetEvent event_buffers[2];
    uint8 this_frame_ind;
} Events_TextureAssetEvent;

Events_TextureAssetEvent just_engine_events_texture_asset_event_create();
void just_engine_events_texture_asset_event_send_single(Events_TextureAssetEvent* events, TextureAssetEvent event);
void just_engine_events_texture_asset_event_send_batch(Events_TextureAssetEvent* events, TextureAssetEvent* event_list, uint32 count);
void just_engine_events_texture_asset_event_swap_buffers(Events_TextureAssetEvent* events);

typedef struct {
    uint32 index;
    Events_TextureAssetEvent* events;
} EventsIter_TextureAssetEvent;

EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_begin_iter(Events_TextureAssetEvent* events, uint32 offset);
EventsIter_TextureAssetEvent just_engine_events_iter_texture_asset_events_begin_iter_all(Events_TextureAssetEvent* events);
/**
 * 
 * @return offset for next frame
 */
uint32 just_engine_events_iter_texture_asset_events_end_iter(EventsIter_TextureAssetEvent* iter);
bool just_engine_events_iter_texture_asset_events_has_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_read_next(EventsIter_TextureAssetEvent* iter);
TextureAssetEvent just_engine_events_iter_texture_asset_events_consume_next(EventsIter_TextureAssetEvent* iter);

#endif // __HEADER_EVENTS_EVENTS

#define __HEADER_ASSET_ASSETSERVER
#ifdef __HEADER_ASSET_ASSETSERVER

typedef struct {
    const char const* asset_folder;
} FileAssetServer;

TextureHandle just_engine_asyncio_file_load_image(
    ThreadPool* RES_threadpool,
    Events_TextureAssetEvent* RES_texture_assets_events,
    TextureAssets* RES_texture_assets,
    FileAssetServer* server,
    const char* filepath
);

#endif // __HEADER_ASSET_ASSETSERVER

#define __HEADER_ANIMATION_ANIMATION
#ifdef __HEADER_ANIMATION_ANIMATION

typedef enum {
    Timer_NonRepeating,
    Timer_Repeating,
} TimerMode;

typedef struct {
    TimerMode mode;
    float time_setup;
    float time_elapsed;
    bool finished;
} Timer;

Timer new_timer(float setup_secs, TimerMode mode);
void reset_timer(Timer* timer);
void tick_timer(Timer* timer, float delta_time_seconds);
bool timer_is_finished(Timer* timer);

typedef struct {
    TimerMode mode;
    int checkpoint_count;
    int index;
    float* checkpoints;
    float time_elapsed;
    bool pulsed;
    bool finished;
} SequenceTimer;

SequenceTimer new_sequence_timer(float* checkpoints, int count, TimerMode mode);
SequenceTimer new_sequence_timer_evenly_spaced(float time_between, int count, TimerMode mode);
void reset_sequence_timer(SequenceTimer* stimer);
void tick_sequence_timer(SequenceTimer* stimer, float delta_time_seconds);
bool sequence_timer_has_pulsed(SequenceTimer* stimer);
bool sequence_timer_is_finished(SequenceTimer* stimer);

typedef struct {
    SequenceTimer timer;
    TextureHandle* textures;
    int texture_count;
    int current_texture_ind;
} SpriteAnimation;

SpriteAnimation new_sprite_animation(SequenceTimer timer, TextureHandle* textures, int texture_count);
void tick_animation_timer(SpriteAnimation* anim, float delta_time_seconds);
TextureHandle get_current_texture(SpriteAnimation* anim);

typedef struct {
    SequenceTimer timer;
    TextureHandle sprite_sheet_texture;
    Vector2 sprite_size;
    int rows;
    int cols;
    int count;
    int current_sprite_ind;
} SpriteSheetAnimation;

SpriteSheetAnimation new_sprite_sheet_animation(
    SequenceTimer timer,
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int rows,
    int cols,
    int count
);
SpriteSheetAnimation new_sprite_sheet_animation_single_row(
    SequenceTimer timer,
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int count
);
SpriteSheetAnimation new_sprite_sheet_animation_single_row_even_timer(
    TextureHandle sprite_sheet_texture,
    Vector2 sprite_size,
    int count,
    float time_between,
    TimerMode mode
);
void reset_sprite_sheet_animation(SpriteSheetAnimation* anim);
void tick_sprite_sheet_animation_timer(SpriteSheetAnimation* anim, float delta_time_seconds);

typedef struct {
    TextureHandle texture_handle;
    Rectangle sprite;
} SpriteSheetSprite;

SpriteSheetSprite sprite_sheet_get_current_sprite(SpriteSheetAnimation* anim);

typedef struct {
    RectSize sprite_size;
    uint32 frame_count;
    uint32 current;
} FrameSpriteSheetAnimation;

/**
 * Requires single row sprite sheet
 * With frame_count number of frames
 */
FrameSpriteSheetAnimation new_frame_sprite_sheet_animation(
    RectSize sprite_size,
    uint32 frame_count
);
void reset_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
void tick_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
void tick_back_frame_sprite_sheet_animation(FrameSpriteSheetAnimation* anim);
Rectangle sprite_sheet_get_current_frame(FrameSpriteSheetAnimation* anim);

#endif // __HEADER_ANIMATION_ANIMATION

#define __HEADER_PHYSICS_COLLISION
#ifdef __HEADER_PHYSICS_COLLISION

typedef struct {
    Vector2 position;
    Vector2 direction;
} Ray2;

typedef struct {
    Vector2 start;
    Vector2 end;
} LineSegmentCollider;

typedef struct {
    Vector2 center;
    float32 radius;
} CircleCollider;

typedef struct {
    float32 x_left;
    float32 x_right;
    float32 y_top;
    float32 y_bottom;
} AABBCollider;

// TODO: FreeRectangleCollider: arbitrarily rotated rectangle

float32 just_engine_collider_dist_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_point_inside_aabb(AABBCollider a1, Vector2 p);

bool just_engine_check_collision_line_line(LineSegmentCollider l1, LineSegmentCollider l2);
bool just_engine_check_collision_line_circle(LineSegmentCollider l1, CircleCollider c2);
bool just_engine_check_collision_line_aabb(LineSegmentCollider l1, AABBCollider a2);

bool just_engine_check_collision_circle_circle(CircleCollider c1, CircleCollider c2);
bool just_engine_check_collision_circle_aabb(CircleCollider c1, AABBCollider a2);

bool just_engine_check_collision_aabb_aabb(AABBCollider a1, AABBCollider a2);

bool just_engine_check_rayhit_circle(Ray2 ray, CircleCollider c1, float32 max_dist);
bool just_engine_check_rayhit_aabb(Ray2 ray, AABBCollider a1, float32 max_dist);

#endif // __HEADER_PHYSICS_COLLISION

#define __HEADER_SHAPES_SHAPES
#ifdef __HEADER_SHAPES_SHAPES

typedef struct {
    float32 head_radius;
    Vector2 base;
    Vector2 direction; // normalized
    float32 length;
} Arrow;

Vector2 arrow_get_head(Arrow arrow);
void arrow_draw(Arrow arrow, float32 thick, Color color);

#endif // __HEADER_SHAPES_SHAPES

#define __HEADER_UI_UILAYOUT
#ifdef __HEADER_UI_UILAYOUT

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 row_padding;
    uint32 rows;
    //
    Rectangle content_box;
    RectSize row_size;
    uint32 next_row;
} RowLayout;

RowLayout make_row_layout(RowLayout layout);
Rectangle row_layout_next_n(RowLayout* layout, uint32 count);
Rectangle row_layout_next(RowLayout* layout);

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 col_padding;
    uint32 cols;
    //
    Rectangle content_box;
    RectSize col_size;
    uint32 next_col;
} ColumnLayout;

ColumnLayout make_column_layout(ColumnLayout layout);
Rectangle column_layout_next_n(ColumnLayout* layout, uint32 count);
Rectangle column_layout_next(ColumnLayout* layout);

typedef enum {
    Grid_RowMajor,
    Grid_ColumnMajor,
} GridMajor;

typedef struct {
    Rectangle box;
    float32 box_padding;
    float32 cell_padding;
    GridMajor major;
    uint32 rows;
    uint32 cols;
    //
    Rectangle content_box;
    RectSize cell_size;
    uint32 next_cell;
} GridLayout;

GridLayout make_grid_layout(GridLayout layout);
Rectangle grid_layout_next_n(GridLayout* layout, uint32 count);
Rectangle grid_layout_next(GridLayout* layout);

#endif // __HEADER_UI_UILAYOUT

#define __HEADER_UI_JUSTUI
#ifdef __HEADER_UI_JUSTUI

#define CUSTOM_UI_ELEMENT_SLOT_COUNT 100

typedef struct {
    uint32 id;
} UIElementId;

/**
 * Variants [0, CUSTOM_UI_ELEMENT_SLOT_COUNT) are reserved for custom elements
 */
typedef enum {
    UIElementType_Area = CUSTOM_UI_ELEMENT_SLOT_COUNT,
    UIElementType_Button,
    UIElementType_SelectionBox,
    UIElementType_Slider,
    UIElementType_ChoiceList,
    UIElementType_Panel,
} UIElementType;

typedef struct {
    // bool idle;
    bool on_hover;
    bool on_press;
    //
    bool just_begin_hover;
    bool just_end_hover;
    bool just_pressed;
    //
    bool just_clicked;
    Vector2 click_point_relative;
} UIElementState;

// TODO: maybe utilize bitmask
// typedef enum {
//     ON_HOVER,
//     ON_PRESS,
//     //
//     JUST_BEGIN_HOVER,
//     JUST_END_HOVER,
//     JUST_PRESSED,
//     //
//     JUST_CLICKED,
// } UIElementStateEnum;

/**
 * Specific UI Elements have to start with UIElement field
 */
typedef struct {
    UIElementId id;
    UIElementType type;
    UIElementState state;
    uint32 layer;
    Anchor anchor;
    Vector2 position;
    RectSize size;
    bool disabled;
} UIElement;

// ----------------
typedef enum {
    UIEvent_BeginHover,
    UIEvent_StayHover,
    UIEvent_EndHover,
    UIEvent_Pressed,
    UIEvent_Released,
    UIEvent_Update,
    UIEvent_Draw,
    UIEvent_DropElement,
} UIEvent;

typedef struct {
    float32 delta_time;
    Vector2 mouse;      // relative to element top-left
    Vector2 element_origin;
} UIEventContext;

void put_ui_handle_vtable_entry(uint32 type_id, void (*handler)(UIElement* elem, UIEvent event, UIEventContext context));
// ----------------

typedef struct {
    uint32 layer;
    uint32 index;
} ElementSort;

typedef struct {
    BumpAllocator memory;
    uint32 count;
    UIElement** elems;
    ElementSort* layer_sort;
    UIElement* pressed_element;
    bool active;
} UIElementStore;

// ----------------
typedef struct {
    bool is_bordered;
    float32 thick;
    Color color;
} UIBorderStyle;

typedef struct {
    Font font;
    float32 font_size;
    float32 spacing;
    Color color;
} UITextStyle;
// ----------------

typedef struct {
    Color idle_color;
    Color hovered_color;
    UIBorderStyle border;
} AreaStyle;

typedef struct {
    UIElement elem;
    AreaStyle style;
} Area;

typedef struct {
    Color idle_color;
    Color hovered_color;
    Color pressed_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} ButtonStyle;

typedef struct {
    UIElement elem;
    ButtonStyle style;
    Vector2 draw_offset;
    char title[20];
} Button;

void button_consume_click(Button* button);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} SelectionBoxStyle;

typedef struct {
    UIElement elem;
    SelectionBoxStyle style;
    char title[20];
    bool selected;
} SelectionBox;

typedef struct {
    Color line_color;
    Color cursor_color;
    UIBorderStyle border;
    UITextStyle title;
} SliderStyle;

typedef struct {
    UIElement elem;
    SliderStyle style;
    char title[20];
    float32 low_value;
    float32 high_value;
    float32 cursor; // [0, 1]
} Slider;

float32 get_slider_value(Slider* slider);

typedef struct {
    Color selected_color;
    Color unselected_color;
    Color disabled_color;
    UIBorderStyle border;
    UITextStyle title;
} ChoiceListStyle;

typedef struct {
    uint32 id;
    char title[20];
} ChoiceListOption;

typedef struct {
    UIElement elem;
    ChoiceListStyle style;
    GridLayout layout;
    Option(uint32) hovered_option_index;
    uint32 selected_option_id;
    uint32 option_count;
    ChoiceListOption options[20];
} ChoiceList;

typedef struct {
    UIElement elem;
    UIElementStore store;
    bool open;
} Panel;

// ----------------
Area make_ui_area(Area area);
Button make_ui_button(Button button);
SelectionBox make_ui_selection_box(SelectionBox selection_box);
Slider make_ui_slider(Slider slider);
ChoiceList make_ui_choice_list(ChoiceList choice_list);
Panel make_ui_panel(Panel panel);
// ----------------

UIElementStore ui_element_store_new_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new();
UIElementStore ui_element_store_new_active_with_count_hint(uint32 count_hint);
UIElementStore ui_element_store_new_active();
void ui_element_store_drop_elements(UIElementStore* store);
void ui_element_store_drop(UIElementStore* store);

void* get_ui_element_unchecked(UIElementStore* store, UIElementId elem_id);
UIElement* get_ui_element(UIElementStore* store, UIElementId elem_id);

// ----------------
UIElementId put_ui_element(UIElementStore* store, UIElement* elem, MemoryLayout layout);

UIElementId put_ui_element_area(UIElementStore* store, Area area);
UIElementId put_ui_element_button(UIElementStore* store, Button button);
UIElementId put_ui_element_selection_box(UIElementStore* store, SelectionBox sbox);
UIElementId put_ui_element_slider(UIElementStore* store, Slider slider);
UIElementId put_ui_element_choice_list(UIElementStore* store, ChoiceList choice_list);
UIElementId put_ui_element_panel(UIElementStore* store, Panel panel);
// ----------------

void SYSTEM_PRE_UPDATE_handle_input_for_ui_store(
    UIElementStore* store
);

void SYSTEM_UPDATE_update_ui_elements(
    UIElementStore* store,
    float32 delta_time
);

void SYSTEM_RENDER_draw_ui_elements(
    UIElementStore* store
);


#endif // __HEADER_UI_JUSTUI

#define __HEADER_RENDER2D_CAMERA2D
#ifdef __HEADER_RENDER2D_CAMERA2D

#define MAX_CAMERA_COUNT 10
#define PRIMARY_CAMERA_ID 0

typedef struct {
    Camera2D camera;
    Layers layers;
    uint32 sort_index;
} SpriteCamera;

typedef struct {
    uint32 count;
    SpriteCamera cameras[MAX_CAMERA_COUNT];
} SpriteCameraStore;

void set_primary_camera(SpriteCameraStore* store, SpriteCamera camera);
SpriteCamera* get_primary_camera(SpriteCameraStore* store);
void add_camera(SpriteCameraStore* store, SpriteCamera camera);

#endif // __HEADER_RENDER2D_CAMERA2D

#define __HEADER_RENDER2D_SPRITE
#ifdef __HEADER_RENDER2D_SPRITE

typedef enum {
    Rotation_CW = 1,
    Rotation_CCW = -1,
} RotationWay;

typedef struct {
    Anchor anchor;
    Vector2 position;       // position of the anchor
    bool use_source_size;
    Vector2 size;
    Vector2 scale;
    float32 rotation;       // rotation around its anchor
    RotationWay rway;
} SpriteTransform;

typedef struct {
    // -- render start
    TextureHandle texture;
    Color tint;
    bool use_custom_source;
    Rectangle source;
    uint32 z_index;
    // -- render end
    bool use_layer_system; // otherwise renders on the primary camera by default
    Layers layers;
    bool visible;
    bool camera_visible;
} Sprite;

typedef struct {
    TextureHandle texture;
    Color tint;
    bool use_custom_source;
    Rectangle source;
    SpriteTransform transform;
    uint32 z_index;
} RenderSprite;

typedef struct {
    uint32 count;
    uint32 capacity;
    RenderSprite* sprites;
} SortedRenderSprites;

typedef struct {
    uint32 camera_index;
    uint32 sort_index;
} CameraSortElem;

typedef struct {
    uint32 camera_count;
    CameraSortElem camera_sort[MAX_CAMERA_COUNT];
    SortedRenderSprites render_sprites[MAX_CAMERA_COUNT];
} PreparedRenderSprites;

typedef struct {
    uint32 count;
    uint32 capacity;
    uint32 free_count;
    bool* slot_occupied;
    uint32* generations;
    SpriteTransform* transforms;
    Sprite* sprites;
} SpriteStore;

typedef ComponentId SpriteComponentId;

SpriteComponentId spawn_sprite(
    SpriteStore* sprite_store,
    SpriteTransform transform,
    Sprite sprite
);
void despawn_sprite(SpriteStore* sprite_store, SpriteComponentId sprite_id);

void SYSTEM_EXTRACT_RENDER_cull_and_sort_sprites(
    SpriteCameraStore* sprite_camera_store,
    SpriteStore* sprite_store,
    PreparedRenderSprites* prepared_render_sprites
);

void SYSTEM_RENDER_sorted_sprites(
    TextureAssets* RES_texture_assets,
    SpriteCameraStore* sprite_camera_store,
    PreparedRenderSprites* prepared_render_sprites
);

#endif // __HEADER_RENDER2D_SPRITE

#define __HEADER_LIB
#ifdef __HEADER_LIB

void SYSTEM_POST_UPDATE_check_mutated_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_EXTRACT_RENDER_load_textures_for_loaded_or_changed_images(
    TextureAssets* RES_texture_assets,
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_BOUNDARY_swap_event_buffers(
    Events_TextureAssetEvent* RES_texture_asset_events
);

void SYSTEM_FRAME_BOUNDARY_reset_temporary_storage(
    TemporaryStorage* RES_temporary_storage
);

#endif // __HEADER_LIB
