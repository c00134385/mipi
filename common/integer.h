/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER

#if 0
#include <windows.h>
#else

typedef unsigned char    uint8;      // 无符号8位整型变量
typedef signed   char    int8;       // 有符号8位整型变量
typedef unsigned short   uint16;     // 无符号16位整型变量
typedef signed   short   int16;      // 有符号16位整型变量
typedef unsigned int     uint32;     // 无符号32位整型变量
typedef signed   int     int32;      // 有符号32位整型变量
typedef float            fp32;       // 单精度浮点数（32位长度）
typedef double           fp64;       // 双精度浮点数（64位长度）

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/* Boolean type */
typedef enum { FALSE = 0, TRUE } BOOL;

#define bool BOOL
#define false FALSE
#define true TRUE

#endif

#define _INTEGER
#endif
