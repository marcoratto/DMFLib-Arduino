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
#include "fragmenter.h"

void fragmenter_init(fragmenter_t *fragmenter, const uint8_t *data, size_t length) {
    fragmenter->data = data;
    fragmenter->length = length;
    fragmenter->position = 0;
    fragmenter->index = 0;
}

uint8_t fragmenter_has_more(const fragmenter_t *fragmenter) {
    return fragmenter->position < fragmenter->length;
}

size_t fragmenter_next(fragmenter_t *fragmenter, uint8_t *buffer, size_t payload_size) {
    if (!fragmenter_has_more(fragmenter)) return 0;

    uint32_t remaining = fragmenter->length - fragmenter->position;
    
    uint32_t max_payload_with_header = payload_size - CHUNK_HEADER_SIZE;
    
    uint8_t chunk_len = (remaining > max_payload_with_header) ? max_payload_with_header : remaining;

    uint8_t index = (fragmenter->position + chunk_len >= fragmenter->length) ? 0xFF : fragmenter->index++;
    
    buffer[0] = index;
    memcpy(&buffer[CHUNK_HEADER_SIZE], &fragmenter->data[fragmenter->position], chunk_len);

    fragmenter->position += chunk_len;
    return chunk_len + CHUNK_HEADER_SIZE;
}
