#include "catch.hpp"

#include <value_ptr/value_ptr.h>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace bsc;

// clang-format off
struct rc {
  rc(int& c) : c_(c) { ++c_; }
  rc(rc const& o) : rc(o.c_) {}
  ~rc() { --c_; }

  int& c_;
};
// clang-format on

TEST_CASE("value_ptr manages scoped lifetimes")
{
  auto count = 0;

  {
    auto v = value_ptr<rc>(new rc(count));
    REQUIRE(count == 1);

    auto v2 = value_ptr<rc>(new rc(count));
    REQUIRE(count == 2);
  }

  REQUIRE(count == 0);
}

TEST_CASE("value_ptr can hold nullptr")
{
  auto v = value_ptr<int>();
  REQUIRE(!v);

  auto v2 = value_ptr<int>(nullptr);
  REQUIRE(!v2);

  int c = 0;
  auto v3 = value_ptr<rc>(new rc(c));
  REQUIRE(c == 1);
  REQUIRE(!!v3);

  v3 = nullptr;
  REQUIRE(c == 0);
  REQUIRE(!v3);
}

TEST_CASE("value_ptr holds a pointer")
{
  SECTION("with get")
  {
    auto v = value_ptr<int>(new int(3));
    int* ptr = v.get();
    REQUIRE((*ptr = 3));
  }
}

TEST_CASE("value_ptr can be copied")
{
  SECTION("stored values are copied")
  {
    auto count = 0;

    {
      auto v = value_ptr<rc>(new rc(count));
      REQUIRE(count == 1);

      auto v2(v);
      REQUIRE(count == 2);
    }

    REQUIRE(count == 0);
  }

  SECTION("self assignment works as expected")
  {
    auto count = 0;

    auto v = value_ptr<rc>(new rc(count));
    REQUIRE(count == 1);

    v = v;
    REQUIRE(count == 1);
  }

  SECTION("stored pointers are different")
  {
    auto v = value_ptr<int>(new int(3));
    auto v2 = v;

    REQUIRE(v.get() != v2.get());
    REQUIRE(*v.get() == *v2.get());
  }
}

TEST_CASE("value_ptr can be moved")
{
  SECTION("no copies are made")
  {
    auto count = 0;

    {
      auto v = value_ptr<rc>(new rc(count));
      REQUIRE(count == 1);

      auto v2 = std::move(v);
      REQUIRE(count == 1);
      REQUIRE(!v);
    }

    REQUIRE(count == 0);
  }

  SECTION("data is moved correctly")
  {
    auto v = value_ptr<int>(new int(65));
    REQUIRE(*v == 65);

    auto v2(std::move(v));
    REQUIRE(*v2 == 65);
    REQUIRE(!v);
  }

  SECTION("self assignment works")
  {
    auto count = 0;

    auto v = value_ptr<rc>(new rc(count));
    REQUIRE(count == 1);

    v = std::move(v);
    REQUIRE(!!v);
    REQUIRE(count == 1);
  }
}

struct S {
  virtual ~S() {}
  virtual int value() { return 33; }
};

struct T : S {
  int value() override { return 89; }
};

TEST_CASE("value ptr behaves polymorphically")
{
  auto v = value_ptr<S>(new T());
  REQUIRE(v->value() == 89);

  auto v2 = v;
  REQUIRE(v2->value() == 89);

  v = value_ptr<S>(new S());
  REQUIRE(v->value() == 33);
}

TEST_CASE("managed pointer can be released")
{
  auto count = 0;
  rc* ptr;

  {
    auto v = value_ptr<rc>(new rc(count));
    REQUIRE(count == 1);

    ptr = v.release();
  }

  REQUIRE(count == 1);
  delete ptr;
  REQUIRE(count == 0);
}

TEST_CASE("managed pointer can be reset")
{
  auto count = 0;
  auto v = value_ptr<rc>(new rc(count));

  REQUIRE(count == 1);
  v.reset();
  REQUIRE(count == 0);

  v.reset(new rc(count));
  REQUIRE(count == 1);

  v.reset(new rc(count));
  REQUIRE(count == 1);
}

TEST_CASE("pointers can be hashed")
{
  auto ptr = value_ptr<int>(new int(54));
  REQUIRE(std::hash<decltype(ptr)>()(ptr) == std::hash<int*>()(ptr.get()));
}

TEST_CASE("value pointers can go into standard containers")
{
  SECTION("vectors")
  {
    auto v1 = std::vector<value_ptr<int>>{};
    v1.emplace_back(new int(34));
    v1.emplace_back(new int(78));
    v1.emplace_back(new int(-89));

    REQUIRE(*v1.at(0) == 34);
    REQUIRE(*v1.at(1) == 78);
    REQUIRE(*v1.at(2) == -89);

    auto count = 0;
    {
      auto v2 = std::vector<value_ptr<rc>>{};
      v2.emplace_back(new rc(count));
      v2.emplace_back(new rc(count));
      v2.emplace_back(new rc(count));
      REQUIRE(count == 3);
    }
    REQUIRE(count == 0);
  }

  SECTION("sets")
  {
    auto v1 = std::set<value_ptr<int>>{};
    v1.emplace(new int(34));
    v1.emplace(new int(78));
    v1.emplace(new int(-89));

    REQUIRE(v1.size() == 3);

    auto contains = [](int i) {
      return [i](value_ptr<int> const& vp) { return *vp == i; };
    };

    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(34)) != v1.end());
    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(78)) != v1.end());
    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(-89)) != v1.end());

    auto count = 0;
    {
      auto v2 = std::set<value_ptr<rc>>{};
      v2.emplace(new rc(count));
      v2.emplace(new rc(count));
      v2.emplace(new rc(count));
      REQUIRE(count == 3);
    }
    REQUIRE(count == 0);
  }

  SECTION("unordered sets")
  {

    auto v1 = std::unordered_set<value_ptr<int>>{};
    v1.emplace(new int(34));
    v1.emplace(new int(78));
    v1.emplace(new int(-89));

    REQUIRE(v1.size() == 3);

    auto contains = [](int i) {
      return [i](value_ptr<int> const& vp) { return *vp == i; };
    };

    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(34)) != v1.end());
    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(78)) != v1.end());
    REQUIRE(std::find_if(v1.begin(), v1.end(), contains(-89)) != v1.end());

    auto count = 0;
    {
      auto v2 = std::unordered_set<value_ptr<rc>>{};
      v2.emplace(new rc(count));
      v2.emplace(new rc(count));
      v2.emplace(new rc(count));
      REQUIRE(count == 3);
    }
    REQUIRE(count == 0);
  }

  SECTION("maps")
  {
    auto m1 = std::map<value_ptr<int>, int>{};
    m1.emplace(value_ptr<int>(new int(4)), 5);
    m1.emplace(value_ptr<int>(new int(42)), 43);
    m1.emplace(value_ptr<int>(new int(-1)), 0);

    for (auto const& pair : m1) {
      auto const& k = pair.first;
      auto const& v = pair.second;

      REQUIRE(*k == v - 1);
    }

    auto count = 0;
    {
      auto m2 = std::map<value_ptr<rc>, int>{};
      m2.emplace(value_ptr<rc>(new rc(count)), 0);
      m2.emplace(value_ptr<rc>(new rc(count)), 1);
      REQUIRE(count == 2);
    }
    REQUIRE(count == 0);
  }

  SECTION("unordered maps")
  {
    auto m1 = std::unordered_map<value_ptr<int>, int>{};
    m1.emplace(value_ptr<int>(new int(4)), 5);
    m1.emplace(value_ptr<int>(new int(42)), 43);
    m1.emplace(value_ptr<int>(new int(-1)), 0);

    for (auto const& pair : m1) {
      auto const& k = pair.first;
      auto const& v = pair.second;

      REQUIRE(*k == v - 1);
    }

    auto count = 0;
    {
      auto m2 = std::map<value_ptr<rc>, int>{};
      m2.emplace(value_ptr<rc>(new rc(count)), 0);
      m2.emplace(value_ptr<rc>(new rc(count)), 1);
      REQUIRE(count == 2);
    }
    REQUIRE(count == 0);
  }
}

TEST_CASE("value_ptr can be constructed from compatible pointers")
{
  auto vp = value_ptr<T>(new T());
  auto vp2 = value_ptr<S>(vp);

  REQUIRE(vp2->value() == 89);
}

TEST_CASE("make_val can be used")
{
  SECTION("vals are propagated")
  {
    auto vp = make_val<int>(4);
    REQUIRE(*vp == 4);
  }

  SECTION("lifetimes work properly")
  {
    int count = 0;
    {
      auto vp = make_val<rc>(count);
      REQUIRE(count == 1);

      auto vp2 = make_val<rc>(count);
      REQUIRE(count == 2);

      make_val<rc>(count);
      REQUIRE(count == 2);
    }
    REQUIRE(count == 0);
  }
}

struct Base {
  virtual ~Base() = default;
  virtual int val() { return 0; }
};

struct Derived : Base {
  Derived(int v)
      : v_(v)
  {
  }

  Derived(Derived const& od)
      : v_(od.v_ + 1)
  {
  }

  int val() override { return v_; }
  int v_;
};

TEST_CASE("make_derived_val can be used")
{
  SECTION("values are propagated")
  {
    auto vp = make_derived_val<Base, Derived>(34);
    REQUIRE(vp->val() == 34);
  }

  SECTION("derived copies are made")
  {
    auto vp = make_derived_val<Base, Derived>(22);
    REQUIRE(vp->val() == 22);

    auto vp2 = vp;
    REQUIRE(vp->val() == 22);
    REQUIRE(vp2->val() == 23);
  }
}

TEST_CASE("can convert to unique_pointer")
{
  SECTION("values are propagated")
  {
    auto vp = make_val<int>(5);
    auto up = vp.to_unique();

    REQUIRE(*up == 5);
    REQUIRE(!vp);
  }

  SECTION("lifetimes work properly")
  {
    auto count = 0;
    {
      auto vp = make_val<rc>(count);
      REQUIRE(count == 1);

      auto up = vp.to_unique();
      REQUIRE(!vp);
      REQUIRE(up);
      REQUIRE(count == 1);
    }
    REQUIRE(count == 0);
  }
}
