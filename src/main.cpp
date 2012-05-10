#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.hpp"

#define NUM_PRODUCERS 4
#define BUFFER_SIZE 8*1024
#define ITERATIONS 1000L * 1000L * 1000L

struct producer_t {
  int iterations;
  int id;
  Buffer<slot_t>* buf;
};

void *
consumer(void *b)
{
  Buffer<slot_t> *buf;
  const slot_t *slot;
  long curr = 0;

  buf = (Buffer<slot_t> *)b;

  for (;;) {
    slot = buf->get(curr);
    assert(slot->index == curr);
    buf->release(slot);

    curr++;
    if (curr == ITERATIONS) {
      return NULL;
    }
  }

  /* Shouldn't get here. */
  assert(NULL);
  return NULL;
}

void *
producer(void *p)
{
  producer_t* producer;
  slot_t*     slot;
  long        iterations;

  producer = (producer_t *)p;

  for (iterations = producer->iterations; iterations; iterations--) {
    slot = producer->buf->claim();
    assert(slot);
    producer->buf->commit(slot);
  }

  return NULL;
}

int
main(int argc, char *argv[])
{
  Buffer<slot_t> buf(BUFFER_SIZE);
  producer_t producers[NUM_PRODUCERS];
  pthread_t threads[NUM_PRODUCERS + 1];

  int i, ret;

  for (i = 0; i < NUM_PRODUCERS; i++) {
    producers[i].iterations = ITERATIONS / NUM_PRODUCERS;
    producers[i].buf = &buf;
    producers[i].id = i;

    ret = pthread_create(&threads[i], NULL, producer, &producers[i]);
    if (ret != 0) {
      fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
      exit(EXIT_FAILURE);
    }
  }

  ret = pthread_create(&threads[NUM_PRODUCERS], NULL, consumer, &buf);
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

  return EXIT_SUCCESS;
}
