#include <assert.h>
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
  buf->read   = -1;
  buf->size   = round2(len);
  buf->mask   = buf->size - 1;
  buf->state  = calloc(buf->size, sizeof(long));
  buf->data   = calloc(buf->size, sizeof(struct slot_t));

  return buf;
}

void
buffer_destroy(struct buffer_t *buf)
{
  free(buf->state);
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
      index  = buf->read & buf->mask;

      if (index != (cursor & buf->mask)) {
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

  absolute = index & buf->mask;
  buf->data[absolute].index = index;

  assert(buf->state[absolute] == FREE);
  buf->state[absolute] = CLAIMED;

  return &buf->data[absolute];
}

void
buffer_commit(struct buffer_t *buf, const struct slot_t *slot)
{
  assert(buf->state[slot->index & buf->mask] == CLAIMED);

  buf->state[slot->index & buf->mask] = COMMITTED;
  return;
}

const struct slot_t *
buffer_read(struct buffer_t *buf, long index)
{
  long i  = index & buf->mask;
  volatile long *s = &buf->state[i];

  while (*s != COMMITTED) { }

  *s = CLAIMED;

  /*
   * buf->read should contain the lowest index read by all consumers.
   */
  buf->read = index;
  return &buf->data[i];
}

void
buffer_return(struct buffer_t *buf, const struct slot_t *slot)
{
  assert(buf->state[slot->index & buf->mask] == CLAIMED);

  buf->state[slot->index & buf->mask] = FREE;
  return;
}
