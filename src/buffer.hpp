#pragma once

struct slot_t {
  long index;
};

template<typename T>
class Buffer {
public:
  Buffer(long len): _cursor(0),
                    _read(-1)
  {
    _size   = round2(len);
    _mask   = _size - 1;
    _state  = new State[_size];
    _data   = new T[_size];
  }

  ~Buffer()
  {
    delete[] _state;
    delete[] _data;
  }

  T* claim();
  void commit(T* slot);

  T const* get(long index);
  void release(T const* slot);

private:
  enum State {
    FREE,
    CLAIMED,
    COMMITTED
  };

  volatile long _cursor; // next claimable index
  long res1, res2, res3, res4, res5, res6, res7; // cache line padding
  volatile long _read;   // least read index (slowest consumer)

  long   _size;
  long   _mask;
  State* _state;
  T*     _data;

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

};
