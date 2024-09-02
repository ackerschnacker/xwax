#include <stdio.h>
#include <stdlib.h>

#include "circular_buffer.h"

/* 
 * Reads an element from the circular buffer by specifying an index. 
 * Returns a pointer to the element relative i addresses from the read pointer.
 */
int *cbuf_element_read(struct circular_buffer *cbuf, const ptrdiff_t i)
{
	if (cbuf == NULL)
		return NULL;

	/* Wrap pointer if less than 0 */
	if (cbuf->read_ptr < 0)
		cbuf->read_ptr += cbuf->size;

	/* Calculate the relative index of the array */
	ptrdiff_t index = cbuf->read_ptr + i;

	/* Relative index must not exceed array size */
	if ((size_t)index >= cbuf->size)
		index -= cbuf->size;

	return &cbuf->array[index];
}

/* 
 * Writes an element to the circular buffer by specifying an index. 
 * The element will be placed i addresses relative from the write pointer. 
 */
void cbuf_element_write(struct circular_buffer *cbuf, const ptrdiff_t i, const int item)
{
	if (cbuf == NULL)
		return;

	if (cbuf->write_ptr < 0)
		cbuf->write_ptr += cbuf->size;

	ptrdiff_t index = cbuf->write_ptr + i;

	if ((size_t)index >= cbuf->size)
		index -= cbuf->size;

	cbuf->array[index] = item;
}

/* 
 * Increments the write pointer of the circular buffer
 */
void cbuf_writeptr_inc(struct circular_buffer *cbuf)
{
	if (cbuf == NULL)
		return;

	cbuf->write_ptr++;
	if (cbuf->write_ptr >= cbuf->size)
		cbuf->write_ptr -= cbuf->size;
}

/* 
 * Decrements the write pointer of the circular buffer
 */
void cbuf_writeptr_dec(struct circular_buffer *cbuf)
{
	if (cbuf == NULL)
		return;

	cbuf->write_ptr--;
	if (cbuf->write_ptr < 0)
		cbuf->write_ptr += cbuf->size;
}

/* 
 * Increments the read pointer of the circular buffer
 */
void cbuf_readptr_inc(struct circular_buffer *cbuf)
{
	if (cbuf == NULL)
		return;

	cbuf->read_ptr++;
	if (cbuf->read_ptr >= cbuf->size)
		cbuf->read_ptr -= cbuf->size;
}

/* 
 * Decrements the read pointer of the circular buffer
 */
void cbuf_readptr_dec(struct circular_buffer *cbuf)
{
	if (cbuf == NULL)
		return;

	cbuf->read_ptr--;
	if (cbuf->read_ptr < 0)
		cbuf->read_ptr += cbuf->size;
}

/*
 * Pushes an element into the circular buffer and increments the write pointer
 */
void cbuf_push(struct circular_buffer *cbuf, const int item)
{
	if (cbuf == NULL)
		return;

	cbuf->array[cbuf->write_ptr] = item;
	cbuf_writeptr_inc(cbuf);
}
