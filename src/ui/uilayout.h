#pragma once

#include "raylib.h"

#include "base.h"

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