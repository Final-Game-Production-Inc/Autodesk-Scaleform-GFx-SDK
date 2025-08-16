/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Movie
 * Module   : Library User's Header
 * File     : cri_movie_pc.h
 * Date     : 2013-11-26
 *
 ****************************************************************************/
/*!
 *	\file		cri_movie_pc.h
 */
#ifndef	CRI_MOVIE_PC_H_INCLUDED		/* Re-definition prevention */
#define	CRI_MOVIE_PC_H_INCLUDED

/***************************************************************************
 *      Include file
 ***************************************************************************/
#include <windows.h>

#include <cri_xpt.h>
#include <cri_movie.h>

/***************************************************************************
 *      Variable Declaration
 ***************************************************************************/
namespace CriMv {
	extern CriError ErrorContainer;
}

/***************************************************************************
 *      Prototype Functions
 ***************************************************************************/
namespace CriMv {
	/*EN
	 * \ingroup MODULE_OPTION
	 * \brief		Set processor parameters for decoding on PC (Library Global)
	 *
 	 *  \param		num_threads		Number of additional threads for load distribution in decoding (Maximum 7 threads)
	 *  \param		affinity_masks 	Pointer to an array of thread affinity masks for each thread specified with num_threads.
	 *  \param		priorities		Pointer to an array of thread priority for each thread specified with num_threads.
	 *  \param		err				Optional error code
	 * 
	 * This function sets the processor parameters for decoding as default setting of library global. 
	 * Use it when you want to change processors or thread priority for decoding load distribution.
	 * 
	 * If this function is called, it must be called before library initialization.
	 * 
	 * On initialization, CRI Movie prepares 7 worker threads for distributed decoding.<br>
	 *
	 * \a num_threads specifies how many worker threads CRI Movie should use.<br>
	 * \a affinity_mask is an array of affinity masks for the worker threads.  This array must have
	 * \a num_threads many elements. The format of \a affinity_mask is same as for the value passed to the 
	 * Win32 <b>SetThreadAffinityMask()</b> API function.<br>
	 * \a priorities is an array of thread priority for the worker threads. This array must have
	 * \a num_threads many elements. The format of \a priorities is same as for the value passed to the 
	 * Win32 <b>SetThreadPriority()</b> API function.<br>
     *
	 * If this function is not called, 7 distributed decoding threads will run in parallel by default.
	 * Also, processor assignment of the decoding threads is handled by the operating system, and their 
	 * priority will be normal.
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		PCでデコード処理に使う追加プロセッサ設定（ライブラリ全体）
	 *  \param		num_threads		負荷分散デコード用に使用する追加スレッドの数 (最大７つ）
	 *  \param		affinity_masks	スレッドアフィニティマスクの配列へのポインタ。num_threadsで指定したスレッドごとのマスク値。
	 *  \param		priorities		追加スレッドの優先度配列へのポインタ。
	 *  \param		err				エラー情報（省略可）
	 * 
	 * デコード処理を分散して処理するためのプロセッサをライブラリ全体に指定します。
	 * デコード処理に行うプロセッサやスレッド優先度を変更したい場合に使用してください。
	 * 本関数はライブラリ初期化前に呼び出す必要があります。
	 * 
	 * CRI Movieは初期化の際に７つの分散デコード用のワーカースレッドを用意します。
	 * num_threads引数で、そのうちのいくつのスレッドを実際に使用するかを指定できます。
	 * アプリケーションから明示的にプロセッサ割り当てを行いたい場合、個々のスレッドに対して
	 * アフィニティマスクとスレッド優先度を設定してください。
	 * アフィニティマスクの値は、Win32 APIのSetThreadAffinityMaskの引数と同じ書式です。
     * スレッド優先度は、Win32 APIのSetThreadPriorityの引数と同じ書式です。
	 * 
	 * この関数を呼ばなかった場合、７つのスレッドで並列デコードを行います。
	 * デコードスレッドのプロセッサは割り当ては全てOS任せで、優先度はスレッド標準になります。
	 */
	void SetUsableProcessors_PC(
					CriSint32 num_threads, 
					const DWORD_PTR *affinity_masks, 
					const int *priorities,
					CriError &err = CriMv::ErrorContainer);
}


#endif	/* CRI_MOVIE_PC_H_INCLUDED */
