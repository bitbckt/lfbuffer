# Lock-free Buffer

This is a proof-of-concept of an N producer, single consumer ring
buffer in C.

# Support

The code has been tested only on the machine which produced it:

* Linux 3.2.0 amd64 (Debian)
* GCC 4.6.3
* GNU libc 2.13
* Intel Xeon W3530 (Bloomfield)

In particular, this code is sensitive to variations on memory bus
architecture, speculative CPU operations  and compiler
optimizations. I have removed what I believe to be unnecessary memory
barriers and limited CPU-specific atomic operations to a single CAS
(see buffer_claim()). YMMV.

# License

Copyright 2012 Brandon Mitchell

Licensed under the Apache License, Version 2.0: http://www.apache.org/licenses/LICENSE-2.0
