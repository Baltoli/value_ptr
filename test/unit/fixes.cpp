#include "catch.hpp"

#include <value_ptr/value_ptr.h>

using namespace bsc;

struct slice_base {
  virtual char f() const { return 'S'; }
  virtual ~slice_base() {}
};

struct slice : slice_base {
  char f() const override { return 'T'; }
};

TEST_CASE("#15: calling reset then copying slices objects")
{
  auto ptr = make_val<slice_base>();
  REQUIRE(ptr->f() == 'S');

  ptr.reset(new slice());
  REQUIRE(ptr->f() == 'T');

  auto p2 = ptr;
  REQUIRE(p2->f() == 'T');
}
