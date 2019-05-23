# value_ptr

[![Build Status](https://travis-ci.com/Baltoli/value_ptr.svg?branch=master)](https://travis-ci.com/Baltoli/value_ptr)

This library implements a smart pointer with value (deep copying) semantics.
When a value pointer is copied, the new smart pointer instance will point to a
new instance of the stored object. Similarly, the smart pointer will ensure that
the pointed-to object is destroyed when appropriate.

These semantics become useful when you need to store members of a polymorphic
hierarchy in a container, while ensuring that copies, destructors etc. are
respected.

## Example

```c++
value_ptr<int> v_ptr(new int(2));
value_ptr<int> v_ptr2 = v_ptr;

assert(v_ptr != v_ptr2); // stored pointers are different
*v_ptr2 = 3;
assert(*v_ptr != *v_ptr2); // changes are independent
```

## Installation

* **Single Header**: Download `value_ptr.h` from this repository and place it on
  your include path.
