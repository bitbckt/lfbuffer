#include "buffer.hpp"

template<typename T>
T* Buffer<T>::claim()
{
  long index, absolute, cur;

  /*
   * FIXME: Add thread yield, back off, and dropped events.
   */
  for (;;) {
    for (;;) {
      cur   = _cursor;
      index = _read & _mask;

      if (index != (cur & _mask)) {
        break;
      }
    }

    /*
     * The slowest reader has finished reading _at least_
     * buf->cursor. A producer is free to claim it.
     */
    index = __sync_val_compare_and_swap(&_cursor, cur, cur + 1);

    if (index == cur) {
      /* Got it! */
      break;
    } /* ... another producer got it. */
  }

  absolute = index & _mask;
  _state[absolute] = CLAIMED;
  _data[absolute].index = index;
  return &_data[absolute];
}

template<typename T>
void Buffer<T>::commit(T* slot)
{
  _state[slot->index & _mask] = COMMITTED;
  return;
}

template<typename T>
T const* Buffer<T>::get(long index)
{
  long i = index & _mask;
  volatile State* state = &_state[i];

  while (*state != COMMITTED) { }
  *state = CLAIMED;

  _read = index;
  return &_data[i];
}

template<typename T>
void Buffer<T>::release(T const* slot)
{
  _state[slot->index & _mask] = FREE;
  return;
}

template class Buffer<slot_t>;
