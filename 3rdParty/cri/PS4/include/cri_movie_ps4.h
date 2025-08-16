/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Movie
 * Module   : Library User's Header
 * File     : cri_movie_ps4.h
 * Date     : 2013-11-25
 *
 ****************************************************************************/
/*!
 *	\file		cri_movie_ps4.h
 */
#ifndef	CRI_MOVIE_PS4_H_INCLUDED		/* Re-definition prevention */
#define	CRI_MOVIE_PS4_H_INCLUDED

/***************************************************************************
 *      Include file
 ***************************************************************************/
#include <kernel.h>

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
	 * \brief		Set processor parameters for decoding (Library Global)
	 *
 	 *  \param		num_threads		Number of additional threads for load distribution in decoding (Maximum 5 threads)
	 *  \param		affinity_mask 	Pointer to an array of thread affinity masks for each thread specified with num_threads.
	 *  \param		priorities		Pointer to an array of thread priority for each thread specified with num_threads.
	 *  \param		err				Optional error code
	 * 
	 * This function sets the processor parameters for decoding as default setting of library global. 
	 * Use it when you want to change processors or thread priority for decoding load distribution.
	 * 
	 * If this function is called, it must be called before library initialization.
	 * 
	 * On initialization, CRI Movie prepares 5 worker threads for distributed decoding.<br>
	 *
	 * \a num_threads specifies how many worker threads CRI Movie should use.<br>
	 * \a affinity_mask is an array of affinity masks for the worker threads.  This array must have
	 * \a num_threads many elements. 
	 * \a priorities is an array of thread priority for the worker threads. This array must have
	 * \a num_threads many elements. 
	 * The format of \a affinity_mask and \a priorities is same as for the value passed to the OS standard API.<br>
     *
	 * If this function is not called, 5 distributed decoding threads will run in parallel by default.
	 * Also, processor assignment of the decoding threads is handled by the operating system, and their 
	 * priority will be 725 (below main thread).
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		デコード処理に使う追加プロセッサ設定（ライブラリ全体）
	 *  \param		num_threads		負荷分散デコード用に使用する追加スレッドの数 (最大５つ）
	 *  \param		affinity_masks	スレッドアフィニティマスクの配列へのポインタ。num_threadsで指定したスレッドごとのマスク値。
	 *  \param		priorities		追加スレッドの優先度配列へのポインタ。
	 *  \param		err				エラー情報（省略可）
	 * 
	 * デコード処理を分散して処理するためのプロセッサをライブラリ全体に指定します。
	 * デコード処理に行うプロセッサやスレッド優先度を変更したい場合に使用してください。
	 * 本関数はライブラリ初期化前に呼び出す必要があります。
	 * 
	 * CRI Movieは初期化の際に５つの分散デコード用のワーカースレッドを用意します。
	 * num_threads引数で、そのうちのいくつのスレッドを実際に使用するかを指定できます。
	 * アプリケーションから明示的にプロセッサ割り当てを行いたい場合、個々のスレッドに対して
	 * アフィニティマスクとスレッド優先度を設定してください。
	 * アフィニティマスクとスレッド優先度の値は、OS標準関数の引数と同じ書式です。
	 * 
	 * この関数を呼ばなかった場合、５つのスレッドで並列デコードを行います。
	 * デコードスレッドのプロセッサは割り当ては全てOS任せで、優先度は725(メインスレッドより下）になります。
	 */
	void SetUsableProcessors_PS4(
					CriSint32 num_threads, 
					const SceKernelCpumask *affinity_masks, 
					const int *priorities,
					CriError &err = CriMv::ErrorContainer);
}


#endif	/* CRI_MOVIE_PS4_H_INCLUDED */
