#include "wb_buffer.h"

#include "pdp_memory.h"

wb_buffer::wb_buffer(pdp_memory *mem)
{
    memory = mem;
    overflow_flag = false;
    length = 0;
}


bool wb_buffer::push(wb_item_t item) {
    //  Buffer overflow
    if(length >= WB_BUFFER_SIZE) {
        overflow_flag = true;
        if(!flush())
            return false;

    //  Not overflow
    } else {
        overflow_flag = false;
    }

    buffer[length] = item;
    length++;
    return true;
}


bool wb_buffer::flush() {
    for(int i = 0; i < length; ++i)
        if(!memory->w_write(buffer[i].adr, buffer[i].val))
            return false;

    length = 0;
    return true;
}


bool wb_buffer::was_overflow() {
    return overflow_flag;
}
