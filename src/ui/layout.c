
#include "base.h"

typedef enum {
    HandleOverflow_Fit,
    HandleOverflow_Fit_KeepRatio,
    HandleOverflow_Fixed_EnlargeBox,
} HandleOverflow;

typedef struct {
    uint32 count;
    HandleOverflow handle_overflow;
    URectSize box_size;
    URectSize element_size;
    uint32 element_margin;
    uint32 padding;
} RowLayout;

RowLayout layout_fixed(RowLayout rl) {
    URectSize box_size = {
        .width = (rl.count * (rl.element_size.width + rl.element_margin)) - rl.element_margin + (2 * rl.padding),
        .height = rl.element_size.height + (2 * rl.padding),
    };

    if (box_size.width <= rl.box_size.width && box_size.height <= rl.box_size.height) {
        return rl;
    }

    URectSize content_space = {
        .width = rl.box_size.width - (2 * rl.padding),
        .height = rl.box_size.height - (2 * rl.padding),
    };
    uint32 total_margin = (rl.count - 1) * rl.element_margin;

    URectSize elem_max_size = {
        .width = (content_space.width - total_margin) / rl.count,
        .height = content_space.height,
    };

    switch (rl.handle_overflow) {
    case HandleOverflow_Fit:
        rl.element_size = elem_max_size;
        break;
    case HandleOverflow_Fit_KeepRatio:
        rl.element_size.width = (content_space.width - total_margin) / rl.count;
        rl.element_size.height = content_space.height;
        break;
    default:
        break;
    }

    return rl;
}