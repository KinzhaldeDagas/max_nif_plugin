
#ifndef GEOM_EXPORT_H
#define GEOM_EXPORT_H

#ifdef GEOM_STATIC_DEFINE
#  define GEOM_EXPORT
#  define GEOM_NO_EXPORT
#else
#  ifndef GEOM_EXPORT
#    ifdef Geom_EXPORTS
        /* We are building this library */
#      define GEOM_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define GEOM_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef GEOM_NO_EXPORT
#    define GEOM_NO_EXPORT 
#  endif
#endif

#ifndef GEOM_DEPRECATED
#  define GEOM_DEPRECATED __declspec(deprecated)
#endif

#ifndef GEOM_DEPRECATED_EXPORT
#  define GEOM_DEPRECATED_EXPORT GEOM_EXPORT GEOM_DEPRECATED
#endif

#ifndef GEOM_DEPRECATED_NO_EXPORT
#  define GEOM_DEPRECATED_NO_EXPORT GEOM_NO_EXPORT GEOM_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GEOM_NO_DEPRECATED
#    define GEOM_NO_DEPRECATED
#  endif
#endif

#endif /* GEOM_EXPORT_H */
