#include "ring_buffer.h"
#include <string.h>
#include <sys/param.h>

void ringbuf_init(ringbuf_t *rb, uint8_t *buf, size_t buf_size) {
	rb->buf = buf;
	rb->size = buf_size;
	rb->begin = rb->end = 0;
}

void ringbuf_reset(ringbuf_t *rb) {
	rb->begin = rb->end = 0;
}

size_t ringbuf_bytes_used(const ringbuf_t *rb) {
	if (rb->begin <= rb->end) {
		return rb->end - rb->begin;
	} else {
		return rb->size - rb->begin + rb->end;
	}
}

size_t ringbuf_bytes_free(const ringbuf_t *rb) {
	if (rb->begin <= rb->end) {
		return rb->size - rb->end + rb->begin - 1;
	} else {
		return rb->begin - rb->end - 1;
	}
}

size_t ringbuf_capacity(const ringbuf_t *rb) {
	return rb->size - 1;
}

size_t ringbuf_write(ringbuf_t *rb, const uint8_t *from, size_t n) {
	size_t free_size = ringbuf_bytes_free(rb);
	size_t overwritten = free_size >= n ? 0 : n - free_size;

	size_t written;
	while (n > 0) {
		written = MIN(n, rb->size - rb->end);
		memcpy(rb->buf + rb->end, from, written);
		from += written;
		n -= written;
		rb->end += written;
		if (rb->end == rb->size)
			rb->end = 0;
	}

	if (overwritten > 0) {
		if (rb->end == rb->size - 1)
			rb->begin = 0;
		else
			rb->begin = rb->end + 1;
	}
	return overwritten;
}

size_t ringbuf_read(ringbuf_t *rb, uint8_t *to, size_t n) {
	size_t available = ringbuf_bytes_used(rb);
	size_t read;
	if (rb->begin > rb->end) {
		read = MIN(rb->size - rb->begin, n);
		memcpy(to, rb->buf + rb->begin, read);
		rb->begin += read;
		if (rb->begin == rb->size)
			rb->begin = 0;
		if (read == n)
			return read;
		to += read;
		n -= read;
	}
	read = MIN(rb->end - rb->begin, n);
	memcpy(to, rb->buf + rb->begin, read);
	rb->begin += read;
	return MIN(available, n);
}
