#pragma once

#define LINE_PADDING 64 - sizeof(long)

struct slot_t {
  long  index;
  void *data;
};

struct buffer_t {
  volatile long cursor; /* next claimable index */

  char res1[LINE_PADDING];

  volatile long seq;    /* last committed index */

  char res2[LINE_PADDING];

  volatile long read;   /* lowest read point (slowest consumer) */

  char res3[LINE_PADDING];

  long           size;   /* size of the buffer */
  long           mask;   /* bitmask for cheap % */
  struct slot_t *data;   /* ... data */
};

struct buffer_t *buffer_init(long len);
void buffer_destroy(struct buffer_t *buf);

struct slot_t *buffer_claim(struct buffer_t *buf);
void buffer_commit(struct buffer_t *buf, const struct slot_t *slot);

long buffer_poll(struct buffer_t *buf, long next);
const struct slot_t *buffer_read(struct buffer_t *buf, long index);
