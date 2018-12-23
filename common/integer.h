/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER

#if 0
#include <windows.h>
#else

typedef unsigned char    uint8;      // �޷���8λ���ͱ���
typedef signed   char    int8;       // �з���8λ���ͱ���
typedef unsigned short   uint16;     // �޷���16λ���ͱ���
typedef signed   short   int16;      // �з���16λ���ͱ���
typedef unsigned int     uint32;     // �޷���32λ���ͱ���
typedef signed   int     int32;      // �з���32λ���ͱ���
typedef float            fp32;       // �����ȸ�������32λ���ȣ�
typedef double           fp64;       // ˫���ȸ�������64λ���ȣ�

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
