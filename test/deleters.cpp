#include "catch.hpp"

#include <value_ptr/value_ptr.h>

#include <iostream>

using namespace bsc;

// clang-format off
struct nrc {
  nrc(int& c) : c_(c) { ++c_; }
  nrc(nrc const& o) : nrc(o.c_) {}
  nrc(nrc&&) = delete;
  ~nrc() = default;

  int& c_;
};
// clang-format on

TEST_CASE("counts don't get reset by default")
{
  int count = 0;
  {
    auto v = value_ptr<nrc>(new nrc(count));
  }
  REQUIRE(count == 1);

  auto v2 = new value_ptr<nrc>(new nrc(count));
  REQUIRE(count == 2);
  delete v2;

  REQUIRE(count == 2);
}
