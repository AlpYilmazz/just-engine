
#include "raylib.h"

#include "base.h"

#include "uilayout.h"

// RowLayoutBegin
// RowLayout

RowLayout make_row_layout(RowLayout layout) {
    Rectangle content_box = (Rectangle) {
        .x = layout.box.x + layout.box_padding,
        .y = layout.box.y + layout.box_padding,
        .width = layout.box.width - 2*layout.box_padding,
        .height = layout.box.height - 2*layout.box_padding,
    };
    RectSize row_size = {
        .width = content_box.width,
        .height = (content_box.height - (layout.rows - 1)*layout.row_padding) / layout.rows,                 
    };

    return (RowLayout) {
        .box = layout.box,
        .box_padding = layout.box_padding,
        .row_padding = layout.row_padding,
        .rows = layout.rows,
        //
        .content_box = content_box,
        .row_size = row_size,
        .next_row = 0,
    };
}

Rectangle row_layout_next_n(RowLayout* layout, uint32 count) {
    uint32 i_row = layout->next_row;
    layout->next_row += count;

    float32 row_stride = layout->row_size.height + layout->row_padding;
    return (Rectangle) {
        .x = layout->content_box.x,
        .y = layout->content_box.y + (i_row * row_stride),
        .width = layout->row_size.width,
        .height = (count * row_stride) - layout->row_padding,
    };
}

Rectangle row_layout_next(RowLayout* layout) {
    return row_layout_next_n(layout, 1);
}

// ColumnLayoutBegin
// ColumnLayout

ColumnLayout make_column_layout(ColumnLayout layout) {
    Rectangle content_box = (Rectangle) {
        .x = layout.box.x + layout.box_padding,
        .y = layout.box.y + layout.box_padding,
        .width = layout.box.width - 2*layout.box_padding,
        .height = layout.box.height - 2*layout.box_padding,
    };
    RectSize col_size = {
        .width = (content_box.width - (layout.cols - 1)*layout.col_padding) / layout.cols,                 
        .height = content_box.height,
    };

    return (ColumnLayout) {
        .box = layout.box,
        .box_padding = layout.box_padding,
        .col_padding = layout.col_padding,
        .cols = layout.cols,
        //
        .content_box = content_box,
        .col_size = col_size,
        .next_col = 0,
    };
}

Rectangle column_layout_next_n(ColumnLayout* layout, uint32 count) {
    uint32 i_col = layout->next_col;
    layout->next_col += count;

    float32 col_stride = layout->col_size.height + layout->col_padding;
    return (Rectangle) {
        .x = layout->content_box.x + (i_col * col_stride),
        .y = layout->content_box.y,
        .width = (count * col_stride) - layout->col_padding,
        .height = layout->col_size.height,
    };
}

Rectangle column_layout_next(ColumnLayout* layout) {
    return column_layout_next_n(layout, 1);
}

// GridMajor
// GridLayoutBegin
// GridLayout

GridLayout make_grid_layout(GridLayout layout) {
    Rectangle content_box = (Rectangle) {
        .x = layout.box.x + layout.box_padding,
        .y = layout.box.y + layout.box_padding,
        .width = layout.box.width - 2*layout.box_padding,
        .height = layout.box.height - 2*layout.box_padding,
    };
    RectSize cell_size = {
        .width = (content_box.width - (layout.cols - 1)*layout.cell_padding) / layout.cols,                 
        .height = (content_box.height - (layout.rows - 1)*layout.cell_padding) / layout.rows,
    };

    return (GridLayout) {
        .box = layout.box,
        .box_padding = layout.box_padding,
        .cell_padding = layout.cell_padding,
        .major = layout.major,
        .rows = layout.rows,
        .cols = layout.cols,
        //
        .content_box = content_box,
        .cell_size = cell_size,
        .next_cell = 0,
    };
}

// count cannot overflow into next row
static Rectangle grid_layout_row_major_next_n(GridLayout* layout, uint32 count) {
    uint32 i_row = layout->next_cell / layout->cols;
    uint32 j_col = layout->next_cell % layout->cols;

    count = MIN(layout->cols - j_col, count);
    layout->next_cell += count;

    float32 row_stride = layout->cell_size.height + layout->cell_padding;
    float32 col_stride = layout->cell_size.width + layout->cell_padding;
    return (Rectangle) {
        .x = layout->content_box.x + (j_col * col_stride),
        .y = layout->content_box.y + (i_row * row_stride),
        .width = (count * col_stride) - layout->cell_padding,
        .height = layout->cell_size.height,
    };
}

// count cannot overflow into next column
static Rectangle grid_layout_column_major_next_n(GridLayout* layout, uint32 count) {
    uint32 i_col = layout->next_cell / layout->rows;
    uint32 j_row = layout->next_cell % layout->rows;

    count = MIN(layout->rows - j_row, count);
    layout->next_cell += count;

    float32 row_stride = layout->cell_size.height + layout->cell_padding;
    float32 col_stride = layout->cell_size.width + layout->cell_padding;
    return (Rectangle) {
        .x = layout->content_box.x + (i_col * col_stride),
        .y = layout->content_box.y + (j_row * row_stride),
        .width = layout->cell_size.width,
        .height = (count * row_stride) - layout->cell_padding,
    };
}

Rectangle grid_layout_next_n(GridLayout* layout, uint32 count) {
    switch (layout->major) {
        case Grid_RowMajor:
            return grid_layout_row_major_next_n(layout, count);
        case Grid_ColumnMajor:
            return grid_layout_column_major_next_n(layout, count);
    }
    PANIC("Incorrect GridMajor! Variants: Grid_RowMajor, Grid_ColumnMajor\n");
}

Rectangle grid_layout_next(GridLayout* layout) {
    return grid_layout_next_n(layout, 1);
}
