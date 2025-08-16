/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2006-2010 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

#ifndef	_CRI_SJ_H_INCLUDED
#define	_CRI_SJ_H_INCLUDED
/****************************************************************************
 *																			*
 *				CRI Stream Joint "CriSj" Library							*
 *																			*
 *				2005-06-21	written by M.Oshimi								*
 *																			*
 ****************************************************************************/

/**************************************************************************** 
 *      Include file														* 
 ****************************************************************************/
#include "cri_xpt.h"
#include "cri_heap.h"

/**************************************************************************** 
 *		MACRO CONSTANT														* 
 ****************************************************************************/

/*	Version number of CriSj	*/
#define	CRISJ_NAME_STRINGS		"CRI Stream Joint"
#define	CRISJ_VERSION_STRINGS	"1.01.00"

#define CRISJUNI_MAX_LINE	(4)
#define	CRICHUNK_MAX_SIZE	(0xffffffff)

/***********************************************************************
 *		Process MACRO
 ***********************************************************************/

/**************************************************************************** 
 *      Data type declaration												* 
 ****************************************************************************/

/*	ストリームライン	*/
typedef enum {
	 CRISJ_LINE_FREE = (0),
	 CRISJ_LINE_DATA = (1),
	 CRISJ_LINE_HOLD = (2),
	 CRISJ_LINE_EXTRA = (3),
	 /* enum be 4bytes */
	 CRISJ_LINE_ENUM_BE_SINT32 = 0x7FFFFFFF
} CriSjLine;

/*	チャンク */
#ifndef TYPEDEF_CRICHUNK
#define TYPEDEF_CRICHUNK
typedef struct {
	CriUint8 *data;				/*	データ			*/
	CriUint32 size;				/*	バイト数		*/
} CriChunk;
#endif

/*	CriSjハンドル		*/
/*	CriSj handle		*/
typedef struct {
	struct _crisj_function_table *vtbl;
	const CriChar8 *name;
} CriSjObj, *CriSj;

typedef struct _crisj_function_table {
	/*	ハンドルの消去														*/
	void (*Destroy)(CriSj sj);
	/*		リセット														*/
	void (*Reset)(CriSj sj);
	/*	チャンクの取得	(FIFOの先頭から取得)								*/
	void (*GetChunk)(CriSj sj, CriSjLine id, CriUint32 nbyte, CriChunk *ck);
	/*	チャンクを戻す　(FIFOの先頭に挿入)									*/
	void (*UngetChunk)(CriSj sj, CriSjLine id, CriChunk *ck);
	/*	チャンクを挿入	(FIFOの最後に挿入)									*/
	void (*PutChunk)(CriSj sj, CriSjLine id, CriChunk *ck);
	/*	ラインから取得できる総バイト数の取得								*/
	CriUint32 (*GetTotalSize)(CriSj sj, CriSjLine id);
} CriSjVirtualFunctionTable;

typedef enum {
	CRISJ_UNIMODE_SEPARATE	=	(0),
	CRISJ_UNIMODE_JOIN		=	(1),
	/* enum be 4bytes */
	CRISJ_UNIMODE_ENUM_BE_SINT32 = 0x7FFFFFFF
} CriSjUniversalMode;


typedef struct CriSjRbfConfig {
	CriBool use_cs;
	CriUint32 buffer_size;
	CriUint32 extra_size;
	CriUint32 alignment;
	const CriChar8 *buffer_name;
} CriSjRbfConfig;

typedef struct CriSjMemConfig {
	CriBool use_cs;
	CriUint8 *data;
	CriUint32 data_size;
} CriSjMemConfig;

typedef struct CriSjUniConfig {
	CriBool use_cs;
	CriSjUniversalMode mode;
	CriUint32 num_chunks;
} CriSjUniConfig;

/****************************************************************************
 *      Function Declaration												*
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*	リングバッファ型SJの作成	*/
CriSint32 CRIAPI criSjRbf_CalculateWorkSize(const CriSjRbfConfig* config);
CriSj CRIAPI criSjRbf_Create(const CriSjRbfConfig *config, void *work, CriSint32 work_size);
CriSj CRIAPI criSjRbf_CreateByHeap(const CriSjRbfConfig *config, CriHeap heap, CriHeapType heap_type);

/*	常駐メモリ型SJの作成	*/
CriSint32 CRIAPI criSjMem_CalculateWorkSize(const CriSjMemConfig* config);
CriSj CRIAPI criSjMem_Create(const CriSjMemConfig *config, void *work, CriSint32 work_size);
CriSj CRIAPI criSjMem_CreateByHeap(const CriSjMemConfig *config, CriHeap heap, CriHeapType heap_type);

/*	ユニバーサルSJの作成	*/
CriSint32 CRIAPI criSjUni_CalculateWorkSize(const CriSjUniConfig* config);
CriSj CRIAPI criSjUni_Create(const CriSjUniConfig *config, void *work, CriSint32 work_size);
CriSj CRIAPI criSjUni_CreateByHeap(const CriSjUniConfig *config, CriHeap heap, CriHeapType heap_type);
/*	チェインプール数の取得	*/
CriSint32 CRIAPI criSjUni_GetNumChainPool(CriSj sj);

void CRIAPI criSj_Destroy(CriSj sj);
void CRIAPI criSj_Reset(CriSj sj);
void CRIAPI criSj_GetChunk(CriSj sj, CriSjLine line, CriUint32 nbyte, CriChunk *ck);
void CRIAPI criSj_UngetChunk(CriSj sj, CriSjLine line, CriChunk *ck);
void CRIAPI criSj_PutChunk(CriSj sj, CriSjLine line, CriChunk *ck);
CriUint32 CRIAPI criSj_GetTotalSize(CriSj sj, CriSjLine line);
void CRIAPI criSj_SplitChunk(CriChunk *ck, CriUint32 nbyte, CriChunk *ck1, CriChunk *ck2);

/***
*		New APIs
***/

CriUint32 CRIAPI criSj_GetInputFreeSize(CriSj sj);
void CRIAPI criSj_GetInputChunk(CriSj sj, CriChunk *ck);
void CRIAPI criSj_PutInputChunk(CriSj sj, CriChunk *ck, CriUint32 input_size);
CriUint32 CRIAPI criSj_GetOutputDataSize(CriSj sj);
void CRIAPI criSj_GetOutputChunk(CriSj sj, CriChunk *ck);
void CRIAPI criSj_PutOutputChunk(CriSj sj, CriChunk *ck, CriUint32 output_size);

CriUint32 CRIAPI criSj_PutOutputChunk2(CriSj sj, CriChunk *ck1, CriChunk *ck2, CriUint32 output_size);


/***
*		Old Interface (for compatibility)
***/
typedef struct _CriSjConfig {
	CriBool use_cs;
	CriHeapType heap_type;
} CriSjConfig;

CriSj CRIAPI criSj_CreateRingBuffer(CriHeap heap, CriUint32 bsize, CriUint32 xsize, CriUint32 align);
CriSj CRIAPI criSj_CreateNamedRingBuffer(CriHeap heap, CriUint32 bsize, CriUint32 xsize, CriUint32 align, const CriChar8 *bname);
CriSj CRIAPI criSj_CreateRingBufferWithConfig(CriHeap heap, CriUint32 bsize, CriUint32 xsize, CriUint32 align, const CriSjConfig *config);
CriSj CRIAPI criSj_CreateNamedRingBufferWithConfig(CriHeap heap, CriUint32 bsize, CriUint32 xsize, CriUint32 align, const CriChar8 *bname, const CriSjConfig *config);
CriSint32 CRIAPI criSjRbf_GetRequiredMemorySizeWithConfig(CriUint32 bsize, CriUint32 xsize, CriUint32 align, const CriSjConfig *config);

CriSj CRIAPI criSj_CreateMemory(CriHeap heap, CriUint8 *data, CriUint32 dtsize, CriUint32 align);
CriSj CRIAPI criSj_CreateMemoryWithConfig(CriHeap heap, CriUint8 *data, CriUint32 dtsize, CriUint32 align, const CriSjConfig *config);
CriSint32 CRIAPI criSjMem_GetRequiredMemorySizeWithConfig(CriUint32 align, const CriSjConfig *config);

CriSj CRIAPI criSj_CreateUniversal(CriHeap heap, CriSjUniversalMode mode, CriUint32 nchunk);
CriSj CRIAPI criSj_CreateUniversalWithConfig(CriHeap heap, CriSjUniversalMode mode, CriUint32 nchunk, const CriSjConfig *config);
CriSint32 CRIAPI criSjUni_GetRequiredMemorySizeWithConfig(CriSjUniversalMode mode, CriUint32 nchunk, const CriSjConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

/* end of file */
