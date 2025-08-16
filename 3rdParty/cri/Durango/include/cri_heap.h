#ifndef	_CRI_HEAP_H_INCLUDED
#define	_CRI_HEAP_H_INCLUDED
/****************************************************************************
 *																			*
 *				CRI Heap Manager "CriHeap" Library							*
 *																			*
 *				2005-03-17	written by satouo								*
 *																			*
 ****************************************************************************/
/*!
 *	\file		cri_heap.h
 */

/**************************************************************************** 
 *      インクルードファイル												* 
 *      Include file														* 
 ****************************************************************************/
#include "cri_xpt.h"

#ifdef XPT_TGT_PC
#pragma pack(push)
#pragma pack(1) //The boundary value of structure is adjusted to 1 byte. 
#endif

/**************************************************************************** 
 *		定数マクロ															* 
 *		MACRO CONSTANT														* 
 ****************************************************************************/
/*	Version number of CRIHEAP	*/
#define CRIHEAP_NAME_STRINGS		"CRI Heap"
#define	CRIHEAP_VERSION_STRINGS		"1.21.02"

/* Default memory alignment */
#define	CRIHEAP_DEFAULT_MEM_ALIGN	(8)

/**************************************************************************** 
 *		列挙定数マクロ														* 
 *		ENUM CONSTANT														* 
 ****************************************************************************/
/*JP
 * \brief メモリのタイプ
 */
/*EN
 * \brief Types of heap
 */
typedef enum {
	CRIHEAP_TYPE_FIX       = (1),	
	CRIHEAP_TYPE_TEMPORARY = (2),
	CRIHEAP_TYPE_DYNAMIC   = (3), // unuse
	CRIHEAP_TYPE_NONE      = (0),
	/* enum be 4bytes */
	CRIHEAP_TYPE_ENUM_BE_SINT32 = 0x7FFFFFFF
} CriHeapType;
	
/**************************************************************************** 
 *		データ型															* 
 *      Data type declaration												* 
 ****************************************************************************/
/*	CriHeapハンドル		*/
/*	CriHeap handle		*/
#ifndef CRIHEAP_DEFINED
#define	CRIHEAP_DEFINED

/* internal */
typedef struct CriHeapBlockTag {
	struct CriHeapBlockTag *prevblock;
	struct CriHeapBlockTag *nextblock;
	CriSint32 memsize;
	CriUint8  used;
	CriUint8  type;
	CriUint16 alignspc;
	CriUint16 gap;
	CriChar8  *nameadr;
} CriHeapBlock, *CriHeapBlockPtr;

/*JP
 * \brief CRI Heapハンドル
 * \struct CriHeap
 * \ingroup CRIHEAP_BASIC
 * CRI Heapを用いてメモリアロケートを行うために必要なハンドルです。<br>
 * このハンドルに対して、メモリアロケートやメモリフリーを行います。
 * \sa criHeap_Create(), criHeap_Destroy()
 */
/*EN
 * \brief CRI Heap handle
 * \struct CriHeap
 * \ingroup CRIHEAP_BASIC
 * \par Description:
 * The heap handle.  This data structure needs to exist through the life cycle of
 * the heap.  It allows the library to allocate and deallocate memory
 * internally within the heap.  Typically this handle gets allocated at the
 * beginning of the heap memory that you've provided for the heap, but don't
 * depend on this behavior.  The heap itself is opaque.  You can of course
 * allocate multiple non-contiguous heaps, but these will be separate heap 
 * structures and a single allocation won't choose between them.
 *
 * \sa criHeap_Create(), criHeap_Destroy()
 */
typedef	struct _criheap_struct {
	struct _criheap_vfunctiontable *vtbl;
	CriSint32			totalsize;
	CriSint32			peaksize;
	CriSint32			currentsize;
	CriHeapBlock	*topblock;	
	CriHeapBlock	*taleblock;	
} CriHeapObj, *CriHeap;

/*JP
 * \brief CRI Heap仮想関数テーブル
 * \struct criHeapVirtualFunctionTable
 * \ingroup CRIHEAP_BASIC
 * 独自のメモリアロケート関数を実装する際に使用する仮想関数テーブル構造体です。<br>
 * 各関数はマルチスレッドセーフである必要があります。<br>
 * \par 備考:
 * ３番目のアロケート関数は現在未使用です。
 */
/*EN
 * \brief CRI Heap Virtual Function Table
 * \struct criHeapVirtualFunctionTable
 * \ingroup CRIHEAP_BASIC
 * \par Description:
 * The virtual functions table for original allocation functions.<br>
 * The third allocation function is not in use currently.
 */
typedef struct _criheap_vfunctiontable {
	/*JP
	 * \brief メモリのアロケート（メモリ領域の先端から）
	 * \par 説明:
	 * criHeap_AllocFix関数呼び出し時に呼ばれます。
	 * \sa criHeap_AllocFix()
	 */
	/*EN
	 * \brief Memory allocation (allocating from the top of the memory area)
	 * \par Description:
	 * Called from criHeap_AllocFix().
	 * \sa criHeap_AllocFix()
	 */
	void *(*AllocFix)(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align);
	/*JP
	 * \brief メモリのアロケート（メモリ領域の終端から）
	 * \par 説明:
	 * criHeap_AllocFix関数呼び出し時に呼ばれます。
	 * \sa criHeap_AllocTemporary()
	 */
	/*EN
	 * \brief Memory allocation (allocating from the tail of the memory area)
	 * \par Description:
	 * Called from criHeap_AllocTemporary().
	 * \sa criHeap_AllocTemporary()
	 */
	void *(*AllocTemporary)(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align);
	/*JP
	 * \brief 未使用関数
	 */
	/*EN
	 * \brief Unused
	 */
	void *(*AllocDynamic)(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align); // unused

	/*JP
	 * \brief メモリのフリー
	 * \par 説明:
	 * criHeap_Free関数呼び出し時に呼ばれます。
	 * \sa criHeap_Free()
	 */
	/*EN
	 * \brief Memory deallocation
	 * \par Description:
	 * Called from criHeap_Free().
	 * \sa criHeap_Free()
	 */
	CriSint32 (*Free)(CriHeap heap, void *ptr);
} criHeapVirtualFunctionTable;

#endif


/****************************************************************************
 *		関数の宣言
 *      Function Declaration
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==========================================================================*
 *		ライブラリの初期化と終了処理
 * 		Initialize and Finalize of Library
 *==========================================================================*/
/*JP
 * \brief CRI Heapライブラリの初期化
 * \ingroup CRIHEAP_BASIC
 * \par 説明:
 * CRI Heapライブラリを使用するために必要な初期化関数です。<br>
 * CRI Heapを使用する際はあらかじめ本関数をコールする必要があります。
 * \sa criHeap_Finalize()
 */
/*EN
 * \brief Initialize the heap manager.
 * \ingroup CRIHEAP_BASIC
 * \par Description:
 * This function initializes the heap manager and prepares it to manage heaps.
 * This function only has an effect the first time it is called in your program.
 * Subsequent calls only increment an internal counter.  This allows your 
 * program to call criHeap_Initialize() in pairs with criHeap_Finalize() 
 * throughout your program modules, and only the initial call to criHeap_Initialize()
 * and the final call to criHeap_Finalize() will have any effect.
 * \if ps2
 * \par PS2 only:
 * This function does a sanity check to make sure that the size of the CriHeapObj
 * data structure is a factor of 16.  If it is not a factor of 16, this function
 * hangs.
 * \endif
 * \sa criHeap_Finalize()
 */
void CRIAPI criHeap_Initialize(void);

/*JP
 * \brief CRI Heapライブラリの終了
 * \ingroup CRIHEAP_BASIC
 * \par 説明:
 * CRI Heapライブラリを終了するために必要な終了関数です。<br>
 * CRI Heapの使用を終了する際は本関数をコールしてください。
 * \sa criHeap_Initialize()
 */
/*EN
 * \brief Finalize the heap manager.
 * \ingroup CRIHEAP_BASIC
 * \par Description:
 * This function finalizes the heap manager.  If the criHeap_Initialize() function
 * is called <i>n</i> times, then the <i>n</I>th time that criHeap_Finalize()
 * is called, this function invalidates any heaps currently in use.  Calling
 * any heap function after the <i>n</i>th call to criHeap_Finalize() will
 * have unpredictable results.
 * \sa criHeap_Initialize()
 */
void CRIAPI criHeap_Finalize(void);

/*==========================================================================*
 *		ハンドルの生成・解放
 *==========================================================================*/
/*JP
 * \brief CRI Heapハンドルの生成
 * \ingroup CRIHEAP_BASIC
 * \param ptr メモリ領域のポインタ。
 * \param size メモリ領域のサイズ。
 * \return CRI Heapハンドル。<br>生成に失敗した場合は、NULLが返ります。
 * \par 説明:
 * CRI Heapハンドルを生成します。<br>
 * 本関数で与えるメモリ領域は、CRI Heap自身のハンドル領域やアロケート時に
 * 確保されるメモリ領域となります。<br>
 * 管理領域を含むため、メモリ領域すべてがアロケート出来ない点に注意してください。
 * 管理領域の目安は「１ハンドルあたりsizeof(CriHeapObj)」＋「１アロケートあたり
 * sizeof(CriHeapBlock)＋各メモリアライメントに必要なサイズ」となります。
 * \code
 * CriChar8 heap_buffer[0x2000];
 * CriHeap heap = criHeap_Create((void *)heap_buffer, sizeof(heap_buffer);
 * \endcode
 * \sa criHeap_Destroy()
 */
/*EN
 * \brief Create a contiguous heap in memory.
 * \ingroup CRIHEAP_BASIC
 * \param ptr A pointer to the start of the memory region to use as a heap.
 * \param size The size of the region to be used as a heap.
 * \return A valid CriHeap handle if successful, or NULL if unsuccessful.
 * \par Description:
 * This function allows you to set aside a region of memory for this library's
 * use as a "heap".  Heap allocation is required before active playback
 * can begin.
 * This function will fail and return NULL if the size of the memory region
 * is smaller than the CriHeapObj structure.
 * \code
 * CriChar8 heap_buffer[0x2000];
 * CriHeap heap = criHeap_Create((void *)heap_buffer, sizeof(heap_buffer);
 * \endcode
 * \sa criHeap_Destroy()
 */
CriHeap CRIAPI criHeap_Create(void *ptr, CriSint32 size);

/*JP
 * \brief 非スレッドセーフなCRI Heapハンドルの生成
 * \ingroup CRIHEAP_BASIC
 * \param ptr メモリ領域のポインタ。
 * \param size メモリ領域のサイズ。
 * \return CRI Heapハンドル。<br>生成に失敗した場合は、NULLが返ります。
 * \par 説明:
 * CRI Heapハンドルを生成します。<br>
 * 本関数を使用して作成されたヒープは、排他制御が行なわれません。<br>
 * 複数スレッドから同時に参照されるヒープの作成には、criHeap_Create関数をご利用ください。
 * \sa criHeap_Create(), criHeap_Destroy()
 */
/*EN
 * \brief Create a contiguous heap in memory.
 * \ingroup CRIHEAP_BASIC
 * \param ptr A pointer to the start of the memory region to use as a heap.
 * \param size The size of the region to be used as a heap.
 * \return A valid CriHeap handle if successful, or NULL if unsuccessful.
 * \par Description:
 * This function creates a heap to use the static memory as dynamically allocatable memory.
 * The heap created by using this function is not threadsafe.
 * If heap will be accessed by multiple threads, you must create the heap by using the criHeap_Create function.
 * \sa criHeap_Create(), criHeap_Destroy()
 */
CriHeap CRIAPI criHeap_CreateNoSerialize(void *ptr, CriSint32 size);

/*JP
 * \brief CRI Heapハンドルの解放
 * \ingroup CRIHEAP_BASIC
 * \param heap CRI Heapハンドル。
 * \par 説明:
 * CRI Heapハンドルを解放します。<br>criHeap_Create() で指定されたメモリ領域が
 * 解放され、CRI Heapハンドルは無効となります。
 * \sa criHeap_Create()
 */
/*EN
 * \brief Destroy a previously created heap.
 * \ingroup CRIHEAP_BASIC
 * \param heap A CriHeap handle previously created with criHeap_Create().
 * \par Description:
 * This function frees all internal allocations previously performed on
 * the heap and frees the memory.  Internally, this function does in
 * fact walk through the heap, finding and freeing all allocations, e.g.
 * it is not stubbed.  So calling this function on a trashed heap
 * will have unpredictable results.
 * \sa criHeap_Create()
 */
void CRIAPI criHeap_Destroy(CriHeap heap);

/*==========================================================================*
 *		Allocation
 *==========================================================================*/
/*JP
 * \brief メモリのアロケート（メモリ領域の先端から）
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heapハンドル。
 * \param size アロケートするメモリのサイズ。
 * \param name アロケートするメモリの名前。名前はデバッグ時に使用されます。
 * \param align アロケートするメモリのアライメント。
 * \return アロケートしたメモリへのポインタ。<br>アロケートに失敗した場合はNULLが返ります。
 * \par 説明:
 * メモリ領域の先端側から確保するメモリアロケーション関数です。ヒープ領域内に\ref block_fix "Fixブロック"を作成します。<br>
 * メモリの断片化を防ぐため、同じサイズ／アライメントの解放済み\ref block_fix "Fixブロック"を先端側から探して、再利用するように試みます。それが見つからない場合は、未使用の空き領域（\ref block_free "Freeブロック"）を分割してメモリを取得します。
 * \code
 * CriChar8 heap_buffer[0x2000];
 * CriHeap heap = criHeap_Create((void *)heap_buffer, sizeof(heap_buffer);
 * void *memptr = criHeap_AllocFix(heap, 0x100, "forTexture1", 64);
 * \endcode
 * \sa criHeap_AllocTemporary(), criHeap_Free()
 */
/*EN
 * \brief Memory allocation (allocating from the top of the memory area)
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heap handle
 * \param size Memory block size to be allocated
 * \param name Memory block name, which is used on debugging
 * \param align Memory block alignment
 * \return A pointer to the allocated memory block.<br>Returns NULL if unsuccessful.
 * \par Description:
 * This function allocates a memory block from the top of the memory area and creates a \ref block_fix "Fix block" in the heap area.<br>
 * To prevent memory fragmentation, a released \ref block_fix "Fix block" with the same size and alignment is searched for in the heap area from the top to the bottom and tries to reused the block.  And if not found, an unused free block (\ref block_free "Free block") is divided and a new block is allocated.
 * \code
 * CriChar8 heap_buffer[0x2000];
 * CriHeap heap = criHeap_Create((void *)heap_buffer, sizeof(heap_buffer);
 * void *memptr = criHeap_AllocFix(heap, 0x100, "forTexture1", 64);
 * \endcode
 * \sa criHeap_AllocTemporary(), criHeap_Free()
 */
void * CRIAPI criHeap_AllocFix(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align);

/*JP
 * \brief メモリのアロケート（メモリ領域の終端から）
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heapハンドル。
 * \param size アロケートするメモリのサイズ。
 * \param name アロケートするメモリの名前。名前はデバッグ時に使用されます。
 * \param align アロケートするメモリのアライメント。
 * \return アロケートしたメモリへのポインタ。<br>アロケートに失敗した場合はNULLが返ります。
 * \par 説明:
 * メモリ領域の終端側から確保するメモリアロケーション関数です。ヒープ領域内に\ref block_temporary "Temporaryブロック"を作成します。<br>
 * 要求したサイズ／アライメントをアロケート可能な解放済み\ref block_temporary "Temporaryブロック"を終端側から探して、分割または再利用するように試みます。それが見つからない場合は、未使用の空き領域（\ref block_free "Freeブロック"）を分割してメモリを取得します。<br>
 * ヒープ領域の終端側からメモリをアロケートするため、先端側での断片化を防ぐことが出来ます。<br>
 * 主に、一時的に利用するメモリに使用します。
 * \sa criHeap_AllocFix(), criHeap_Free()
 */
/*EN
 * \brief Memory allocation (allocating from the tail of the memory area)
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heap handle
 * \param size Memory block size to be allocated
 * \param name Memory block name, which is used on debugging
 * \param align Memory block alignment
 * \return A pointer to the allocated memory block.<br>Returns NULL if unsuccessful.
 * \par Description:
 * This function allocates a memory block from the bottom of the memory area and creates a \ref block_temporary "Temporary block" in the heap area.<br>
 * A released \ref block_temporary "Temporary block" available with the requested size and alignment is searched for in the heap area from the bottom and tries to divide and to reuse the block. And if not found, an unused free block (\ref block_free "Free block") is divided and a new block is allocated.<br>
 * Allocating from the bottom of the heap area will prevent fragmentation in the top of the heap area.<br>
 * Temporary block is mainly used for the memory area that is temporarily used.
 * \sa criHeap_AllocFix(), criHeap_Free()
 */
void * CRIAPI criHeap_AllocTemporary(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align);

/* DYNAMIC (unuse) */
//void * CRIAPI criHeap_AllocDynamic(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align);

void * CRIAPI criHeap_Alloc(CriHeap heap, CriSint32 size, const CriChar8 *name, CriSint32 align, CriSint32 type);

/*==========================================================================*
 *		Free
 *==========================================================================*/
/*JP
 * \brief メモリのフリー
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heapハンドル。
 * \param ptr アロケートしたメモリのポインタ。
 * \return フリーされたメモリサイズ。
 * \par 説明:
 * アロケートされたメモリをフリーします。<br>
 * \code
 * void *memptr = criHeap_AllocFix(heap, 0x100, "forTexture1", 64);
 * criHeap_Free(heap, memptr);
 * \endcode
 * \sa criHeap_AllocFix(), criHeap_AllocTemporary()
 */
/*EN
 * \brief Memory deallocation
 * \ingroup CRIHEAP_ALLOC
 * \param heap CRI Heap handle
 * \param ptr The pointer to the allocated memory block
 * \return Deallocated memory block size
 * \par Description:
 * This function deallocates the allocated memory block.<br>
 * \code
 * void *memptr = criHeap_AllocFix(heap, 0x100, "forTexture1", 64);
 * criHeap_Free(heap, memptr);
 * \endcode
 * \sa criHeap_AllocFix(), criHeap_AllocTemporary()
 */
CriSint32 CRIAPI criHeap_Free(CriHeap heap, void *ptr);

/*==========================================================================*
 *		排他制御用関数
 *==========================================================================*/
CriSint32 CRIAPI criHeap_EnterCriticalSection(void);
CriSint32 CRIAPI criHeap_LeaveCriticalSection(void);

/*==========================================================================*
 *		その他の関数
 *==========================================================================*/
/* For only internal use */
CriSint32 CRIAPI criHeap_SwitchAllocFunctions(CriHeap heap);

/*==========================================================================*
 *		デバッグ関連
 *==========================================================================*/
/*JP
 * \brief ピークメモリサイズの取得
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \return 使用されたメモリの最大値。
 * \par 説明:
 * 過去、使用されたメモリ量の最大値を返します。<br>
 * 最大値はcriHeap_DebugResetPeakMemorySize() でリセットすることが出来ます。
 * \sa criHeap_DebugResetPeakMemorySize()
 */
/*EN
 * \brief Returns peak memory usage of the heap.
 * \ingroup CRIHEAP_DEBUG
 * \param heap The heap previously allocated by criHeap_Create().
 * \return The maximum number of bytes used by the heap.
 * \par Description:
 * The amount of memory used by these libraries is variable and depends
 * on the number of simultaneous streams being read, the seek and error
 * frequency within the stream being read, video resolution and other
 * factors.  This function allows you to tune the allocation of the heap
 * to achieve a required performance level while allocating minimal
 * heap space for this library.  To get accurate readings, this function
 * should typically be called just before criHeap_Destroy(), and after
 * exercising all the video and audio functions in your program.
 */
CriSint32 CRIAPI criHeap_DebugGetPeakMemorySize(CriHeap heap);

/*JP
 * \brief ピークメモリサイズのリセット
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \par 説明:
 * ピークメモリサイズをリセットします。
 * \sa criHeap_DebugGetPeakMemorySize()
 */
/*EN
 * \brief Reset peak memory size
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap handle
 * \par Description:
 * The peak memory size is reset.
 * \sa criHeap_DebugGetPeakMemorySize()
 */
void CRIAPI criHeap_DebugResetPeakMemorySize(CriHeap heap);

/*JP
 * \brief メモリブロック数の取得
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \return メモリブロック数。
 * \par 説明:
 * アロケートされたメモリのブロック数を返します。<br>
 * CRI Heapハンドル生成直後は未使用のメモリブロックが１つ存在する状態
 * となります。また、フリーされた断片化されたメモリブロックもこの数に
 * 含みます。
 * \sa criHeap_DebugGetUsedBlocks()
 */
/*EN
 * \brief Get number of allocated memory blocks
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap handle
 * \return Number of allocated memory blocks
 * \par Description:
 * The number of allocated memory blocks is returned.<br>
 * One unused memory block exists right after CRI Heap handle creation.
 * The number of released fragmented memory blocks is also included.
 * \sa criHeap_DebugGetUsedBlocks()
 */
 CriSint32 CRIAPI criHeap_DebugGetNumBlocks(CriHeap heap);

/*JP
 * \brief 使用メモリブロック数の取得
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \return 使用メモリブロック数。
 * \par 説明:
 * アロケートされているメモリブロックの数を返します。
 * \sa criHeap_DebugGetNumBlocks()
 */
/*EN
 * \brief Get number of used memory blocks
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap
 * \return Number of used memory blocks
 * \par Description:
 * The number of used memory blocks is returned.<br>
 * \sa criHeap_DebugGetNumBlocks()
 */
CriSint32 CRIAPI criHeap_DebugGetUsedBlocks(CriHeap heap);

/*JP
 * \brief メモリブロック名の取得
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \param ptr アロケートしたメモリのポインタ。
 * \return メモリブロック名。
 * \par 説明:
 * アロケート時に設定したメモリブロック名へのポインタを返します。<br>
 */
/*EN
 * \brief Get memory block name
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap
 * \param ptr A pointer to the allocated memory block
 * \return Memory block name
 * \par Description:
 * The pointer to the memory block name specified on allocation is returned.<br>
 */
CriChar8 * CRIAPI criHeap_DebugGetBlockName(CriHeap heap, void *ptr);

/* メモリブロック番号の取得 */
//CriSint32 CRIAPI criHeap_DebugGetBlockNumberFromPointer(CriHeap heap, void *ptr);

/* メモリブロック情報の取得 */
//void CRIAPI criHeap_DebugGetBlockInformation(CriHeap heap, 
//								CriSint32 blocknumber, CriHeapBlock *heapblock);

/* メモリブロック情報の表示 */
//void CRIAPI criHeap_DebugPrintBlockInformation(CriHeap heap,
//								CriSint32 blocknumber, CriHeapBlock *heapblock);

/*JP
 * \brief メモリブロック情報の表示
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \par 説明:
 * 現在のメモリブロック情報を表示します。<br>
 * printf関数などの標準出力に表示を行います。
 */
/*EN
 * \brief Print memory block information
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap
 * \par Description:
 * The current memory block information is displayed.<br>
 * It is displayed on the standard output for the printf function.
 */
 void CRIAPI criHeap_DebugPrintBlockInformationAll(CriHeap heap);

/*JP
 * \brief メモリアロケートサイズの取得
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heapハンドル。
 * \par 説明:
 * 現在のメモリアロケートサイズの合計値を返します。
 */
/*EN
 * \brief Get total allocation size
 * \ingroup CRIHEAP_DEBUG
 * \param heap CRI Heap
 * \par Description:
 * The total size of allocated memory blocks is returned.
 */
CriSint32 CRIAPI criHeap_DebugGetTotalAllocSize(CriHeap heap);

/* フリーサイズの取得 */
//CriSint32 CRIAPI criHeap_DebugGetTotalFreeSize(CriHeap heap);

/* メモリアロケートサイズの取得(タイプ別) */
CriSint32 CRIAPI criHeap_DebugGetAllocSize(CriHeap heap, CriSint32 type);

/* ヒープヘッダサイズの取得 */
CriSint64 CRIAPI criHeap_DebugGetHandleHeaderSize(CriHeap heap);

/* メモリブロックヘッダサイズの取得  ptr: Allocated Pointer */
CriSint64 CRIAPI criHeap_DebugGetMemBlockHeaderSize(void *ptr);

/* 追加で必要となるサイズの最大値 */
CriSint32 CRIAPI criHeap_DebugGetWorstExtraSize(CriSint32 alignment);

/* criHeap_AllocFixでアロケート可能なサイズ */
CriSint32 CRIAPI criHeap_DebugGetFixAllocatableSize(CriHeap heap, CriSint32 alignment);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef XPT_TGT_PC
#pragma pack(pop)
#endif

#endif // _CRI_HEAP_H_INCLUDED

/* end of file */
