/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2005-2013 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Movie
 * Module   : Library User's Header
 * File     : cri_movie_core.h
 * Date     : 2013-11-21
 * Version  : (see CRIMVPLY_VER)
 *
 ****************************************************************************/
/*!
 *	\file		cri_movie_core.h
 */
#ifndef	CRI_MOVIE_CORE_H_INCLUDED		/* Re-definition prevention */
#define	CRI_MOVIE_CORE_H_INCLUDED

/*	Version No.	*/
#define	CRIMVPLY_VER		"3.50"
#define	CRIMVPLY_NAME		"criMvPly"

/***************************************************************************
 *      Include file
 ***************************************************************************/
#include <cri_xpt.h>
#include <cri_sj.h>
#include <cri_heap.h>

/***************************************************************************
 *      MACRO CONSTANT
 ***************************************************************************/

/*EN
 * \brief	Audio OFF setting of Audio Track
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetAudioTrack()
 */
/*JP
 * \brief	オーディオ再生OFFの指定値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetAudioTrack()
 */
#define CRIMV_AUDIO_TRACK_OFF		(-1)

/*EN
 * \brief	Default setting of Audio Track
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetAudioTrack()
 */
/*JP
 * \brief	オーディオチャネルのデフォルト値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetAudioTrack()
 */
#define CRIMV_AUDIO_TRACK_AUTO	(100)

/*EN
 * \brief	Maximum number of PCM tracks in one audio stream
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	オーディオデータ内の最大PCMトラック数
 * \ingroup MDL_MV_OPTION
 */
#define CRIMV_PCM_BUFFER_MAX		(8)

/*EN
 * \brief	Subtitle OFF setting
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetSubtitleChannel()
 */
/*JP
 * \brief	字幕再生OFFの指定値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetSubtitleChannel()
 */
#define CRIMV_SUBTITLE_CHANNEL_OFF	(-1)

/*EN
 * \brief	Maximum number of video tracks in a movie file
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	ムービファイル内の最大ビデオストリーム数
 * \ingroup MDL_MV_OPTION
 */
#define CRIMV_MAX_VIDEO_NUM			(1)

/*EN
 * \brief	Maximum number of audio tracks in a movie file
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	ムービファイル内の最大オーディオストリーム数
 * \ingroup MDL_MV_OPTION
 */
#define CRIMV_MAX_AUDIO_NUM			(32)

/*EN
 * \brief	Maximum number of alpha tracks in a movie file
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	ムービファイル内の最大アルファストリーム数
 * \ingroup MDL_MV_OPTION
 */
#define CRIMV_MAX_ALPHA_NUM			(1)

#if defined(XPT_TGT_PC)
/*EN
 * \brief	Number of extra threads for multicore decoding
 * \ingroup MDL_MV_OPTION
 * The number of additional decoding threads that CRI Movie library internally creates.
 * The threads are intended to run on multiple processors in parallel.
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
/*JP
 * \brief	マルチコアデコード用の追加のデコードスレッド数
 * \ingroup MDL_MV_OPTION
 * CRI Movieライブラリが内部で作成する追加のデコードの数です。これらのスレッドは、マルチコアPC上で
 * デコード処理を並列分散させるために作られます。
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
#define CRIMV_NUM_EXT_DECTHREAD_PC		(3)

/*EN
 * \brief	Default affnity mask of a thread
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
/*JP
 * \brief	スレッドアフィニティマスクのデフォルト設定値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
#define CRIMV_DEFAULT_AFFNITY_MASK_PC		(0xFFFFFFFF)


/*EN
 * \brief	Default priority of a thread
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
/*JP
 * \brief スレッドのデフォルト優先度
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetUsableProcessors_PC()
 */
#define CRIMV_DEFAULT_THREAD_PRIORITY_PC		(0x8000000)
#endif

/*EN
 * \brief	Default value of the read buffer size
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetReadBufferSize()
 */
/*JP
 * \brief	リードバッファサイズをデフォルト値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetReadBufferSize()
 */
#define CRIMV_READ_BUFFER_SIZE_AUTO	(-1)


/***************************************************************************
 *      Library Spec Defenition
 ***************************************************************************/

/* <These definitions are for CRI internal use> */

/* 2007-09-06:URUSHI オーディオ処理のサブモジュール化                   *
 * MvPlyのオーディオ処理部分をCriMvPlyAmngという新たなクラスに切り出す。*
 * CriMvPlyAmngはデマルチプレクサから全トラックデータを受け取って、     *
 * トラックごとに割り振られたAdecに処理を渡します。                     *
 * 目的は以下の二つ                                                     *
 * 1) オーディオトラックの動的切替                                      *
 * 2) ループ再生での異なるＡＶ尺の同期                                  */

#define NUM_MAX_ADEC  (2)	/* Adecの最大数 */
/* ADECのインデックス定義 */
/* このindexを使ってCriMvPlyAmngからAdecをコントロールしてください。 */
#define MAIN_ADEC_IDX (0)		// メイントラック用
#define SUB_ADEC_IDX  (1)		// ボイストラック用

/* オーディオの動的切替機能をサポートするか */
//#define ENABLE_DYNAMIC_AUDIO_SWITCH

#if defined(ENABLE_DYNAMIC_AUDIO_SWITCH)
	#define CRIMVPLYAMNG_TRACK_OFF (512)			// 切り替え先トラック番号のデフォルト値（切替ＯＦＦ）

	/* トラックの動的切替のための状態定義 */
	typedef enum _crimvplyamng_track_state {
		CRIMVPLYAMNG_TRACK_STATE_FIXED = (1),      // デフォルト
		CRIMVPLYAMNG_TRACK_STATE_PREP_SWITCHING,   // ユーザが切替を命令し、切替の準備段階（時刻判定のための基準時間をセット）
		CRIMVPLYAMNG_TRACK_STATE_SWITCHING         // 切替元と先の時刻判定を行なって切替を行なう状態
	} CriMvPlyAmngTrackState;
#endif

/* 連結再生時、2個目以降のヘッダを取得できるようにするため */
#define CRIMVPLY_HEAD_CONTAINER_NUM  (2)

/* 再読み込み閾値のデフォルト値 */
#define CRIMV_DEFAULT_RELOAD_THRESHOLD		(0.8f)		// 0.8[sec]

/* 再生準備中の貯金フレーム数デフォルト値 */
#define CRIMV_DEFAULT_NUM_FRAMES_FOR_PREP	(-1) /* デフォルト：不使用 (フレームプール数を採用) */

/* 内部ワーク領域の確保にCRI Heapを使用しない */
#define CRIMV_REMOVE_CRIHEAP

/* 機種固有フレーム情報数 */
#define CRIMV_FRAME_DETAILS_NUM		(2)

/* CriVavfios で指定する外部ファイルのパスの上限 */
#if	defined(XPT_TGT_IOS)
	#define CRIMVPLY_VAVFIOS_MAX_FILEPATH	(256)
#endif

/***************************************************************************
 *      Process MACRO
 ***************************************************************************/
#define criMv_SetDefaultHandleConfig(p_config)	\
{\
	(p_config)->readbuffer_size = 0;\
}

/***************************************************************************
 *      Enum declaration
 ***************************************************************************/
/*EN
 * \brief	Speaker index of CRI Movie PCM output
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	CRI Movie のPCM出力のスピーカー配置
 * \ingroup MDL_MV_OPTION
 */
typedef enum {
	CRIMV_PCM_BUFFER_L		 = 0,	/*EN< The LEFT channel of CRI Movie output 					*/
									/*JP< CRI Movie オーディオ出力の LEFT チャンネル 			*/
	CRIMV_PCM_BUFFER_R		 = 1,	/*EN< The RIGHT channel of CRI Movie output 				*/
									/*JP< CRI Movie オーディオ出力の RIGHT チャンネル 			*/
	CRIMV_PCM_BUFFER_LS		 = 2,	/*EN< The Surround LEFT channel of CRI Movie output 		*/
									/*JP< CRI Movie オーディオ出力の Surround LEFT チャンネル 	*/
	CRIMV_PCM_BUFFER_RS		 = 3,	/*EN< The Surround RIGHT channel of CRI Movie output 		*/
									/*JP< CRI Movie オーディオ出力の Surround RIGHT チャンネル 	*/
	CRIMV_PCM_BUFFER_C		 = 4,	/*EN< The CENTER channel of CRI Movie output 				*/
									/*JP< CRI Movie オーディオ出力の CENTER チャンネル 			*/
	CRIMV_PCM_BUFFER_LFE	 = 5,	/*EN< The LFE channel of CRI Movie output 					*/
									/*JP< CRI Movie オーディオ出力の LFE チャンネル 			*/
	CRIMV_PCM_BUFFER_EXT1	 = 6,	/*EN< The EXT1(Rear Left) channel of CRI Movie output 		*/
									/*JP< CRI Movie オーディオ出力の EXT1(Rear Left) チャンネル */
	CRIMV_PCM_BUFFER_EXT2	 = 7,	/*EN< The EXT2(Rear Right) channel of CRI Movie output 		*/
									/*JP< CRI Movie オーディオ出力の EXT2(Rear Right) チャンネル */

	/* Keep enum 4bytes */
	CRIMV_PCM_BUFFER_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvPcmBufferIndex;


/*EN
 * \brief	Composite mode of alpha movie
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	アルファムービの合成モード
 * \ingroup MDL_MV_OPTION
 */
typedef enum {
	CRIMV_COMPO_OPAQ		 = 0,	/*EN< Opacity, no alpha value								*/
									/*JP< 不透明、アルファ情報なし								*/
	CRIMV_COMPO_ALPHFULL	 = 1,	/*EN< Full alpha blending (8bits-alpha data) 				*/
									/*JP< フルAlpha合成（アルファ用データが8ビット)				*/
	CRIMV_COMPO_ALPH3STEP	 = 2,	/*EN< 3 Step Alpha											*/
									/*JP< 3値アルファ											*/
	CRIMV_COMPO_ALPH32BIT	 = 3,	/*EN< Full alpha blending (32bits color + alpha data)		*/
									/*JP< フルAlpha、（カラーとアルファデータで32ビット）		*/
	CRIMV_COMPO_ALPH1BIT	 = 4,	/*EN< Alpha blending (24bits color + 1->8bits alpha)        */
									/*JP< フルAlpha、（カラーとアルファデータで32bit、値は2値） */
	CRIMV_COMPO_ALPH2BIT	 = 5,	/*EN< Alpha blending (24bits color + 2->8bits alpha)        */
									/*JP< フルAlpha、（カラーとアルファデータで32bit、値は4値） */
	CRIMV_COMPO_ALPH3BIT	 = 6,	/*EN< Alpha blending (24bits color + 3->8bits alpha)        */
									/*JP< フルAlpha、（カラーとアルファデータで32bit、値は8値） */
	CRIMV_COMPO_ALPH4BIT	 = 7,	/*EN< Alpha blending (24bits color + 4->8bits alpha)        */
									/*JP< フルAlpha、（カラーとアルファデータで32bit、値は16値）*/

	/* Keep enum 4bytes */
	CRIMV_COMPO_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvAlphaType;


/*EN
 * \brief	Result of the last video frame retrieval
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	前回のビデオフレーム取得の結果
 * \ingroup MDL_MV_OPTION
 */
typedef enum {
	CRIMV_LASTFRAME_OK				= 0,	/*EN< Succeeded													*/
											/*JP< 取得成功													*/
	CRIMV_LASTFRAME_TIME_EARLY		= 1,	/*EN< Failed. The frame is not yet the time to draw				*/
											/*JP< 取得失敗。フレーム表示時刻が再生時間に達していなかった    */
	CRIMV_LASTFRAME_DECODE_DELAY    = 2,	/*EN< Failed. The frame to draw is not decoded yet				*/
											/*JP< 取得失敗。ビデオフレームのデコードが間に合わなかった	    */
	CRIMV_LASTFRAME_DISCARDED       = 3,	/*EN< Failed. The video frame is discarded by app				*/
											/*JP< 取得失敗。アプリによって破棄された						*/
	//CRIMV_LASTFRAME_NO_INPUT_DATA = 4,	//Not supported yet
	//CRIMV_LASTFRAME_SKIPPED       = 5,	//Not supported yet
	//CRIMV_LASTFRAME_DEMUX_STUCKED = 6,	//Not supported yet

	/* Keep enum 4bytes */
	CRIMV_LASTFRAME_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvLastFrameResult;

#if defined(XPT_TGT_PS3PPU)
/*EN
 * \brief	Graphic Environment type for PS3
 * \ingroup MDL_MV_OPTION
 */
/*JP
 * \brief	PS3のグラフィック環境
 * \ingroup MDL_MV_OPTION
 */
typedef enum {
	CRIMV_GRAPHIC_ENV_GCM	= 0,	/*EN< GCM. (or same ARGB 32bit texture format of GCM)			*/
									/*JP< GCM環境 (またはテクスチャフォーマットがGCMと同じ環境)		*/
	CRIMV_GRAPHIC_ENV_PSGL	= 1,	/*EN< PSGL. (or same ARGB 32bit texture format of PSGL)			*/
									/*JP< PSGL環境 (またはテクスチャフォーマットがPSGLと同じ環境)	*/

	/* Keep enum 4bytes */
	CRIMV_GRAPHIC_ENV_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvGraphicEnv;
#endif


/* CRI Movie Interanl handle status
 * 
 * The possible legal states of a CriMvPly handle.  Over the course of your application
 * the CriMvPly handle will walk through the following states in roughly the following
 * order.  In states that are waiting states, you can request the
 * CriMvPly handle to advance to the next state by calling the criMvPly_IncrementState()
 * function.  You can check the status of a valid CriMvPly handle at any time
 * by calling criMvPly_GetStatus().
 *
 * It is not possible to jump from a state to an arbitrary other state.  Normal play
 * proceeds from the CRIMVPLY_STATUS_STOP state through the CRIMV_PLY_STATUS_PLAYEND
 * state in that order.  Premature termination can be requested with the criMvPly_Stop()
 * function.
 *
 * \sa criMvPly_IncrementState(), criMvPly_GetStatus(), criMvPly_Stop()
 */
typedef enum {
	CRIMVPLY_STATUS_STOP			= 0,	/* Standstill.  No processing is occurring.
											 * CriMvPly handles are created into this state.
											 */
											/* 停止中 */
	CRIMVPLY_STATUS_DECHDR			= 1,	/* The CriMvPly structure is now parsing the header
											 * of the file, including information on height and width
											 * of the video stream.
											 */
											/* ヘッダ解析中 */
	CRIMVPLY_STATUS_WAIT_PREP		= 2,	/* The header has been decoded and criMvPly_GetStreamingParameters()
											 * will now provide valid values.  Typically you will call
											 * criMvPly_AllocateWorkBuffer() with this information at this point.
											 */
											/* PREP状態へのIncrementState待ち<BR>
											   AllocateWorkBufferしてから次へいくこと */
	CRIMVPLY_STATUS_PREP			= 3,	/* Transition to this state to acknowledge to the
											 * CriMvPly handle that you have allocated your work buffers. */
											/* 再生準備中 */
	CRIMVPLY_STATUS_WAIT_PLAYING	= 4,	/* The audio and video decoders are now ready to begin playback.*/
											/* PLAYING状態へのIncrementState待ち<BR>
											   この状態で既にビデオとオーディオのデコード結果は取得できる。*/
	CRIMVPLY_STATUS_PLAYING			= 5,	/* The decoders are currently decoding and playing output. */
											/* 再生中 */
	CRIMVPLY_STATUS_WAIT_PLAYEND	= 6,	/* The library is waiting for you to acknowledge the end of the movie.  You
											 * have informed the CriMvPly structure that an end-of-file condition exists,
											 * but final frames of video and audio may still be pending in your application. */
											/* PLAYEND状態へのIncrementState待ち */
	CRIMVPLY_STATUS_PLAYEND			= 7,	/* You have acknowledged the end of the movie.  Teardown can occur at this point. */
											/* 再生終了 */
	CRIMVPLY_STATUS_STOP_PROCESSING	= 8,	/* A request to stop has been received by the CriMvPly structure,
											 *    that is, you have called criMvPly_Stop(), and a stop is now pending. */
											/* 停止処理中 */
	CRIMVPLY_STATUS_WAIT_STOP		= 9,	/* The CriMvPly handle has acknowledged the stop request and
											 * you may now call criMvPly_IncrementState() to transition to
											 * the CRIMVPLY_STATUS_STOP state. */
											/* STOP状態へのIncrementState待ち */
	CRIMVPLY_STATUS_ERROR			= 10,	/* An error has occurred. */
											/* エラー */

	/* Keep enum 4bytes */
	CRIMVPLY_STATUS_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvPlyStatus;


/* Sofdec2 */
typedef enum {
	CRIMVPLY_META_FLAG_OFF  = 0x0000,
	CRIMVPLY_META_FLAG_CUE  = 0x0001,
	CRIMVPLY_META_FLAG_SEEK = 0x0002,
	CRIMVPLY_META_FLAG_ALL  = CRIMVPLY_META_FLAG_CUE + CRIMVPLY_META_FLAG_SEEK,

	/* Keep enum 4bytes */
	CRIMVPLY_META_FLAG_MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvMetaFlag;

/* Color Conversion*/
typedef enum{
	CRIMV_COLORSPACE_CONVERSION_TYPE_ITU_R_BT601_LIMITED	= 0,
	CRIMV_COLORSPACE_CONVERSION_TYPE_ITU_R_BT601_FULLRANGE	= 1,

	/* Keep enum 4bytes */
	CRIMV_COLORSPACE_CONVERSION_TYPE_ENUM_SINT32 			= 0x7FFFFFFF
}CriMvColorSpaceConversionType;

typedef void *(*CriMvMetaDataWorkMallocFunc)(void *obj, CriUint32 size);
typedef void (*CriMvMetaDataWorkFreeFunc)(void *obj, void *mem);

/* OUTER_FRAMEPOOL_WORK */
#define CriMvFramepoolWorkMallocFunc   CriMvMetaDataWorkMallocFunc
#define CriMvFramepoolWorkFreeFunc     CriMvMetaDataWorkFreeFunc

/***************************************************************************
 *      Data type declaration
 ***************************************************************************/

/*EN
 * \brief Audio parameters
 * \ingroup MDL_MV_INFO
 * 
 * \sa CriMvStreamingParameters, CriMvWaveInfo
 */
/*JP 
 * \brief オーディオパラメータ
 * \ingroup MDL_MV_INFO
 * 
 * オーディオストリームのパラメータ
 * \sa CriMvStreamingParameters, CriMvWaveInfo
 */
typedef struct {
	CriUint32		sampling_rate;		/*EN< Sampling rate */
										/*JP< サンプリング周波数 */
	CriUint32		num_channel;		/*EN< Number of channels. Monaural = 1, Stereo = 2 */
										/*JP< オーディオチャネル数 */
	CriUint32		total_samples;		/*EN< Total number of samples */
										/*JP< 総サンプル数 */
	CriUint32		output_buffer_samples;	/*EN< Output wave buffer size */
											/*JP< サウンド出力バッファのサンプル数 */
	CriUint32		codec_type;			/*EN< Codec type */
										/*JP< コーデック種別 */

} CriMvAudioParameters;

/*EN
 * \brief Video Parameters
 * \ingroup MDL_MV_INFO
 * \sa CriMvStreamingParameters
 */
/*JP 
 * \brief ビデオパラメータ
 * \ingroup MDL_MV_INFO
 * ビデオストリームのパラメータ
 * \sa CriMvStreamingParameters
 */
typedef struct {
	CriUint32		max_width;			/*EN< Maximum video width for stream. (multiple of 8) */
										/*JP< ムービ最大幅（８の倍数） */
	CriUint32		max_height;			/*EN< Maximum video height for stream. (multiple of 8)*/
										/*JP< ムービ最大高さ（８の倍数） */
	CriUint32		disp_width;			/*EN< Width of the image to draw. */
										/*JP< 表示したい映像の横ピクセル数（左端から） */
	CriUint32		disp_height;		/*EN< Height of the image to draw. */
										/*JP< 表示したい映像の縦ピクセル数（上端から） */
	CriUint32		num_frame_pool;		/*EN< Number of frame pools required for stream */
										/*JP< フレームプール数 */
	CriUint32		framerate;			/*EN< Frame rate per second [x1000]. */
										/*JP< フレームレート[x1000] */
	CriUint32		framerate_n;		/*EN< Frame rate (in rational as numerator). framerate_n/framerate_d = framerate */ /* UTODO: 変数名 */
										/*JP< フレームレートの分子(有理数形式)。framerate_n/framerate_d = framerate */
	CriUint32		framerate_d;		/*EN< Frame rate (in rational as denominator). */
										/*JP< フレームレートの分母(有理数形式)。 */
	CriUint32		total_frames;		/*EN< Total number of video frames */
										/*JP< 総フレーム数 */

	CriUint32		material_width;		/*EN< Width of the video source resolustion before encoding. */
										/*JP< エンコード前のビデオ素材の横ピクセル数 */
	CriUint32		material_height;	/*EN< Height of the video source resolustion before encoding. */
										/*JP< エンコード前のビデオ素材の縦ピクセル数 */
	CriUint32		screen_width;		/*EN< Screen width set by encoding and cropping. 
										 *    This parameter is only available when you encoded the movie with "Widescreen TV Support" option. 
										 *    Normally this value is 0. */
										/*JP< エンコード時に指定したスクリーン幅。
										 *    この値はエンコード時に「ワイドテレビ支援機能」を使用した場合のみ有効になります。
										 *    通常は０です。 */

	CriUint32		codec_type;			/*EN< Video Codec Type. If you encoded the movie for PS2, this value is 2.
										 *    Normally this value is 1 or 0(no info). 
										 *    If the codec_type is 1, the CRI Movie for ONLY PS2 can play the movie file. */
										/*JP< ビデオコーデック種別。PS2用にエンコードした場合 2になります。
										 *    通常は 1または 0(情報無し)です。
										 *    コーデック種別が２のムービは、PS2版ライブラリで「のみ」再生可能です。 */
	CriUint32		codec_dc_option;	/*EN< Video Codec DC Option. If you encoded the movie for PS2, this value is 10.
										 *    Normally this value is 11 or 0(no info). 
										 *    If the codec_type is 11, the CRI Movie for PS2 can NOT play the movie file. */
										/*JP< ビデオコーデックのDCオプション種別。PS2用にエンコードした場合10になります。
										 *    通常は11または 0(情報無し)です。
										 *    コーデックDCオプションが11のムービは、PS2版ライブラリ「では」再生できません。 */
	CriUint32	color_conversion_type;	/*EN< Color space converion type. Fullrange or Limited.*/
										/*JP< 色変換タイプ。 */
	CriSint32	capacity_of_picsize;	/*EN< Capacity size of video pictures. */
										/*JP< ピクチャサイズ上限値 */
	CriUint32	average_bitrate;		/*EN< Average bitrate. */
										/*JP< 平均ビットレート */
} CriMvVideoParameters;

/*EN
 * \brief Alpha Parameters
 * \ingroup MDL_MV_INFO
 * \sa CriMvStreamingParameters
 */
/*JP 
 * \brief アルファパラメータ
 * \ingroup MDL_MV_INFO
 * アルファストリームのパラメータ
 * \sa CriMvStreamingParameters
 */
typedef struct {
	CriUint32		max_width;			/*EN< Maximum alpha width for stream */
										/*JP< アルファフレームの最大幅 */
	CriUint32		max_height;			/*EN< Maximum alpha height for stream */
										/*JP< アルファフレームの最大高さ */
	CriUint32		disp_width;			/*EN< valid alpha width */
										/*JP< アルファフレームの実有効幅 */
	CriUint32		disp_height;		/*EN< valid alpha height */
										/*JP< アルファフレームの実有効高さ */
	CriUint32		framerate;			/*EN< Frame rate per second [x1000]. */
										/*JP< アルファのフレームレート[x1000] */
	CriUint32		framerate_n;		/*EN< Frame rate (in rational as numerator). framerate_n/framerate_d = framerate */ /* UTODO: 変数名 */
										/*JP< フレームレートの分子(有理数形式)。framerate_n/framerate_d = framerate */
	CriUint32		framerate_d;		/*EN< Frame rate (in rational as denominator). */
										/*JP< フレームレートの分母(有理数形式)。 */
	CriUint32		total_frames;		/*EN< Total number of alpha frames */
										/*JP< 総フレーム数 */
	CriMvAlphaType	alpha_type;			/*EN< Alpha Composite Type. */
										/*JP< アルファ合成種別。 */
	CriUint32		codec_type;			/*EN< Internal use only. Do not access this */
										/*JP< ライブラリ内部使用変数 */
	CriUint32	color_conversion_type;	/*EN< Color space converion type. Fullrange or Limited.*/
										/*JP< 色変換タイプ。 */
	CriSint32	capacity_of_picsize;	/*EN< Capacity size of video pictures. */
										/*JP< ピクチャサイズ上限値 */
	CriUint32	average_bitrate;		/*EN< Average bitrate. */
										/*JP< 平均ビットレート */
} CriMvAlphaParameters;


/*EN
 * \brief Streaming Parameters
 * \ingroup MDL_MV_INFO
 * This structure includes streaming parameters and playing parameters.
 * \sa CriMvEasyPlayer::GetMovieInfo()
 */
/*JP
 * \brief ストリーミング再生パラメータ
 * \ingroup MDL_MV_INFO
 * ストリーミング再生パラメータ。<br>
 * ストリーム自体の情報と、再生のために必要なパラメータの両方を含んでいる。
 * \sa CriMvEasyPlayer::GetMovieInfo()
 */
typedef struct {
	/* Stream */
	CriUint32		is_playable;		/*EN< Flag of the movie file is playable or not. 1 is playable. 0 is not playable.*/
										/*JP< 再生可能フラグ（1: 再生可能、0: 再生不可） */
	CriFloat32		buffering_time;		/*EN< Amount of time to buffer in the stream, in seconds */
										/*JP< 読み込みデータのバッファリング時間。単位[sec]。 */
	CriUint32		max_bitrate;		/*EN< Maximum bits per second for stream. This value includes video and audio both. */
										/*JP< 最大ビットレート(絵と音の合計) */
	CriUint32		max_chunk_size;		/*EN< Maximum chunk size of incoming stream (USF) file */
										/*JP< 最大USFチャンクサイズ */
	CriUint32		min_buffer_size;	/*EN< Minimum buffer size for reading */
										/*JP< 最低限必要な読み込みバッファサイズ。<br> オーディオとビデオの合計 */
	CriSint32		read_buffer_size;	/*EN< Input buffer size for reading data */
										/*JP< リードバッファサイズ */
	/* Video */
	CriUint32		num_video;			/*EN< Number of simultaneous video streams */
										/*JP< ビデオデコーダの数。現在は1固定。*/
	CriMvVideoParameters	video_prm[CRIMV_MAX_VIDEO_NUM];		/*EN< Video parameters see CriMvVideoParameters struct for details */
																/*JP< ビデオパラメータ */
	/* Audio */
	CriUint32		num_audio;			/*EN< Number of simultaneous audio streams */
										/*JP< オーディオデコーダの数。現在は1固定。*/
	CriSint32		track_play_audio;		/*EN< Track of audio playback. */
											/*JP< 再生するオーディオチャネル番号。-1指定で再生無し。 */
	CriMvAudioParameters	audio_prm[CRIMV_MAX_AUDIO_NUM];		/*EN< Audio parameters see CriMvAudioParameters struct for details */
																/*JP< オーディオパラメータ */
	/* Subtitle */
	CriUint32		num_subtitle;			/*EN< Number of subtitles */
											/*JP< 字幕チャネル数 */
	CriSint32		channel_play_subtitle;	/*EN< Channel for playing subtitles */
											/*JP< 再生する字幕チャネル番号 */
	CriUint32		max_subtitle_size;		/*EN< Maximum size of subtitle data */
											/*JP< 字幕データの最大サイズ*/

	/* Composite mode */
	CriUint32		num_alpha;				/*EN< Number of alpha channels (current spec allows only one) */
											/*JP< アルファデコーダの数。現在は1固定。 */
	CriMvAlphaParameters	alpha_prm[CRIMV_MAX_ALPHA_NUM]; /*EN< Alpha parameters see CriMvAlphaParameters struct for details */
															/*JP< アルファパラメータ */

	CriBool			seekinfo_flag;			/*EN< Flag of the movie file inclues seek info */
											/*JP< シーク情報フラグ */
	CriUint32		format_ver;				/*EN< Format version */
											/*JP< フォーマットバージョン */
} CriMvStreamingParameters;


/*EN 
 * \brief Input Buffer Information
 * \ingroup MDL_MV_INFO
 * \sa CriMvEasyPlayer::GetInputBufferInfo(),
 *     CriMvEasyPlayer::SetReloadThresholdTime(), CriMvEasyPlayer::SetBufferingTime()
 */
/*JP 
 * \brief 入力バッファ情報
 * \ingroup MDL_MV_INFO
 * \sa CriMvEasyPlayer::GetInputBufferInfo(),
 *     CriMvEasyPlayer::SetReloadThresholdTime(), CriMvEasyPlayer::SetBufferingTime()
 */
typedef struct {
	CriUint32		buffer_size;		/*EN< Input buffer size [byte] */
										/*JP< 入力バッファサイズ[byte] */
	CriUint32		data_size;			/*EN< Data size in input buffer[byte] */
										/*JP< 入力バッファにあるデータサイズ[byte] */
	CriUint32		reload_threshold;	/*EN< Re-load threshold. When data size is less than re-load threshold, next read is requested. */
										/*JP< 再読み込み閾値[byte]。データサイズがこの値以下になると読み込みを行います。 */
} CriMvInputBufferInfo;


// TEMP: for internal use
typedef struct {
	CriUint8	*imageptr;
	CriUint32	bufsize;		// [Byte]
	CriUint32	line_pitch;		// [Byte]
	CriUint32	line_size;		// [Byte]
	CriUint32	num_lines;
} CriMvImageBufferInfo;

/*EN 
 * \brief Video Frame Information
 * \ingroup MDL_MV_INFO
 * \sa CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB(), CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers,
 *     CriMvEasyPlayer::GetFrameOnTimeAsYUV422(), CriMvEasyPlayer::DiscardNextFrame()
 */
/*JP 
 * \brief ビデオフレーム情報
 * \ingroup MDL_MV_INFO
 * ビデオフレーム情報
 * \sa CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB(), CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers,
 *     CriMvEasyPlayer::GetFrameOnTimeAsYUV422(), CriMvEasyPlayer::DiscardNextFrame()
 */
typedef struct {
	CriUint8		*imageptr;		/*EN< Pointer to image data */
									/*JP< 画像データのポインタ */
	CriSint32		frame_id;		/*EN< Frame ID ot the playback */
									/*JP< フレーム識別ID（ループ／連結再生時は通算） */
	CriUint32		width;			/*EN< Width of movie frame [pixel] (multiple of 8) */
									/*JP< ムービの横幅[pixel] (８の倍数) */
	CriUint32		height;			/*EN< Height of movie frame [pixel] (multiple of 8) */
									/*JP< ムービの高さ[pixel] (８の倍数) */
	CriUint32		pitch;			/*EN< Pitch of movie frame [byte]*/
									/*JP< ムービのピッチ[byte] */
	CriUint32		disp_width;		/*EN< Width of the image to draw. */
									/*JP< 表示したい映像の横ピクセル数（左端から） */
	CriUint32		disp_height;	/*EN< Height of the image to draw. */
									/*JP< 表示したい映像の縦ピクセル数（上端から） */
	CriUint32		framerate;		/*EN< Frames per second times 1000 */
									/*JP< フレームレートの1000倍の値 */
	CriUint32		framerate_n;	/*EN< Frame rate (in rational as numerator). framerate_n/framerate_d = framerate */ /* UTODO: 変数名 */
									/*JP< フレームレートの分子(有理数形式)。framerate_n/framerate_d = framerate */
	CriUint32		framerate_d;	/*EN< Frame rate (in rational as denominator). */
									/*JP< フレームレートの分母(有理数形式)。 */
	CriUint64		time;			/*EN< Frame time ('time / tunit' indicates time in seconds) */
									/*JP< 時刻。time / tunit で秒を表す。 */
	CriUint64		tunit;			/*EN< Unit of time measurement */
									/*JP< 時刻単位 */
	CriUint32		cnt_concatenated_movie;		/*EN< Number of concatenated movie data */
												/*JP< ムービの連結回数 */
	CriSint32		frame_id_per_data;	/*EN< Frame ID of the movie data */
										/*JP< ムービデータごとのフレーム番号 */

	CriBool			csc_flag;	/*EN< This is temporary variable. Please don't access. */
								/*JP< テスト中の変数です。アクセスしないでください。 */

	CriMvAlphaType		alpha_type;		/*EN< Composite mode */
										/*JP< アルファの合成モード*/

	void			*details_ptr[CRIMV_FRAME_DETAILS_NUM];	// for internal use

	CriSint32				num_images;			// TEMP: for internal use
	CriMvImageBufferInfo	image_info[4];		// TEMP: for internal use

	CriUint32	color_conversion_type;	/*EN< Color space converion type. Fullrange or Limited.*/
										/*JP< 色変換タイプ。 */
	CriUint32	total_frames_per_data;	/*EN< Total frames of the movie data*/
										/*JP< ムービデータ単位の総フレーム数 */
	CriUint32	cnt_skipped_frames;		/*EN< Number of skipped frames to decode */
										/*JP< デコードスキップされたフレーム数 */
} CriMvFrameInfo;

/*EN 
 * \brief Subtitle Information
 * \ingroup MDL_MV_INFO
 * \sa CriMvEasyPlayer::GetSubtitleOnTime()
 */
/*JP 
 * \brief 字幕情報
 * \ingroup MDL_MV_INFO
 * \sa CriMvEasyPlayer::GetSubtitleOnTime()
 */
typedef struct {
	CriUint8		*dataptr;		/*EN< Pointer to subtitle data */
									/*JP< 字幕データのポインタ */
	CriUint32		data_size;		/*EN< Size of subtitle data */
									/*JP< 字幕データサイズ */
	CriSint32		channel_no;		/*EN< Channel number of subtitle data */
									/*JP< 字幕データのチャネル番号 */
	CriUint64		time_unit;		/*EN< Unit of time measurement */
									/*JP< 時刻単位 */
	CriUint64		in_time;		/*EN< Display start time */
									/*JP< 表示開始時刻*/
	CriUint64		duration_time;	/*EN< Display duration time */
									/*JP< 表示持続時間  */
	CriUint32		cnt_concatenated_movie;		/*EN< Number of concatenated movie data */
												/*JP< ムービの連結回数 */
	CriUint64		in_time_per_data;	/*EN< Display start time per movie data*/
										/*JP< ムービデータごとに表示開始時刻*/
} CriMvSubtitleInfo;

/*EN
 * \brief Event Point Info
 * \ingroup MDL_MV_INFO
 * Event point info is the each timing info was embeded to movie data as cue point info.
 * \sa CriMvEasyPlayer::GetCuePointInfo()
 */
/*JP
 * \brief イベントポイント情報
 * \ingroup MDL_MV_INFO
 * キューポイント機能でムービデータに埋め込まれた個々のタイミング情報です。
 * \sa CriMvEasyPlayer::GetCuePointInfo()
 */
typedef struct {
	CriChar8		*cue_name;			/*EN< The name string of event point. Char code depends on cue point text. */
										/*JP< イベントポイント名。文字コードはキューポイント情報テキストに従います。 */
	CriUint32		size_name;			/*EN< The data size of name string */
										/*JP< イベントポイント名のデータサイズ */
	CriUint64		time;				/*EN< Timer counter */
										/*JP< タイマカウント */
	CriUint64		tunit;				/*EN< Counter per 1 second. "count / unit" indicates the timer on the second time scale. */
										/*JP< １秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。 */
	CriSint32		type;				/*EN< Event point type */
										/*JP< イベントポイント種別 */
	CriChar8		*param_string;		/*EN< The string of user parameters. Char code depends on cue point text. */
										/*JP< ユーザパラメータ文字列。文字コードはキューポイント情報テキストに従います。 */
	CriUint32		size_param;			/*EN< The data size of user parameters string */
										/*JP< ユーザパラメータ文字列のデータサイズ */
	CriUint32		cnt_callback;		/*EN< The counter of calling cue point callback. */
										/*JP< キューポイントコールバックの呼び出しカウンタ */
} CriMvEventPoint;

/*EN
 * \brief Cue Point Info
 * \ingroup MDL_MV_INFO
 * Cue point info includes the number of event points and the list.
 * \sa CriMvEasyPlayer::GetCuePointInfo()
 */
/*JP
 * \brief キューポイント情報
 * \ingroup MDL_MV_INFO
 * キューポイント情報は、イベントポイントの個数と一覧です。<br>
 * \sa CriMvEasyPlayer::GetCuePointInfo()
 */
typedef struct {
	CriUint32			num_eventpoint;		/*EN< The number of event points */
											/*JP< イベントポイント個数 */
	CriMvEventPoint		*eventtable;		/*EN< The list of event points */
											/*JP< イベントポイント一覧 */
} CriMvCuePointInfo;

/*EN
 * \brief YUV Texture Buffer Parameters
 * \ingroup MDL_MV_INFO
 * The output buffer parameters for CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers().
 * CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers() outputs data for pixel shader.<BR>
 * If an application doesn't play alpha movie, CRI Movie library doesn't use alpha buffer parameters.<BR>
 * \sa CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers()
 */
/*JP
 * \brief YUV個別バッファ情報
 * \ingroup MDL_MV_INFO
 * CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers() の出力バッファ情報です。<br>
 * CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers() はPixel Shader 向けのデコード結果を出力します。<br>
 * アルファムービ再生を行わない場合（不透明の通常再生）は、Alphaテクスチャ関連のパラメータは使用しません。<BR>
 * \sa CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers()
 */
typedef struct {
	CriUint8	*y_imagebuf;	/*EN< Pointer to the buffer of Y texture */
								/*JP< Yテクスチャのバッファポインタ */
	CriUint32	y_bufsize;		/*EN< Size of the buffer of Y texture [byte] */
								/*JP< Yテクスチャのバッファサイズ[byte] */
	CriUint32	y_pitch;		/*EN< Pitch of the buffer of Y texture [byte] */
								/*JP< Yテクスチャのピッチ[byte] */
	CriUint8	*u_imagebuf;	/*EN< Pointer to the buffer of U texture */
								/*JP< Uテクスチャのバッファポインタ */
	CriUint32	u_bufsize;		/*EN< Size of the buffer of U texture [byte] */
								/*JP< Uテクスチャのバッファサイズ[byte] */
	CriUint32	u_pitch;		/*EN< Pitch of the buffer of U texture [byte] */
								/*JP< Uテクスチャのピッチ[byte] */
	CriUint8	*v_imagebuf;	/*EN< Pointer to the buffer of V texture */
								/*JP< Vテクスチャのバッファポインタ */
	CriUint32	v_bufsize;		/*EN< Size of the buffer of V texture [byte] */
								/*JP< Vテクスチャのバッファサイズ[byte] */
	CriUint32	v_pitch;		/*EN< Pitch of the buffer of V texture [byte] */
								/*JP< Vテクスチャのピッチ[byte] */
	CriUint8	*a_imagebuf;	/*EN< Pointer to the buffer of Alpha texture */
								/*JP< Alphaテクスチャのバッファポインタ */
	CriUint32	a_bufsize;		/*EN< Size of the buffer of Alpha texture [byte] */
								/*JP< Alphaテクスチャのバッファサイズ[byte] */
	CriUint32	a_pitch;		/*EN< Pitch of the buffer of Alpha texture [byte] */
								/*JP< Alphaテクスチャのピッチ[byte] */
} CriMvYuvBuffers;


/*EN
 * \brief Playback Information
 * \ingroup MDL_MV_INFO
 * The output playback information of CriMvEasyPlayer::GetPlaybackInfo().
 * These parameters represents current movie playback information such as decode delay of movie data and 
 * interval of video frames retrieval.
 * \sa CriMvEasyPlayer::GetPlaybackInfo()
 */
/*JP
 * \brief 再生情報
 * \ingroup MDL_MV_INFO
 * CriMvEasyPlayer::GetPlaybackInfo() の出力再生情報です。<br>
 * フレームの取得間隔やデコードの遅延などの現在再生しているムービの再生情報を表します。<br>
 * \sa CriMvEasyPlayer::GetPlaybackInfo()
 */
typedef struct {
	CriUint64  cnt_app_loop;						/*EN< Loop count of application. Precisely, this is a number of calls of CriMvEasyPlayer::Update(). The count up will start after app is able to acquire the first video frame */
													/*JP< アプリケーションのループカウント。具体的には CriMvEasyPlayer::Update() の呼び出し回数になります。最初のフレームが取得可能になるとカウントが始まります。*/
	CriUint64  cnt_frame_interval[4];				/*EN< Interval of video frames retrieval. These values are count up when CriMvEasyPlayer::IsNextFrameOnTime() returns TRUE.
													 *
													 * The interval of video frames retrieval indicates a number of the loop count when your application calls CriMvEasyPlayer::IsNextFrameOnTime() in the main loop. 
													 * In case that the application waits for vertical retrace, 1 interval equals about 16.7 msec.
													 * The index of array represents the count of intervals as follows:
													 * <table>
													 * <TR><TH> Index <TH> Interval of video frames retrieval
													 * <TR><TD> 0 <TD> Every main loop
													 * <TR><TD> 1 <TD> 2 main loops
													 * <TR><TD> 2 <TD> 3 main loops
													 * <TR><TD> 3 <TD> 4 or more main loops
													 * </table>													
													 * With these values, you can check if the application gets video frames with appropriate intervals. Please use the values as measuring playback smoothness
													 *
													 * In order to use these values, the application must meet the following conditions:
													 * - The main loop should work periodically and stably (Ideally sync with vertical retrace)
													 * - The application should call CriMvEasyPlayer::IsNextFrameOnTime() once in everly main loop
													 * 
													 * For example, if the application runs at 59.94fps by waiting for vertical retrace and a framerate of playing movie file is 29.97fps,
													 * only cnt_frame_interval[1] should be increased.
													 */
													/*JP< フレームの取得間隔。これらの値は、 CriMvEasyPlayer::IsNextFrameOnTime() がTRUEを返した時にカウントアップされます。<BR>
													 *
													 *  フレームの取得間隔とは、アプリケーションがメインループ内でフレーム取得関数を読んだ時のループの回数を意味します。
													 *  メインループがVSyncと同期している場合は、1 Interval = 約16.7msecということになります。
													 *  配列のインデックスは、以下のように取得間隔を表します。<BR>
													 * <table>
													 * <TR><TH> インデックス <TH> フレームの取得間隔
													 * <TR><TD> 0 <TD> 毎メインループ
													 * <TR><TD> 1 <TD> 2 メインループ
													 * <TR><TD> 2 <TD> 3 メインループ
													 * <TR><TD> 3 <TD> 4 メインループ以上
													 * </table>
													 * これらの値を見ることで、アプリが正しい間隔でフレームを取得できたのかどうかをチェックすることができます。ムービが滑らかに再生できているかの目安にしてください。<BR>
													 *
													 * ただし前提として、以下の条件をアプリが満たしている必要があります。
													 * - アプリがVSyncなど、一定の周期で安定して動作している
													 * - メインループ内で毎回 CriMvEasyPlayer::IsNextFrameOnTime() を呼び出す
													 *
													 * 上記の条件下において、例えばアプリが59.94fpsで動作している状態で、フレームレートが29.97fpsのムービを再生した場合、cnt_frame_interval[1]のみが増え続けれれば
													 * 正しい間隔でフレームの取得が出来たことになります。
													 */
	CriUint64  cnt_time_early;						/*EN< A count of how many times CriMvEasyPlayer::IsNextFrameOnTime() returns FALSE due to the determination if it is the time to provide the next video frame */
													/*JP< CriMvEasyPlayer::IsNextFrameOnTime() が、フレーム表示時刻判定によりFALSEを返した回数。*/
	CriUint64  cnt_decode_delay;					/*EN< A count of how many times CriMvEasyPlayer::IsNextFrameOnTime() returns FALSE due to the delay of decoding movie data */
													/*JP< CriMvEasyPlayer::IsNextFrameOnTime()が、ビデオフレームのデコード遅延によりFALSEを返した回数 */
	CriFloat32 time_max_delay;						/*EN< Maximum delay time [msec] of the actual time a video frame retrieved against the original time should be retrieved */
													/*JP< ビデオフレームを取得した実際の時刻と、本来表示すべき時刻との最大遅延時間 [msec]。 */
	CriFloat32 time_average_delay;					/*EN< Average delay time [msec] of the actual time a video frame retrieved against the original time should be retrieved */
													/*JP< ビデオフレームを取得した実際の時刻と、本来表示すべき時刻との平均遅延時間 [msec]。 */
} CriMvPlaybackInfo;

#if defined(XPT_TGT_PS3PPU)
/*EN
 * \brief Parameters of SPURS and PPU for decoding
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMv::SetupSpursParameters_PS3(), CriMv::CalcSpursWorkSize_PS3()
 */
/*JP
 * \brief デコードに使うSPURSおよびPPUのパラメータ
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMv::SetupSpursParameters_PS3(), CriMv::CalcSpursWorkSize_PS3()
 */
typedef	struct {
	void	*spurs_handler;			/*EN< SPURS handler	*/
									/*JP< SPURSハンドル	*/
	void	*spurs_work;			/*EN< SPURS work area. The size is spurs_worksize. The alignment is 128 byte. */
									/*JP< SPURS用ワークバッファ。バッファサイズは spurs_worksize で128バイト境界。 */
	CriSint32	spurs_worksize;			/*EN< SPURS work size. This size is calculated by CriMv::CalcSpursWorkSize_PS3 function. */
										/*JP< SPURS用ワークサイズ。CriMv::CalcSpursWorkSize_PS3 関数で取得した値。 */
	CriSint32	spurs_max_contention;	/*EN< SPURS max contention	*/
										/*JP< SPURS でムービデコード用に使うSPUの最大数	*/
	CriUint8	*spurs_task_priority;	/*EN< SPURS task priority x 8	*/
										/*JP< SPURS のタスクプライオリティ配列。配列要素は８個。	*/

	CriUint32	ppu_num;				/*EN< The number of PPU for decoding (0-2) */
										/*JP< The number of PPU for decoding (0-2) */
	CriSint32	ppu_thread_prio;		/*EN< PPU Thread Priority. This priority is used for decoding thread in the case of ppu_num equal 2. */
										/*JP< PPU Thread Priority. この値は ppu_num に2を指定した場合に作成するスレッドに使われる。	*/
} CriMvProcessorParameters_PS3;

// [NOT SUPPORT on normal library]
// for SPU Thread
typedef	struct {
	CriUint32	ppu_num;		/* The number of PPU for decoding (0-2) */
	CriSint32	ppu_prio;		/* PPU Thread Priority */
	CriUint32	spu_num;		/* The number of SPU for decoding (0-6) */
	CriSint32	spu_grp_prio;	/* SPU Thread Group Priority */
} CriMvSpuThreadParameters_PS3;

#endif

#if defined(XPT_TGT_XBOX360)
/*EN
 * \brief Parameters of Xbox360 processors for decoding
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMvEasyPlayer::SetUsableProcessors_XBOX360()
 */
/*JP
 * \brief デコードに使うXbox360プロセッサのパラメータ
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMvEasyPlayer::SetUsableProcessors_XBOX360()
 */
typedef	struct {
	CriBool	processor0_flag;	/*EN< Processor 0 (Core 0, Thread 0) usable flag  */
								/*JP< プロセッサ0 (コア0スレッド0) 使用可能フラグ */
	CriBool	processor1_flag;	/*EN< Processor 1 (Core 0, Thread 1) usable flag  */
								/*JP< プロセッサ1 (コア0スレッド1) 使用可能フラグ */
	CriBool	processor2_flag;	/*EN< Processor 2 (Core 1, Thread 0) usable flag  */
								/*JP< プロセッサ2 (コア1スレッド0) 使用可能フラグ */
	CriBool	processor3_flag;	/*EN< Processor 3 (Core 1, Thread 1) usable flag  */
								/*JP< プロセッサ3 (コア1スレッド1) 使用可能フラグ */
	CriBool	processor4_flag;	/*EN< Processor 4 (Core 2, Thread 0) usable flag  */
								/*JP< プロセッサ4 (コア2スレッド0) 使用可能フラグ */
	CriBool	processor5_flag;	/*EN< Processor 5 (Core 2, Thread 1) usable flag  */
								/*JP< プロセッサ5 (コア2スレッド1) 使用可能フラグ */
	CriSint32 thread_priority;	/*EN< Priority of decoding threads on the active processors */
								/*JP< 各プロセッサ上でデコード処理を行うスレッドの優先度 */
} CriMvProcessorParameters_XBOX360;
#endif

#if defined(XPT_TGT_VITA)
/*EN
 * \brief AVC Decoder Parameters 
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMv::SetupAvcDecoderParameters_VITA()
 */
/*JP
 * \brief AVCデコーダパラメータ
 * \ingroup MDL_MV_BASIC
 * 
 * \sa CriMv::SetupAvcDecoderParameters_VITA()
 */
typedef	struct {
	CriUint32 horizontal;		/*EN< Maximum width for decoding (in pixel)  */
								/*JP< 最大デコード画像の横幅 (単位：ピクセル) */
	CriUint32 vertical;			/*EN< Maximum height for decoding (in pixel)  */
								/*JP< 最大デコード画像の高さ (単位：ピクセル) */
	CriUint32 n_ref_frames;		/*EN< Maximum reference frames on decoding (default:3) */
								/*JP< デコード時の最大参照画像の枚数 */
	CriUint32 n_decoders;		/*EN< Maximum number of avc decoders (max:1) */
								/*JP< 同時に使用するAVCでコーダの最大数 (1固定) */
	
} CriMvAvcDecoderParameters_VITA;
#endif

/*--------------------------------------------------------------------------*/
/*		<Library Internal Use Only>											*/
/*--------------------------------------------------------------------------*/
typedef enum {
	CRIMV_PCM_FORMAT_SINT16	 	= 0,
	CRIMV_PCM_FORMAT_FLOAT32	= 1,

	/* Keep enum 4bytes */
	CRIMV_PCM_FORMAT__MAKE_ENUM_SINT32 = 0x7FFFFFFF
} CriMvPcmFormat;
/*
 * \brief 16bit wave data information
 * \ingroup MDL_MV_INFO
 * Information about a 16-bit waveform.
 */
/* 16bit Waveform 情報 */
typedef struct {
	CriUint32		num_channel;	/* Number of Channels. monaural = 1, stereo = 2 */
									/* Number of Channels. monaural = 1, stereo = 2 */
	CriUint32		num_samples;	/* Number of sample */
									/* サンプル数 */
	CriUint32		sampling_rate;	/* Sampling rate */
									/* サンプリング周波数 */
} CriMvWaveInfo;

/* オーディオヘッダ */
typedef struct {
	/* ストリーミングパラメータと共通 */
	CriUint32		sampling_rate;
	CriUint32		num_channel;
	CriUint32		total_samples;
	CriUint32		codec_type;

	CriUint32		metadata_count;
	CriUint32		metadata_size;

	/* ヘッダ固有 */
	CriUint32		a_input_xsize;
} CriMvPlyAudioHeader;

/* ビデオヘッダ */
typedef struct {
	/* ヘッダ固有 */
	CriUint32		width;
	CriUint32		height;
	CriUint32		disp_width;
	CriUint32		disp_height;
	CriUint32		framerate_n;
	CriUint32		framerate_d;
	CriUint32		total_frames;

	CriUint32		material_width;		/* width of video original source. 0 means no info. */
	CriUint32		material_height;	/* height of video original source. 0 means no info. */
	CriUint32		screen_width;		/* screen width for Wii */

	CriUint32		codec_type;
	CriUint32		codec_dc_option;	/* 11 or 10 */

	CriUint32		metadata_count;
	CriUint32		metadata_size;

	CriUint32		pre_padding;
	CriUint32		color_conversion_type;
	CriSint32		max_picture_size;
	CriSint32		average_bitrate;
} CriMvPlyVideoHeader;

/* サブタイトルヘッダ */
typedef struct {
	CriBool			is_subtitle_data;
	CriUint32		num_channel;
	CriUint64		time_unit;
	CriUint32		max_subtitle_size;
} CriMvPlySubtitleHeader;

/* キューポイントヘッダ */
typedef struct {
	CriBool			is_cuepoint_data;
	CriUint32		metadata_count;
	CriUint32		metadata_size;
	CriUint32		num_eventpoint;
	CriUint64		time_unit;
} CriMvPlyCuePointHeader;

/* アルファヘッダ */
typedef struct {
	/* ヘッダ固有 */
	CriUint32		width;
	CriUint32		height;
	CriUint32		disp_width;
	CriUint32		disp_height;
	CriUint32		framerate_n;
	CriUint32		framerate_d;
	CriUint32		total_frames;

	CriMvAlphaType	alpha_type;
	CriUint32		codec_type;

	CriUint32		metadata_count;
	CriUint32		metadata_size;

	CriUint32		pre_padding;
	CriUint32		color_conversion_type;
	CriSint32		max_picture_size;
	CriSint32		average_bitrate;
} CriMvPlyAlphaHeader;

/* アルファのみのフレーム情報 */
typedef struct {
	CriUint8		*imageptr;		/*EN< Pointer to image data */
									/*JP< 画像データのポインタ */
	CriSint32		frame_id;		/*EN< Frame ID */
									/*JP< フレーム識別ID */
	CriUint32		width;			/*EN< Width of movie frame [pixel] */
									/*JP< ムービの横幅[pixel] */
	CriUint32		height;			/*EN< Height of movie frame [pixel] */
									/*JP< ムービの高さ[pixel] */
	CriUint32		disp_width;		/*EN< Width of image [pixel] */
									/*JP< 有効な映像の横幅[pixel] */
	CriUint32		disp_height;	/*EN< Height of image [pixel] */
									/*JP< 有効な映像の高さ[pixel] */
	CriUint32		pitch;			/*EN< Pitch of movie frame [byte]*/
									/*JP< ムービのピッチ[byte] */
	CriUint64		time;			/*EN< Frame time ('time / tunit' indicates time in seconds) */
									/*JP< 時刻。time / tunit で秒を表す。 */
	CriUint64		tunit;			/*EN< Unit of time measurement */
									/*JP< 時刻単位 */
	CriSint32		frame_id_per_data;	/*EN< Frame ID of the movie data */
										/*JP< ムービデータごとのフレーム番号 */
	CriMvAlphaType	alpha_type;		/*EN< Composite mode */
									/*JP< アルファの合成モード*/
	void			*detail_ptr;	/* TEMP: for internal use */
	CriUint32	color_conversion_type;	/*EN< Color space converion type. Fullrange or Limited.*/
										/*JP< 色変換タイプ。 */
} CriMvAlphaFrameInfo;

// 内部管理用。ムービ情報をユーザに渡す時はこれとほぼ同じだろうか。
/* <Library Internal Use Only> Information of USF File */
typedef struct {
	CriBool					is_usf_file;
	CriUint32				max_chunk_size;
	CriUint32				min_buffer_size;
	CriUint32				bitrate;
	CriUint32				format_version;
	/* Video */
	CriUint32				num_video;
	CriMvPlyVideoHeader		videohead[CRIMV_MAX_VIDEO_NUM];
	/* Audio */
	CriUint32				num_audio;
	CriMvPlyAudioHeader		audiohead[CRIMV_MAX_AUDIO_NUM];
	/* Subtitle */
	CriUint32				num_subtitle;
	CriMvPlySubtitleHeader	subtitlehead;
	/* Alpha */
	CriUint32				num_alpha;
	CriMvPlyAlphaHeader		alphahead[CRIMV_MAX_ALPHA_NUM];
	/* CuePoint */
	CriUint32				num_cuepoint;
	CriMvPlyCuePointHeader	cuepointhead;
} CriMvPlyHeaderInfo;

/* <Library Internal Use Only> Video Elementary Stream */
typedef struct {
	CriUint32		fcid;
	CriSint32		track_no;	// チャンクのチャネル番号
	void			*vdec;
} CriMvPlyVideo;

typedef struct {
	CriUint32		fcid;
	CriSint32		track_no;	// チャンクのチャネル番号
	void			*dec;
} CriMvPlyAlpha;

/* ムービヘッダを管理するための構造体 */
typedef struct {
	CriMvPlyHeaderInfo	info;
	CriBool				write_new_head_flag;			// CRIDが見つかって次のヘッダを書き込む準備ができたか？
	CriUint32			num_remaining_adec_head;		// 必要な残りのオーディオヘッダの数
	CriUint32			num_remaining_vdec_head;		// 必要な残りのビデオのヘッダの数
	CriUint32			num_remaining_subtitle_head;	// 必要な残りの字幕のヘッダの数
	CriUint32			num_remaining_alpha_head;		// 必要な残りのアルファのヘッダ数
	CriUint32			num_remaining_cuepoint_head;	// 必要な残りのキューポイントのヘッダ数
	/* 2010-08-19: TEMP: CONCAT_KAI: Don't refer this member. */
	CriUint64			accumulated_tcount;
} CriMvHeaderInfoContainer;

typedef struct {
	CriBool		is_play_audio;
	CriUint32	fcid;
	CriUint32	track_no;							// チャンクのチャネル番号
	void		*adec;								// 実際のオーディオコーデック
	CriUint32	num_channel;						// データのチャネル数
	CriUint32	sampling_rate;						// サンプリング周波数
	CriUint32	output_buffer_samples;
	CriSj		sji;								// UNI
	CriSj		sjo[CRIMV_PCM_BUFFER_MAX];			// RBF
	CriUint32	sjo_bufsize[CRIMV_PCM_BUFFER_MAX];
	CriBool		term_supply;						// データ供給終了通知フラグ
	CriBool		is_working;							// コンテンツチャンク処理中

#if defined(ENABLE_DYNAMIC_AUDIO_SWITCH)
	CriUint32	next_track_no;							// ユーザが指定した切替先のトラック番号
	CriUint32	last_track_switch_time;					// 切替元のトラックの最後にチャンクをとった時刻
	CriUint32	last_track_switch_tunit;				// 上記時刻の単位 (in Hz?)
	CriMvPlyAmngTrackState	switch_state;				// トラック切替による状態
#endif
} *CriMvPlyAdec, CriMvPlyAdecObj;

typedef struct {

	CriMvPlyAdec	mvply_adecs[NUM_MAX_ADEC];
	CriSint32		num_adecs;
	CriUint32		size_smpl;
	CriSj			sji;
	CriUint32		chunk_num_per_server;

	CriBool (*cbfunc_nofify_found_header)(void *usrobj, CriChunk *ckc, CriUint8 chno);
	void *usrobj_nofify_found_header;

} *CriMvPlyAmng, CriMvPlyAmngObj;


/* シークブロック情報 */
typedef struct {
	CriSint32   top_frame_id;
} CriMvSeekBlockInfo;

/* ストリーマ用情報 */
typedef struct {
	CriUint32	max_chunk_size;
	CriUint32	average_bitrate;
} CriMvStreamerInfo;


/* ハンドル作成用コンフィグ構造体 */
typedef struct {
	CriUint32	readbuffer_size;
} CriMvHandleConfig;


/*JP CRI Movie ハンドル */
/*EN
 * A handle for a single movie.  If multiple movies are to be played simultaneously,
 * create a CriMvPly handle for each movie.
 * 
 * \sa criMvPly_Create(), criMvPly_Destroy() */
typedef struct {
	/*** Member variable is <Library Internal Use Only> ***/
	CriBool			used;
	CriMvPlyStatus	stat;
	CriBool			request_stop;
	CriBool			restrain_supply;
	CriBool			term_supply;
	void			*cs_work;
	void			*cshn;
	/* USF Header */

	CriMvHeaderInfoContainer	headinfo_container[CRIMVPLY_HEAD_CONTAINER_NUM];
	Uint16						cur_dechead_idx;
	CriUint32					cnt_dechead;		/* ヘッダ解析ごとに更新 */
	CriUint32					cnt_concat;			/* GetFrameで更新 */

	CriMvPlyHeaderInfo	headinfo;
	CriUint32			num_headck;			/* ヘッダ解析処理したチャンク数 */
	CriFloat32			def_buffering_time;
	CriUint32			def_max_stream;
	CriUint32			def_sound_output_buffer_samples;
	CriSint32			def_track_play_audio;		/* -1 でオーディオ再生無し */

	/* デリゲートパラメータ構造体 */
	CriSint32		size_dlgparams;
	CriUint8		*ptr_dlgparams;

	CriBool			is_prepare_work;
	CriMvStreamingParameters	stmprm;		/* ストリーミングパラメータの記録 */
	/* Demultiplexer */
	CriSint32		inputtype;			/* ストリーミングかメモリか？メモリ＝ユニＳＪ再生 */
	CriBool			is_usf_data;		/* 入力ファイルはUSFファイルか？ */
	void			*demux;				/* USFデマルチプレクサハンドル		*/
	CriUint32		max_demuxout;		/* デマルチプレクサ出力の最大種別数	*/
	CriUint32		num_demuxout;		/* デマルチプレクサ出力に設定済みの種別数	*/
	CriSj			headanaly_in_sj;	// RBF
	CriSj			headanaly_out_sj;	// UNI
	CriSj			read_sj;			// RBF
	CriChunk		readck;

	CriSj			memplay_sj;			// UNI (for memory playback)
	CriChunk		movie_on_mem;		/* メモリ指定のムービデータ記憶用（１個） */
	CriUint32		offset_content;		/* メモリ指定先頭データのコンテンツ本体までのサイズ */

	/* === ハンドル作成時に確保 === */
	CriHeap			heap_gen;
	/* ヘッダ解析用の読み込み領域 */
	CriUint32		headanaly_bufsize;
	/* ハンドル内部メモリは最初に10kbyte確保して使いまわす。具体的にはヘッダ解析用。 */
	CriHeap			local_heap;			/* ハンドル内部専用Heap */
	CriSint32		local_bufsize;		/* ハンドル内部専用Heap用のバッファサイズ */
	CriUint8		*local_bufptr;		/* ハンドル内部専用Heap用のバッファポインタ */
	/* === メタワークバッファ (ヘッダ解析時に確保) === */
	CriHeap			heap_meta;
	/* === ワークバッファ作成時に確保 === */
	CriHeap			heap_core;
	/* 読み込みバッファ */
	CriUint32		size_readbuf_main;
	CriUint32		size_readbuf_ext;
	/* Video Decoder */
	CriMvPlyVideo	video;
	CriUint32		framerate_n;
	CriUint32		framerate_d;
	/* Audio Decoder */
	CriMvPlyAmng	audio_mngr;
	CriMvPcmFormat	pcmfmt;
	CriUint32		size_smpl;
	CriHeap			heap_audio2;
	/* Subtitle */
	CriSj			sjo_subtitle;
	CriSint32		concat_subtitle_cnt;	/* 字幕の連結処理回数 */
	CriSint32		ch_subtitle;	/* 字幕の連結処理回数 */
	/* Alpha */
	CriMvPlyAlpha	alpha;

	/* 折り返しチャンク対応用（使うかどうかに関係なく変数だけは定義する） */
	CriUint32		bufsize_read_main;	/* 入力SJのバッファ本体サイズ */
	CriUint32		bufsize_read_ext;	/* 入力SJののりしろサイズ */
	CriUint8		*read_sj_bufptr;	/* 入力RBSJの先頭バッファアドレス */

	/* ストリーミングパラメータに入れるという手段もアリかも？ */
	CriSint32		seek_frame_id;			/* シークしたいフレームID（GOPの途中の可能性あり） */
	CriSint32		video_gop_top_id;		/* シーク後のビデオGOP先頭フレームID : 0以下でシーク無し */
	CriSint32		alpha_gop_top_id;		/* シーク後のアルファGOP先頭フレームID : 0以下でシーク無し */
	CriBool			seek_video_prep_flag;	/* シーク再生のビデオ準備完了フラグ（GOP途中まで進んだか？） */
	CriBool			seek_alpha_prep_flag;	/* シーク再生のアルファ準備完了フラグ（GOP途中まで進んだか？） */
	CriBool			seek_audio_prep_flag;	/* シーク再生のオーディオ準備完了フラグ（シーク指定時刻まで捨てたか？） */

	CriSint32		dechdr_stage;		/* DECHDRの進み具合 */
	CriSint32		sji_meta_bufsize;	/* メタデータ用入力バッファサイズ */
	CriSj			sji_meta;			/* メタデータ用入力SJ */
	CriUint32		cnt_meta_ck;		/* メタデータ用入力SJ */
	void			*video_seektbl_ptr;
	CriUint32		video_seektbl_size;
	CriSint32		video_gop_num;
	void			*alpha_seektbl_ptr;
	CriUint32		alpha_seektbl_size;
	CriSint32		alpha_gop_num;

	void			*audio_header_ptr[CRIMV_MAX_AUDIO_NUM];
	CriUint16		audio_header_size[CRIMV_MAX_AUDIO_NUM];

	void					*cuepoint_meta_ptr;
	CriUint32				cuepoint_meta_size;
	CriMvCuePointInfo		cuepoint_info;

	CriMvInputBufferInfo	ibuf_info;
	CriFloat32				reload_sec_threshold;

	CriSint32		num_frames_for_prep;

	/* For Sofdec2 */
	CriHeap		heap_playback;
	CriMvMetaDataWorkMallocFunc cbfunc_meta_alloc;
	CriMvMetaDataWorkFreeFunc cbfunc_meta_free;
	void* usrobj_meta_data;	
	void* meta_data_work_allocated;		/* ユーザアロケータで確保されたメタデータワーク */
	void* event_table_work_allocated;	/* ユーザアロケータで確保されたイベントテーブル */
	CriMvStreamerInfo streamer_info;

	/* OUTER_FRAMEPOOL_WORK */
	CriMvFramepoolWorkMallocFunc cbfunc_framepool_alloc;
	CriMvFramepoolWorkFreeFunc   cbfunc_framepool_free;
	void* usrobj_framepool;
	void* framepool_work_allocated;	/* ユーザアロケータで確保されたフレームプールワーク（解放必要） */
	void* framepool_work_set;			/* 直接バッファ指定されたフレームプールワーク（解放不要） */

	/* For Debug */
	volatile CriUint8 end_sequence_info;
	CriUint64Adr	header_ptr;
	CriSint64		header_size;
	CriUint64Adr	body_ptr;
	CriSint64		body_size;

	CriBool			sync_flag;

	CriBool			error_flag;

#if	defined(XPT_TGT_IOS)
	CriChar8		vavfios_filepath[CRIMVPLY_VAVFIOS_MAX_FILEPATH];
#endif

	/* For Tools */
	void			*extended_mvinfo_config;

	/* ハンドル作成コンフィグ関連 */
	CriBool				use_hn_config_flag;		/* ハンドル作成コンフィグ指定があったかどうか */
	CriMvHandleConfig	hn_config;

} *CriMvPly, CriMvPlyObj;

/***************************************************************************
 *      Function Declaration
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* ライブラリ初期化 */
/*
 *  \brief		ライブラリの初期化
 *	\param		なし
 *	\return		なし
 *	\par 説明:
 *	ライブラリの初期化を行います。<BR>
 *	複数回連続で初期化した場合は、最初の１回のみ初期化処理を実行します。
 */
/*
 *  \ingroup MODULE_INIT
 *  \brief		Initialize library
 *
 *  This function initializes the CRI Movie library, including internal audio,
 *  streaming and video subsystems.  This function must be 
 *  called before any other function in this library will work properly.  
 *  This function initializes only the first time it is called; if it is
 *  called again, it simply increments an internal counter and returns; it 
 *  does not re-initialize, nor does it create an error condition.
 *
 *  Therefore, it is safe to call criMvPly_Initialize() and criMvPly_Finalize()
 *  at the beginning and end, respectively, within each of the independent
 *  modules in your program.  If you match these functions call for call,
 *  only the first criMvPly_Initialize() function and the last criMvPly_Finalize()
 *  functions should have any effect.
 *
 *  \sa criMvPly_Finalize()
 */
void CRIAPI criMvPly_Initialize(void);

/* ライブラリ終了 */
/*
 *  \brief		ライブラリ終了
 *	\param		なし
 *	\return		なし
 *	\par 説明:
 *	ライブラリの終了処理を行います。<BR>
 *	複数回初期化をしていた場合は、同じ回数だけ終了処理を実行してください。
 */
/*
 * \ingroup MODULE_INIT
 *  \brief		Finalize library
 *	
 *	This function deallocates any resources in use by the library.
 *  It should be called the same number of times that the criMvPly_Initialize() 
 *  function is called.  If the criMvPly_Initialize() function is called <i>n</i>
 *  times, on the <i>n</i>th time the criMvPly_Finalize() function is called,
 *  criMvPly_Finalize() releases any resources allocated by CRI Movie.
 *
 *  Therefore, it is safe to call criMvPly_Initialize() and criMvPly_Finalize()
 *  at the beginning and end, respectively, within each of the independent
 *  modules in your program.  If you match these functions call for call,
 *  only the first criMvPly_Initialize() function and the last criMvPly_Finalize()
 *  functions should have any effect.
 *
 *  \sa criMvPly_Initialize()
 */
void CRIAPI criMvPly_Finalize(void);

/* ハンドル作成 */
/*
 *  \brief		ハンドル作成
 *  \param		heap : メモリ確保に使用するHeapハンドル
 *	\return		CriMvPlyハンドル
 *	\par 説明:
 *	CRI Movie ハンドルを作成します。<BR>
 *	必要なハンドル管理領域はHeapハンドルを使って自動的に確保します。<BR>
 *	作成直後のハンドル状態はSTOP状態です。
 */
/*
 *  \ingroup MODULE_INIT
 *  \brief		Create a handle
 *  \param		heap A valid CriHeap handle
 *	\return		A valid CriMvPly handle, or NULL if the handle cannot be allocated
 *	
 *  This function creates a CriMvPly handle in the CRIMVPLY_STATUS_STOP state.
 * Memory for the handle is allocated from the CriHeap structure that you provide.
 * Any memory allocation failure during this function results in a text error message to output, and the program hangs.
 * Make sure to initialize and create your heap with criHeap_Initialize() and
 * criHeap_Create() before calling this function.
 *
 * \sa CriMvPly, CriMvPlyStatus, criHeap_Initialize(), criHeap_Create()
 */
CriMvPly CRIAPI criMvPly_Create(CriHeap heap);

/* コンフィグ指定のハンドル作成 */
/* config がNULL指定の場合はコンフィグ指定無しと同様 */
CriMvPly CRIAPI criMvPly_CreateWithConfig(CriHeap heap, CriMvHandleConfig *config);

/* ハンドル破棄 */
/*
 *  \brief		ハンドル破棄
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	CRI Movie ハンドルの破棄を行います。<BR>
 *	ハンドル作成時に引数で指定したHeapハンドルを使って、ハンドル管理領域を解放します。<BR>
 *	ワークバッファを確保したままの場合は、ワークバッファ確保時に指定したHeapハンドルを使ってワークバッファも解放します。<BR>
 *	ハンドルの破棄は、ハンドル状態がSTOPかPLAYENDの時にしか実行できません。
 */
/*
 *  \ingroup MODULE_INIT
 *  \brief		Destroy a handle
 *  \param		mvply A valid CriMvPly handle to be destroyed
 *	
 *	This function destroys the CriMvPly handle previously created
 *  with criMvPly_Create().
 *
 *  You can only destroy the handle if it is in either the CRIMVPLY_STATUS_STOP or the
 *  the CRIMVPLY_STATUS_PLAYEND states.  Attempting to destroy the handle in
 *  any other state will produce an error message.  You can check the status of
 * the handle at any time with criMvPly_GetStatus().
 *
 *  Any work buffers allocated via criMvPly_AllocateWorkBuffer(), if they are
 * still associated with the handle, are freed when criMvPly_Destroy() is called.
 *
 * \sa CriMvPly, CriMvPlyStatus, criMvPly_GetStatus(), criMvPly_AllocateWorkBuffer()
 */
void CRIAPI criMvPly_Destroy(CriMvPly mvply);

/* ストリーミングパラメータの取得 */
/*
 *  \brief		ストリーミングパラメータの取得
 *  \param		mvply : CRI Movie ハンドル
 *  \param		stmprm : ストリーミングパラメータ
 *	\return		なし
 *	\par 説明:
 *	ヘッダ解析の結果をもとに、ムービ再生に必要なストリーミングパラメータを取得します。<BR>
 *	ハンドル状態がWAIT_PREPになると取得できるようになります。<BR>
 *	このパラメータをもとにcriMvPly_AllocateWorkBuffer関数を呼び出すことができます。<BR>
 *	必要ならばこのパラメータの値を変更して、例えば音ありムービで音を再生しない、といったことも可能です。<BR>
 */
/*
 *  \ingroup MODULE_BUFFER
 *  \brief		Get streaming parameters
 *  \param		mvply A valid CriMvPly handle
 *  \param		stmprm An empty CriMvStreamingParameters structure to be filled with data
 *	
 * This function permits you to get detailed information about the stream and
 * dynamically allocate resources  just before the video and audio sequence
 * begins playback.
 *
 * This function does nothing if the current state of the CriMvPly handle is 
 * CRIMVPLY_STATUS_DECHDR or CRIMVPLY_STATUS_STOP.  The only useful state in
 * which to call criMvPly_GetStreamingParameters() is the CRIMVPLY_WAIT_PREP
 * status.  When the CriMvPly handle is in the CRIMVPLY_WAIT_PREP status,
 * calling this function will cause the CriMvStreamingParameters field to be
 * filled with data.
 *
 * Some of the CriMvStreamingParameters, such as buffering time and the 
 * maximum number of files to read, are copied from the CriMvPly structure.
 * However, maximum bitrate, video size, audio stream rate and channel
 * info are calculated from the incoming stream.
 *
 * After calling criMvPly_GetStreamingParameters(), you can programmatically 
 * override any of the fields in the CriMvStreamingParameters struct yourself
 * before calling criMvPly_AllocateWorkBuffer() with it.  For example, you might
 * need to read a stream containing both audio and video, but only output the
 * video from the stream.  In this case you could allocate trivial audio buffers 
 * for output by modifying the CriMvStreamingParameters struct accordingly after
 * calling this function.
 *
 * \sa criMvPly_AllocateWorkBuffer(), CriMvPly, CriMvStreamingParameters
 */
 void CRIAPI criMvPly_GetStreamingParameters(CriMvPly mvply, CriMvStreamingParameters *stmprm);

/* ワークバッファの確保 */
/*
 *  \brief		ワークバッファの確保
 *  \param		mvply : CRI Movie ハンドル
 *  \param		heap : 
 *  \param		stmprm : ストリーミングパラメータ
 *	\return		なし
 *	\par 説明:
 *	引数のHeapハンドルを使って、読み込みバッファやビデオ／オーディオのワークバッファを確保します。<BR>
 *	この関数を呼び出し可能なのは、STOP状態かWAIT_PREP状態の時のみです。<BR>
 *	同じハンドルに対して２度呼び出すと、１度目のワークバッファを全て解放してから、改めてワークバッファを確保します。<BR>
 *	criMvPly_Start関数よりも先にワークバッファを確保しておくこともできます。
 */
/*
 *  \ingroup MODULE_BUFFER
 *  \brief		Allocate internal streaming work buffers
 *  \param		mvply A CriMvPly handle
 *  \param		heap A CriHeap handle
 *  \param		stmprm An initialized CriMvStreamingParameters structure
 * 
 * This function allocates internal streaming buffers for the CriMvPly movie
 * player from the CriHeap.  The amount of memory required is based
 * on the maximum bitrate of the stream, the requested buffering time, the
 * maximum chunk size, and the height and width of the incoming video frame.
 * However, a small amount of memory is allocated for the
 * video and audio decoders from the CriHeap as well.
 *
 * The CriHeap handle passed as a parameter to this function need not 
 * be the same CriHeap handle you passed to the criMvPly_Create() function.
 * You may prefer to use either one or two heaps.
 *
 * This function can be called only if the CriMvPly handle is in the
 * CRIMVPLY_STATUS_STOP or the CRIMVPLY_STATUS_WAIT_PREP status.  Calling
 * this function any other time will produce an error message.
 * 
 * If this function is called twice without calling criMvPly_FreeWorkBuffer(),
 * it releases the previously allocated buffers before allocating them again.
 *
 * This function must be called sometime before criMvPly_Start().
 *
 * \if ps2
 * \par PS2 only:
 * The PS2 implementation of this function additionally allocates buffers for
 * internal DMA tags.  If these allocations fail due to lack of memory, the
 * library will hang.
 * \endif
 *
 * \sa CriMvPly, CriHeap, CriMvPlyStatus, CriMvStreamingParameters, criMvPly_FreeWorkBuffer(),
 * criMvPly_Start()
 */
CriBool CRIAPI criMvPly_AllocateWorkBuffer(CriMvPly mvply, CriHeap heap, CriMvStreamingParameters *stmprm);

/* ワークバッファの解放 */
/*
 *  \brief		ワークバッファの解放
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	criMvPly_AllocateWorkBuffer関数で確保したワークバッファを全て解放します。<BR>
 *	この関数を呼び出し可能なのは、STOP／WAIT_PREP／PLAYEND状態の時のみです。<BR>
 *	CRI Movie Ver.0.60 では未実装です。
 */
/*
 * \ingroup MODULE_BUFFER
 *  \brief		Release streaming work buffer
 *  \param		mvply A valid CriMvPly handle
 *	
 * This function releases streaming work buffers allocated from the CriHeap
 * previously associated with criMvPly_AllocateWorkBuffer().
 * This function should only be called at CRIMVPLY_STATUS_STOP,
 * CRIMVPLY_STATUS_WAIT_PREP or CRIMVPLY_STATUS_PLAYEND states.  However, this
 * function does not verify the current stream status before releasing
 * all the buffers; it merely deallocates them.  Expect interesting crashes
 * if you call this function while playing a movie.
 * 
 * You can verify the current CriMvPly handle status with criMvPly_GetStatus() if
 * necessary.
 *
 */
void CRIAPI criMvPly_FreeWorkBuffer(CriMvPly mvply);

/* 再生するオーディオチャネルの設定 */
/*
 *  \brief		再生するオーディオチャネルの設定
 *  \param		mvply : CRI Movie ハンドル
 *  \param		ch : オーディオチャネル番号
 *	\return		なし
 *	\par 説明:
 *	CriMvStreamingParameters構造体のメンバ track_play_audioのデフォルト値を設定します。<BR>
 *	criMvPly_GetStreamingParameters関数でCriMvStreamingParameters構造体を取得したときにこの値が格納されます。<BR>
 *	何も設定していない場合、buffering_timeには0が入います。
 *	-1を指定するとオーディオを再生しない設定になります。
 */
void CRIAPI criMvPly_SetAudioTrack(CriMvPly mvply, CriSint32 track);

/* バッファリング時間(単位[sec])の設定 */
/*
 *  \brief		バッファリング時間(単位[sec])の設定
 *  \param		mvply : CRI Movie ハンドル
 *  \param		time : バッファリング時間
 *	\return		なし
 *	\par 説明:
 *	CriMvStreamingParameters構造体のメンバbuffering_timeのデフォルト値を設定します。<BR>
 *	criMvPly_GetStreamingParameters関数で CriMvStreamingParameters構造体を取得したときにこの値が格納されます。<BR>
 *	何も設定していない場合、buffering_timeには1.0秒が入っています。
 */
/*
 * \ingroup MODULE_BUFFER
 *  \brief		Set default buffering time (unit[sec])
 *  \param		mvply A valid CriMvPly handle
 *  \param		time Buffering time in seconds
 *	
 * This function tells the CriMvPly handle how much time of the stream to buffer in
 * memory.  Buffering is necessary to cover seeks, error retries, latency and
 * other various hiccups in most data sources.
 * 
 * This value is stored in the buffering_time field of the CriMvStreamingParameters
 * struct.  It is set to a default of 1.0 seconds when the CriMvPly handle is created.
 * This is typically safe for most DVD type file systems.
 *
 * \sa CriMvPly, CriMvStreamingParameters
 */
//void CRIAPI criMvPly_SetBufferingTime(CriMvPly mvply, CriFloat32 time);

/* 同時読み込みファイル数の設定 */
/*
 *  \brief		同時読み込みファイル数の設定
 *  \param		mvply : CRI Movie ハンドル
 *  \param		max_stm : 同時読み込みファイル数
 *	\return		なし
 *	\par 説明:
 *	CriMvStreamingParameters構造体のメンバmax_simultaneous_read_filesのデフォルト値を設定します。<BR>
 *	criMvPly_GetStreamingParameters関数で CriMvStreamingParameters構造体を取得したときにこの値が格納されます。<BR>
 *	何も設定していない場合、max_simultaneous_read_filesには1が入っています。
 */
/*
 * \ingroup MODULE_BUFFER
 *  \brief		Set  maximum number of simultaneous streams
 *  \param		mvply A valid CriMvPly handle
 *  \param		max_stm The maximum number of simultaneous streams
 *	
 *	This function sets the default value of the "max_simultaneous_read_files" field of
 * the CriMvStreamingParameters struct.  Currently, setting this value has no effect.
 */
//void CRIAPI criMvPly_SetMaxSimultaneousStreams(CriMvPly mvply, CriUint32 max_stm);

/* サウンド出力バッファサンプル数の設定 */
/*
 *  \brief		GetWave16で要求する最大サンプル数の設定
 *  \param		mvply : CRI Movie ハンドル
 *  \param		max_smpl : 最大サンプル数
 *	\return		なし
 *	\par 説明:
 *	CriMvAudioParameters構造体のメンバoutput_buffer_samplesのデフォルト値を設定します。<BR>
 *	criMvPly_GetStreamingParameters関数で CriMvStreamingParameters構造体を取得したときにこの値が格納されます。<BR>
 *	何も設定していない場合、output_buffer_samplesには16*1024が入っています。
 */
/*
 *  \ingroup MODULE_AUDIO
 *  \brief		Set default sound output buffer samples
 *  \param		mvply A valid CriMvPly handle
 *  \param		smpls : sound output buffer samples
 *	
 *	This function sets the default value of the "output_buffer_samples" field in the
 * CriMvAudioParameters struct.  The default value is 16384, which is set when
 * criMvPly_Create() is called.
 *
 *  This function only has an effect if it is called before the
 * criMvPly_AllocateWorkBuffer() function is called, since this is when the audio
 * output buffer is allocated.
 *
 * \sa CriMvAudioParameters, criMvPly_Create(), criMvPly_AllocateWorkBuffer(),
 * criMvPly_GetWave16()
 */
//void criMvPly_SetMaxSamplesOfGetWave16(CriMvPly mvply, CriUint32 max_smpl);
void CRIAPI criMvPly_SetSoundOutputBufferSamples(CriMvPly mvply, CriUint32 smpls);

/* ハンドル状態の取得 */
/*
 *  \brief		ハンドル状態の取得
 *  \param		mvply : CRI Movie ハンドル
 *	\return		ハンドル状態
 *	\par 説明:
 *	ハンドル状態を取得します。
 */
/*
 *  \ingroup MODULE_STATE
 *  \brief		Get the handle status
 *  \param		mvply A valid CriMvPly handle
 *	\return		One of the CriMvPlyStatus enum values
 *	
 *	This function gets the current status of the CRI Movie handle.  Check the
 * following link for possible return values.
 *
 * \sa CriMvPlyStatus
 */
CriMvPlyStatus CRIAPI criMvPly_GetStatus(CriMvPly mvply);

/* WAIT状態から次の状態への遷移通知 */
/*
 *  \brief		WAIT状態から次の状態への遷移通知
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	ハンドル状態をWAIT_**** 状態から次の状態に遷移させます。状態に応じて次のように使用します。<BR>
 *	・WAIT_PREP 状態 : criMvPly_AllocateWorkBuffer関数でワークを確保しおわったら呼び出してください。<BR>
 *	・WAIT_PLAYING 状態 : ビデオフレーム、オーディオデータを取得して表示・出力の準備ができたら、
 *							出力を開始して、本関数を呼び出してください。<BR>
 *	・WAIT_PLAYEND 状態 : 最後のビデオフレームの表示、最後のオーディオデータの出力が終了したら呼び出してください。<BR>
 *	・WAIT_STOP 状態 : ビデオやオーディオの出力が停止してもいい状態になったら、呼び出してください。<BR>
 *	本関数を呼び出すと各状態は即座に次の状態に遷移します。<BR>
 *	本関数を WAIT_**** 以外の状態で呼び出しても、状態は何も変わりません。
 */
/*
 *  \ingroup MODULE_STATE
 *  \brief		Notify transition from WAIT status
 *  \param		mvply A valid CriMvPly handle
 *	
 *	This function notifies the CriMvPly handle that your application is ready to
 * go from the current WAIT state to the next state.  There are exactly four states
 * in which it is appropriate to call this function:
 * 
 * - CRIMVPLY_STATUS_WAIT_PREP After your application has allocated buffers with
 *		criMvPly_AllocateWorkBuffer()
 * - CRIMVPLY_STATUS_WAIT_PLAYING After your application has prerolled stream data (if
 * 		necessary)
 * - CRIMVPLY_STATUS_WAIT_PLAYEND After your application has displayed the last frames of audio and
 *		video from the stream
 * - CRIMVPLY_STATUS_WAIT_STOP After your application suspends playback from the stream
 *
 * You can check the current status of the CriMvPly handle by calling the criMvPly_GetStatus()
 * function.  This function has no effect if called in states other than those listed above.
 *
 * \sa CriMvPly, CriMvPlyStatus
 */
void CRIAPI criMvPly_IncrementState(CriMvPly mvply);

/* 状態の更新 */
/*
 *  \brief		CriMvPlyモジュールのサーバ関数
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 * 主にデマルチプレクサ内部のデータの更新を行います。<br>
 *	本関数はアプリケーションのメインスレッド側でで毎回呼び出すようにしてください。<BR>
 */
void CRIAPI criMvPly_Update(CriMvPly mvply);

/* 再生開始 */
/*
 *  \brief		再生開始
 *  \param		mvply : CRI Movie ハンドル
 *	\return		再生開始できた場合はCRI_TRUE, 失敗した場合はCRI_FALSE
 *	\par 説明:
 *	再生のための処理を開始します。<BR>
 *	本関数呼出し後、ハンドル状態はDECHDRに遷移します。<BR>
 */
/*
 *  \ingroup MODULE_STATE
 *  \brief		Start of playback processing
 *  \param		mvply A valid CriMvPly handle
 *	
 *  This function initiates playback processing.  This function should be called
 * after the CriMvPly handle is created and the data source has been opened,
 * but before the work buffers are allocated with criMvPly_AllocateWorkBuffer().
 * This function sets the current status of the CriMvPly handle to 
 * CRIMVPLY_STATUS_DECHDR, which prepares it to decode the header information
 * from the data source.
 *
 * \sa CriMvPly, CriMvPlyStatus, criMvPly_AllocateWorkBuffer()
 */
CriBool CRIAPI criMvPly_Start(CriMvPly mvply);

/* 再生停止リクエスト(即時復帰) */
/*
 *  \brief		再生停止リクエスト(即時復帰)
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	再生停止のリクエストを発行して即時復帰します。<BR>
 *	本関数呼出し後、ハンドル状態はSTOP_PROCESSING状態に遷移します。<BR>
 *	停止のための処理が終わると、ハンドル状態がWAIT_STOPに遷移します。<BR>
 *	WAIT_STOP状態になったら、criMvPly_IncrementState関数でSTOP状態に遷移させて、
 *	アプリケーションの停止処理を行ってください。
 */
/*
 *  \ingroup MODULE_STATE
 *  \brief		Non-blocking request to stop playback
 *  \param		mvply A currently playing CriMvPly handle
 *	
 *	This function records a request to terminate playback.  Termination of 
 * playback is not synchronous to this function; this function sets the current
 * state of the CriMvPly handle to CRIMVPLY_STATUS_STOP_PROCESSING.  After
 * movie processing is halted, the state of the handle transitions to
 * CRIMWPLY_STATUS_WAIT_STOP.
 *
 *	This function is useful for prematurely terminating a movie, e.g. "press
 * X to skip this movie".
 *
 * Video frames will keep being delivered until you detect a CRIMVPLY_STATUS_WAIT_STOP
 * state in the CriMvPly handle, and then call criMvPly_IncrementState to transition
 * back to the CRIMVPLY_STATUS_STOP state.
 *
 * \note Pausing is not accomplished through this function.  The system clock, including
 * whether or not to pause or advance frames, is controlled entirely through user
 * code.  So the effect of "pausing" a CriMvPly handle can be accomplished by simply
 * not updating your system clock as long as your pause is in effect.
 *
 * \sa CriMvPly, CriMvPlyStatus
 */
void CRIAPI criMvPly_Stop(CriMvPly mvply);

/* サーバ処理(ハンドル指定) */
/*
 *  \brief		サーバ処理(ハンドル指定)
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	CRI Movie ハンドルを指定してサーバ処理を実行します。<br>
 *	各WAIT_**** 状態への状態遷移はサーバ関数内で実行されます。
 */
/*
 *  \ingroup MODULE_VIDEO
 *  \brief		Execute heartbeat functions for a handle
 *  \param		mvply A valid CriMvPly handle
 *	
 *	This function executes heartbeat functions for the specified CriMvPly handle only,
 * including handoff and parsing of input buffers and audio decoding.  Additionally,
 * it checks for buffer situations in which the CriMvPly handle should transition to 
 * one of the four WAIT states of CriMvPlyStatus, and it makes these transitions if
 * necessary.
 *
 * However, video decoding does NOT occur in criMvPly_Execute().
 * 
 * Expect that criMvPly_Execute() will take a relatively low CPU load. Typically,
 * this function should be called on every vertical blank.  However, it may be called
 * more frequently in a CriMvPly wait state, in conjunction with criMvPly_IncrementState(),
 * in order to "force" a transition into the next state without waiting for 
 * another vertical blank.  This type of transition is not generically
 * necessary.
 * 
 * \sa CriMvPly, CriMvPlyStatus, criMvPly_ExecuteAll()
 */
void CRIAPI criMvPly_Execute(CriMvPly mvply);

/* ファイル読み込みバッファの空きチャンク取得 */
/*
 *  \brief		ファイル読み込みバッファの空きチャンク取得
 *  \param		mvply : CRI Movie ハンドル
 *  \param		ck : チャンク
 *	\return		なし
 *	\par 説明:
 *	ファイル読み込みバッファの空き領域を取得します。<BR>
 *  取得した空き領域（チャンクと呼びます）は、データ書き込みを通知するさいに
 *  そのまま使用しますので、アプリケーションで記憶してください。<BR>
 *	１度に取得できるチャンクは１つのみです。<BR>
 *  チャンクが取得できたか否かは、チャンクのサイズで判定できます。<BR>
 *	データの書き込みが終わったら、criMvPly_PutInputChunk関数で書き込みサイズを通知してください。<BR>
 */
/*
 *  \ingroup MODULE_SUPPLY
 *  \brief		Get a free chunk from file reading buffer
 *  \param		mvply A valid CriMvPly handle
 *  \param		ck A CriChunk structure to be filled with data by this function
 *
 *  This function selects an empty internal buffer for your data source to read its data into.
 * An area of this type is referred to as a "chunk."
 * After calling this function, the ck->data and ck->size
 * fields will provide a valid pointer and size, respectively, that your data 
 * source should copy its data into.
 * If no buffers can internally be allocated, this function will return 0 as the
 * ck->size field.  If this occurs, your program should choke input until a free
 * buffer can be allocated.
 * Typical data sources are native file reading, sequential memory access, or
 * playback from a network source.
 * The ck->size field is dynamically calculated when criMvPly_AllocateWorkBuffer() 
 * is called; it is calculated based on expected data rate and video resolutions 
 * embedded in the stream file.
 *
 * After you receive a valid chunk from criMvPly_GetInputChunk(),
 * you can fill the provided chunk up to the ck->size limit.  After you 
 * fill the chunk with valid data, call the criMvPly_PutInputChunk() function
 * to queue the data for processing. The functions criMvPly_GetInputChunk() and
 * criMvPly_PutInputChunk() should be called in equal pairs; calling these
 * functions out of order will produce odd results.  No dynamic chunk reordering
 * is permitted; serial calls with out-of-order chunks will be
 * flagged at run-time as an error.
 *
 * This function will only return a valid chunk if the CriMvPly handle is in
 * one of two playback states: CRIMVPLY_STATUS_PLAY or CRIMVPLY_STATUS_DECHDR.
 * You can verify the current playback state with criMvPly_GetStatus().
 * 
 * \sa criMvPly_GetStatus(), criMvPly_PutInputChunk(), criMvPly_AllocateWorkBuffer(), 
 * CriChunk, CriMvPlyStatus
 */
void CRIAPI criMvPly_GetInputChunk(CriMvPly mvply, CriChunk *ck);

/* ファイル読み込みバッファへのデータ書き込み通知 */
/*
 *  \brief		ファイル読み込みバッファへのデータ書き込み通知
 *  \param		mvply : CRI Movie ハンドル
 *  \param		ck : 
 *  \param		inputsize : 
 *	\return		なし
 *	\par 説明:
 *	criMvPly_GetInputChunk関数で取得した空き領域（チャンクと呼びます）に
 *  データを書き込み終わったら、引数inputsizeにデータサイズを入れて本関数を呼び出してください。<BR>
 *  その際、チャンクは criMvPly_GetInputChunk関数で取得したものと同じチャンクを必ず指定してください。
 */
/*
 *  \ingroup MODULE_SUPPLY
 *  \brief		Put a data chunk into file read buffer
 *  \param		mvply A valid CriMvPly handle
 *  \param		ck A CriChunk structure containing source data
 *  \param		inputsize The number of bytes actually supplied
 *
 *  This function informs the CriMvPly handle that the CriChunk structure
 * now contains valid data from the data source.  Typically, you would call
 * this function after your asynchronous file read reports that the buffer is 
 * full of data.
 *
 * The inputsize field should contain the number of bytes actually provided.
 * This value can be less than or equal to ck->size.  In an end-of-file condition,
 * be sure to supply the actual number of bytes remaining in the file, and not
 * merely the size of the input buffer, to criMvPly_PutInputChunk().
 * Do not modify the contents of the CriChunk after calling this function;
 * instead, call criMvPly_GetInputChunk() to get a new chunk for further input.
 *
 * This function invalidates the CriChunk provided if the function is called
 * while the CriMvPly handle is in the CRIMVPLY_STATUS_STOP, the 
 * CRIMVPLY_STATUS_WAIT_PREP, or the CRIMVPLY_STATUS_STOP_PROCESSING state.
 * You can verify the current playback state with criMvPly_GetStatus().
 *
 * After you receive a valid chunk from criMvPly_GetInputChunk(),
 * you can fill the provided chunk up to the ck->size limit.  After you call
 * fill the chunk with valid data, call the criMvPly_PutInputChunk() function
 * to queue the data for processing. The function criMvPly_GetInputChunk() and
 * criMvPly_PutInputChunk() should be called in equal pairs; calling these
 * functions out of order will produce odd results.  No dynamic chunk reordering
 * is permitted; serial calls with out-of-order chunks will be
 * flagged at run-time as an error.
 *
 * After putting the final chunk of data in the stream, call
 * criMvPly_TerminateSupply() to indicate that an end-of-file condition exists.
 * 
 *  \sa criMvPly_GetStatus(), criMvPly_GetInputChunk(), criMvPly_AllocateWorkBuffer(), 
 * criMvPly_TerminateSupply(), CriChunk, CriMvPlyStatus
 */
void CRIAPI criMvPly_PutInputChunk(CriMvPly mvply, CriChunk *ck, CriUint32 inputsize);

/* ファイル読み込み終了の通知 */
/*
 *  \brief		ファイル読み込み終了の通知
 *  \param		mvply : CRI Movie ハンドル
 *	\return		なし
 *	\par 説明:
 *	再生したい全てのデータを読み込んで、 criMvPly_PutInputChunk関数で通知し終わったら、
 *	本関数でファイル読み込み終了の通知を必ず行ってください。<BR>
 *	終了を通知された時点で読み込みバッファに書き込まれた全てのデータをデコードし終わると、
 *	ハンドル状態はWAIT_PLAYENDに遷移します。<BR>
 *	本関数を呼び出さない限り、WAIT_PLAYEND状態になることはありません。
 */
/*
 *  \ingroup MODULE_SUPPLY
 *  \brief		Notify end of reading data
 *  \param		mvply A valid CriMvPly structure
 *	
 * After you put all the source data into the CriMvPly handle with
 * criMvPly_PutInputChunk(), indicate the end of the movie file by calling
 * criMvPly_TerminateSupply().  After calling this function, the CriMvPly
 * handle's status is changed by the library to CRIMVPLY_STATUS_WAIT_PLAYEND
 * and the library completes processing of whatever frames it has internally
 * buffered. 
 *
 * If you do not call this function, the CriMvPly handle will never transition
 * to the CRIMVPLY_STATUS_WAIT_PLAYEND state, making teardown impossible.
 *
 * \sa criMvPly_PutInputChunk(), CriMvPly, CriMvPlyStatus
 */
void CRIAPI criMvPly_TerminateSupply(CriMvPly mvply);

#if 0//defined(XPT_TGT_EE)
/* RGB32フォーマットのビデオフレームの取得 */
/*
 * \if ps2
 *  \brief		RGB32フォーマットのビデオフレームの取得
 *  \param		mvply : CRI Movie ハンドル
 *  \param		imagebuf : ビデオフレームバッファ
 *  \param		bufsize : バッファサイズ
 *  \param		frameinfo : フレーム情報
 *	\return		取得できた場合はTRUE, できなかった場合はFALSE
 *	\par 説明:
 *	引数で指定したバッファに、PS2のマクロブロック並びRGBA32フォーマットでフレームを取得します。<BR>
 *	引数CriMvFrameInfo構造体には、取得したフレームについての情報が格納されます。<BR>
 *	フレームが取得できるのは、ハンドル状態がWAIT_PLAYING／PLAYINGの時のみです。<BR>
 *	それ以外の状態で呼び出す、または入力データ不足の場合には、本関数はフレームの取得に失敗し、即座に復帰します。<BR>
 *	フレームが取得できなかった場合は、関数値でFALSEが返ります。<BR>
 *	実際のビデオデコード処理も本関数内で動くため、フレーム取得できる場合には、処理の重い関数となります。
 * \endif
 */
/*
 * \if ps2
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame in PS2 macroblock RGB32 format
 *  \param		mvply A valid CriMvPly handle
 *  \param		imagebuf a pointer to the video buffer in memory to receive the frame
 *  \param		bufsize video buffer size in bytes
 *  \param		frameinfo Information about the decoded frame
 *	\return		returns true if a frame has been copied into the buffer, false otherwise
 *	
 *  This function finds and decodes the current frame, if any, to the imagebuf buffer.
 *  The format of this buffer is specific to the PS2.  This function will only return
 * a valid frame if the CriMvPly handle is in the CRIMVPLY_STATUS_WAIT_PLAYING or the
 * CRIMVPLY_STATUS_PLAYING state.  
 *
 * This function is an EE-intensive activity, and the amount of time required
 * is variable, depending on the complexity and size of the video frame being decoded.
 * Therefore the preferred method of calling this function is in a low priority thread,
 * separate from your I/O, buffer management, and criMvPly_Execute() routines.
 * 
 * If this function returns true, the preferred display time of the video frame is calculated
 * as:
 *
 * \code
 * frameinfo.time / frameinfo.tunit 
 * \endcode
 * 
 * Humans notice audio stuttering much more readily than a dropped video frame during
 * a video decode process.  If the frame reported by criMvPly_GetFrameRGBA32_PS2()
 * arrives after your system clock says the frame should be displayed, you should
 * simply drop the frame without bothering to DMA it to video memory.
 * 
 * Here is an example showing how to drop frames in this case:
 *
 * \dontinclude crimvt01_simple_playback_ps2.c
 * \skip Get video frame
 * \until *tutor_update_video_frame_on_display_time*
 * 
 * It is not preferred, but it is possible, to call this function in a single-threaded
 * playback model.  In this case, it is important to allocate more heap space
 * and service the CriMvPly handle with criMvPly_Execute() or criMvPly_ExecuteAll()
 * frequently, as well as criMvPly_GetWave16() frequently.  This helps to cover
 * for the case where other I/O needs to occur when a frame is currently being decoded by
 * criMvPly_GetFrameRGBA32_PS2().
 *
 * \image html crimvply_getframergba32_ps2.png The DMA reordering step on PS2
 * 
 * In order to get acceptable performance on the PS2, a macroblock reordering step
 * must take place during the DMA transfer from EE RAM to video RAM.
 * \endif
 */
CriBool CRIAPI criMvPly_GetFrameRGBA32_PS2(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 bufsize, CriMvFrameInfo *frameinfo);

CriBool CRIAPI criMvPly_DecodeFrameRGBA32_PS2(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 bufsize, CriMvFrameInfo *frameinfo);
#endif

#if defined(XPT_TGT_PC) || defined(XPT_TGT_XBOX360) || defined(XPT_TGT_WII) || defined(XPT_TGT_SH7269)  || defined(XPT_TGT_TRGP6K)
/*
 * \if pc
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame in YUV422 format
 *  \param		mvply A valid CriMvPly handle
 *  \param		imagebuf A pointer to the video buffer in memory to receive the frame
 *  \param      pitch Number of bytes in one row of the video buffer
 *  \param		bufsize Video buffer size in bytes
 *  \param		frameinfo Information about the decoded frame
 *	\return		returns true if a frame has been copied into the buffer, false otherwise
 *	
 *  This function finds and decodes the current frame, if any, to the imagebuf buffer.
 * This function will only return a valid frame if the CriMvPly handle is in the
 * CRIMVPLY_STATUS_WAIT_PLAYING or the CRIMVPLY_STATUS_PLAYING state.  
 *
 * This function is a CPU-intensive activity, and the amount of time required
 * is variable, depending on the complexity and size of the video frame being decoded.
 * Therefore the preferred method of calling this function is in a low priority thread,
 * separate from your I/O, buffer management, and criMvPly_Execute() routines.
 * 
 * If this function returns true, the preferred display time of the video frame is calculated
 * as:
 *
 * \code
 * frameinfo.time / frameinfo.tunit 
 * \endcode
 * 
 * It is not preferred, but it is possible, to call this function in a single-threaded
 * playback model.  In this case, it is important to allocate more heap space
 * and service the CriMvPly handle with criMvPly_Execute() or criMvPly_ExecuteAll()
 * frequently, as well as criMvPly_GetWave16() frequently.
 *
 * \endif
 */
CriBool CRIAPI criMvPly_GetFrameYUV422(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo *frameinfo);
#endif

/* 16bit WAVEフォーマットのオーディオデータ取得 */
/*
 *  \brief		16bit WAVEフォーマットのオーディオデータ取得
 *  \param		mvply : CRI Movie ハンドル
 *  \param		nch : チャネル数
 *  \param		waveptr : オーディオデータバッファ
 *  \param		wavesmpl : 要求サンプル数(＜バッファサイズ)
 *  \param		waveinfo : 16bit Waveform 情報
 *	\return		取得できたサンプル数
 *	\par 説明:
 *	16bitのWAVEフォーマットでオーディオデータを取得します。引数waveptrには、nch分のバッファポインタを格納した
 *	配列を指定してください。<BR>
 *	引数CriMvWaveInfo構造体には、取得したオーディオデータについての情報が格納されます。<BR>
 *	入力データ不足などで要求されたサンプル数のデコードができない場合もあります。<BR>
 *	(未実装機能) 本関数の処理が重くなってでも、なるべく要求された多くのオーディオデータを取得するモード。
 */
/*
 *  \ingroup MODULE_AUDIO
 *  \brief		Get 16bit wave audio data
 *  \param		mvply A currently playing CriMvPly handle
 *  \param		nch The number of audio channels to get in this call
 *  \param		waveptr An array of audio data buffers to copy audio data into
 *  \param		wavesmpl The number of requested wave data samples (must be less than buffer size)
 *  \param		waveinfo A structure filled by this function with info about this wave
 *	\return		The number of wave data samples actually copied into the buffer
 *	
 *  This function copies currently decoding audio data into your output buffer
 * for you to send to the audio output.  The output format is a sixteen-bit PCM 
 * format.  The data provided is "current", e.g. you should try to minimize latency.
 * while delivering the audio data to the output device.
 * 
 * This function will return an empty audio buffer if the input to the CriMvPly
 * handle is starving for data, or if criMvPly_Execute() or criMvPly_ExecuteAll()
 * has not been called recently.
 *
 * Note that this function actually copies data.  However, the expected bandwidth
 * for moving audio data in memory is minimal -- about 176400 bytes per second for
 * a stereo stream, which is typically a fraction of 1% of the bandwidth available 
 * on modern game systems.
 *
 * For debugging tips on stuttering, see \ref crim_section_stuttering .
 */
CriUint32 CRIAPI criMvPly_GetWave16(CriMvPly mvply, CriUint32 nch, CriSint16 *waveptr[],
								  CriUint32 wavesmpl, CriMvWaveInfo *waveinfo);
CriUint32 CRIAPI criMvPly_GetWave32(CriMvPly mvply, CriUint32 nch, CriFloat32 *waveptr[],
								  CriUint32 wavesmpl, CriMvWaveInfo *waveinfo);


#if defined(XPT_TGT_EE)
/* スクラッチパッドRAM使用設定 */
/* スクラッチパッドRAMの使用設定(ハンドル作成前に呼び出すこと) */
/*
 *  \brief		スクラッチパッドRAM使用設定
 *  \param		sw : 
 *	\return		なし
 *	\par 説明:
 *	スクラッチパッドRAMの使用設定を行います。<BR>
 *  デフォルトはOFFです。
 */
/*
 * \if ps2
 *  \ingroup MODULE_INIT
 *  \brief		Enable or disable PS2 scratch pad (SPRAM) usage
 *  \param		sw : ON to enable scratch pad use, OFF to disable
 *	
 *  This function determines whether the video decode step uses the PS2
 * SPRAM memory area for its work.  The performance of this library is 
 * increased by around 30% when using SPRAM; however, this use might
 * conflict with graphics engines that depend on exclusive access to SPRAM.
 * However, on the PS2, the video decode step occurs synchronously to the 
 * criMvPly_GetFrameRGBA32_PS2() function, so you can take appropriate external
 * locking measures to intelligently synchronize SPRAM utilization.
 * The value set by criMvPly_SetUseScratchPadRAM_PS2() is internally checked
 * exactly once, during the criMvPly_Create() step; calling this function
 * after criMvPly_Create() has no effect.  The default setting for this
 * function is OFF. 
 * \endif
 */
void CRIAPI criMvPly_SetUseScratchPadRAM_PS2(CriBool sw);
#endif


/* メモリからの再生開始 */
/*
 *  \brief		メモリからの再生開始
 *  \param		mvply : CRI Movie ハンドル
 *  \param		memptr : メモリ上のムービデータの先頭アドレス
 *  \param		memsize : メモリ上のムービデータのサイズ
 *	\return		なし
 *	\par 説明:
 *	メモリからのムービ再生を開始します。<BR>
 *  本関数の呼び出し前に、あらかじめムービデータの全てをメモリ上に読み込んでおいてください。
 */
/*
 *  \ingroup MODULE_STATE
 *  \brief		Start playback from movie file on memory
 *  \param		mvply A currently playing CriMvPly handle
 *  \param		memptr A address of movie file
 *  \param		memsize The size of movie file
 *
 *  Start playback from memory.<BR>
 *  Please read movie file to memory before playback.
 */
void CRIAPI criMvPly_StartMemory(CriMvPly mvply, CriUint8* memptr, CriUint32 memsize);


/*
 * YUV個別バッファへのフレーム取得
 */
/*
 * \if xbox360
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame to Y,U,V independently texture buffers
 *  \param		mvply A valid CriMvPly handle
 *  \param		yuvbuffers Information about Y,U,V independently texture buffers
 *  \param		frameinfo Information about the decoded frame
 *	\return		returns true if a frame has been copied into the buffer, false otherwise
 *	
 *  This function finds and decodes the current frame, if any, to the Y,U,V texture buffers.
 * This function will only return a valid frame if the CriMvPly handle is in the
 * CRIMVPLY_STATUS_WAIT_PLAYING or the CRIMVPLY_STATUS_PLAYING state.  
 * \endif
 */
CriBool CRIAPI criMvPly_GetFrameYUVBuffers(CriMvPly mvply, CriMvYuvBuffers *yuvbuffers, CriMvFrameInfo *frameinfo);


/*
 * ビデオのデコード
 */
/*
 *  \ingroup MODULE_VIDEO
 *  \param		mvply The movie player handle
 *	\return		returns the number of decoded frames
 * 
 */
CriUint32 CRIAPI criMvPly_DecodeVideo(CriMvPly mvply);

/*
 * ヘッダのデコード
 */
/*
 *
 *  \ingroup MODULE_VIDEO
 *  \param		mvply The movie player handle
 *
 */
void CRIAPI criMvPly_DecodeHeader(CriMvPly mvply);

/*
 * デコードスキップ指示
 *	\par 説明:
 * この関数を実行した回数だけ、その後のデコード時に自動的に１枚Bピクチャをスキップする。<br>
 * スキップ指示を出した次のフレームからは、実際のスキップが実行されていなくても
 * 表示時刻はスキップしたものとして補正される。
 */
/*
 *  \ingroup MODULE_VIDEO
 *  \param		mvply The movie player handle
 *	\return		returns the number of decoded frames
 * 
 * After calling SkipFrame function, Decoding function skip  B-picture. 
 * To avoid that application judge continuous wrong skip, After calling
 * SkipFrame function, next frame time will be adjusted.
 */
void CRIAPI criMvPly_SkipFrame(CriMvPly mvply);


#if 0
/*
 * YUVA8フォーマットのフレーム取得
 */
/*
 * \if ps3
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame in YUVA8 format.
 *  \param		mvply A valid CriMvPly handle
 *  \param		imagebuf A pointer to the video buffer in memory to receive the frame
 *  \param      pitch Number of bytes in one row of the video buffer
 *  \param		bufsize Video buffer size in bytes
 *  \param		frameinfo Information about the decoded frame
 *	\return		returns true if a frame has been copied into the buffer, false otherwise
 *	
 *  This function finds and decodes the current frame, if any, to the imagebuf buffer.
 * This function will only return a valid frame if the CriMvPly handle is in the
 * CRIMVPLY_STATUS_WAIT_PLAYING or the CRIMVPLY_STATUS_PLAYING state.  
 * \endif
 */
CriBool CRIAPI criMvPly_GetFrameYUVA8_PS3(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo *frameinfo);
#endif

/*
 * ARGB8888フォーマットでフレーム取得
 */
/*
 * \if ps3
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame in ARGB8888 format.
 *  \param		mvply A valid CriMvPly handle
 *  \param		imagebuf A pointer to the video buffer in memory to receive the frame
 *  \param      pitch Number of bytes in one row of the video buffer
 *  \param		bufsize Video buffer size in bytes
 *  \param		frameinfo Information about the decoded frame
 *	\return		returns true if a frame has been copied into the buffer, false otherwise
 *	
 *  This function finds and decodes the current frame, if any, to the imagebuf buffer.
 * This function will only return a valid frame if the CriMvPly handle is in the
 * CRIMVPLY_STATUS_WAIT_PLAYING or the CRIMVPLY_STATUS_PLAYING state.  
 * \endif
 */
CriBool CRIAPI criMvPly_GetFrameARGB8888(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo *frameinfo);
void CRIAPI criMvPly_InitializeFrameARGB8888(void);


#if defined(XPT_TGT_IPHONE) || defined(XPT_TGT_WINMO) || defined(XPT_TGT_ANDROID) || defined(XPT_TGT_CQSH2A) || defined(XPT_TGT_ACRODEA) || defined(XPT_TGT_NACL) || defined(XPT_TGT_SH7269)  || defined(XPT_TGT_TRGP6K)
/*
 * RGB565フォーマットでフレーム取得
 */
/*
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a video frame in RGB565 format
 */
CriBool CRIAPI criMvPly_GetFrameRGB565(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo *frameinfo);
void CRIAPI criMvPly_InitializeFrameRGB565(void);
#endif
	
/*
 * 次のフレームの情報だけ取得する
 */
/*
 *  \ingroup MODULE_VIDEO
 *  \brief		Get a information of next video frame (without actual video frame).
 */
CriUint8* CRIAPI criMvPly_GetNextFrameInfo(CriMvPly mvply, CriMvFrameInfo *frameinfo);

/*
 * 次のフレームを捨てる
 */
CriBool CRIAPI criMvPly_DiscardNextFrame(CriMvPly mvply, CriMvFrameInfo *frameinfo);

/*
 * 再生準備完了状態(PREPからWAIT_PLAYING)になるまでに貯金するフレーム数の指定
 * この関数を呼び出さなければ、貯金フレーム数 = フレームプール数
 */
void CRIAPI criMvPly_SetNumberOfFramesForPrep(CriMvPly mvply, CriSint32 nframes);

/*
 *  For Debug use.
 */
void CRIAPI criMvPly_SetSeekPosition(CriMvPly mvply, CriSint32 seek_frame_id, CriSint32 video_gop_top_id);
void CRIAPI criMvPly_SetSeekAlphaPosition(CriMvPly mvply, CriSint32 alpha_gop_top_id);
void CRIAPI criMvPly_CalcSeekPosition(CriMvPly mvply, void *seektbl_ptr, CriUint32 seektbl_size, Sint32 frame_id, Uint64 *offset, Sint32 *gop_top_id);

/* for specific use */
/* ボディアドレスの設定 */
void CRIAPI criMvPly_SetBodyData(CriMvPly mvply, const CriUint64Adr body_ptr, CriSint64 body_size);

/* 入力SJおよびバッファサイズの取得（バッファサイズ、リロードサイズはNULL指定で省略） */
CriSj CRIAPI criMvPly_GetInputSj(CriMvPly mvply, CriUint32 *buffer_size, CriUint32 *reload_threshold);

/* 名前＆タイプ指定によるイベントポイント情報の取得 */
//Bool criMvPly_SearchEventPointByName(CriMvPly mvply, Char8 *cue_name, Sint32 type, CriMvEventPoint *eventinfo);
/* イベントポイント情報からフレームIDへの変換 */
//Sint32 criMvPly_CalcFrameIdFromCuePoint(CriMvPly mvply, CriMvEventPoint *eventinfo);

CriBool CRIAPI criMvPly_AttachSubAudio(CriMvPly mvply, CriHeap heap, CriUint32 track);
CriUint32 CRIAPI criMvPly_GetSubAudioWave16(CriMvPly mvply, CriUint32 nch, CriSint16 *waveptr[], CriUint32 wavesmpl, CriMvWaveInfo *waveinfo);
CriUint32 CRIAPI criMvPly_GetSubAudioWave32(CriMvPly mvply, CriUint32 nch, CriFloat32 *waveptr[], CriUint32 wavesmpl, CriMvWaveInfo *waveinfo);
void CRIAPI criMvPly_DetachSubAudio(CriMvPly mvply);

void CRIAPI criMvPly_GetSubtitle(CriMvPly mvply, CriUint8 *bufptr, CriUint32 bufsize, CriMvSubtitleInfo *info);
void CRIAPI criMvPly_GetNextSubtitleInfo(CriMvPly mvply, CriMvSubtitleInfo *info);

/* 入力バッファのデータ量を見る[byte] */
CriUint32 CRIAPI criMvPly_PeekInputBufferData(CriMvPly mvply);
/* メモリ上のムービを入力SJに追加する */
void CRIAPI criMvPly_AddInputMemory(CriMvPly mvply, CriUint8* memptr, CriUint32 memsize);


/* 取得できるオーディオデータのサンプル数を調べる */
CriUint32 CRIAPI criMvPly_GetDataSizeMainAudio(CriMvPly mvply, CriUint32 nch);
/* 取得できるオーディオデータのサンプル数を調べる */
CriUint32 CRIAPI criMvPly_GetDataSizeSubAudio(CriMvPly mvply, CriUint32 nch);

/* メインのオーディオの再生が終了しているかどうかを調べる */
CriBool CRIAPI criMvPly_IsEndMainAudioPlayback(CriMvPly mvply);
/* サブのオーディオの再生が終了しているかどうかを調べる */
CriBool CRIAPI criMvPly_IsEndSubAudioPlayback(CriMvPly mvply);

/* メインオーディオが活動中かどうかを調べる (デコード中かつ出力バッファがある状態) */
CriBool CRIAPI criMvPly_IsActiveMainAudioPlayback(CriMvPly mvply);

/* ワーク確保前に設定変更すること */
void CRIAPI criMvPly_SetPcmFormat(CriMvPly mvply, CriMvPcmFormat pcmfmt);

#if defined(XPT_TGT_PC)
/* [PC] マルチプロセッサの指定 */
//void CRIAPI criMvPly_SetProcessorParameters_PC(CriMvPly mvply, Sint32 thread_num, Uint32 *affinity_masks, Sint32 *priorities);
#endif

#if defined(XPT_TGT_XBOX360)
/* [Xbox360] マルチプロセッサの指定 */
void CRIAPI criMvPly_SetProcessorParameters_XBOX360(CriMvPly mvply, Sint32 thread_num, CriUint32 processor_mask, CriSint32 *priorities);
#endif

#if defined(XPT_TGT_PS3PPU)
/* [PS3] SPURSの指定 */
void CRIAPI criMvPly_SetupSpursParameters_PS3(const CriMvProcessorParameters_PS3 *processor_param);
	/* [PS3] SPUスレッドによるマルチプロセッサの指定 */
void CRIAPI criMvPly_SetupSpuThreadParameters_PS3(const CriMvSpuThreadParameters_PS3 *spu_thread_param);
void CRIAPI criMvPly_SetGraphicEnv(CriMvGraphicEnv env);
CriMvGraphicEnv CRIAPI criMvPly_GetGraphicEnv(void);
#endif

/* フレームプール情報の取得 */
void CRIAPI criMvPly_GetFramePoolInfo(CriMvPly mvply, CriSint32 *num_input, CriUint32* num_data, CriUint32* num_ref, CriUint32* num_hold, CriUint32* num_free);

/* 再生中でも字幕チャネルを切り替える */
void CRIAPI criMvPly_SetSubtitleChannel(CriMvPly mvply, CriSint32 chno);

/* ポインタだけ取得してフレームプール内のバッファをロックする */
CriBool CRIAPI criMvPly_LockFrameBuffer(CriMvPly mvply, CriMvYuvBuffers *yuvbuffers, CriMvFrameInfo *frameinfo);
/* ロックしていたフレームプールを解放する */
CriBool CRIAPI criMvPly_UnlockFrameBuffer(CriMvPly mvply, CriMvFrameInfo *frameinfo);

/*
 * For Sofdec2
 */
CriMvPly CRIAPI criMvPly_CreateWithWork(void *work, CriSint32 size, CriMvHandleConfig *config);
CriSint32 CRIAPI criMvPly_CalcHandleWorkSize(CriMvHandleConfig *config);
CriSint32 CRIAPI criMvPly_CalcPlaybackWorkSize(CriMvPly mvply, CriMvStreamingParameters *stmprm);
CriBool CRIAPI criMvPly_AllocateWorkBufferWithWork(CriMvPly mvply, void *work ,Sint32 work_size, CriMvStreamingParameters *stmprm);
void CRIAPI criMvPly_SetMetaDataWorkAllocator(CriMvPly mvply, CriMvMetaDataWorkMallocFunc allocfunc, CriMvMetaDataWorkFreeFunc freefunc, void *usrobj, CriMvMetaFlag meta_flag);
CriSint32 criMvPly_CalcSubAudioWorkSize(CriMvPly mvply, const CriMvAudioParameters *aprm);
CriBool criMvPly_CopyFrameYUVBuffers(CriMvPly mvply, CriMvYuvBuffers *yuvbuffers, 
								  const CriMvFrameInfo *frameinfo,const CriMvAlphaFrameInfo *alpha_frameinfo);
CriBool criMvPly_CopyFrameARGB8888Buffer(CriMvPly mvply,  CriUint8 *dst_buf, CriUint32 dst_pitch, CriUint32 dst_bufsize, 
									  const CriMvYuvBuffers *src_bufs, const CriMvFrameInfo *src_vinf, const CriMvAlphaFrameInfo *src_ainf);
#if defined(XPT_TGT_IPHONE) || defined(XPT_TGT_WINMO) || defined(XPT_TGT_ANDROID) || defined(XPT_TGT_CQSH2A) || defined(XPT_TGT_ACRODEA) || defined(XPT_TGT_NACL) || defined(XPT_TGT_SH7269)  || defined(XPT_TGT_TRGP6K) || defined(XPT_TGT_TRGP6K)
CriBool criMvPly_CopyFrameRGB565Buffer(CriMvPly mvply,  CriUint8 *dst_buf, CriUint32 dst_pitch, CriUint32 dst_bufsize, 
									  const CriMvYuvBuffers *src_bufs, const CriMvFrameInfo *src_vinf, const CriMvAlphaFrameInfo *src_ainf);
#endif
CriBool criMvPly_LockAlphaFrameBuffer(CriMvPly mvply, CriMvYuvBuffers *yuvbuffers, CriMvAlphaFrameInfo *alpha_frameinfo);
CriBool criMvPly_UnlockAlphaFrameBuffer(CriMvPly mvply, CriMvAlphaFrameInfo *alpha_frameinfo);
CriSint32 criMvPly_GetNumPictureData(CriMvPly mvply);
const CriMvPlyHeaderInfo* criMvPly_GetCurrentStreamInfo(CriMvPly mvply);
/* for debug */
CriBool CRIAPI criMvPly_GetAlphaFrame(CriMvPly mvply, CriUint8 *imagebuf, CriUint32 pitch, CriMvAlphaFrameInfo *alpha_frameinfo);

/* OUTER_FRAMEPOOL_WORK */
/* フレームプール用ワーク計算。ハンドルはNULL指定OK。 */
CriSint32 criMvPly_CalcFramepoolWorkSize(CriMvPly mvply, const CriMvStreamingParameters *stmprm);
/* フレームプール用ワーク設定 */
void criMvPly_SetFramepoolWork(CriMvPly mvply, void *work, CriSint32 work_size);
/* フレームプール用ワークアロケータ設定 */
void criMvPly_SetFramepoolWorkAllocator(CriMvPly mvply, CriMvFramepoolWorkMallocFunc allocfunc, CriMvFramepoolWorkFreeFunc freefunc, void *usrobj);

/* シークブロック情報の取得 */
/* <入力>
 * - seektbl_ptr : UTFアドレス
 * - seektbl_size : UTFサイズ
 * - num_seekblock : 出力配列の要素数
 * <出力>
 * - blockinfo : シークブロック情報配列へのポインタ（num_seekblock分の領域を確保して渡すこと）
 */
void criMvPly_GetSeekBlockInfo(CriMvPly mvply, void *seektbl_ptr, CriUint32 seektbl_size, CriSint32 num_seekblock, CriMvSeekBlockInfo *blockinfo);

/* フレームレートの強制指定 */
void criMvPly_SetVideoFramerate(CriMvPly mvply, CriUint32 framerate_n, CriUint32 framerate_d);

/* 同期有無の設定 */
void criMvPly_SetSyncFlag(CriMvPly mvply, CriBool sync_flag);

/* 再生可能かの問い合わせ */
CriBool criMvPly_IsPlayable(CriMvPly mvply, const CriMvStreamingParameters *stmprm);



#ifdef __cplusplus
}
#endif

#endif	/* CRI_MOVIE_CORE_H_INCLUDED */
