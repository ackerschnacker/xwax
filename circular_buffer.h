#ifndef CIRCULAR_BUFFER_H

#define CIRCULAR_BUFFER_H

#include <stddef.h>

struct circular_buffer {
	ptrdiff_t write_ptr;
	ptrdiff_t read_ptr;
	int array[10];
	size_t size;
};

int cbuf_init(struct circular_buffer *cb);
void cbuf_destroy(struct circular_buffer *cb);
void cbuf_element_write(struct circular_buffer *cb, const ptrdiff_t i, const int item);
int *cbuf_element_read(struct circular_buffer *cb, const ptrdiff_t i);
void cbuf_writeptr_inc(struct circular_buffer *cb);
void cbuf_writeptr_dec(struct circular_buffer *cb);
void cbuf_readptr_inc(struct circular_buffer *cb);
void cbuf_readptr_dec(struct circular_buffer *cb);
void cbuf_push(struct circular_buffer *cb, const int item);
void cbuf_print(struct circular_buffer *cb);

#endif /* end of include guard CIRCULAR_BUFFER_H */
