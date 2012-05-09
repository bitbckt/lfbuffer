#pragma once

#define LINE_PADDING 64 - sizeof(long)

enum state_t {
  FREE,
  CLAIMED,
  COMMITTED
};

struct slot_t {
  long  index;
  void *data;
};

struct buffer_t {
  volatile long cursor; /* next claimable index */

  char res1[LINE_PADDING];

  volatile long read;   /* lowest read point (slowest consumer) */

  char res3[LINE_PADDING];

  long           size;   /* size of the buffer */
  long           mask;   /* bitmask for cheap % */
  long          *state;  /* slot states */
  struct slot_t *data;   /* ... data */
};

struct buffer_t *buffer_init(long len);
void buffer_destroy(struct buffer_t *buf);

struct slot_t *buffer_claim(struct buffer_t *buf);
void buffer_commit(struct buffer_t *buf, const struct slot_t *slot);

const struct slot_t *buffer_read(struct buffer_t *buf, long index);
void buffer_return(struct buffer_t *buf, const struct slot_t *slot);
