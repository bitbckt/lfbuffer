#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#define NUM_PRODUCERS 4
#define BUFFER_SIZE 8*1024
#define ITERATIONS 1000L * 1000L * 1000L

struct producer_t {
  int iterations;
  int id;
  struct buffer_t *buf;
};

void *
consumer(void *b)
{
  struct buffer_t *buf;
  const struct slot_t *slot;
  long max, i;
  long curr = 0;

  buf = (struct buffer_t *)b;

  for (;;) {
    max = buffer_poll(buf, curr);
    assert(max >= curr);

    for (i = curr; i <= max; i++) {
      slot = buffer_read(buf, curr);
      assert(slot->index == curr);

      curr++;

      if (curr == ITERATIONS) {
        return NULL;
      }
    }
  }

  /* Shouldn't get here. */
  assert(NULL);
  return NULL;
}

void *
producer(void *p)
{
  struct producer_t *producer;
  struct slot_t     *slot;
  long iterations;

  producer = (struct producer_t *)p;

  for (iterations = producer->iterations; iterations; iterations--) {
    slot = buffer_claim(producer->buf);
    assert(slot);
    buffer_commit(producer->buf, slot);
  }

  return NULL;
}

int
main(int argc, char *argv[])
{
  struct buffer_t  *buf;
  struct producer_t producers[NUM_PRODUCERS];
  pthread_t threads[NUM_PRODUCERS + 1];

  int i, ret;

  buf = buffer_init(BUFFER_SIZE);

  for (i = 0; i < NUM_PRODUCERS; i++) {
    producers[i].iterations = ITERATIONS / NUM_PRODUCERS;
    producers[i].buf = buf;
    producers[i].id = i;

    ret = pthread_create(&threads[i], NULL, producer, &producers[i]);
    if (ret != 0) {
      fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
      exit(EXIT_FAILURE);
    }
  }

  ret = pthread_create(&threads[NUM_PRODUCERS], NULL, consumer, buf);
  if (ret != 0) {
    fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < NUM_PRODUCERS + 1; i++) {
    ret = pthread_join(threads[i], NULL);
    if (ret != 0) {
      fprintf(stderr, "pthread_join(): %s\n", strerror(ret));
      exit(EXIT_FAILURE);
    }
  }

  buffer_destroy(buf);
  return EXIT_SUCCESS;
}
