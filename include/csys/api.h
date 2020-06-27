// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_API_H
#define CSYS_API_H

#ifdef CSYS_COMPILED_LIB
#  undef CSYS_HEADER_ONLY
#  define CSYS_INLINE

#  ifdef CSYS_SHARED_LIB
// Windows Shared Library.
#    if defined(_WIN32)
#    	ifdef csys_EXPORTS
#    	  define CSYS_API __declspec(dllexport)
#    	else
#    	  define CSYS_API __declspec(dllimport)
#    	endif
 // Linux shared library.
#	 else
#      ifdef csys_EXPORTS
#        define CSYS_API __attribute__((visibility("default")))
#      else
#        define CSYS_API __attribute__((visibility("default")))
#      endif
#    endif
#  else
#    define CSYS_API
#  endif

// No export.
#  ifndef CSYS_NO_EXPORT
#    if defined(_WIN32)
#      define CSYS_NO_EXPORT
#    else
#      define CSYS_NO_EXPORT __attribute__((visibility("hidden")))
#    endif
#  endif

#else
#  define CSYS_API
#  define CSYS_NO_EXPORT
#  define CSYS_HEADER_ONLY
#  define CSYS_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CSYS_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define CSYS_DEPRECATED __declspec(deprecated)
#else
#define CSYS_DEPRECATED
#endif

#ifndef CSYS_DEPRECATED_EXPORT
#  define CSYS_DEPRECATED_EXPORT CSYS_API CSYS_DEPRECATED
#endif

#ifndef CSYS_DEPRECATED_NO_EXPORT
#  define CSYS_DEPRECATED_NO_EXPORT CSYS_NO_EXPORT CSYS_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CSYS_NO_DEPRECATED
#    define CSYS_NO_DEPRECATED
#  endif
#endif

#endif /* CSYS_API_H */
