/*****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 1998-2012 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Middleware Library
 * Module   : CRI Common Header / Post-Process
 * File     : cri_xpt_post.h
 * Date     : 2012-05-15
 * Version  : 2.07
 *
 *****************************************************************************/
#ifndef CRI_INCL_CRI_XPT_POST_H
#define CRI_INCL_CRI_XPT_POST_H


/*****************************************************************************
 *      マクロ定義
 *      Macro definition 
 *****************************************************************************/

/* コンパイル時の静的アサート
 *   C++ BoostのSTATIC_ASSERTと同等の機能をC言語で実装する。
 *   条件が真であることを検証する。偽の場合はコンパイルエラーを起こして、コン
 *   パイルを停止させる。
 * Static Asart when compiling
 *   This function is equal to STATIC_ASSERT of C++ Boost, and is imprementated
 *   by C language. If the condition is false, a compiler error is caused and
 *   it stop compiling.
 */
#define XPT_STATIC_ASSERT(cond) extern int xpt_static_assert_array[(cond)?1:-1]


/*****************************************************************************
 *      コンパイルオプションの検証
 *      Compiler option test
 *****************************************************************************/

/* enum型の検証
 * 　enum型がint型に整合することを検証する。
 * 　適切なエラーメッセージを表示したいので、可能ならコンパイルオプションによる
 * 　チェックを行う。不可能な場合、汎用的な静的アサートによるチェックが働く。
 * Enum type test
 *   It is verified that the enum type adjusts to the int type. 
 *   To display an appropriate error message, it checks the compiler option 
 *   if possible. General static asart test works if it is impossible. 
 */
#if defined(__MWERKS__) /* CodeWarrior */
    //#if !__option(enumsalwaysint)
    //    #error ERROR cri_xpt_post.h : Compiler option 'Enums Always Int' should be ON.
    //#endif
#elif defined(__GNUC__)
    //#if Compiler option -fshort-enums is used
    //    #error ERROR cri_xpt_post.h : Compiler option '-fshort-enums' should not be used.
    //#endif
#endif
//typedef enum {XPT_TEST_ENUM_0, XPT_TEST_ENUM_1, XPT_TEST_ENUM_2} XptTestEnum;
//XPT_STATIC_ASSERT(sizeof(XptTestEnum) == sizeof(Uint32));	/* enum should be 4byte. */

/*****************************************************************************
 * 定数マクロ
 * Macros of constant value
 *****************************************************************************/

/* NULL ポインタ型 */
#if !defined(CRI_NULL)
#ifdef __cplusplus
#define CRI_NULL	(0)
#else
#define CRI_NULL	((void *)0)
#endif
#endif

/* 論理定数１（偽、真） */
#if !defined(CRI_FALSE)
#define CRI_FALSE	(0)
#endif
#if !defined(CRI_TRUE)
#define CRI_TRUE	(1)
#endif

/* 論理定数２（スイッチ） */
#if !defined(CRI_OFF)
#define CRI_OFF		(0)
#endif
#if !defined(CRI_ON)
#define CRI_ON		(1)
#endif

/* 結果判定の定数（成功、失敗） */
#if !defined(CRI_OK)
#define CRI_OK		(0)			/* 成功 */
#endif
#if !defined(CRI_NG)
#define CRI_NG		(-1)		/* 失敗 */
#endif

/*****************************************************************************
 * ポインタまたは64ビットアドレスを格納できる整数型
 * 備考: ポインタ32bitかつROMアドレス空間64bitの環境で、両者を透過的に扱える。
 * The integer type which can hold an pointer or an address up to 64 bits.
 * NOTE: In the environment of pointer 32bit and ROM address space 64bit, 
 *       this type can treat both transparently. 
 *****************************************************************************/

#if !defined(_TYPEDEF_CriUint64Adr)
#define _TYPEDEF_CriUint64Adr
typedef CriUint64				CriUint64Adr;		/* ポインタまたは64ビットアドレスを格納できる整数型 */
#endif

#if !defined(CRI_PTR_TO_UINT64ADR)
#define CRI_PTR_TO_UINT64ADR(ptr)		((CriUint64Adr)(ptr))	/* Convert pointer to CriUint64Adr */
#endif

#if !defined(CRI_UINT64ADR_TO_PTR)
#define CRI_UINT64ADR_TO_PTR(uint64adr)	((void *)(CriUintPtr)(uint64adr))	/* Convert CriUint64Adr to pointer */
#endif

#if !defined(CRI_XPT_DISABLE_UNPREFIXED_TYPE)

/*****************************************************************************
 * 互換性維持のための定数マクロ
 * Macros for compatibility with old version
 *****************************************************************************/

/* NULL ポインタ型 */
#if !defined(NULL)
#define NULL		(CRI_NULL)
#endif

/* 論理定数１（偽、真） */
#if !defined(FALSE)
#define FALSE		(CRI_FALSE)
#endif
#if !defined(TRUE)
#define TRUE		(CRI_TRUE)
#endif

/* 論理定数２（スイッチ） */
#if !defined(OFF)
#define OFF			(CRI_OFF)
#endif
#if !defined(ON)
#define ON			(CRI_ON)
#endif

/* 結果判定の定数（成功、失敗） */
#if !defined(OK)
#define OK			(CRI_OK)
#endif
#if !defined(NG)
#define NG			(CRI_NG)
#endif

/*****************************************************************************
 * 互換性維持のためのデータ型宣言
 * Types for compatibility with old version
 *****************************************************************************/

#if !defined(_TYPEDEF_Uint8)
#define _TYPEDEF_Uint8
typedef CriUint8				Uint8;		/* 符号なし１バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint8)
#define _TYPEDEF_Sint8
typedef CriSint8				Sint8;		/* 符号つき１バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint16)
#define _TYPEDEF_Uint16
typedef CriUint16				Uint16;		/* 符号なし２バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint16)
#define _TYPEDEF_Sint16
typedef CriSint16				Sint16;		/* 符号つき２バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint32)
#define _TYPEDEF_Uint32
typedef CriUint32				Uint32;		/* 符号なし４バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint32)
#define _TYPEDEF_Sint32
typedef CriSint32				Sint32;		/* 符号つき４バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint64)
#define _TYPEDEF_Uint64
typedef CriUint64				Uint64;		/* 符号なし８バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint64)
#define _TYPEDEF_Sint64
typedef CriSint64				Sint64;		/* 符号つき８バイト整数 */
#endif

#if !defined(_TYPEDEF_Uint128)
#define _TYPEDEF_Uint128
typedef CriUint128				Uint128;	/* 符号なし16バイト整数 */
#endif

#if !defined(_TYPEDEF_Sint128)
#define _TYPEDEF_Sint128
typedef CriSint128				Sint128;	/* 符号つき16バイト整数 */
#endif

#if !defined(_TYPEDEF_Float16)
#define _TYPEDEF_Float16
typedef CriFloat16				Float16;	/* ２バイト実数 */
#endif

#if !defined(_TYPEDEF_Float32)
#define _TYPEDEF_Float32
typedef CriFloat32				Float32;	/* ４バイト実数 */
#endif

#if !defined(_TYPEDEF_Float64)
#define _TYPEDEF_Float64
typedef CriFloat64				Float64;	/* ８バイト実数 */
#endif

#if !defined(_TYPEDEF_Fixed32)
#define _TYPEDEF_Fixed32
typedef CriFixed32				Fixed32;	/* 固定小数点32ビット */
#endif

/* X11 API（xtrapproto.h）との競合回避用 */
#if !defined(_TYPEDEF_Bool) && !defined(Bool)
#define _TYPEDEF_Bool
typedef CriBool					Bool;		/* 論理型（論理定数を値にとる） */
#endif

#if !defined(_TYPEDEF_Char8)
#define _TYPEDEF_Char8
typedef CriChar8				Char8;		/* 文字型 */
#endif

#if !defined(_TYPEDEF_SintPtr)
#define _TYPEDEF_SintPtr
typedef CriSintPtr				SintPtr;
#endif

#if !defined(_TYPEDEF_UintPtr)
#define _TYPEDEF_UintPtr
typedef CriUintPtr				UintPtr;
#endif

#endif	/* CRI_XPT_DISABLE_UNPREFIXED_TYPE */

#endif	/* CRI_INCL_CRI_XPT_POST_H */

/* end of file */
