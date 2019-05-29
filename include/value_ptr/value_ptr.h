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
template <typename T, typename Deleter = std::default_delete<T>>
class value_ptr {
public:
  /**
   * Typedef to the raw pointer type equivalent to this class.
   */
  using pointer = T*;

  template <typename, typename>
  friend class value_ptr;

private:
  Deleter deleter_;

  struct pmr_concept {
    virtual ~pmr_concept() {}

    virtual pmr_concept* clone() = 0;
    virtual T* get() = 0;
    virtual T& operator*() = 0;
    virtual T* release() = 0;
  };

  template <typename D>
  struct pmr_model : pmr_concept {
    pmr_model(D* ptr, Deleter& del)
        : ptr_(ptr)
        , model_deleter_(del)
    {
    }

    ~pmr_model()
    {
      if (ptr_) {
        model_deleter_(ptr_);
      }
    }

    pmr_model<D>* clone() override
    {
      return new pmr_model<D>(new D(*ptr_), model_deleter_);
    }

    D* get() override { return ptr_; }

    D& operator*() override { return *ptr_; }

    D* release() override
    {
      auto ptr = ptr_;
      ptr_ = nullptr;
      return ptr;
    }

    D* ptr_;
    Deleter& model_deleter_;
  };

public:
  /**
   * Construct a value_ptr from an underlying raw pointer.
   *
   * This constructor takes ownership of the pointer passed to it.
   */
  template <typename U,
      typename
      = typename std::enable_if<std::is_default_constructible<Deleter>::value
          && !std::is_pointer<Deleter>::value>::type>
  explicit value_ptr(U* ptr)
      : deleter_()
      , impl_(new pmr_model<U>(ptr, deleter_))
  {
  }

  /**
   * Construct a value_ptr from another value_ptr.
   */
  template <typename U, typename D,
      typename = typename std::enable_if<
          std::is_convertible<typename value_ptr<U, D>::pointer, pointer>::value
          && !std::is_same<T, U>::value
          && std::is_assignable<Deleter, D>::value>::type>
  value_ptr(value_ptr<U, D> const& other)
  {
    auto clone = other.impl_->clone();
    impl_ = new pmr_model<U>(clone->release(), deleter_);
    delete clone;
  }

  /**
   * Construct a value_ptr from nullptr.
   */
  template <typename
      = typename std::enable_if<std::is_default_constructible<Deleter>::value
          && !std::is_pointer<Deleter>::value>::type>
  value_ptr(std::nullptr_t)
      : deleter_()
      , impl_(nullptr)
  {
  }

  /**
   * Default-constructing a value_ptr is equivalent to constructing with
   * nullptr.
   */
  template <typename
      = typename std::enable_if<std::is_default_constructible<Deleter>::value
          && !std::is_pointer<Deleter>::value>::type>
  value_ptr()
      : value_ptr(nullptr)
  {
  }

  value_ptr(value_ptr const& other)
      : deleter_(other.deleter_)
      , impl_(other.impl_ ? other.impl_->clone() : nullptr)
  {
  }

  value_ptr& operator=(value_ptr other)
  {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  value_ptr(value_ptr&& other)
      : deleter_(std::move(other.deleter_))
      , impl_(std::move(other.impl_))
  {
    other.impl_ = nullptr;
  }

  value_ptr& operator=(std::nullptr_t)
  {
    reset();
    return *this;
  }

  /**
   * Destroys the stored value if it exists.
   */
  ~value_ptr()
  {
    if (impl_) {
      delete impl_;
    }
  }

  /*
   * Get the underlying raw pointer.
   */
  T* get() const { return impl_->get(); }

  /*
   * Arrow operator returns the underlying raw pointer for chaining.
   */
  T* operator->() const { return impl_->get(); }

  /*
   * Dereferences the underlying raw pointer.
   */
  T& operator*() const { return **impl_; }

  /*
   * Conversion to bool (true if an underlying raw pointer is stored, false
   * otherwise).
   */
  explicit operator bool() const { return static_cast<bool>(impl_); }

  /*
   * Get the deleter object responsible for deleting managed objects.
   */
  Deleter& get_deleter() { return deleter_; }
  Deleter const& get_deleter() const { return deleter_; }

  /**
   * Get the underlying raw pointer and release ownership.
   *
   * After calling, this object will be in a reset state (i.e. modelling a null
   * pointer). The returned pointer is no longer owned by this object and must
   * be managed by the caller.
   */
  T* release()
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
  void reset(T* ptr = nullptr)
  {
    if (impl_) {
      delete impl_;
    }

    if (ptr) {
      impl_ = new pmr_model<T>(ptr, deleter_);
    } else {
      impl_ = nullptr;
    }
  }

  /**
   * Specialization to enable ADL swap.
   */
  void swap(value_ptr& other)
  {
    using std::swap;
    swap(impl_, other.impl_);
    swap(deleter_, other.deleter_);
  }

  /**
   * Get a uniquely owning pointer to the managed object.
   *
   * After calling, this object will be reset as if release had been called.
   */
  std::unique_ptr<T, Deleter> to_unique()
  {
    return std::unique_ptr<T, Deleter>(release(), deleter_);
  }

protected:
  pmr_concept* impl_;
};

template <typename T1, typename T2, typename D1, typename D2>
bool operator==(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  return a.get() == b.get();
}

template <typename T1, typename T2, typename D1, typename D2>
bool operator!=(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  return a.get() != b.get();
}

template <typename T1, typename T2, typename D1, typename D2>
bool operator<(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  using CT = typename std::common_type<typename value_ptr<T1, D1>::pointer,
      typename value_ptr<T2, D2>::pointer>::type;
  return std::less<CT>()(a.get(), b.get());
}

template <typename T1, typename T2, typename D1, typename D2>
bool operator<=(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  return !(b < a);
}

template <typename T1, typename T2, typename D1, typename D2>
bool operator>(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  return b < a;
}

template <typename T1, typename T2, typename D1, typename D2>
bool operator>=(value_ptr<T1, D1> const& a, value_ptr<T2, D2> const& b)
{
  return !(a < b);
}

template <typename T, typename D>
bool operator==(value_ptr<T, D> const& a, std::nullptr_t)
{
  return !a;
}

template <typename T, typename D>
bool operator==(std::nullptr_t, value_ptr<T, D> const& a)
{
  return !a;
}

template <typename T, typename D>
bool operator!=(value_ptr<T, D> const& a, std::nullptr_t)
{
  return (bool)a;
}

template <typename T, typename D>
bool operator!=(std::nullptr_t, value_ptr<T, D> const& a)
{
  return (bool)a;
}

template <typename T, typename D>
bool operator<(value_ptr<T, D> const& a, std::nullptr_t)
{
  return std::less<typename value_ptr<T, D>::pointer>()(a.get(), nullptr);
}

template <typename T, typename D>
bool operator<(std::nullptr_t, value_ptr<T, D> const& a)
{
  return std::less<typename value_ptr<T, D>::pointer>()(nullptr, a.get());
}

template <typename T, typename D>
bool operator<=(value_ptr<T, D> const& a, std::nullptr_t)
{
  return !(nullptr < a);
}

template <typename T, typename D>
bool operator<=(std::nullptr_t, value_ptr<T, D> const& a)
{
  return !(a < nullptr);
}

template <typename T, typename D>
bool operator>(value_ptr<T, D> const& a, std::nullptr_t)
{
  return nullptr < a;
}

template <typename T, typename D>
bool operator>(std::nullptr_t, value_ptr<T, D> const& a)
{
  return a < nullptr;
}

template <typename T, typename D>
bool operator>=(value_ptr<T, D> const& a, std::nullptr_t)
{
  return !(a < nullptr);
}

template <typename T, typename D>
bool operator>=(std::nullptr_t, value_ptr<T, D> const& a)
{
  return !(nullptr < a);
}

template <typename T, typename D>
void swap(value_ptr<T, D>& a, value_ptr<T, D>& b)
{
  a.swap(b);
}

template <typename T, typename... Args>
auto make_val(Args&&... args) -> value_ptr<T>
{
  return value_ptr<T>(new T(std::forward<Args>(args)...));
}
} // namespace bsc

namespace std {

template <typename T, typename D>
struct hash<bsc::value_ptr<T, D>> {
  std::size_t operator()(bsc::value_ptr<T, D> const& ptr) const
  {
    return std::hash<typename bsc::value_ptr<T, D>::pointer>()(ptr.get());
  }
};
} // namespace std
