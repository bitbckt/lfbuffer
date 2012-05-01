#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

static inline int
round2(long index)
{
  index -= 1;
  index |= index >> 1;
  index |= index >> 2;
  index |= index >> 4;
  index |= index >> 8;
  index |= index >> 16;
  index |= index >> 32;

  return index + 1;
}

struct buffer_t *
buffer_init(long len)
{
  struct buffer_t *buf;

  buf = malloc(sizeof(struct buffer_t));

  buf->cursor = 0;
  buf->seq    = -1;
  buf->read   = -1;
  buf->size   = round2(len);
  buf->data   = calloc(buf->size, sizeof(struct slot_t));

  return buf;
}

void
buffer_destroy(struct buffer_t *buf)
{
  free(buf->data);
  free(buf);
  return;
}

struct slot_t *
buffer_claim(struct buffer_t *buf)
{
  long index, absolute, cursor;

  /*
   * FIXME: Add thread yield, back off, and dropped events.
   */
  for (;;) {
    for (;;) {
      cursor = buf->cursor;
      index  = buf->read & (buf->size - 1);

      if (index != (cursor & (buf->size - 1))) {
        break;
      }
    }

    /*
     * The slowest reader has finished reading _at least_
     * buf->cursor. A producer is free to claim it.
     */
    index = __sync_val_compare_and_swap(&buf->cursor, cursor, cursor + 1);

    if (index == cursor) {
      /* Got it! */
      break;
    } /* ... another producer got it. */
  }

  /*
   * This thread has exclusive ownership of index, therefore writes
   * to buf->data[absolute] need no synchronization.
   */
  absolute = index & (buf->size - 1);
  buf->data[absolute].index = index;

  return &buf->data[absolute];
}

void
buffer_commit(struct buffer_t *buf, const struct slot_t *slot)
{
  /*
   * FIXME: Add thread yield, back off, and dropped events.
   */
  while (buf->seq < (slot->index - 1)) { }

  /*
   * At this point, no other thread may commit a slot at index
   * greater than slot->index until this store.
   */
  buf->seq = slot->index;
  return;
}

/*
 * Consumers which have buffer_read() up to index N call this function
 * with N + 1.
 */
long
buffer_poll(struct buffer_t *buf, long index)
{
  /*
   * FIXME: Add thread yield and/or back off.
   */
  while (buf->seq < index) { }

  /*
   * The lowest committed index is >= index. Consumers may now read from
   * index to buf->seq.
   */
  return buf->seq;
}

const struct slot_t *
buffer_read(struct buffer_t *buf, long index)
{
  /*
   * buf->read should contain the lowest index read by all consumers.
   * FIXME: handle multiple consumers.
   */
  buf->read = index;
  index &= buf->size - 1;
  return &buf->data[index];
}
