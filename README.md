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
using namespace bsc;
```

The pointer can be used like other smart pointers:
```c++
value_ptr<int> v_ptr(new int(2));
assert(*v_ptr == 2);
```

When a `value_ptr` is copied, the underlying object is copied. This behaviour is
different to a `shared_ptr`, which copies the ref-counted *pointer*.
```c++
value_ptr<int> v_ptr(new int(2));
auto v_ptr2 = v_ptr;
assert(*v_ptr == *v_ptr2);

*v_ptr = 3;
assert(*v_ptr != *v_ptr2);
```

A forwarding convenience function is defined:
```c++
value_ptr<int> v_ptr = make_val<int>(54);
```

When a `value_ptr` is constructed with a derived class, it can be used
polymorphically as the base class but will respect the copy constructor of the
derived class:
```c++
struct S {};
struct T : S {};

auto v_ptr = value_ptr<S>(new T());
auto v_ptr2 = v_ptr; // calls hypothetical T(const& T);
```

## Installation

* **Single Header**: Download `value_ptr.h` from this repository and place it on
  your include path.
