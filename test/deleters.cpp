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

struct nrc_delete {
  nrc_delete() = default;

  void operator()(nrc *n) const { n->c_--; }
};
// clang-format on

TEST_CASE("counts don't get reset by default")
{
  int count = 0;
  {
    auto v = make_val<nrc>(count);
  }
  REQUIRE(count == 1);

  auto v2 = make_val<nrc>(count);
  REQUIRE(count == 2);
}

TEST_CASE("counts get reset if using a custom deleter")
{
  int count = 0;
  {
    auto v = value_ptr<nrc, nrc_delete>(new nrc(count));
    REQUIRE(count == 1);

    auto v2 = v;
    REQUIRE(count == 2);

    auto v3 = std::move(v2);
    REQUIRE(count == 2);
  }
  REQUIRE(count == 0);
}
