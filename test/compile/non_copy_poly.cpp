#include <value_ptr/value_ptr.h>

using namespace bsc;

struct S {
  S() = default;
  S(S const&) = default;
};

struct T : S {
  T() = default;
  T(T const&) = delete;
};

void f()
{
  auto ptr = value_ptr<S>(new S());
  auto p2 = ptr;
  p2.reset(new T());
  auto p3 = p2;
}
