typedef struct SArray {
    // FIFO not stack, because seems more useful? (applicable to more situations)
    size_t push;
    size_t pop;
    size_t cap;
} SArray;

// return 1, if looped around, so that user knows and can do whatever it wants...
//  after calling this function, push will point to most recently added item, pop will point to oldest item?
static void sArrayInit(SArray * buffer, size_t cap) {
    buffer->push = -1;
    buffer->pop = -1;
    buffer->cap = cap;
}
// -1, -1
//  0, -1 - push - size = 1
//  1, -1 - push - size = 2
//  1, 0  - pop - size = 1
//  1, 1  - pop - size = 0
static size_t sArrayAdd(SArray * buffer) {
    // push always and remove oldest element if needed lol
    if (++buffer->push > buffer->cap) {
        buffer->push = 0;
    }
    if (buffer->push == buffer->pop) {
        buffer->pop++;
        return 1; // return 1 if removed oldest element
    }
    return 0;
}
static void sArrayRemove(SArray * buffer) {
    // if removed all elements, do nothing... else increment pop buffer and loop if needed.
    if (buffer->push != buffer->pop) {
        if (++buffer->pop > buffer->cap) {
            buffer->pop = 0;
        }
    }
}
// I feel like there might be a simplified way but who cares.
static size_t sArraySize(SArray * buffer) {
    size_t size = 0;
    if (buffer->push < buffer->pop) {
        // size to push index + size from pop index to end of buffer , 
        //  +1 because push index starts at zero, not +2 because cap and how numbers work.
        size = buffer->push + (buffer->cap - buffer->pop) + 1;
    } else if (buffer->push > buffer->pop) {
        size = buffer->push - buffer->pop;
    } // else equals so return 0
    return size; 
}