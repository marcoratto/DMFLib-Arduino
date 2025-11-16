/*
MIT License

Copyright (c) 2025 Marco Ratto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "ArduinoDMFLib.h"

#include "debug.h"
/**
 * @brief Polynomial value for CRC-32 RCS calculation
 */
#define POLYNOMAL_CRC_CONSTANT    0xEDB88320

#define LORAWAN_HEADER_MIN_SIZE 13
#define LORAWAN_HEADER_MAX_SIZE 28

#define SF7_PAYLOAD  222  
#define SF8_PAYLOAD  222 
#define SF9_PAYLOAD  115   
#define SF10_PAYLOAD  51    
#define SF11_PAYLOAD  51  
#define SF12_PAYLOAD  51

typedef struct {
    uint8_t index;
    uint8_t length;
    uint8_t *buffer;
} Chunk_t;

static const uint8_t sf_payload_sizes[] = {
    SF7_PAYLOAD,
    SF8_PAYLOAD,
    SF9_PAYLOAD,
    SF10_PAYLOAD,
    SF11_PAYLOAD,
    SF12_PAYLOAD
};

#define SF_PAYLOADS_COUNT (sizeof(sf_payload_sizes)/sizeof(sf_payload_sizes[0]))

// Funzione che ritorna un numero casuale tra min e max inclusi
size_t random_between(size_t min, size_t max) {
    if (min > max) {
        // Scambia i valori se min > max
        size_t temp = min;
        min = max;
        max = temp;
    }

    // Calcola range
    size_t range = max - min + 1;

    // Genera numero casuale in modo sicuro usando rand() e double
    size_t rnd = min + (size_t)(range * ((double)rand() / (RAND_MAX + 1.0)));

    return rnd;
}

size_t random_payload_size() {
    int idx = rand() % SF_PAYLOADS_COUNT;
    size_t base_size = sf_payload_sizes[idx];

    // Calcolo random tra 13 e 28
    size_t random_subtract = LORAWAN_HEADER_MIN_SIZE + (rand() % (LORAWAN_HEADER_MAX_SIZE - LORAWAN_HEADER_MIN_SIZE + 1));

    // Sottrazione
    return base_size - random_subtract;
}

void print_chunk_hex(const uint8_t *chunk, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        DEBUG_PRINTF("%02X", chunk[i]);
    }
    DEBUG_PRINTF("\n");
}

void generate_random_message(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = (rand() % 256);
    }
}

static uint32_t calculate_crc(uint8_t *data, size_t size) {
    uint32_t crc = 0xFFFFFFFF;
    uint16_t i = 0;
    uint16_t j = 0;
    
    /* CRC32 algorithm implementation */
    for(i = 0; i < size; i++) {
        uint8_t ch = data[i];
        for(j = 0; j < 8; j++) {
            uint32_t b = (ch ^ crc) & 0x01;
            crc >>= 1;
            if(b) {
                crc = crc ^ POLYNOMAL_CRC_CONSTANT;
            }
            ch >>= 1;
        }
    }
    
    return ~crc;
}

void shuffle_chunks(Chunk_t *chunks, size_t n) {
    // inizializza il generatore casuale
    srand((unsigned int)time(NULL));

    for (size_t i = n - 1; i > 0; i--) {
        // scegli un indice casuale da 0 a i
        size_t j = rand() % (i + 1);

        // scambia chunks[i] con chunks[j]
        Chunk_t temp = chunks[i];
        chunks[i] = chunks[j];
        chunks[j] = temp;
    }
}

void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    while (!Serial) {
        delay(1000);
    }
    DEBUG_PRINTLN("Lorawan Fragmentation using ArduinoDMFLib");    

    randomSeed(analogRead(0));

    DEBUG_PRINTLN("* Step #1: create random message");

    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
        size_t msg_len = random_between(128, 256);
    #else
        size_t msg_len = random_between(128, 1024);
    #endif

    // Generate random message between 128 and 512 bytes
    uint8_t *message = (uint8_t *) malloc(msg_len);
    if (!message) {
        DEBUG_PRINTLN("Error to allocate memory for 'message'!");
        return;
    }

	size_t message_reassembled_len = msg_len;
    uint8_t *message_reassembled = (uint8_t *) malloc(message_reassembled_len);
    if (!message_reassembled) {
        DEBUG_PRINTLN("Error to allocate memory for 'message_reassembled'!");
        return;
    }

    // Max number of chunks of the array
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
        int num_chunks = 5;  
    #else
        int num_chunks = MAX_CHUNKS;  
    #endif
    Chunk_t *chunks = (Chunk_t *) malloc(num_chunks * sizeof(Chunk_t));
    
    if (!chunks) {
         DEBUG_PRINTLN("Error to allocate memory for 'chunks'!");
        return;
    }
    
    generate_random_message(message, msg_len);
    DEBUG_PRINTF("Original message length: %u bytes\n", msg_len);
    uint32_t original_crc32 = calculate_crc(message, msg_len);
    DEBUG_PRINTF("CRC-32: 0x%08X\n", original_crc32);

	print_chunk_hex(message, msg_len);

    fragmenter_t fragmenter;
    fragmenter_init(&fragmenter, message, msg_len);

    DEBUG_PRINTLN("* Step #2: create chunks");

    int total_chunks = 0;
    while (fragmenter_has_more(&fragmenter)) {
        uint32_t payload_size = random_payload_size();
        DEBUG_PRINTF("Random Payload Size: %d\n", payload_size);
        
        chunks[total_chunks].index = total_chunks;
        
        // Allocazione del buffer per questo chunk
        chunks[total_chunks].buffer = (uint8_t *) malloc(payload_size + 1);
        
        if (!chunks[total_chunks].buffer) {
            DEBUG_PRINTF("Error to allocate memory for 'chunks[%d]'!\n", total_chunks);
            return;
        }
        
        memset(chunks[total_chunks].buffer, 0, payload_size);
        
        uint32_t chunk_size = fragmenter_next(&fragmenter, chunks[total_chunks].buffer, payload_size);
		chunks[total_chunks].length = chunk_size;

        DEBUG_PRINTF("Chunk [index=%d len=%u]\n", chunks[total_chunks].buffer[0], chunk_size);
        print_chunk_hex(chunks[total_chunks].buffer, chunk_size);
        total_chunks++;
    }
    DEBUG_PRINTF("Number of chunks is %d\n", fragmenter.index);

    // DEBUG_PRINTLN("* Step #2: Shuffle chunks");

    // shuffle_chunks(chunks, total_chunks);

    // for (int j=0; j<total_chunks; j++) {
	// 	print_chunk_hex(chunks[j].buffer, chunks[j].length);
	// }

	DEBUG_PRINTLN("* Step #3: Reassembly ordered chunks");

    uint8_t reassembler_ret_code = 0;

    reassembler_t reassembler;
    reassembler_ret_code = reassembler_init(&reassembler, message_reassembled, message_reassembled_len);
    DEBUG_PRINTF("reassembler_ret_code is %d\n", reassembler_ret_code);
    for (int j=0; j<total_chunks; j++) {
		DEBUG_PRINTF("Reassembly chunk [index=%d len=%u]\n", chunks[j].buffer[0], chunks[j].length);
		print_chunk_hex(chunks[j].buffer, chunks[j].length);
		reassembler_ret_code = reassembler_add_chunk(&reassembler, chunks[j].buffer, chunks[j].length);
        DEBUG_PRINTF("reassembler_ret_code is %d\n", reassembler_ret_code);
        if (reassembler_ret_code == REASSEMBLER_COMPLETE) {
			DEBUG_PRINTLN("Reassembly completed successfully!");
		}
	}

	DEBUG_PRINTF("Length: %u\n", reassembler.current_size);
	DEBUG_PRINTF("Data:\n");
	print_chunk_hex(reassembler.data_buffer, reassembler.current_size);
    uint32_t after_crc32 = calculate_crc(reassembler.data_buffer, reassembler.current_size);
    DEBUG_PRINTF("CRC-32: 0x%08X\n", after_crc32);
    if (original_crc32 == after_crc32) {
		DEBUG_PRINTLN("CRC-32 is valid!");
	} else {
		DEBUG_PRINTLN("CRC-32 NOT VALID!");
	}

    DEBUG_PRINTLN("* Step #4: Finished");
    for (int j=0; j<total_chunks; j++) {
        free(chunks[j].buffer);
    }
    free(message);
}

void loop() {
  // do nothing
}
