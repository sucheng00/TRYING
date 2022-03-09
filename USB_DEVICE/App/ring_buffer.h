#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint8_t *buf;
	size_t size;
	size_t begin;
	size_t end;
} ringbuf_t;

/**
 * Initializes a ring buffer with a pre-allocated memory area.
 * Capacity of the ring buffer = memory size - 1.
 * @param rb the ring buffer to initialize
 * @param buf the address of the pre-allocated memory
 * @param buf_size the size of the memory, must be larger than 1
 */
void ringbuf_init(ringbuf_t *rb, uint8_t *buf, size_t buf_size);

/**
 * Discards all data in the ring buffer.
 */
void ringbuf_reset(ringbuf_t *rb);

/**
 * Calculates the used space size of the ring buffer.
 */
size_t ringbuf_bytes_used(const ringbuf_t *rb);

/**
 * Calculates the free space size of the ring buffer.
 */
size_t ringbuf_bytes_free(const ringbuf_t *rb);

/**
 * Gets the capacity of the ring buffer.
 */
size_t ringbuf_capacity(const ringbuf_t *rb);

/**
 * Writes n bytes to the ring buffer.
 * Old data will be overwritten if there is no enough space in the ring buffer.
 * @returns the number of discarded bytes
 */
size_t ringbuf_write(ringbuf_t *rb, const uint8_t *from, size_t n);

/**
 * Reads n bytes from the ring buffer.
 * Fewer than n bytes is read if there is no enough data in the ring buffer.
 * @returns the number of bytes read
 */
size_t ringbuf_read(ringbuf_t *rb, uint8_t *to, size_t n);
