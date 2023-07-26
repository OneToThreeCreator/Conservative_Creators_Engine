#ifndef CCE_ATTRIBUTES_H
#define CCE_ATTRIBUTES_H

#if defined(__GNUC__) && __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5)
#define CCE_PURE_FN     __attribute__ ((pure))
#define CCE_CONST_FN    __attribute__ ((const))
#define CCE_NOALIAS_FN  __attribute__ ((pure))
#define CCE_NORETURN_FN __attribute__ ((noreturn))
#define CCE_NONNULL_FN  __attribute__ ((nonnull))
#define CCE_NOTHROW_FN  __attribute__ ((nothrow))
#elif defined(_MSC_VER) && _MSC_VER >= 1800
#define CCE_PURE_FN     
#define CCE_CONST_FN    __declspec(noalias)
#define CCE_NOALIAS_FN  __declspec(noalias)
#define CCE_NORETURN_FN __declspec(noreturn)
#define CCE_NONNULL_FN  
#define CCE_NOTHROW_FN  __declspec(nothrow)
#else
#define CCE_PURE_FN
#define CCE_CONST_FN
#define CCE_NOALIAS_FN
#define CCE_NORETURN_FN
#define CCE_NONNULL_FN  
#define CCE_NOTHROW_FN
#endif

#endif // CCE_ATTRIBUTES_H
