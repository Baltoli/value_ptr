/**
 * Implementation of a smart pointer with value semantics.
 *
 * The implementation strategy taken by this library is to type-erase the
 * polymorphic stored objects using a templated inner class that retains the
 * "real" type of the stored object so that its copy constructor can be called.
 *
 * Users of this library don't need to worry about these details - the interface
 * of the smart pointer is well-documented and hides implementation-specific
 * details.
 */
#pragma once

#include <functional>
#include <memory>
#include <type_traits>

namespace bsc {

/**
 * Smart pointer class with value semantics.
 */
template <typename T>
class value_ptr {
public:
  /**
   * Typedef to the raw pointer type equivalent to this class.
   */
  using pointer = T*;
  using element_type = T;

  template <typename U>
  friend class value_ptr;

private:
  struct pmr_concept {
    virtual ~pmr_concept() {}

    virtual pmr_concept* clone() = 0;
    virtual T* get() noexcept = 0;
    virtual T& operator*() noexcept = 0;
    virtual T* release() noexcept = 0;
  };

  template <typename D>
  struct pmr_model : pmr_concept {
    pmr_model(D* ptr) noexcept
        : ptr_(ptr)
    {
    }

    ~pmr_model() noexcept(std::is_nothrow_destructible<D>::value)
    {
      if (ptr_) {
        delete ptr_;
      }
    }

    // noexcept only if the underlying type can be copied without throwing.
    pmr_model<D>* clone() noexcept(
        std::is_nothrow_copy_constructible<D>::value) override
    {
      return new pmr_model<D>(new D(*ptr_));
    }

    D* get() noexcept override { return ptr_; }

    D& operator*() noexcept override { return *ptr_; }

    D* release() noexcept override
    {
      auto ptr = ptr_;
      ptr_ = nullptr;
      return ptr;
    }

    D* ptr_;
  };

public:
  /**
   * Construct a value_ptr from an underlying raw pointer.
   *
   * This constructor takes ownership of the pointer passed to it.
   */
  template <typename U,
      typename
      = typename std::enable_if<std::is_convertible<U*, pointer>::value>>
  explicit value_ptr(U* ptr) noexcept
      : impl_(new pmr_model<U>(ptr))
  {
  }

  /**
   * Construct a value_ptr from another value_ptr.
   */
  template <typename U,
      typename = typename std::enable_if<
          std::is_convertible<typename value_ptr<U>::pointer, pointer>::value
          && !std::is_same<T, U>::value>::type>
  value_ptr(value_ptr<U> const& other) noexcept(
      std::is_nothrow_copy_constructible<U>::value)
  {
    auto clone = other.impl_->clone();
    impl_ = new pmr_model<U>(clone->release());
    delete clone;
  }

  /**
   * Construct a value_ptr from nullptr.
   */
  constexpr value_ptr(std::nullptr_t) noexcept
      : impl_(nullptr)
  {
  }

  /**
   * Default-constructing a value_ptr is equivalent to constructing with
   * nullptr.
   */
  constexpr value_ptr() noexcept
      : value_ptr(nullptr)
  {
  }

  value_ptr(value_ptr<T> const& other) noexcept(
      std::is_nothrow_copy_constructible<T>::value)
      : impl_(other.impl_ ? other.impl_->clone() : nullptr)
  {
  }

  value_ptr<T>& operator=(value_ptr<T> other) noexcept(
      std::is_nothrow_copy_constructible<T>::value)
  {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  value_ptr(value_ptr<T>&& other) noexcept
      : impl_(std::move(other.impl_))
  {
    other.impl_ = nullptr;
  }

  value_ptr<T>& operator=(std::nullptr_t) noexcept(
      noexcept(std::declval<value_ptr<T>>().reset()))
  {
    reset();
    return *this;
  }

  /**
   * Destroys the stored value if it exists.
   */
  ~value_ptr() noexcept(std::is_nothrow_destructible<T>::value)
  {
    if (impl_) {
      delete impl_;
    }
  }

  /*
   * Get the underlying raw pointer.
   */
  T* get() const noexcept { return impl_->get(); }

  /*
   * Arrow operator returns the underlying raw pointer for chaining.
   */
  T* operator->() const noexcept { return impl_->get(); }

  /*
   * Dereferences the underlying raw pointer.
   */
  T& operator*() const noexcept { return **impl_; }

  /*
   * Conversion to bool (true if an underlying raw pointer is stored, false
   * otherwise).
   */
  explicit operator bool() const noexcept { return static_cast<bool>(impl_); }

  /**
   * Get the underlying raw pointer and release ownership.
   *
   * After calling, this object will be in a reset state (i.e. modelling a null
   * pointer). The returned pointer is no longer owned by this object and must
   * be managed by the caller.
   */
  T* release() noexcept
  {
    auto ptr = impl_->release();
    delete impl_;
    impl_ = nullptr;
    return ptr;
  }

  /**
   * Modify the underlying object.
   *
   * A call to reset while this object is managing an object will cause the
   * managed object to be destroyed. After calling, this object will manage ptr.
   */
  template <typename U>
  void reset(U* ptr) noexcept(std::is_nothrow_destructible<T>::value&&
          std::is_nothrow_copy_constructible<U>::value)
  {
    delete impl_;

    if (ptr) {
      impl_ = new pmr_model<U>(ptr);
    } else {
      impl_ = nullptr;
    }
  }

  void reset(std::nullptr_t = nullptr) noexcept(
      std::is_nothrow_destructible<T>::value)
  {
    delete impl_;
    impl_ = nullptr;
  }

  /**
   * Specialization to enable ADL swap.
   */
  void swap(value_ptr<T>& other) noexcept
  {
    using std::swap;
    swap(impl_, other.impl_);
  }

  /**
   * Get a uniquely owning pointer to the managed object.
   *
   * After calling, this object will be reset as if release had been called.
   */
  std::unique_ptr<T> to_unique() noexcept
  {
    return std::unique_ptr<T>(release());
  }

protected:
  pmr_concept* impl_;
};

template <typename T1, typename T2>
bool operator==(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  return a.get() == b.get();
}

template <typename T1, typename T2>
bool operator!=(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  return a.get() != b.get();
}

template <typename T1, typename T2>
bool operator<(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  using CT = typename std::common_type<typename value_ptr<T1>::pointer,
      typename value_ptr<T2>::pointer>::type;
  return std::less<CT>()(a.get(), b.get());
}

template <typename T1, typename T2>
bool operator<=(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  return !(b < a);
}

template <typename T1, typename T2>
bool operator>(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  return b < a;
}

template <typename T1, typename T2>
bool operator>=(value_ptr<T1> const& a, value_ptr<T2> const& b) noexcept
{
  return !(a < b);
}

template <typename T>
bool operator==(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return !a;
}

template <typename T>
bool operator==(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return !a;
}

template <typename T>
bool operator!=(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return (bool)a;
}

template <typename T>
bool operator!=(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return (bool)a;
}

template <typename T>
bool operator<(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return std::less<typename value_ptr<T>::pointer>()(a.get(), nullptr);
}

template <typename T>
bool operator<(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return std::less<typename value_ptr<T>::pointer>()(nullptr, a.get());
}

template <typename T>
bool operator<=(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return !(nullptr < a);
}

template <typename T>
bool operator<=(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return !(a < nullptr);
}

template <typename T>
bool operator>(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return nullptr < a;
}

template <typename T>
bool operator>(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return a < nullptr;
}

template <typename T>
bool operator>=(value_ptr<T> const& a, std::nullptr_t) noexcept
{
  return !(a < nullptr);
}

template <typename T>
bool operator>=(std::nullptr_t, value_ptr<T> const& a) noexcept
{
  return !(nullptr < a);
}

template <typename T>
void swap(value_ptr<T>& a, value_ptr<T>& b) noexcept
{
  a.swap(b);
}

template <typename T, typename... Args>
value_ptr<T> make_val(Args&&... args)
{
  return value_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename Base, typename Derived, typename... Args>
typename std::enable_if<std::is_base_of<Base, Derived>::value,
    value_ptr<Base>>::type
make_derived_val(Args&&... args)
{
  return value_ptr<Base>(new Derived(std::forward<Args>(args)...));
}

} // namespace bsc

namespace std {

template <typename T>
struct hash<bsc::value_ptr<T>> {
  std::size_t operator()(bsc::value_ptr<T> const& ptr) const
  {
    return std::hash<typename bsc::value_ptr<T>::pointer>()(ptr.get());
  }
};

} // namespace std
