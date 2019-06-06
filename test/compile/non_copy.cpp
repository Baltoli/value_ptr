#include <value_ptr/value_ptr.h>

using namespace bsc;

struct S {
  S() = default;
  S(S const&) = delete;
};

void f() { auto ptr = value_ptr<S>(new S()); }
