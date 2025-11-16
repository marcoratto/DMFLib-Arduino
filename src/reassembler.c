/*
 * MIT License
 * 
 * Copyright (c) 2025 Marco Ratto
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "reassembler.h" // Include our header for definitions and prototypes

// --- Reassembler Function Implementations ---

ReassemblerReturnCode reassembler_init(reassembler_t *reassembler, uint8_t *data_buffer, size_t buffer_max_size) {
    if (reassembler == NULL || data_buffer == NULL || buffer_max_size == 0) {
        return REASSEMBLER_NOT_INIT;
    }
    reassembler->data_buffer = data_buffer;
    reassembler->buffer_max_size = buffer_max_size;
    reassembler->current_size = 0;
    reassembler->last_index_received = 0;
    reassembler->last_chunk_received = 0;
    reassembler->initialized = 1; // Mark as initialized
    return REASSEMBLER_INIT;
}

ReassemblerReturnCode reassembler_reset(reassembler_t *reassembler) {
    if (reassembler == NULL || !reassembler->initialized) {
        return REASSEMBLER_NOT_INIT;
    }
    reassembler->current_size = 0;
    reassembler->last_index_received = 0;
    reassembler->last_chunk_received = 0;
    // The data_buffer and buffer_max_size remain as set during initialization.
    return REASSEMBLER_RESET;
}

ReassemblerReturnCode reassembler_add_chunk(reassembler_t *reassembler, const uint8_t *chunk_buffer, size_t chunk_len) {
    uint8_t index;
    size_t payload_len;
    if (reassembler == NULL || !reassembler->initialized || chunk_buffer == NULL) {
        return REASSEMBLER_NOT_INIT;
    }

    if (reassembler->last_chunk_received) {
        return REASSEMBLER_LAST_CHUNK_ALREADY_RECEIVED;
    }

    if (chunk_len < CHUNK_HEADER_SIZE) {
        return REASSEMBLER_CHUNK_TOO_SMALL;
    }

    index = chunk_buffer[0];
    payload_len = chunk_len - CHUNK_HEADER_SIZE;

    if (index == 0xFF) { // This is the last chunk
        reassembler->last_chunk_received = 1;
    } else { // This is a regular chunk
        if (index != reassembler->last_index_received) {
            return REASSEMBLER_SEQUENCE_ERROR; // Sequence error
        }
        reassembler->last_index_received++; // Increment expected index for the next chunk
        payload_len = chunk_len - CHUNK_HEADER_SIZE;
    }

    // Check for buffer overflow BEFORE copying
    if (reassembler->current_size + payload_len > reassembler->buffer_max_size) {
       return REASSEMBLER_BUFFER_OVERFLOW; 
    }

    // Copy payload into the reassembly buffer
    memcpy(&reassembler->data_buffer[reassembler->current_size], &chunk_buffer[CHUNK_HEADER_SIZE], payload_len);
    reassembler->current_size += payload_len; // Update accumulated data size

    if (reassembler->last_chunk_received) {
        return REASSEMBLER_COMPLETE; 
    }

    return REASSEMBLER_INCOMPLETE;
}
