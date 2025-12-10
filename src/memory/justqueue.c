
#include "justqueue.h"

Queue_usize Queue_usize__queue_new(usize capacity) {
    Queue_usize q = {0};
    dynarray_reserve(q, capacity);
    return q;
}

void Queue_usize__queue_free(Queue_usize* q) {
    dynarray_free(*q);
}

void Queue_usize__queue_reset(Queue_usize* q) {
    void* items = q->items;
    *q = (Queue_usize) {0};
    q->items = items;
}

bool Queue_usize__queue_is_full(Queue_usize* q) {
    return q->count == q->capacity;
}

bool Queue_usize__queue_is_empty(Queue_usize* q) {
    return q->count == 0;
}

bool Queue_usize__queue_has_next(Queue_usize* q) {
    return q->count > 0;
}

bool Queue_usize__queue_push(Queue_usize* q, usize item) {
    if (Queue_usize__queue_is_full(q)) {
        return false;
    }
    q->items[q->tail] = item;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    return true;
}

bool Queue_usize__queue_pop(Queue_usize* q, usize* set_item) {
    if (Queue_usize__queue_has_next(q)) {
        *set_item = q->items[q->head];
        q->count--;
        q->head = (q->head + 1) % q->capacity;
        return true;
    }
    return false;
}