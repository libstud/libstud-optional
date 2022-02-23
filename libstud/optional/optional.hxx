#pragma once


#if   defined(LIBSTUD_OPTIONAL_STD)
   //
   // The user can force either way by defining LIBSTUD_OPTIONAL_STD to 0/1.
   //
#elif defined(_MSC_VER)
   //
   // Available from 19.10 (15.0). Except it (or the compiler) doesn't seem to
   // be constexpr-correct. Things appear to be fixed in 19.20 (16.0) but
   // optional is now only available in the C++17 mode or later.
   //
#  if _MSC_VER >= 1920
#    if defined(_MSVC_LANG) && _MSVC_LANG >= 201703L // See /Zc:__cplusplus
#      define LIBSTUD_OPTIONAL_STD 1
#    endif
#  endif
//
// Note: the Clang check must come before GCC since it also defines __GNUC__.
//
#elif defined(__clang__)
   //
   // Clang's libc++ has it since 4 but we might also be using libstdc++. For
   // the latter we will check for the presence of the <optional> header which
   // only appeared in GCC 7. Also assume both are only available in C++17.
   //
   // Note that on Mac OS it can still be <experimental/optional>.
   //
#  if __cplusplus >= 201703L
#    if __has_include(<__config>)
#      include <__config>          // _LIBCPP_VERSION
#      if _LIBCPP_VERSION >= 4000 && __has_include(<optional>)
#        define LIBSTUD_OPTIONAL_STD 1
#      endif
#    elif __has_include(<optional>)
#      define LIBSTUD_OPTIONAL_STD 1
#    endif
#  endif
#elif defined(__GNUC__)
   //
   // Available from GCC 7 but only in the C++17 mode. Note also that from 8
   // <optional> defines __cpp_lib_optional.
   //
#  if __GNUC__ >= 7 && __cplusplus >= 201703L
#    define LIBSTUD_OPTIONAL_STD 1
#  endif
#endif

#ifndef LIBSTUD_OPTIONAL_STD
#  define LIBSTUD_OPTIONAL_STD 0
#endif

#if LIBSTUD_OPTIONAL_STD
#  include <optional>
#else
#  include <utility>     // move()
#  include <functional>  // hash
#  include <type_traits> // is_*
#endif

#if LIBSTUD_OPTIONAL_STD

namespace stud
{
  template <typename T>
  using optional = std::optional<T>;

  using std::nullopt_t;
  using std::nullopt;
}

#else

namespace stud
{
  struct nullopt_t {constexpr explicit nullopt_t (int) {}};
  constexpr nullopt_t nullopt (1);

  namespace details
  {
    template <typename T, bool = std::is_trivially_destructible<T>::value>
    struct optional_data;

    template <typename T>
    struct optional_data<T, false>
    {
      struct empty {};

      union
      {
        empty e_;
        T d_;
      };
      bool v_;

#if !defined(_MSC_VER) || _MSC_VER > 1900
      constexpr optional_data ():           e_ (),              v_ (false) {}
      constexpr optional_data (nullopt_t):  e_ (),              v_ (false) {}
      constexpr optional_data (const T& v): d_ (v),             v_ (true)  {}
      constexpr optional_data (T&& v):      d_ (std::move (v)), v_ (true)  {}
#else
      optional_data ():           e_ (),              v_ (false) {}
      optional_data (nullopt_t):  e_ (),              v_ (false) {}
      optional_data (const T& v): d_ (v),             v_ (true)  {}
      optional_data (T&& v):      d_ (std::move (v)), v_ (true)  {}
#endif

#if (!defined(_MSC_VER) || _MSC_VER > 1900) &&  \
    (!defined(__GNUC__) || __GNUC__ > 4 || defined(__clang__))
      constexpr optional_data (const optional_data& o): v_ (o.v_) {if (v_) new (&d_) T (o.d_);}
      constexpr optional_data (optional_data&& o):      v_ (o.v_) {if (v_) new (&d_) T (std::move (o.d_));}
#else
      optional_data (const optional_data& o): v_ (o.v_) {if (v_) new (&d_) T (o.d_);}
      optional_data (optional_data&& o):      v_ (o.v_) {if (v_) new (&d_) T (std::move (o.d_));}
#endif

      optional_data& operator= (nullopt_t);
      optional_data& operator= (const T&);
      optional_data& operator= (T&&);

      optional_data& operator= (const optional_data&);
      optional_data& operator= (optional_data&&);

      ~optional_data ();
    };

    template <typename T>
    struct optional_data<T, true>
    {
      struct empty {};

      union
      {
        empty e_;
        T d_;
      };
      bool v_;

#if !defined(_MSC_VER) || _MSC_VER > 1900
      constexpr optional_data ():           e_ (),              v_ (false) {}
      constexpr optional_data (nullopt_t):  e_ (),              v_ (false) {}
      constexpr optional_data (const T& v): d_ (v),             v_ (true)  {}
      constexpr optional_data (T&& v):      d_ (std::move (v)), v_ (true)  {}
#else
      optional_data ():           e_ (),              v_ (false) {}
      optional_data (nullopt_t):  e_ (),              v_ (false) {}
      optional_data (const T& v): d_ (v),             v_ (true)  {}
      optional_data (T&& v):      d_ (std::move (v)), v_ (true)  {}
#endif

#if (!defined(_MSC_VER) || _MSC_VER > 1900) && \
    (!defined(__GNUC__) || __GNUC__ > 4 || defined(__clang__))
      constexpr optional_data (const optional_data& o): v_ (o.v_) {if (v_) new (&d_) T (o.d_);}
      constexpr optional_data (optional_data&& o):      v_ (o.v_) {if (v_) new (&d_) T (std::move (o.d_));}
#else
      optional_data (const optional_data& o): v_ (o.v_) {if (v_) new (&d_) T (o.d_);}
      optional_data (optional_data&& o):      v_ (o.v_) {if (v_) new (&d_) T (std::move (o.d_));}
#endif

      optional_data& operator= (nullopt_t);
      optional_data& operator= (const T&);
      optional_data& operator= (T&&);

      optional_data& operator= (const optional_data&);
      optional_data& operator= (optional_data&&);
    };

    template <typename T,
              bool = std::is_copy_constructible<T>::value,
              bool = std::is_move_constructible<T>::value>
    struct optional_ctors: optional_data<T>
    {
      using optional_data<T>::optional_data;
    };

    template <typename T>
    struct optional_ctors<T, false, true>: optional_ctors<T, true, true>
    {
      using optional_ctors<T, true, true>::optional_ctors;

#if !defined(_MSC_VER) || _MSC_VER > 1900
      constexpr optional_ctors () = default;
#else
      optional_ctors () = default;
#endif

      optional_ctors (const optional_ctors&) = delete;

#if (!defined(_MSC_VER) || _MSC_VER > 1900) &&                  \
    (!defined(__GNUC__) || __GNUC__ > 4 || defined(__clang__))
      constexpr optional_ctors (optional_ctors&&) = default;
#else
      optional_ctors (optional_ctors&&) = default;
#endif

      optional_ctors& operator= (const optional_ctors&) = default;
      optional_ctors& operator= (optional_ctors&&) = default;
    };

    template <typename T>
    struct optional_ctors<T, true, false>: optional_ctors<T, true, true>
    {
      using optional_ctors<T, true, true>::optional_ctors;

#if !defined(_MSC_VER) || _MSC_VER > 1900
      constexpr optional_ctors () = default;
#else
      optional_ctors () = default;
#endif

#if (!defined(_MSC_VER) || _MSC_VER > 1900) &&                  \
    (!defined(__GNUC__) || __GNUC__ > 4 || defined(__clang__))
      constexpr optional_ctors (const optional_ctors&) = default;
#else
      optional_ctors (const optional_ctors&) = default;
#endif

      optional_ctors (optional_ctors&&) = delete;

      optional_ctors& operator= (const optional_ctors&) = default;
      optional_ctors& operator= (optional_ctors&&) = default;
    };

    template <typename T>
    struct optional_ctors<T, false, false>: optional_ctors<T, true, true>
    {
      using optional_ctors<T, true, true>::optional_ctors;

#if !defined(_MSC_VER) || _MSC_VER > 1900
      constexpr optional_ctors () = default;
#else
      optional_ctors () = default;
#endif

      optional_ctors (const optional_ctors&) = delete;
      optional_ctors (optional_ctors&&) = delete;

      optional_ctors& operator= (const optional_ctors&) = default;
      optional_ctors& operator= (optional_ctors&&) = default;
    };
  }

  template <typename T>
  class optional: private details::optional_ctors<T>
  {
    using base = details::optional_ctors<T>;

  public:
    using value_type = T;

#if !defined(_MSC_VER) || _MSC_VER > 1900
    constexpr optional ()                                 {}
    constexpr optional (nullopt_t)                        {}
    constexpr optional (const T& v): base (v)             {}
    constexpr optional (T&& v):      base (std::move (v)) {}
#else
    optional ()                                 {}
    optional (nullopt_t)                        {}
    optional (const T& v): base (v)             {}
    optional (T&& v):      base (std::move (v)) {}
#endif

#if (!defined(_MSC_VER) || _MSC_VER > 1900) &&  \
    (!defined(__GNUC__) || __GNUC__ > 4 || defined(__clang__))
    constexpr optional (const optional&) = default;
    constexpr optional (optional&&)      = default;
#else
    optional (const optional&) = default;
    optional (optional&&)      = default;
#endif

    optional& operator= (nullopt_t v) {static_cast<base&> (*this) = v;             return *this;}
    optional& operator= (const T& v)  {static_cast<base&> (*this) = v;             return *this;}
    optional& operator= (T&& v)       {static_cast<base&> (*this) = std::move (v); return *this;}

    optional& operator= (const optional&) = default;
    optional& operator= (optional&&)      = default;

    T&       value ()       {return this->d_;}
    const T& value () const {return this->d_;}

    T*       operator-> ()       {return &this->d_;}
    const T* operator-> () const {return &this->d_;}

    T&       operator* ()       {return this->d_;}
    const T& operator* () const {return this->d_;}

    bool         has_value () const {return this->v_;}
    explicit operator bool () const {return this->v_;}
  };

  template <typename T>
  inline auto
  operator== (const optional<T>& x, const optional<T>& y)
  {
    bool px (x), py (y);
    return px == py && (!px || *x == *y);
  }

  template <typename T>
  inline auto
  operator!= (const optional<T>& x, const optional<T>& y)
  {
    return !(x == y);
  }

  template <typename T>
  inline auto
  operator< (const optional<T>& x, const optional<T>& y)
  {
    bool px (x), py (y);
    return px < py || (px && py && *x < *y);
  }

  template <typename T>
  inline auto
  operator> (const optional<T>& x, const optional<T>& y)
  {
    return y < x;
  }
}

namespace std
{
  template <typename T>
  struct hash<stud::optional<T>>: hash<T>
  {
    using argument_type = stud::optional<T>;

    size_t
    operator() (const stud::optional<T>& o) const
      noexcept (noexcept (hash<T> {} (*o)))
    {
      return o ? hash<T>::operator() (*o) : static_cast<size_t> (-3333);
    }
  };
}

#include <libstud/optional/optional.ixx>

#endif // !LIBSTUD_OPTIONAL_STD
