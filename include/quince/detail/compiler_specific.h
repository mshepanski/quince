#ifndef QUINCE__detail__compiler_specific_h
#define QUINCE__detail__compiler_specific_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/*
    Everything in this file is for quince internal use only.
*/

#ifdef _MSC_VER
# define QUINCE_NORETURN  __declspec(noreturn)
#elif defined(__GNUC__)
# define QUINCE_NORETURN  __attribute__((noreturn))
#endif

#ifdef _MSC_VER
# define QUINCE_STATIC_THREADLOCAL  __declspec(thread)
#elif defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 8
# define QUINCE_STATIC_THREADLOCAL  __thread
#else
# define QUINCE_STATIC_THREADLOCAL  thread_local
#endif

#ifdef _MSC_VER
# define QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING __pragma(warning(push)) __pragma(warning(disable:4250))
# define QUINCE_UNSUPPRESS_MSVC_WARNING         __pragma(warning(pop))
#else
# define QUINCE_SUPPRESS_MSVC_DOMINANCE_WARNING
# define QUINCE_UNSUPPRESS_MSVC_WARNING
#endif

#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9

  // From http://isocpp.org/files/papers/N3656.txt
# include <memory>

  namespace std {
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
  }

#endif

#endif
