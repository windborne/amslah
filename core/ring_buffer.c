#include "ring_buffer.h"

// Initialize or reset a buffer
void buffer_init(RingBuffer_t* buffer) { 
    buffer->write_index = 0;
    buffer->read_index = 0;
    buffer->is_full = false;
    for (uint32_t i=0; i < RING_BUFFER_CAPACITY; ++i) {
        buffer -> data[i] = 0;
    }
}

// Returns the number of bytes that were actually written.
// Stops writing if there would be a buffer overflow.
uint32_t write_to_buffer(RingBuffer_t* buffer, const char* source, uint32_t n_bytes) {
    if (buffer == NULL || source == NULL) {
        return 0;
    }

    uint32_t space_in_buffer = (RING_BUFFER_CAPACITY - buffer_length(buffer));

    if (n_bytes > space_in_buffer) {
        n_bytes = space_in_buffer;
    }

    uint32_t bytes_written = 0;
    for (bytes_written = 0; bytes_written < n_bytes; ++bytes_written) {
        buffer->data[buffer->write_index] = ((uint8_t *) source)[bytes_written];
        buffer->write_index = (buffer->write_index + 1) % RING_BUFFER_CAPACITY;
    }
    if (buffer->write_index == buffer->read_index && bytes_written > 0) {
        buffer->is_full = true;
    }
    return bytes_written;
}

// Read n bytes from the buffer.
// Returns the number of bytes actually read from the buffer.
uint32_t read_from_buffer(RingBuffer_t* buffer, void* destination, uint32_t n_bytes) {
    if (buffer == NULL || destination == NULL) {
        return 0;
    }
    if (n_bytes > buffer_length(buffer)) {
        n_bytes = buffer_length(buffer);
    }
    uint32_t bytes_read = 0;
    for (bytes_read = 0; bytes_read < n_bytes; ++bytes_read) {
        ((uint8_t *) destination)[bytes_read] = buffer->data[buffer->read_index];
        buffer->data[buffer->read_index] = 0;
        buffer->read_index = (buffer->read_index + 1) % RING_BUFFER_CAPACITY;
    }
    if (bytes_read > 0) {
        buffer->is_full = false;
    }
    return bytes_read;
}

// Get the number of bytes currently in the buffer
uint32_t buffer_length(RingBuffer_t* buffer) {
    if (buffer->is_full) {
        return RING_BUFFER_CAPACITY;
    }
    return (RING_BUFFER_CAPACITY + buffer->write_index - buffer->read_index) % RING_BUFFER_CAPACITY;
}

// Return the total buffer capacity
uint32_t buffer_capacity(RingBuffer_t* buffer) {
    return RING_BUFFER_CAPACITY;
}

// Write a single byte to the buffer; return false if the buffer was full.
bool write_byte_to_buffer(RingBuffer_t* buffer, uint8_t next_byte) {
    if (buffer == NULL) {
        return false;
    }
    if (buffer->is_full) {
        return false;
    }
    buffer->data[buffer->write_index] = next_byte;
    buffer->write_index = (buffer->write_index + 1) % RING_BUFFER_CAPACITY;
    if (buffer->write_index == buffer->read_index) {
        buffer->is_full = true;
    }
    return true;
}

// Read the next byte from the buffer; return 0 if the buffer is empty.
uint8_t read_next_from_buffer(RingBuffer_t* buffer) {
    if (buffer == NULL) {
        return 0;
    }
    if (buffer_length(buffer) == 0) {
        return 0;
    }
    uint8_t next_byte = buffer->data[buffer->read_index];
    buffer->data[buffer->read_index] = 0;
    buffer->read_index = (buffer->read_index + 1) % RING_BUFFER_CAPACITY;
    buffer->is_full = false;
    return next_byte;
}

// Return the position of the first occurence of the search byte
uint32_t search_first_byte(RingBuffer_t* buffer, uint8_t search_byte) {
    if (buffer == NULL) {
        return 0;
    }
    for (uint32_t i = 0; i < buffer_length(buffer); ++i) {
        if (buffer->data[(buffer->read_index + i) % RING_BUFFER_CAPACITY] == search_byte) {
            return i;
        }
    }
    return 0;
}

// Return the position of the first occurrence of the substring
uint32_t search_first_substring(RingBuffer_t* buffer, const char* search_string, uint32_t len) {
    if (buffer == NULL || search_string == NULL || buffer_length(buffer) < len) {
        return 0;
    }
    for (uint32_t i = 0; i < (buffer_length(buffer) - len); ++i) {
        for (uint32_t j = 0; j < len; ++j) {
            if (buffer->data[(buffer->read_index + i + j) % RING_BUFFER_CAPACITY] != ((uint8_t*) search_string)[j]) {
                break;
            }
            return i;
        }
    }
    return 0;
}

// Advance the buffer until the first occurence of the search byte
uint32_t flush_until_byte(RingBuffer_t* buffer, uint8_t search_byte) {
    if (buffer == NULL) {
        return 0;
    }
    uint32_t n_bytes = search_first_byte(buffer, search_byte);
    for (uint32_t bytes_flushed = 0; bytes_flushed < n_bytes; ++bytes_flushed) {
        buffer->data[buffer->read_index] = 0;
        buffer->read_index = (buffer->read_index + 1) % RING_BUFFER_CAPACITY;
    }
    return n_bytes;
}

// Flush up to and including the first occurrence of the substring, or not at all if not present.
// Alternatively, flush the specified number of bytes if search string is NULL.
uint32_t flush_through_substring(RingBuffer_t* buffer, const char* search_string, uint32_t len) {
    if (buffer == NULL) {
        return 0;
    }
    uint32_t n_bytes = 0;
    if (search_string != NULL) {
        n_bytes = search_first_substring(buffer, search_string, len) + len;
    } else {
        if (len > buffer_length(buffer)) {
            n_bytes = buffer_length(buffer);
        } else {
            n_bytes = len;
        }
    }
    for (uint32_t bytes_flushed = 0; bytes_flushed < n_bytes; ++bytes_flushed) {
        buffer->data[buffer->read_index] = 0;
        buffer->read_index = (buffer->read_index + 1) % RING_BUFFER_CAPACITY;
    }
    return n_bytes;
}

uint8_t peek_first(RingBuffer_t* buffer) {
    if (buffer == NULL) {
        return 0;
    }
    return buffer->data[buffer->read_index];
}
