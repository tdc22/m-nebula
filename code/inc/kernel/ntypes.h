#ifndef N_TYPES_H
#define N_TYPES_H
/*!
  \file
*/
/**
    @defgroup NebulaDataTypes Nebula Data Types
    @ingroup NebulaKernelModule
    @brief Various useful datastructures for managing lists,
    vectors, hashes, and so on.
*/
/**
    @defgroup NebulaUtilityFunctions Nebula Utility Functions
    @ingroup NebulaKernelModule
    @brief Various useful utility functions.
*/
//--------------------------------------------------------------------
//  OVERVIEW
//  General Nebula typedefs and definitions.
//
//  06-Feb-2000 floh    added #include <errno.h>
//
//  (C) 1999 A.Weissflog
//--------------------------------------------------------------------
#ifndef __XBxX__
#include <errno.h>
#include <stdio.h>
#endif

#ifndef N_SYSTEM_H
#include "kernel/nsystem.h"
#endif

#ifndef N_DEBUG_H
#include "kernel/ndebug.h"
#endif

#ifndef N_DEFCLASS_H
#include "kernel/ndefclass.h"
#endif

//--------------------------------------------------------------------
//  Shortcut Typedefs
//--------------------------------------------------------------------
typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef wchar_t        wchar;
typedef float		   float2[2];
typedef float		   float3[3];
typedef float		   float4[4];
typedef unsigned int   nFourCC;
typedef double		   nTime;

#ifndef TRUE            // obsolete, use 'true' instead
#define TRUE (1)
#endif

#ifndef FALSE           // obsolete, use 'false' instead
#define FALSE (0)
#endif

#ifndef NULL
#define NULL (0L)
#endif

#define N_MAXSTR (256)     // maximum length for complete path
//--------------------------------------------------------------------
#define N_MAXPATH (512)     // maximum length for complete path
#define N_MAXNAMELEN (32)   // maximum length for single path component

//--------------------------------------------------------------------
#define nID(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|(d))
#define MAKE_FOURCC(ch0,ch1,ch2,ch3) (ch0 | ch1<<8 | ch2<<16 | ch3<<24)
#define FOURCC(i) (((i&0xff000000)>>24) | ((i&0x00ff0000)>>8) | ((i&0x0000ff00)<<8) | ((i&0x000000ff)<<24))
#define N_WHITESPACE " \r\n\t"

inline void n_setbit(int bit, int& bits)   { bits |= bit; }
inline void n_unsetbit(int bit, int& bits) { bits &= ~bit;	 }
inline void n_bsetbit(bool b, int bit, int& bits) 
{ 
	if (b) n_setbit(bit, bits); 
	else   n_unsetbit(bit, bits);
}

inline bool n_getbit(int bit, int bits)	  { return ((bits & bit) != 0);}	

#if defined(__LINUX__) || defined(__MACOSX__)
#define n_stricmp strcasecmp
#define n_ltoa(val,buf,radix) sprintf(buf,"%ld",val);
#define	n_inline static inline
#else
#define n_stricmp _stricmp
#define n_ltoa _ltoa
#define	n_inline inline
#endif

//--------------------------------------------------------------------
//  public kernel functions
//--------------------------------------------------------------------
#if defined(N_KERNEL)
N_EXPORT void n_printf(const char *,...);
N_EXPORT void n_message(const char *,...);
N_EXPORT void n_error(const char *,...);
N_EXPORT void n_sleep(double);
N_EXPORT void *nn_malloc(size_t, const char *, int);
N_EXPORT void *nn_calloc(size_t, size_t, const char *, int);
N_EXPORT void n_free(void *);
N_EXPORT char *n_strdup(const char *);
N_EXPORT char *n_strncpy2(char *, const char *, size_t);
N_EXPORT bool n_strmatch(const char *, const char *);
N_EXPORT void n_strcat(char *, const char *, size_t);
N_EXPORT wchar* n_str2wstr(const char* str);
N_EXPORT int n_mkdir(const char *);

N_EXPORT void n_barf(const char *, const char *, int);
N_EXPORT wchar *n_wstrdup(const wchar *);

N_EXPORT 

void *n_dllopen(const char *);
void  n_dllclose(void *);
void *n_dllsymbol(void *, const char *);

#else

N_IMPORT void n_printf(const char *,...);
N_IMPORT void n_message(const char *,...);
N_IMPORT void n_error(const char *,...);
N_IMPORT void n_sleep(double);
N_IMPORT void *nn_malloc(size_t, const char *, int);
N_IMPORT void *nn_calloc(size_t, size_t, const char *, int);
N_IMPORT void n_free(void *);
N_IMPORT char *n_strdup(const char *);
N_IMPORT char *n_strncpy2(char *, const char *, size_t);
N_IMPORT bool n_strmatch(const char *, const char *);
N_IMPORT void n_strcat(char *, const char *, size_t);
N_EXPORT wchar* n_str2wstr(const char* str);
N_IMPORT int n_mkdir(const char *);

N_IMPORT void n_barf(const char *, const char *, int);
N_IMPORT wchar* n_wstrdup(const wchar *);

#endif

//--------------------------------------------------------------------
//  Nebula mem manager wrappers.
//--------------------------------------------------------------------
#ifdef __STANDALONE__
#define n_malloc(s) malloc(s)
#define n_calloc(s,n) calloc(s,n)
#define n_free(p) free(p)
#else
#define n_malloc(s) nn_malloc(s,__FILE__,__LINE__)
#define n_calloc(s,n) nn_calloc(s,n,__FILE__,__LINE__)
#endif

#ifdef new
#undef new
#endif

#ifdef delete
#undef delete
#endif

#ifdef __NEBULA_MEM_MANAGER__
n_inline void * operator new(size_t size)
{
    void *p = nn_malloc(size, __FILE__, __LINE__);
    if (!p) n_error("operator new out of mem!");
    // printf("0x%lx = new(%d)\n",p,size);
    return p;
}
n_inline void *operator new[](size_t size)
{
    void *p = nn_malloc(size, __FILE__, __LINE__);
    if (!p) n_error("operator new out of mem!");
    // printf("0x%lx = new(%d)\n",p,size);
    return p;
}

n_inline void * operator new(size_t size, const char* file, int line)
{
    void *p = nn_malloc(size, file, line);
    if (!p) n_error("operator new out of mem!");
    // printf("0x%lx = new(%d)\n",p,size);
    return p;
}
n_inline void *operator new[](size_t size, const char* file, int line)
{
    void *p = nn_malloc(size, file, line);
    if (!p) n_error("operator new out of mem!");
    // printf("0x%lx = new(%d)\n",p,size);
    return p;
}

n_inline void operator delete(void *p)
{
    // printf("delete(0x%lx)\n",p);
    n_free(p);
}

#pragma warning( push )
#pragma warning( disable : 4100 )

n_inline void operator delete(void *p, const char* file, int line)
{
    n_free(p);
}

n_inline void operator delete[](void *p)
{
    n_free(p);
}
n_inline void operator delete[](void *p, const char* file, int line)
{
    n_free(p);
}

#pragma warning( pop ) 

#define n_new new(__FILE__, __LINE__)
#define n_delete delete

#define n_memset memset
/*
inline
void n_memset(void* dst, int n32, unsigned long i)
{
	__asm {
		movq mm0, n32
		punpckldq mm0, mm0
		mov edi, dst

loopwrite:
		movntq 0[edi], mm0
		movntq 8[edi], mm0		

		add edi, 16
		sub i, 2
		jg loopwrite

		emms
	}
}
*/
#else
#define n_new new
#define n_delete delete
#define n_memset memset
#endif
//--------------------------------------------------------------------
#endif
