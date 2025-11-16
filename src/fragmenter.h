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
#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include <stdint.h>
#include <string.h>
#include "common.h"

typedef struct {
    const uint8_t *data;            // Pointer to the buffer
    size_t length;                // Length to the buffer
    uint32_t position;              // Position to the buffer
    uint8_t index;                  // Index of the fragment
} fragmenter_t;

/**
 * @brief Initializes the fragmenter structure for splitting a message into chunks.
 *
 * This function prepares the fragmenter to operate on a given data buffer,
 * setting the internal pointers and the total message length.
 * After this call, the fragmenter is ready to produce chunks
 * using the fragmenter_next() function.
 *
 * @param chunker Pointer to the fragmenter structure to initialize.
 * @param data Pointer to the original message data to be fragmented.
 * @param length Total message length in bytes.
 */
void fragmenter_init(fragmenter_t *chunker, const uint8_t *data, size_t length);

/**
 * @brief Checks whether there are more fragments to generate.
 *
 * This function inspects the fragmenter's internal state to determine
 * whether there is still data left to be extracted into chunks.
 *
 * @param chunker Pointer to the fragmenter structure.
 * @return 1 if more fragments are available, 0 otherwise.
 */
uint8_t fragmenter_has_more(const fragmenter_t *chunker);

/**
 * @brief Generates the next data fragment.
 *
 * Copies the next portion of data into the specified buffer,
 * up to the maximum size defined by payload_size.
 * After the call, the fragmenter updates its internal position
 * to point to the next fragment.
 *
 * @param chunker Pointer to the fragmenter structure.
 * @param buffer Destination buffer where the fragment will be written.
 * @param payload_size Maximum size of the payload in bytes.
 * @return The actual number of bytes copied into the buffer.
 */
size_t fragmenter_next(fragmenter_t *chunker, uint8_t *buffer, size_t payload_size);

#endif
