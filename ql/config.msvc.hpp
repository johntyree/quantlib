/*
*/

// $Id$


#ifndef quantlib_config_msvc_h
#define quantlib_config_msvc_h

// disable useless warning
// 'identifier' : decorated name length exceeded,
//                name was truncated in debug info
#pragma warning(disable: 4786)

#define REQUIRES_DUMMY_RETURN

#define HAVE_CMATH
#define QL_SQRT   sqrt
#define QL_FABS   fabs
#define QL_EXP    exp
#define QL_LOG    log
#define QL_SIN    sin
#define QL_COS    cos
#define QL_POW    pow
#define QL_MODF   modf

#define HAVE_LIMITS

#define HAVE_CTIME
#define QL_CLOCK    clock
#define QL_TIME     time

#define HAVE_CCTYPE
#define QL_STRLEN   strlen
#define QL_TOLOWER  tolower
#define QL_TOUPPER  toupper

#define QL_MIN      std::_cpp_min
#define QL_MAX      std::_cpp_max

#define BROKEN_TEMPLATE_SPECIALIZATION
#define BROKEN_TEMPLATE_METHOD_CALLS
#define HAVE_EXPRESSION_TEMPLATES
#define HAVE_TEMPLATE_METAPROGRAMMING

#define HAVE_INCOMPLETE_ITERATOR_SUPPORT
#define GARBLED_REVERSE_ITERATOR

#define QL_PATCH_MICROSOFT_BUGS

/*** libraries to be linked***/

#ifdef _DEBUG
    #pragma comment(lib,"QuantLib_d.lib")
#else
    #pragma comment(lib,"QuantLib.lib")
#endif

#ifndef _MT
    #ifndef _DLL 
        #ifdef _DEBUG
            #error Set 'C/C++ | Code Generation | Debug Multithreaded DLL'
        #else
            #error Set 'C/C++ | Code Generation | Multithreaded DLL'
        #endif
    #endif
#endif

#ifndef _CPPRTTI
    #error Check the 'C/C++ | C++ Language | Use RTTI' option
#endif

#endif
