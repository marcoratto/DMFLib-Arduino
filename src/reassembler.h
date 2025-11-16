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
#ifndef REASSEMBLER_H
#define REASSEMBLER_H

#include <stdint.h> 		// For types like uint8_t, uint16_t, uint32_t
#include <stddef.h> 	    // For size_t
#include <stdio.h>          // For printf, fprintf
#include <string.h>        // For memcpy

#include "common.h"

typedef enum {
    REASSEMBLER_INIT = 0,                          // Reassembler initialized 
    REASSEMBLER_NOT_INIT = 1,                      // Reassembler not initialized 
    REASSEMBLER_COMPLETE = 2,                      // Reassembler complete
    REASSEMBLER_INCOMPLETE = 3,                    // Reassembler still in progress
    REASSEMBLER_CHUNK_TOO_SMALL = 4,               // Chunk too small to contain minimum header
    REASSEMBLER_BUFFER_OVERFLOW = 5,               // Buffer overflow
    REASSEMBLER_LAST_CHUNK_ALREADY_RECEIVED = 6,   // Last chunk already received
    REASSEMBLER_SEQUENCE_ERROR = 7,                // Sequence error
    REASSEMBLER_RESET = 8                          // Reassembler reset
} ReassemblerReturnCode __attribute__((packed));

typedef struct {
    uint8_t *data_buffer;           // Pointer of the target buffer allocated from the caller
    size_t buffer_max_size;         // Max size of the target buffer
    size_t current_size;            // Bytes reassembled 
    uint8_t last_index_received;    // Index of the last chunk received (check the right order)
    uint8_t last_chunk_received;    // Flag: 1 if the last chunk (0xFF) has arrived
    uint8_t initialized;            // Flag: 1 if the Reassembler is initizialed
} reassembler_t;

// --- Function Prototypes ---

/**
 * @brief Initializes the reassembler structure for message reconstruction.
 *
 * This function prepares the reassembler to receive incoming chunks and
 * rebuild the original message into the provided data buffer.
 * The reassembler will start in an empty state, ready to accept chunks
 * via reassembler_add_chunk().
 *
 * @param reassembler Pointer to the reassembler structure to initialize.
 * @param data_buffer Pointer to the buffer where the reassembled message will be stored.
 * @param buffer_max_size Maximum size of the reassembly buffer in bytes.
 * @return A ReassemblerReturnCode indicating the result of the initialization.
 */
ReassemblerReturnCode reassembler_init(reassembler_t *reassembler, uint8_t *data_buffer, size_t buffer_max_size);

/**
 * @brief Resets the internal state of the reassembler.
 *
 * Clears any partially received data and returns the reassembler to its initial state.
 * This function should be called if an error occurs or when starting
 * the reconstruction of a new message.
 *
 * @param reassembler Pointer to the reassembler structure to reset.
 * @return A ReassemblerReturnCode indicating the result of the operation.
 */
ReassemblerReturnCode reassembler_reset(reassembler_t *reassembler);

/**
 * @brief Adds a new chunk to the ongoing reassembly process.
 *
 * This function appends a received chunk to the current message being reconstructed.
 * If the chunk completes the message, the reassembler updates its internal state
 * to indicate that the full message has been reassembled.
 *
 * @param reassembler Pointer to the reassembler structure.
 * @param chunk_buffer Pointer to the received chunk data.
 * @param chunk_len Length of the received chunk in bytes.
 * @return A ReassemblerReturnCode indicating the result of the operation
 *         (e.g., success, buffer overflow, or message complete).
 */
ReassemblerReturnCode reassembler_add_chunk(reassembler_t *reassembler, const uint8_t *chunk_buffer, size_t chunk_len);

#endif // REASSEMBLER_H
