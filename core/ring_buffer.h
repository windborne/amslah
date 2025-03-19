// Ring buffer implementation for UART processing

#pragma once

#include "amslah.h"
#include <stdint.h>

#define RING_BUFFER_CAPACITY (256)

// Got a compile time error here? Buffer capacity should be a power of 2.
typedef char assert_failed[((RING_BUFFER_CAPACITY & (RING_BUFFER_CAPACITY - 1)) == 0) ? 1 : -1];

typedef struct RingBuffer {
    uint8_t data[RING_BUFFER_CAPACITY];
    uint32_t write_index;
    uint32_t read_index;
    bool is_full;
} RingBuffer_t;

void buffer_init(RingBuffer_t* buffer);
uint32_t write_to_buffer(RingBuffer_t* buffer, const char* source, uint32_t n_bytes);
uint32_t read_from_buffer(RingBuffer_t* buffer, void* destination, uint32_t n_bytes);
uint32_t buffer_length(RingBuffer_t* buffer);
uint32_t buffer_capacity(RingBuffer_t* buffer);
uint8_t read_next_from_buffer(RingBuffer_t* buffer);
bool write_byte_to_buffer(RingBuffer_t* buffer, uint8_t next_byte);


uint32_t search_first_byte(RingBuffer_t* buffer, uint8_t search_byte);
uint32_t flush_until_byte(RingBuffer_t* buffer, uint8_t search_byte);
uint32_t flush_through_substring(RingBuffer_t* buffer, const char* search_string, uint32_t len);
uint32_t search_first_substring(RingBuffer_t* buffer, const char* search_string, uint32_t len);
uint8_t peek_first(RingBuffer_t* buffer);
