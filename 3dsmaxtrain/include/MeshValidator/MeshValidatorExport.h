
#ifndef MV_EXPORT_H
#define MV_EXPORT_H

#ifdef MV_STATIC_DEFINE
#  define MV_EXPORT
#  define MV_NO_EXPORT
#else
#  ifndef MV_EXPORT
#    ifdef MeshValidator_EXPORTS
        /* We are building this library */
#      define MV_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define MV_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef MV_NO_EXPORT
#    define MV_NO_EXPORT 
#  endif
#endif

#ifndef MV_DEPRECATED
#  define MV_DEPRECATED __declspec(deprecated)
#endif

#ifndef MV_DEPRECATED_EXPORT
#  define MV_DEPRECATED_EXPORT MV_EXPORT MV_DEPRECATED
#endif

#ifndef MV_DEPRECATED_NO_EXPORT
#  define MV_DEPRECATED_NO_EXPORT MV_NO_EXPORT MV_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MV_NO_DEPRECATED
#    define MV_NO_DEPRECATED
#  endif
#endif

#endif /* MV_EXPORT_H */
