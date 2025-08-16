/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2006-2013 CRI Middleware Co., Ltd.
 *
 * Library  : CRI Movie
 * Module   : Library User's Header
 * File     : cri_movie.h
 * Date     : 2013-11-27
 * Version  : (see CRIMOVIE_VER)
 *
 ****************************************************************************/
/*!
 *	\file		cri_movie.h
 */
#ifndef	CRI_MOVIE_H_INCLUDED		/* Re-definition prevention */
#define	CRI_MOVIE_H_INCLUDED

/*	Version No.	*/
#define	CRIMOVIE_VER		"3.50"
#define	CRIMOVIE_NAME		"CRI Movie"

/***************************************************************************
 *      Include file
 ***************************************************************************/
#include <cri_xpt.h>
#include <cri_heap.h>
#include <cri_allocator.h>
#include <cri_sj.h>
#include <cri_error.h>
#include <cri_movie_core.h>

/***************************************************************************
 *      MACRO CONSTANT
 ***************************************************************************/
/*EN
 * \brief	Maximum length of a filename that can be opened by EasyPlayer
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetFile()
 */
/*JP
 * \brief	EasyPlayerに指定可能なファイル名の最大長さ
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::SetFile()
 */
#define CRIMV_MAX_FILE_NAME			(256)

/*EN
 * \brief	Default audio track setting used by AttachSubAudioInterface(), ReplaceCenterVoice()
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::ReplaceCenterVoice(), CriMvEasyPlayer::AttachSubAudioInterface()
 */
/*JP
 * \brief	サブオーディオ（またはセンターボイス）のデフォルト値
 * \ingroup MDL_MV_OPTION
 * \sa CriMvEasyPlayer::ReplaceCenterVoice(), CriMvEasyPlayer::AttachSubAudioInterface()
 */
#define CRIMV_CENTER_VOICE_OFF		(-1)


/***************************************************************************
 *      Variable Declaration
 ***************************************************************************/
namespace CriMv {
	extern CriError ErrorContainer;
}

/***************************************************************************
 *      Prototype Functions
 ***************************************************************************/
/*EN
 * \brief CRI Movie Namespace
 * \ingroup MDL_MV_BASIC
 * \par
 * Namespace for all CRI Movie methods, constants, and declarations
 */
/*JP
 * \brief CRI Movie Namespace
 * \ingroup MDL_MV_BASIC
 * \par
 * Namespace for all CRI Movie methods, constants, and declarations
 */
namespace CriMv {
	
	 /*EN
     * \brief   Returns the version number and build information of CRI Movie
     * \param   none
     * \return  A string constant
     *
     * Returns the version number and build information of CRI Movie as a constant
     * string, in the form 
     *
     *  "CRI Movie/{PLATFORM} {VERSION} Build:{BUILD DATE}"
     */
    /*JP
     * \brief	CRI Movieのバージョン番号やビルド情報を返します。
	 * \return	ライブラリ情報文字列
	 *
     */
	const CriChar8* CRIAPI GetLibraryVersionString(void);

	/*EN
	 * \brief	Initialize the CRI Movie library
	 * \param	err				Optional error code
	 * 
	 * Does one-time initialization of the CRI Movie library.<BR>
	 * This function must be successfully called before calling CriMvEasyPlayer::Create().<BR>
	 * <BR>
	 * In general, all CRI Movie APIs are available after calling CriMv::Initialize(), 
	 * until CriMv::Finalize() is called.  However, there are some methods that set options
	 * for the library as a whole which must be called before CriMv::Initialize():<br>
	 * <BR>
	 * It is safe to call CriMv::Initialize() more than once, as long as your application calls
	 * CriMv::Finalize() once for each call to CriMv::Initialize().  Multiple calls will not
	 * have an effect on the library, other than to increment or decrement an internal counter.
	 * 
	 * \sa CriMv::Finalize()
	 */
	/*JP
	 * \brief	CRI Movieライブラリの初期化
	 *  \param	err				エラー情報（省略可）
	 * 
	 * CRI Movie ライブラリを初期化します。<br>
	 * CriMvEasyPlayer::Create 関数よりも先に呼び出してください。<br>
	 * <BR>
	 * 原則として全ての CRI Movie ライブラリ関数は初期化後、終了関数呼び出しまでの間にのみ使用します。<BR>
	 * ただし、いくつかの設定関数は初期化関数よりも先に呼び出す必要があるものがあります。
	 * 詳細は各設定関数の説明を参照してください。
	 * <BR>
	 * 初期化関数を複数回呼び出した場合、２回目以降の呼び出しでは呼び出し回数を記録するだけで再初期化は行いません。<BR>
	 * この場合、正しく終了処理を行うには同じ回数だけ終了関数を呼び出す必要があります。
	 * 初期化関数と終了関数は必ず対で呼び出すように実装してください。<BR>
	 * 
	 * \sa CriMv::Finalize()
	 */
	void CRIAPI Initialize(CriError &err = CriMv::ErrorContainer);

	/* MEMO: 
	 * If an application calls this function instead of CriMv::Initialize(), 
	 * an application need to call CriMv::SetupMovieHandleWork() before CriMv::InitializeMana().
	 */
	void CRIAPI InitializeMana(CriError &err = CriMv::ErrorContainer);

	/*EN
	 * \brief	Initialize 32bit ARGB frame conversion
	 * 
	 * This function initializes 32bit ARGB frame conversion.<BR>
	 * When an application uses CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB(), 
	 * please call this function after CriMv::Initialize().<BR>
	 * 
	 * \sa CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB()
	 */
	/*JP
	 * \brief	32bitARGB用フレーム変換の初期化
	 * 
	 * 32bitARGB用フレーム変換処理を初期化します。<br>
	 * CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB() を使用する場合は CRI Movie ライブラリの
	 * 初期化後に必ず呼び出してください。<br>
	 * 
	 * \sa CriMvEasyPlayer::GetFrameOnTimeAs32bitARGB()
	 */
	void CRIAPI InitializeFrame32bitARGB(void);

#if defined(XPT_TGT_IPHONE) || defined(XPT_TGT_WINMO) || defined(XPT_TGT_ANDROID) || defined(XPT_TGT_CQSH2A) || defined(XPT_TGT_ACRODEA) || defined(XPT_TGT_NACL) || defined(XPT_TGT_SH7269) || defined(XPT_TGT_PC) || defined(XPT_TGT_TRGP6K)
	void CRIAPI InitializeFrameRGB565(void);
#endif

	/*EN
	 * \brief	De-initializes the CRI Movie library
	 *  \param	err				Optional error code 
	 * 
	 * This function finalize whole CRI Movie library.<BR>
	 * An application needs to destroy all CriMvEasyPlayer handles and decoding threads before callign CriMv::Finalize().<BR>
	 * <BR>
	 * In principle, all CRI Movie library APIs are enabled after CriMv::Initialize() until CriMv::Finalize().<BR>
	 * But there are some APIs for parameter setting, which need to be called after CriMv::Finalize().
	 * For details, refer to each explanations of setting APIs.
	 * <BR>
	 * \remarks
	 * CriMv::Finalize() must be called once for each call to CriMv::Initialize().  When the
	 * internal initialization count reaches 0, the library will be finalized.
	 * 
	 * \sa CriMv::Initialize()
	 */
	/*JP
	 * \brief	CRI Movieライブラリの終了
	 *  \param	err				エラー情報（省略可）
	 * 
	 * CRI Movie ライブラリを終了します。<br>
	 * この関数を呼び出す前に、全ての CriMvEasyPlayer ハンドルおよびデコードスレッドを破棄してください。<br>
	 * <BR>
	 * 原則として全ての CRI Movie ライブラリ関数は初期化後、終了関数呼び出しまでの間にのみ使用します。<BR>
	 * ただし、いくつかの設定関数は終了関数よりも後に呼び出す必要があるものがあります。
	 * 詳細は各設定関数の説明を参照してください。
	 * <BR>
	 * 初期化関数を複数回呼び出した場合、正しく終了処理を行うには同じ回数だけ終了関数を呼び出す必要があります。
	 * 初期化関数と終了関数は必ず対で呼び出すように実装してください。<BR>
	 * 
	 * \sa CriMv::Initialize()
	 */
	void CRIAPI Finalize(CriError &err = CriMv::ErrorContainer);

#if !defined(XPT_TGT_EE)
	/*EN
	 * \brief	Calculates the work buffer size needed for the given number of movie handles
	 *  \param	max_num			Maximum number of movie handles desired
	 *  \param	err				Optional error code 
	 *  \return					The necessary work buffer size, in bytes
	 * 
	 * Each active movie handle requires an internal work buffer.  If your application
	 * needs to have multiple movies open at one time, it should determine how many handles 
	 * it needs, allocate a buffer of the size returned by this function, and pass it to
	 * CriMv::SetupMovieHandleWork().<br>
	 *
	 * Each CriMvEasy object uses a movie handle.  Alpha movie playback uses two handles.
	 * If you need to play aplha movies, be sure to double the number of handles requested.
	 * 
	 * \remarks
	 * The number of movie handles must be set <b>before</b> calling CriMv::Initialize().
	 * 
	 * \sa CriMv::SetupMovieHandleWork()
	 */
	/*JP
	 * \brief	マルチハンドル用ワークバッファサイズの計算
	 *  \param	max_num			最大ハンドル数（不透明ムービの再生時）
	 *  \param	err				エラー情報（省略可）
	 *  \return					ワークサイズ
	 * 
	 * 同時に使用する CriMvEasyPlayer ハンドルの最大数を増加させる場合に必要なワークバッファ
	 * サイズを計算します。
	 * 
	 * アルファムービを再生するとハンドル資源を２つ消費します。
	 * もし複数のアルファムービ再生を行いたい場合は、最大ハンドル数は倍にして指定してください。
	 * 
	 * \sa CriMv::SetupMovieHandleWork()
	 */
	CriUint32 CRIAPI CalcMovieHandleWork(CriUint32 max_num, CriError &err = CriMv::ErrorContainer);

	/*EN
	 * \brief	Increases the default number of available movie handles
	 *  \param	max_num			Maximium number of movie handles
	 *  \param	workbuf			Work buffer
	 *  \param	worksize		Size of work buffer
	 *  \param	err				Optional error code 
	 *
	 * Call this function if your application needs to increase the maximum number of open movies
	 * beyond the default.  Note that playing an alpha movie requires a second handle.<br>
	 *
	 * The default number of movie handles depends on the platform.<br>
	 *
	 * The work buffer must be allocated based on the size returned by CriMv::CalcMovieHandleWork().<br>
	 * 
	 * \remarks
	 * This function must be called <b>before</b> calling CriMv::Initialize().
	 * 
	 * \sa CriMv::CalcMovieHandleWork()
	 */
	/*JP
	 * \brief	マルチハンドル用ワークバッファの設定
	 *  \param	max_num			最大ハンドル数（不透明ムービの再生時）
	 *  \param	workbuf			ワークバッファアドレス
	 *  \param	worksize		ワークバッファサイズ
	 *  \param	err				エラー情報（省略可）
	 * 
	 * 同時に使用する CriMvEasyPlayer ハンドルの最大数を増加させるためのワークバッファを設定します。
	 * なお、ワークバッファを指定しない場合のハンドル数上限は機種によって異なります。
	 * 
	 * ワークバッファの設定は、 CriMv::Initialize() の呼び出し<B>先</B>に実行してください。
	 * 
	 * \sa CriMv::CalcMovieHandleWork()
	 */
	void CRIAPI SetupMovieHandleWork(CriUint32 max_num, void *workbuf, CriUint32 worksize, CriError &err = CriMv::ErrorContainer);

	/*EN
	 * \brief	Get max number of movie handles
	 *  \param	err				Optional error code 
	 *  \return					Max number of movie handles you set by CriMv::SetupMovieHandleWork().
	 *
	 * Returns the maximum number of movie handles that are available to CRI Movie.<br>
	 *
	 * Note that this is not necessarily equal to the number of movies that can be
	 * opened at a time.  In general, each movie will use one handle; however,
	 * alpha channel movies uses two handles.
	 * 
	 * \sa CriMv::SetupMovieHandleWork(), CriMv::CalcMovieHandleWork()
	 */
	/*JP
	 * \brief	最大ハンドル数の取得
	 *  \param	err				エラー情報（省略可）
	 *  \return					CriMv::SetupMovieHandleWork() で設定した最大ハンドル数
	 * 
	 * CriMv::CalcMovieHandleWork() で最大ハンドル数を増加させた場合に、
	 * 設定した最大ハンドル数を取得します。
	 * 
	 * \sa CriMv::SetupMovieHandleWork(), CriMv::CalcMovieHandleWork()
	 */
	CriUint32 CRIAPI GetMaxNumberOfHandles(CriError &err = CriMv::ErrorContainer);
#endif

	/* For Sofdec2 */
	CriSint32 CRIAPI CalcHandleWorkSize(CriMvHandleConfig *config, CriError &err = CriMv::ErrorContainer);
	void CRIAPI SetDelayDestroySubmodules(CriBool sw);
}

/***************************************************************************
 *      CLASS
 ***************************************************************************/
/*EN
 * \brief	File Reading Interface Class for EasyPlayer
 * \ingroup MDL_IF_READER
 * \par
 * You can implement your own streaming filesystem for movie playback by deriving
 * from this class.<br>
 *
 * Pass an instance of this subclass to CriMvEasyPlayer::Create().<br>
 *
 * This class is designed for asynchronous operation. All functions are pure virtual.
 *
 * \sa CriMvEasyPlayer::Create()
 */
/*JP
 * \brief	ファイル読み込みインタフェース for EasyPlayer
 * \ingroup MDL_IF_READER
 * 
 * このクラスを定義することで、自前のファイルシステムを使ってEasyPlayerの
 * ストリーミング再生が可能になります。<br>
 * 全ての関数は純粋仮想関数として定義されているので、全ての関数を必ず実装してください。
 *
 * \sa CriMvEasyPlayer::Create()
 */
class CriMvFileReaderInterface
{
public:
	/*EN Status of an asynchronous operation */
	/*JP 非同期処理ステータス */
	enum AsyncStatus {
		ASYNC_STATUS_STOP,			/*EN< No action */
									/*JP< 何もしていない状態。*/
		ASYNC_STATUS_BUSY,			/*EN< Currently processing */
									/*JP< 処理中 */
		ASYNC_STATUS_COMPLETE,		/*EN< Processing completed */
									/*JP< 処理終了 */
		ASYNC_STATUS_ERROR,			/*EN< An error occured */
									/*JP< エラー */

		/* Keep enum 4bytes */
		ASYNC_STATUS_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	/*EN Offset values for Seek() */
	/*JP シーク開始位置 */
	enum SeekOrigin {
		SEEK_FROM_BEGIN,			/*EN< Start of file */
									/*JP< ファイル先頭 */
		SEEK_FROM_CURRENT,			/*EN< Current position in file */
									/*JP< ファイルの現在位置 */
		SEEK_FROM_END,				/*EN< End of file */
									/*JP< ファイル終端 */

		/* Keep enum 4bytes */
		SEEK_FROM_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	/*EN
	 * \brief	Opens a file asynchronously
	 * \param	fname		Name of file to open
	 *
	 * Initiates a file open request, and returns immediately.
	 *
	 * To determine success or failure, call CriMvFileReaderInterface::GetOpenStatus().<br>
	 *
	 * \remarks
	 * CRI Movie may call CriMvFileReaderInterface::Read() and CriMvFileReaderInterface::Close()
	 * before CriMvFileReaderInterface::GetOpenStatus() returns ASYNC_STATUS_COMPLETE.
	 * 
	 * \sa CriMvFileReaderInterface::GetOpenStatus()
	 */
	/*JP
	 * \brief	ファイル名によるファイルオープン(即時復帰)
	 * \param	fname ファイル名の文字列
	 * 
	 * ファイル名指定でファイルのオープン要求を出します。<br>
	 * この関数は即時復帰の関数として呼び出されます。<br>
	 * オープン処理が終わったかどうかは CriMvFileReaderInterface::GetOpenStatus関数 
	 * でチェックできるようにしてください。<br>
	 * CRI Movie ライブラリはオープン処理が終わる（＝ CriMvFileReaderInterface::GetOpenStatus関数が
	 * ASYNC_STATUS_COMPLETEを返す）前に、リード、クローズの要求を呼び出す可能性があります。
	 * 
	 * \sa CriMvFileReaderInterface::GetOpenStatus()
	 */
	/* pure */ virtual void Open(CriChar8 *fname)=0;

	/*EN
	 * \brief	Closes a file asynchronously
	 *
	 * Initiates a file close request, and returns immediately.<br>
	 *
	 * To determine success or failure, call CriMvFileReaderInterface::GetCloseStatus().<br>
	 * 
	 * \sa CriMvFileReaderInterface::GetCloseStatus()
	 */
	/*JP
	 * \brief	ファイルのクローズ(即時復帰)
	 * 
	 * オープン済みのファイルのクローズ要求を出します。<br>
	 * この関数は即時復帰の関数として呼び出されます。<br>
	 * クローズ処理が終わったかどうかは CriMvFileReaderInterface::GetCloseStatus() 
	 * でチェックできるようにしてください。
	 * 
	 * \sa CriMvFileReaderInterface::GetCloseStatus()
	 */
	/* pure */ virtual void Close(void)=0;

	/*EN
	 * \brief	Reads from a file
	 * \param	buffer		Buffer to read into
	 * \param	req_size	Size of the buffer
	 * 
	 * Initiates a file read request, and returns immediately.<br>
	 *
	 * The buffer must be available and writable until the read request completes.<br>
	 *
	 * To determine success or failure, call CriMvFileReaderInterface::GetReadStatus().<br>
	 *
	 * To determine the number of bytes actually read, call CriMvFileReaderInterface::GetReadSize()
	 * after CriMvFileReaderInterface::GetReadStatus() has returned ASYNC_STATUS_COMPLETE.
	 * 
	 * \remarks
	 * CRI Movie may call CriMvFileReaderInterface::Close() before 
	 * CriMvFileReaderInterface::GetReadStatus() returns ASYNC_STATUS_COMPLETE.
	 * 
	 * \sa CriMvFileReaderInterface::GetReadStatus(), CriMvFileReaderInterface::GetReadSize()
	 */
	/*JP
	 * \brief	読み込み要求（即時復帰）
	 * \param	buffer		書き出しバッファのポインタ。読み込み要求サイズを満たすだけのバッファを確保しておく必要があります。
	 * \param	req_size	読み込み要求サイズ。単位はバイト単位です。
	 * 
	 * ファイルの読み込み要求を出します。<br>
	 * この関数は即時復帰の関数として呼び出されます。<br>
	 * リード処理が終わったかどうかは CriMvFileReaderInterface::GetReadStatus() 
	 * でチェックできるようにしてください。<br>
	 * CRI Movie ライブラリはリード処理が終わる（＝ CriMvFileReaderInterface::GetReadStatus()が
	 * ASYNC_STATUS_COMPLETEを返す）前に、クローズ要求を呼び出す可能性があります。<br>
	 * この関数は読み込んだサイズを返しません。<br>
	 * 読み込み済みサイズは、 CriMvFileReaderInterface::GetReadStatus()が ASYNC_STATUS_COMPLETEを
	 * 返したあとに CriMvFileReaderInterface::GetReadSize()で返すように実装してください。
	 * 
	 * \sa CriMvFileReaderInterface::GetReadStatus(), CriMvFileReaderInterface::GetReadSize()
	 */
	/* pure */ virtual void Read(CriUint8 *buffer, CriSint64 req_size)=0;

	/*EN
	 * \brief   Gets asynchronous status of a call to CriMvFileReaderInterface::Open()
	 *
	 * \return  Status of the call
	 *
	 * While the Open() call is in process, this will return ASYNC_STATUS_BUSY.<br>
	 * If the call fails, this will return ASYNC_STATUS_ERROR.<br>
	 * On success, this will return ASYNC_STATUS_COMPLETE.<br>
	 * 
	 * \sa CriMvFileReaderInterface::Open()
	 */
	/*JP
	 * \brief	オープンコマンドの状態取得
	 *  \return		オープンコマンドの処理状態。
	 * 
	 * CriMvFileReaderInterface::Open関数の処理状態を取得します。
	 * 
	 * \sa CriMvFileReaderInterface::Open()
	 */
	/* pure */ virtual CriMvFileReaderInterface::AsyncStatus GetOpenStatus(void)=0;

	/*EN
	 * \brief   Gets asynchronous status of a call to CriMvFileReaderInterface::Close()
	 *
	 * \return  Status of the call
	 *
	 * While the Close() call is in process, this will return ASYNC_STATUS_BUSY.<br>
	 * If the call fails, this will return ASYNC_STATUS_ERROR.<br>
	 * On success, this will return ASYNC_STATUS_COMPLETE.<br>
	 * 
	 * \sa CriMvFileReaderInterface::Close()
	 */
	/*JP
	 * \brief	クローズコマンドの状態取得
	 *  \return		クローズコマンドの処理状態。
	 * 
	 * CriMvFileReaderInterface::Close関数の処理状態を取得します。
	 * 
	 * \sa CriMvFileReaderInterface::Close()
	 */
	/* pure */ virtual CriMvFileReaderInterface::AsyncStatus GetCloseStatus(void)=0;

	/*EN
	 * \brief   Gets asynchronous status of a call to CriMvFileReaderInterface::Read()
	 *
	 * \return  Status of the call
	 *
	 * While the Read() call is in process, this will return ASYNC_STATUS_BUSY.<br>
	 * If the call fails, this will return ASYNC_STATUS_ERROR.<br>
	 * On success, this will return ASYNC_STATUS_COMPLETE.<br>
	 * 
	 * \sa CriMvFileReaderInterface::Read()
	 */
	/*JP
	 * \brief	リードコマンドの状態取得
	 *  \return		リードコマンドの処理状態。
	 * 
	 * CriMvFileReaderInterface::Read関数の処理状態を取得します。
	 * 
	 * \sa CriMvFileReaderInterface::Read()
	 */
	/* pure */ virtual CriMvFileReaderInterface::AsyncStatus GetReadStatus(void)=0;

	/*EN
	 * \brief   Gets the number of bytes read by a successful call to CriMvFileReaderInterface::Read()
	 *
	 * \return  Number of bytes read
	 *
	 * Once CriMvFileReaderInterface::GetReadStatus() returns ASYNC_STATUS_COMPLETE, this 
	 * function can be called to determine the number of bytes read by 
	 * CriMvFileReaderInterface::Read().<br>
	 *
	 * If no call to Read() has been made, this function will return 0.<br>
	 *
	 * If called multiple times after completing the read, the same value will be returned each time.
	 * 
	 * \sa CriMvFileReaderInterface::Read(), CriMvFileReaderInterface::GetReadStatus()
	 */
	/*JP
	 * \brief		前回読み込み要求に対する読み込み完了サイズ
	 *  \return		読み込み完了サイズ。単位はByte。
	 * 
	 * 前回の読み込み要求に対して読み込み完了したサイズを返します。
	 * まだ読み込みが１度も要求されていない場合は０を返します。
	 * 読み込み完了後に繰り返しこの関数が呼び出された場合は、すべて同じ値を返します。
	 * 
	 * \sa CriMvFileReaderInterface::Read(), CriMvFileReaderInterface::GetReadStatus()
	 */
	/* pure */ virtual CriSint64 GetReadSize(void)=0;

	/*EN
	 * \brief   Seeks to a new position in the file
	 *
	 * \param   size        Number of bytes to seek relative to \a offset
	 * \param   offset      Starting position of seek
	 * \return  The offset, in bytes, from the previous file position.
	 *
	 * If \a offset is <b>SEEK_FROM_BEGIN</b>, seeking will start from the beginning of the file.<br>
	 * If \a offset is <b>SEEK_FROM_CURRENT</b>, seeking will start from the current file position.<br>
	 * If \a offset is <b>SEEK_FROM_END</b>, seeking will start from the end of the file.<br>
	 *
	 * \sa CriMvFileReaderInterface::SeekOrigin
	 */
	/*JP
	 * \brief	シーク
	 *  \param	size		シークサイズ
	 *  \param	offset		シークの開始位置
	 *  \return		実際にシークした距離。Byte単位。
	 *
	 * \sa CriMvFileReaderInterface::SeekOrigin
	 *
	 */
	/* pure */ virtual CriSint64 Seek(CriSint64 size, CriMvFileReaderInterface::SeekOrigin offset)=0;

	/*EN
	 * \brief   Gets the file size
	 *
	 * \return  File size, in bytes
	 *
	 * This function can safely be called once CriMvFileReaderInterface::Open() has completed
	 * successfully.
	 * 
	 * \sa CriMvFileReaderInterface::Open(), CriMvFileReaderInterface::GetOpenStatus()
	 */
	/*JP
	 * \brief		ファイルサイズの取得
	 *  \return		ファイルサイズ[byte].
	 * 
	 * この関数はファイルオープンの終了後に呼び出されます。
	 * 
	 * \sa CriMvFileReaderInterface::Open(), CriMvFileReaderInterface::GetOpenStatus()
	 */
	/* pure */ virtual CriSint64 GetFileSize(void)=0;

protected:
	virtual ~CriMvFileReaderInterface(void) {}
};

/*EN
 * \brief	Sound Interface Class for EasyPlayer
 * \ingroup MDL_IF_SOUND
 * 
 * \par
 * A class derived from CriMvSoundInterface is required in order to play sound in CRI Movie.
 * Pass an instance of this subclass to CriMvEasyPlayer::Create().<br>
 *
 * If you do not need audio output, you can pass NULL instead.  However, if you do,
 * you can not use a movie timer of type MVEASY_TIMER_AUDIO.  See CriMvEasyPlayer::SetMasterTimer()
 * for more details.<br>
 *
 * Sound data must be provided in either 32 or 16 bit PCM format.
 *
 * All functions are pure virtual.
 *
 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::SetMasterTimer(), CriMvEasyPlayer::TimerType
 */
/*JP
 * \brief	サウンド出力インタフェース
 * \ingroup MDL_IF_SOUND
 *
 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::SetMasterTimer(), CriMvEasyPlayer::TimerType
 */
class CriMvSoundInterface
{
public:
	/*EN
	 * \brief		The status of the Sound Module
	 * 
	 * This is the return value of CriMvSoundInterface::GetStatus().<br>
	 *
	 * After CriMvSoundInterface::Start() is called, the state transitions to MVEASY_SOUND_STATUS_EXEC.<br>
	 * While the state is MVEASY_SOUND_STATUS_EXEC, the sound module calls the callback function
	 * to retrieve sound data as needed.<br>
	 * When the EasyPlayer is stopped, or transits to MVEASY_STATUS_PLAYEND, 
	 * CRI Movie will call CriMvSoundInterface::Stop().  Then, CRI Movie waits for 
	 * CriMvSoundInterface::GetStatus() to return MVEASY_SOUND_STATUS_STOP, 
	 * and calls CriMvSoundInterface::DestroyOutput().
	 * 
	 * \sa CriMvSoundInterface::GetStatus(), CriMvSoundInterface::Start(), 
	 *     CriMvSoundInterface::Stop(), CriMvSoundInterface::DestroyOutput()
	 */
	/*JP
	 * \brief		サウンドモジュールの状態
	 * 
	 * サウンドモジュールの状態を表す列挙型です。<br>
	 * CriMvEasyPlayer::GetStatus() の関数値です。<br>
	 * CriMvSoundInterface::Start() が呼び出されるとMVEASY_SOUND_STATUS_EXEC状態になります。<br>
	 * MVEASY_SOUND_STATUS_EXEC状態の間は、サウンド出力モジュールはコールバック関数を呼び出します。<br>
	 * CRI Movie ライブラリは再生終了または再生停止指示を受けた場合、まず CriMvSoundInterface::Stop()を呼び出します。<br>
	 * その後、STOP状態になるのを待ってから CriMvSoundInterface::DestroyOutput()を呼び出します。
	 * 
	 * \sa CriMvSoundInterface::GetStatus(), CriMvSoundInterface::Start(), 
	 *     CriMvSoundInterface::Stop(), CriMvSoundInterface::DestroyOutput()
	 */
	enum Status {
		MVEASY_SOUND_STATUS_STOP,		/*EN< No sound processing is happening. */
										/*JP< CRI Movie のサウンド出力をしていない状態 */
		MVEASY_SOUND_STATUS_EXEC,		/*EN< Sound data is being retrieved and processed. */
										/*JP< CRI Movie のサウンド出力中 */
		MVEASY_SOUND_STATUS_ERROR,		/*EN< An error has occurred.  */
										/*JP< エラー状態 */

		/* Keep enum 4bytes */
		MVEASY_SOUND_STATUS_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	/*EN
	 * \brief		PCM audio data format
	 * 
	 * CRI Movie only operates on PCM encoded data.  Two formats are supported: 32 bit floating
	 * point values, and 16 bit integer values.
	 *
	 * Return one of these values from CriMvSoundInterface::GetPcmFormat().
	 * According to the returned format, CRI Movie calls a SetCallback for the specified PCM format.
	 *
	 * \sa CriMvSoundInterface::GetPcmFormat(), CriMvSoundInterface::SetCallbackGetFloat32PcmData(), 
	 *     CriMvSoundInterface::SetCallbackGetSint16PcmData()
	 */
	/*JP
	 * \brief		PCMデータフォーマット
	 * 
	 * PCMの出力フォーマットはPcmFormat型で定義されたいずれかでなければいけません。<br>
	 * EasyPlayerは CriMvSoundInterface::GetPcmFormat()で取得できるデータ型のみ使用します。<br>
	 * アプリケーションはこのクラスの全ての関数を実装しなければいけないので、使わないフォーマット
	 * のコールバック登録関数はカラ関数として実装してください。
	 * 
	 * \sa CriMvSoundInterface::GetPcmFormat(), CriMvSoundInterface::SetCallbackGetFloat32PcmData(), 
	 *     CriMvSoundInterface::SetCallbackGetSint16PcmData()
	 */
	enum PcmFormat {
		MVEASY_PCM_FLOAT32,		/*EN< PCM data is in 32 bit floating point format. */
								/*JP< 32bit 浮動小数型のPCMフォーマット */
		MVEASY_PCM_SINT16,		/*EN< PCM data is in 16 bit integer format. */
								/*JP< 16bit 整数型のPCMフォーマット */

		/* Keep enum 4bytes */
		MVEASY_PCM_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	/*EN
	 * \brief		Creates a sound output module
	 *  \param		heap		Handle to a CriHeap object
	 *  \param		channel		Number of sound channels (1 = monaural, 2 = stereo, 6 = 5.1ch)
	 *  \param		samplerate	Sample rate of audio data (ex. 48000 = 48k)
	 *  \return		TRUE if the sound module was created successfully
	 *  \return     FALSE if there was an error
	 * 
	 * Creates a CRI Movie sound output module and prepares it for use.  Memory for the module 
	 * is taken from the provided CriHeap object.<br>
	 *
	 * CRI Movie calls this method once it has determined that a movie has an active audio track
	 * and has analyzed the number of channels (mono, stereo, 5.1ch, etc.) and the sample rate.
	 * 
	 */
	/*JP
	 * \brief		サウンド出力の作成
	 *  \param		heap		メモリハンドル
	 *  \param		channel		出力するサウンドのチャネル数 (1=monaural, 2=stereo, 6=5.1ch)
	 *  \param		samplerate	サンプリングレート (ex. 48k = 48000)
	 *  \return		作成結果。成功の場合はTRUE、失敗の場合はFALSEが返ります。
	 * 
	 * サウンド出力を作成します。<br>
	 * この関数は CRI Movie が再生するサウンドが決定したあとに、そのサウンドのチャネル数や
	 * サンプリングレートを引数として実行されます。
	 * 
	 */
	/* pure */ virtual CriBool CreateOutput(CriHeap heap, CriUint32 channel, CriUint32 samplerate)=0;

	/*EN
	 * \brief       Destroys the sound output module
	 *
	 * Deletes the sound output module that was created by 
	 * CriMvSoundInterface::CreateOutput().<br>
	 *
	 * CRI Movie calls this method once CriMvSoundInterface::GetStatus() returns MVEASY_SOUND_STATUS_STOP.
	 * 
	 */
	/*JP
	 * \brief		サウンド出力の破棄
	 * 
	 * サウンド出力を破棄します。<br>
	 * この関数はサウンド出力が MVEASY_SOUND_STATUS_STOP 状態になった後に呼び出されます。
	 * 
	 */
	/* pure */ virtual void DestroyOutput(void)=0;

	/*EN
	 * \brief       Gets the PCM format of audio data
	 * \return      The type of PCM format supported by this CriMvSoundInterface instance
	 *
	 * CRI Movie supports audio data in one of two PCM formats: 32 bit floating point (MVEASY_PCM_FLOAT32)
	 * or 16 bit integer (MVEASY_PCM_SINT16).  EasyPlayer uses the return value from this method to 
	 * determine which format is being used, and will call the appropriate callback function to retrieve
	 * data samples.
	 * 
	 * \sa CriMvSoundInterface::PcmFormat
	 */
	/*JP
	 * \brief		PCMデータフォーマットの取得
	 *  \return		CriMvSoundInterface が使用するPCMフォーマットを返します。
	 * 
	 * EasyPlayerはこの関数によって、出力するPCMフォーマットを判断します。
	 * 
	 * \sa CriMvSoundInterface::PcmFormat
	 */
	/* pure */ virtual PcmFormat GetPcmFormat(void)=0;

	/*EN
	 * \brief		Sets the callback function for retrieving 32 bit floating point PCM sound samples
	 *  \param	func	Function that is called when CriMvSoundInterface gets PCM data (32bit float)
	 *  \param	obj		Pointer to user-specifed data, passed as the first argument to the callback
	 * 
	 * Sets a function that CriMvSoundInterface will call when it needs audio data in 32 bit floating point 
	 * PCM format.  This callback function takes 4 arguments:<br>
	 *
	 * - \a obj:        The user-specified \a obj parameter passed to SetCallbackGetFloat32PcmData().<br>
	 * - \a nch:        The number of audio channels.  Mono is 1, stereo is 2, 5.1 channel is 6.<br>
	 * - \a pcmbuf:     An array of buffers to hold the returned PCM data.  There must be one element of 
	 *                  this array for each channel.<br>
	 * - \a req_nsmpl:  The number of samples requested.  Each buffer must be large enough to hold this
	 *                  many samples (i.e. \a nch * \a req_nsmpl).
	 * 
	 * \remarks
	 * The maximum number of channels is CRIMV_PCM_BUFFER_MAX
	 *
	 * \sa CriMvSoundInterface::SetCallbackGetSint16PcmData()
	 */
	/*JP
	 * \brief		32bit形式でPCMデータを取得するコールバック関数の登録
	 *  \param	func	CriMvSoundInterface がPCMデータを要求する際に呼びだすコールバック関数
	 *  \param	obj		コールバック関数を実行する際に第一引数に指定するオブジェクト変数
	 * 
	 * CriMvSoundInterface がEasyPlayerにPCMデータを要求する際に呼び出すコールバック関数を登録します。
	 * コールバック関数は４つの引数を持っています。<BR>
	 * - "obj" はコールバック関数内で使用するオブジェクトです。
	 *   コールバック関数を呼び出す際は、関数登録時に指定されたobjを必ずこの引数に入れてください。<BR>
	 * - "nch" は CriMvSoundInterface が要求するオーディオのチャネル数です。モノラルなら1。ステレオなら2。5.1chなら6となります。<BR>
	 * - "pcmbuf" はPCMデータを格納するためのバッファポインタ配列です。<br>
	 *   バッファの実体は CriMvSoundInterface で準備してください。バッファの数は"nch"と同じでなければいけません。<BR>
	 * - "req_nsmpl" は CriMvSoundInterface が要求するPCMデータの最大サンプル数です。<br>
	 *   "pcmbuf"で指定した各バッファ実体には、このサンプル数が書き込まれても大丈夫なだけの領域を必ず準備してください。<BR>
	 * 
	 * 登録されたコールバック関数を呼び出すタイミングは CriMvSoundInterface の任意となります。
	 * 
	 * \sa CriMvSoundInterface::SetCallbackGetSint16PcmData()
	 */
	/* pure */ virtual void SetCallbackGetFloat32PcmData(CriUint32 (*func)(void *obj, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl), void *obj)=0;

	/*EN
	 * \brief       Sets the callback function for retrieving 16 bit integer PCM sound samples
	 * \param       func            Function that will be called when 16 bit integer PCM data is needed
	 * \param       obj             Pointer to user-specifed data, passed as the first argument to the callback
	 *
	 * Sets a function that CriMvSoundInterface will call when it needs audio data in 16 bit integer
	 * PCM format.  This callback function takes 4 arguments:<br>
	 *
	 * - \a obj:        The user-specified \a obj parameter passed to SetCallbackGetSint16PcmData().<br>
	 * - \a nch:        The number of audio channels.  Mono is 1, stereo is 2, 5.1 channel is 6.<br>
	 * - \a pcmbuf:     An array of buffers to hold the returned PCM data.  There must be one element of 
	 *                  this array for each channel.<br>
	 * - \a req_nsmpl:  The number of samples requested.  Each buffer must be large enough to hold this
	 *                  many samples (i.e. \a nch * \a req_nsmpl).
	 *
	 * \remarks
	 * The maximum number of channels is CRIMV_PCM_BUFFER_MAX (currently 8)
	 * 
	 * \sa CriMvSoundInterface::SetCallbackGetFloat32PcmData()
	 */
	/*JP
	 * \brief		16bit形式でPCMデータを取得するコールバック関数の登録
	 *  \param	func	CriMvSoundInterface がPCMデータを要求する際に呼びだすコールバック関数
	 *  \param	obj		コールバック関数を実行する際に第一引数に指定するオブジェクト変数
	 * 
	 * PCMフォーマットが違う以外は、 CriMvSoundInterface::SetCallbackGetFloat32PcmData() と同じです。
	 * 
	 * \sa CriMvSoundInterface::SetCallbackGetFloat32PcmData()
	 */
	/* pure */ virtual void SetCallbackGetSint16PcmData(CriUint32 (*func)(void *obj, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl), void *obj)=0;

	/*EN
	 * \brief       Starts sound output
	 *
	 * CRI Movie will call this method when it needs to start playing sound.  The PCM data callback function
	 * has to be called after this function until movie playback is finished.<br>
	 *
	 * Your sound module should begin incrementing its playback time when this method is called.
	 *
	 * \remarks
	 * On success, this should set the status to CriMvSoundInterface::MVEASY_SOUND_STATUS_EXEC.
	 */
	/*JP
	 * \brief		サウンド出力の開始
	 * 
	 * サウンド出力を開始します。PCMデータ取得用コールバック関数は、本関数の呼出し後から実行してください。
	 */
	/* pure */ virtual void Start(void)=0;

	/*EN
	 * \brief       Stops sound output
	 *
	 * CRI Movie will call this when the movie playback stops, or the status changes to MVEASY_STATUS_PLAYEND.<br>
	 *
	 * When EasyPlayer wants to pause and restart playback, it will call CriMvSoundInterface::Pause()
	 * instead of this method.<br>
	 *
	 * Your sound module should set its playback time to 0 when this method is called.
	 * 
	 * \remarks
	 * This should set the status to CriMvSoundInterface::MVEASY_SOUND_STATUS_STOP.
	 * 
	 * \sa CriMvSoundInterface::Pause(), CriMvSoundInterface::Start()
	 */
	/*JP
	 * \brief		サウンド出力の停止
	 * 
	 * サウンド出力を停止します。再開できるようにする必要はありません。<br>
	 * EasyPlayerが再開処理を行いたい場合は、本関数ではなく、 CriMvSoundInterface::Pause()を呼び出します。<br>
	 * CriMvSoundInterface::Stop() 呼出し後は、コルーバック関数を呼ばないように実装してください。
	 * 
	 * \sa CriMvSoundInterface::Pause(), CriMvSoundInterface::Stop()
	 */
	/* pure */ virtual void Stop(void)=0;

	/*EN
	 * \brief       Gets status of sound module
	 * \return      The module status
	 *
	 * This must return one of the enumerated values in CriMvSoundInterface::Status.<br>
	 *
	 * While this method returns CriMvSoundInterface::MVEASY_SOUND_STATUS_EXEC, EasyPlayer will call the
	 * PCM data callback.<br>
	 * When a movie has finished playing and this method returns CriMvSoundInterface::MVEASY_SOUND_STATUS_STOP, 
	 * EasyPlayer will call CriMvSoundInterface::DestroyOutput().
	 * 
	 * \sa  CriMvSoundInterface::Status
	 */
	/*JP
	 * \brief		サウンドモジュールの状態取得
	 * 
	 * サウンドモジュールの状態を取得します。
	 * 
	 * \sa  CriMvSoundInterface::Status
	 */
	/* pure */ virtual Status GetStatus(void)=0;

	/*EN
	 * \brief       Pauses or resumes sound output
	 * \param       sw          Pause or resume playback.
	 *
	 * If \a sw is 1 (ON), output will be paused.<br>
	 * If \a sw is 0 (OFF), output will be resumed.<br>
	 *
	 * Temporarily pauses or resumes sound output.<br>
	 *
	 * When you pause sound output, you must pause your playback timer as well.
	 */
	/*JP
	 * \brief		サウンド出力の一時停止または再開
	 *  \param	sw		ポーズスイッチ。ポーズONの場合は1、ポーズOFF(レジューム)の場合は0を指定します。
	 * 
	 * 本関数の動作は引数に依存します。<br>
	 * 引数 sw がON(1)なら、一時停止。引数 sw がOFF(0)ならサウンド出力再開です。
	 */
	/* pure */ virtual void Pause(CriBool sw)=0;

	/*EN
	 * \brief       Gets the time, in seconds, that sound has been playing
	 * \param       count       Playback time counter
	 * \param       unit        Counter increment per second
	 *
	 * CRI Movie calls this method periodically for some damn reason.<br>
	 * <br>
	 * The time, in seconds, is specified by \a count / \a unit.<br>
	 *
	 * For example, if \a count was 500 and \a unit was 1000, that would be 0.5 seconds.<br>
	 *
	 * \remarks
	 * Before Start() is called, and after Stop() is called, \a count should be 0.
	 */
	/*JP
	 * \brief		再生時刻の取得
	 *  \param	count	タイマカウント
	 *  \param	unit	１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 * 
	 * タイマ時刻を取得します。時刻はcountとunitの二つの変数で表現します。<br>
	 * count ÷ unit で秒単位の時刻となるような値を返します。<br>
	 * 再生開始前（ CriMvSoundInterface::Start()呼び出し前）および
	 * 再生停止後（ CriMvSoundInterface::Stop()呼び出し後）は、時刻０（タイマカウントが０）を返します。
	 */
	/* pure */ virtual void GetTime(CriUint64 &count, CriUint64 &unit)=0;		// sec = count / unit.

protected:
	virtual ~CriMvSoundInterface(void) {}
};

/*EN
 * \brief	System Timer Interface Class for EasyPlayer
 * \ingroup MDL_IF_TIMER
 *
 * If you want to synchronize video frames with something other than the audio track, or if you
 * need to play a movie that does not have an audio track, you will need to pass an instance of
 * a class derived from CriMvSystemTimerInterface to CriMvEasyPlayer::Create().<br>
 *
 * If you do not need any special timer facilities, you can pass NULL to CriMvEasyPlayer::Create() instead.<br>
 *
 * All functions are pure virtual.
 *
 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::SyncMasterTimer()
 */
/*JP
 * \brief	システムタイマーインタフェース for EasyPlayer
 * \ingroup MDL_IF_TIMER
 * 
 * システムタイマーは音無しムービ再生時に、ビデオフレームの送出タイミングを調整するために使用されます。<br>
 * このクラスを定義することで、自前のタイマシステムを使ってEasyPlayerのストリーミング再生が可能になります。<br>
 * 全ての関数は純粋仮想関数として定義されているので、全ての関数を必ず実装してください。
 * 
 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::SyncMasterTimer()
 */
class CriMvSystemTimerInterface
{
public:
	/*EN
	 * \brief       Starts timer and resets it to 0
	 *
	 * When this method is called, your internal counter should initialize itself to 0 and start
	 * normal operation.
	 */
	/*JP
	 * \brief		タイマ開始
	 * 
	 * タイマのカウントを開始します。この関数が呼ばれた時が時刻０となります。
	 */
	/* pure */ virtual void Start(void)=0;

	/*EN
	 * \brief       Stops the timer and resets it to 0
	 *
	 * When this method is called, your internal counter should stop incrementing, and re-initialize
	 * itself to 0.  After this method has been called, CriMvSystemTimerInterface::GetTime() must
	 * return a time of 0 seconds.
	 */
	/*JP
	 * \brief		タイマ停止
	 * 
	 * タイマのカウントを停止します。この関数が呼ばれたあとに、そのタイマを再開することはありません。
	 */
	/* pure */ virtual void Stop(void)=0;

	/*EN
	 * \brief       Pauses or resumes the timer
	 *
	 * \param       sw          Pause or resume timer operation.
	 *
	 * If \a sw is 1 (ON), the timer will be paused.<br>
	 * If \a sw is 0 (OFF), the timer will be resumed.<br>
	 *
	 * Temporarily pauses or resumes the timer.<br>
	 *
	 * When you pause the timer, you must maintain the previous value of the counter.
	 */
	/*JP
	 * \brief		タイマの一時停止または再開
	 *  \param	sw		ポーズスイッチ。ON(1)なら一時停止、OFF(0)なら再開。
	 * 
	 * 本関数の動作は引数に依存します。<br>
	 * 引数 sw がON(1)なら、一時停止。引数 sw がOFF(0)ならタイマカウント再開です。
	 */
	/* pure */ virtual void Pause(CriBool sw)=0;

	/*EN
	 * \brief       Gets the time, in seconds, that the timer has been running
	 *
	 * \param       count       Timer counter
	 * \param       unit        Counter increment per second
	 *
	 * CRI Movie calls this method periodically to synchronize video playback with the
	 * movie's internal framerate.<br>
	 * <br>
	 * The time, in seconds, is specified by \a count / \a unit.<br>
	 *
	 * For example, if \a count was 500 and \a unit was 1000, that would be 0.5 seconds.<br>
	 *
	 * \remarks
	 * Before Start() is called, and after Stop() is called, \a count should be 0.
	 */
	/*JP
	 * \brief		経過時刻の取得
	 *  \param	count	タイマカウント
	 *  \param	unit	１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 * 
	 * タイマ時刻を取得します。時刻はcountとunitの二つの変数で表現します。<br>
	 * count ÷ unit で秒単位の時刻となるような値を返します。<br>
	 * 再生開始前（ CriMvSystemTimerInterface::Start()呼び出し前）および
	 * 再生停止後（ CriMvSystemTimerInterface::Stop()呼び出し後）は、時刻０（タイマカウントが０）を返します。
	 */
	/* pure */ virtual void GetTime(CriUint64 &count, CriUint64 &unit)=0;

protected:
	virtual ~CriMvSystemTimerInterface(void) {}
};


/*EN
 * \brief	EasyPlayer Interface class for CRI Movie
 * \ingroup MDL_EASY_PLAYER
 */
/*JP
 * \brief	EasyPlayerインタフェース
 * \ingroup MDL_EASY_PLAYER
 */
class CriMvEasyPlayer : public CriAllocator
{
public:
	/*EN
	 * \brief       The possible states an EasyPlayer handle can be in.
	 *
	 * An EasyPlayer handle takes on various states, depending on where it is in the decoding process.
	 * You can check the status of a valid EasyPlayer handle at any time by calling CriMvEasyPlayer::GetStatus().
	 *
	 * An EasyPlayer handle is created in the <b>MVEASY_STATUS_STOP</b> state.  During movie playback, the status
	 * transitions through various states from <b>MVEASY_STATUS_STOP</b> to <b>MVEASY_STATUS_PLAYEND</b>.
	 *
	 * An application does not need to check all states.  At a minimum, it only needs to check for 
	 * <b>MVEASY_STATUS_STOP</b>, <b>MVEASY_STATUS_PLAYING</b>, <b>MVEASY_STATUS_PLAYEND</b>, 
	 * and <b>MVEASY_STATUS_ERROR</b>.
	 *
	 * If an application calls CriMvEasyPlayer::DecodeHeader(), the status of the EasyPlayer handle will change to
	 * <b>MVEASY_STATUS_WAIT_PREP</b> when CRI Movie has finished analyzing the movie information.  The EasyPlayer 
	 * handle will remain in this state until the application calls CriMvEasyPlayer::Prepare() or 
	 * CriMvEasyPlayer::Start().  Once the state has changed to <b>MVEASY_STATUS_WAIT_PREP</b>, information
	 * about the movie can be retrieved by calling CriMvEasyPlayer::GetMovieInfo().
	 *
	 * If an application calls CriMvEasyPlayer::Prepare(), the EasyPlayer handle status will change to 
	 * <b>MVEASY_STATUS_READY</b> once CRI Movie has finished buffering enough input and decoded output for playback.
	 * The handle will remain in this state until CriMvEasyPlayer::Start() is called.  This allows the application
	 * to better control playback timing, since a movie can immediately start playing.
	 *
	 * Once CriMvEasyPlayer::Start() is called and the movie is actively playing, the status will alternate
	 * between <b>MVEASY_STATUS_PLAYING</b> and <b>MVEASY_STATUS_PREP</b>, as CRI Movie plays back frames and decodes
	 * new ones.
	 *
	 * When the movie has finished playing normally, the status of the EasyPlayer handle will automatically change
	 * to <b>MVEASY_STATUS_PLAYEND</b>.  If the movie is in looping mode, however, once the movie reaches the 
	 * end, it will start playing from the beginning and the status will not change to <b>MVEASY_STATUS_PLAYEND</b>.
	 *
	 * When CriMvEasyPlayer::Stop() is called, the status of the handle will change to <b>MVEASY_STATUS_STOP</b> 
	 * once it has finished any decoding and playback that is in progress.  This does not happen immediately,
	 * but will take a few cycles.
	 *
	 * If there are any problems during playback, for instance insufficient memory or invalid input data, the
	 * status will change to <b>MVEASY_STATUS_ERROR</b>.  When the handle is in state <b>MVEASY_STATUS_ERROR</b>,
	 * the application must call CriMvEasyPlayer::Stop() and wait until the state changes to <b>MVEASY_STATUS_STOP</b>
	 * before doing anything else with the handle.
	 *
	 * Once the state is <b>MVEASY_STATUS_STOP</b> or <b>MVEASY_STATUS_PLAYEND</b>, the handle can be deleted by
	 * calling CriMvEasyPlayer::Destroy().
	 *
	 * \attention
	 * As of CRI Movie version 2.00, the handling of the <b>MVEASY_STATUS_ERROR</b> state has changed.  Previously,
	 * a handle could be destroyed when it was in the <b>MVEASY_STATUS_ERROR</b> state.  Now, an application 
	 * must call CriMvEasyPlayer::Stop() and wait for the <b>MVEASY_STATUS_STOP</b> state before destroying
	 * the handle.
	 *
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::Start(), CriMvEasyPlayer::DecodeHeader(), 
	 *     CriMvEasyPlayer::Prepare(), CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::Stop(), 
	 *     CriMvEasyPlayer::Destroy()
	 */
	/*JP
	 * \brief		EasyPlayerハンドル状態
	 * 
	 * EasyPlayer のハンドル状態です。
	 * ハンドル状態は CriMvEasyPlayer::GetStatus() でいつでも取得することが出来ます。<br>
	 * ハンドル作成直後は MVEASY_STATUS_STOP 状態です。
	 * 
	 * ハンドル状態は MVEASY_STATUS_STOP から MVEASY_STATUS_PLAYEND まで順に遷移していきます。<br>
	 * アプリケーションがムービを再生するにあたって、必ずしもこの全ての状態をチェックする必要はありません。<br>
	 * 最低限、MVEASY_STATUS_STOP, MVEASY_STATUS_PLAYING, MVEASY_STATUS_PLAYEND, MVEASY_STATUS_ERROR さえ
	 * チェックすれば、ムービの再生を行うことができます。
	 * 
	 * EasyPlayer ハンドル作成後、ムービの解像度などが既に確定している場合は、アプリケーションは
	 * CriMvEasyPlayer::Start() を直接呼び出すことができます。この場合、ハンドル状態は自動的に
	 * MVEASY_STATUS_PLAYEND まで遷移していきます。
	 * 
	 * 最初に CriMvEasyPlayer::DecodeHeader() を呼び出した場合は、ヘッダ解析が終了するとハンドル状態は
	 * MVEASY_STATUS_WAIT_PREP となり、アプリケーションから CriMvEasyPlayer::Prepare() または
	 * CriMvEasyPlayer::Start() が呼ばれるまで待機します。
	 * 
	 * MVEASY_STATUS_WAIT_PREP状態以降、 CriMvEasyPlayer::GetMovieInfo() でムービ情報を取得することができます。<br>
	 * CriMvEasyPlayer::Prepare() を呼び出した場合は、ヘッダ解析およびデータのバッファリングが終わると、
	 * ハンドル状態は MVEASY_STATUS_READY となり、アプリケーションから CriMvEasyPlayer::Start() が
	 * 呼ばれるまで待機します。これによって再生開始のタイミングを調整することができます。
	 * 
	 * 再生が終了すると自動的に MVEASY_STATUS_PLAYEND になります。
	 * 
	 * CriMvEasyPlayer::Stop() を呼び出した場合は、デコーダの停止処理が終わったあとに MVEASY_STATUS_STOP
	 * 状態になります。 CriMvEasyPlayer::Stop() 終了直後に停止状態になるとは限りません。
	 * 
	 * メモリ不足やデータエラーなど何らかの問題が発生した場合は MVEASY_STATUS_ERROR 状態となります。<BR>
	 * MVEASY_STATUS_ERROR 状態になった場合は CriMvEasyPlayer::Stop() を呼び出してハンドル状態が
	 * MVEASY_STATUS_STOP 状態に遷移させてください。<BR>
	 * 
	 * CriMvEasyPlayer::Destroy() は MVEASY_STATUS_STOP, MVEASY_STATUS_PLAYEND の
	 * いずれかの状態の時のみ呼び出すことができます。
	 * 
	 * \attention
	 * CRI Movie Ver.2.00 で MVEASY_STATUS_ERROR 状態についての仕様が変更になりました。<BR>
	 * MVEASY_STATUS_ERROR 状態でハンドル破棄が出来なくなり、 CriMvEasyPlay::Stop() を呼び出す必要があります。
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::Start(), CriMvEasyPlayer::DecodeHeader(), 
	 *     CriMvEasyPlayer::Prepare(), CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::Stop(), 
	 *     CriMvEasyPlayer::Destroy()
	 */
	enum Status {
		MVEASY_STATUS_STOP,			/*EN< Standstill.  No processing is happening.
									 *    EasyPlayer handles are created in this state. */
									/*JP< 停止中 */
		MVEASY_STATUS_DECHDR,		/*EN< The EasyPlayer handle is now parsing the movie header, 
									 *    including information about the width and height of the video stream. */
									/*JP< ヘッダ解析中 */
		MVEASY_STATUS_WAIT_PREP,	/*EN< The EasyPlayer handle is a waiting for the work buffer to be allocated. */
									/*JP< バッファリング開始待機中 */
		MVEASY_STATUS_PREP,			/*EN< The EasyPlayer handle is now buffering video and audio data. */
									/*JP< 再生準備中 */
		MVEASY_STATUS_READY,		/*EN< Ready to start playback. */
									/*JP< 再生待機 */
		MVEASY_STATUS_PLAYING,		/*EN< The decoders are currently decoding and playing output. */
									/*JP< 再生中 */
		MVEASY_STATUS_PLAYEND,		/*EN< The end of the movie has been reached. */
									/*JP< 再生終了 */
		MVEASY_STATUS_ERROR,		/*EN< An error has occurred. */
									/*JP< エラー */

		/* Keep enum 4bytes */
		MVEASY_STATUS_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	/*EN
	 * \brief		Supported timer types; used to synchronize video frames.
	 */
	/*JP
	 * \brief		タイマ種別
	 */
	enum TimerType {
		MVEASY_TIMER_NONE,		/*EN< No synchronization.  The output is available as soon as
								 *    each frame is decoded. */
								/*JP< ビデオフレームは時刻同期をしません。デコードが終わったフレーム
								 *    はすぐに取得することができます。 */
		MVEASY_TIMER_SYSTEM,	/*EN< Video frames synchronize to the system timer.<br>
								 *    You must provide an instance of CriMvSystemTimerInterface to
								 *    CriMvEasyPlayer::Create().                                                     */
								/*JP< ビデオフレームはシステム時刻に同期します。システム時刻はアプリケーション
								 *    が CriMvSystemTimerInterface としてCriMvEasyハンドルに設定する必要があります。 */
		MVEASY_TIMER_AUDIO,		/*EN< Video frames synchronize with the movie's audio data.<br>
								 *    You must provide an instance of CriMvSoundInterface to CriMvEasyPlayer::Create(). <br> 
								 *    If the movie does not have audio, video frames will synchronize with the system timer. */
								/*JP< ビデオフレームはムービのオーディオ時刻に同期します。
								 *    アプリケーションは GetTime関数を含む CriMvSoundInterface をCriMvEasy
								 *    ハンドルに設定する必要があります。もしもムービデータにオーディオが含まれて
								 *    いない場合は、ビデオはシステム時刻に同期します。 */

		/* Keep enum 4bytes */
		MVEASY_TIMER_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	/*EN
	 * \ingroup     MODULE_INIT
	 *
	 * \brief       Creates an EasyPlayer handle
	 *
	 * \param       heap        Handle to a CriHeap object
	 * \param       freader     File input interface
	 * \param       stimer      System timer interface
	 * \param       sound       Sound module interface
	 * \param       err         Optional error code
	 *
	 * \return      A valid CriMvEasyPlayer handle, or NULL if the handle cannot be allocated
	 *
	 * Creates and initialize a new EasyPlayer handle.  Its status is initially MVEASY_STATUS_STOP.
	 * Memory for the handle is allocated from the provided CriHeap object.
	 *
	 * If memory allocation fails, this function will return NULL.  Be sure to initialize and create
	 * your heap with criHeap_Initialize() and criHeap_Create() before calling this function.
	 *
	 * \remarks
	 * CriMv::Initialize() must be called <b>before</b> calling this function.
	 * 
	 */
	/*JP
	 * \ingroup MODULE_INIT
	 * \brief		EasyPlayerハンドルの作成
	 *  \param		heap	CriHeapハンドル
	 *  \param		freader	ファイル読み込みインタフェース
	 *  \param		stimer	システムタイマインタフェース
	 *  \param		sound	サウンドインタフェース
	 *  \param		err		エラー情報
	 *  \return		CriMvEasyハンドルを返します。エラーが発生した場合は、NULLを返します。
	 *
	 * 本関数は CriMv::Initialize() 呼び出しよりも<B>後</B>に実行してください。<BR>
	 * ハンドル作成後はハンドル状態はMVEASY_STATUS_STOPとなります。<br>
	 * ハンドル確保に必要なメモリは全て、引数で渡された CriHeap を使って確保されます。<br>
	 * メモリ不足などでエラーが発生した場合は、本関数はNULLを返します。
	 * 
	 */
	static CriMvEasyPlayer* CRIAPI Create(CriHeap heap, 
								   CriMvFileReaderInterface *freader, 
								   CriMvSystemTimerInterface *stimer, 
								   CriMvSoundInterface *sound, 
								   CriError &err=CriMv::ErrorContainer);
	/*EN
	 * \ingroup MODULE_INIT
	 * \brief		Destroy a handle
	 *  \param		err		Optional error code 
	 *
	 * \ingroup     MODULE_INIT
	 *
	 * \brief       Destroys an EasyPlayer handle
	 * \param       err         Optional error code
	 *
	 * Destroys an EasyPlayer handle previously created with CriMvEasyPlayer::Create(), and frees its resources.
	 *
	 * An EasyPlayer handle can only be destroyed when it is in the MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND state.
	 * Attempting to destroy a handle when it is in any other state will cause an error.
	 *
	 * Any work buffers allocated via CriHeap, if still associated with the handle, are freed by this call.
	 *
	 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::Status, CriMvEasyPlayer::GetStatus()
	 */
	/*JP
	 * \ingroup MODULE_INIT
	 * \brief		EasyPlayerハンドルの破棄
	 *  \param		err		エラー情報（省略可）
	 *
	 * CriMvEasyPlayer::Create()で作成したEasyPlayerハンドルを破棄します。
	 *
	 * ハンドル状態が MVEASY_STATUS_STOP 、 MVEASY_STATUS_PLAYEND の時にのみハンドルを破棄することができます。<br>
	 * それ以外の状態で呼び出した場合は、エラーになります。<br>
	 * 
	 * ハンドル状態が MVEASY_STATUS_ERROR だった場合は、CriMvEasyPlayer::Stop() を呼び出して
	 * MVEASY_STATUS_STOP 状態になってからハンドル破棄してください。<BR>
	 * ハンドル状態は CriMvEasyPlayer::GetStatus() で確認することができます。
	 *
	 * ハンドル作成時に指定したCriHeapによって確保されたメモリで未解放の全ては、
	 * この関数の呼び出しによって解放されます。
	 *
	 * \sa CriMvEasyPlayer::Create(), CriMvEasyPlayer::Status, CriMvEasyPlayer::GetStatus()
	 */
	void Destroy(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_BASIC
	 *
	 * \brief       Returns status of an EasyPlayer handle.
	 * \param       err         Optional error code
	 * \return      Handle status
	 *
	 * Returns the current status of an EasyPlayer handle.  The status will be one of the values 
	 * defined by CriMvEasyPlayer::Status.
	 *
	 * \sa CriMvEasyPlayer::Status
	 */
	/*JP
	 * \ingroup MODULE_BASIC
	 * \brief		ハンドル状態の取得
	 *  \param		err		エラー情報（省略可）
	 *  \return		ハンドル状態 CriMvEasyPlayer::Status
	 *
	 *  ハンドル状態を取得します。
	 *
	 * \sa CriMvEasyPlayer::Status
	 */
	Status GetStatus(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_BASIC
	 *
	 * \brief       Executes heatbeat functions for an EasyPlayer handle
	 * \param       err         Optional error code
	 *
	 * Performs regular processing for an EasyPlayer handle, including handoff and parsing of input
	 * buffers, and audio decoding.  Additionally, it checks for situations where the handle state
	 * should change to one of the values defined by CriMvEasyPlayer::Status, and changes state as
	 * necessary.
	 *
	 * This function and CriMvEasyPlayer::ExecuteDecode() should be called periodically during movie 
	 * playback or when waiting for CriMvEasyPlayer::DecodeHeader() to complete.  It takes a relatively 
	 * low CPU load, and should typically be called on every vertical blank.
	 *
	 * \remarks
	 * Note that this function does not perform any video decoding.  Decoding is done in 
	 * CriMvEasyPlayer::ExecuteDecode().
	 * 
	 * \sa CriMvEasyPlayer::Status, CriMvEasyPlayer::ExecuteDecode(), CriMvEasyPlayer::DecodeHeader()
	 */
	/*JP
	 * \ingroup MODULE_BASIC
	 * \brief		EasyPlayerサーバ関数
	 *  \param		err		エラー情報（省略可）
	 *
	 * ムービのヘッダ解析や入力バッファ制御、オーディオデコード等を行います。<br>
	 * EasyPlayer ハンドルの状態遷移もこの関数で行います。<br>
	 * この関数はビデオのデコードは行いません。そのためCPU負荷はあまり高くなりません。<br>
	 * 本関数はアプリケーションのメインループで毎回呼び出すようにしてください。
	 * 
	 * \sa CriMvEasyPlayer::Status
	 */
	void Update(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_BASIC
	 * 
	 * \brief       Synchronizes the master timer of an EasyPlayer handle.
	 * \param       err         Optional error code
	 *
	 * Synchronizes the timing of video frames to the master timer used by this EasyPlayer handle.
	 * If the master timer is of type MVEASY_TIMER_AUDIO and the movie does not have audio,
	 * EasyPlayer will use the system timer that was set when the handle was created.
	 *	 
	 * \sa CriMvEasyPlayer::TimerType, CriMvEasyPlayer::SetMasterTimer(), CriMvSystemTimerInterface
	 */
	/*JP
	 * \ingroup MODULE_BASIC
	 * \brief		マスタタイマへの同期
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービ再生時刻をマスタタイマに同期させます。<br>
	 * マスタタイマは CriMvEasyPlayer::SetMasterTimer() によって指定されたタイマを使います。<br>
	 * タイマ種別として MVEASY_TIMER_AUDIO が指定されていて、再生するムービにオーディオが
	 * 含まれていない場合は、ハンドル作成時のシステムタイマを使用します。
	 * 
	 * この関数は、ハンドル作成時に指定したシステムタイマインタフェースの
	 * CriMvSystemTimerInterface::GetTime() を呼び出します。
	 * 
	 * \sa CriMvEasyPlayer::TimerType, CriMvEasyPlayer::SetMasterTimer(), CriMvSystemTimerInterface
	 */
	void SyncMasterTimer(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_BASIC
	 *
	 * \brief       Decodes a video frame
	 * \param       err         Optional error code
	 * \return      \a TRUE     If the movie is currently in a playback state
	 * \return      \a FALSE    If the movie is not playing
	 *
	 * Performs the actual video decoding.  Each call to this function decodes a single frame.
	 * It does not return until the frame has been decoded.
	 *
	 * Video decoding can impose a very heavy CPU load, so calling this function from an application's
	 * main thread may cause your application to miss the vertical blank interval.  To avoid this situation,
	 * ExecuteDecode() can be called from a separate, lower-priority thread.  See the <b>Mutithreaded Decoding</b>
	 * tutorial for a full explanation.
	 *
	 * The return value describes the movie playback state.  If TRUE, the movie is currently playing
	 * (or decoding the movie header) or ready to play.  If FALSE, playback has not yet been started, 
	 * or playback has finished.
	 *
	 * If a decoding thread is used, an application must wait until this function returns FALSE before destroying 
	 * the thread.  Otherwise, the handle state will not transition to MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND,
	 * and the EasyPlayer handle cannot be destroyed.
	 * 
	 * \attention
	 * ExecuteDecode() is the only CRI Movie function that is safe to call from a separate thread.
	 * No other CRI Movie functions should be considered to be thead-safe.
	 * 
	 */
	/*JP
	 * \ingroup MODULE_BASIC
	 * \brief		ビデオデコード
	 *  \param		err		エラー情報（省略可）
	 *  \return				ムービ再生中はTRUE 、再生終了また停止後はFALSEを返します。
	 * 
	 * ビデオデータのデコードを行います。<br>
	 * 本関数は１ピクチャ分のデコードを終わるまで終了しません。<br>
	 * ピクチャデコードは負荷の高い処理なので、アプリケーションのメインスレッドから呼び出すと処理落ちが発生する可能性があります。<br>
	 * その場合は、メインスレッドよりも優先度の低い別スレッドから呼び出すようにしてください。
	 * 
	 * 本関数の返り値は、ムービ再生の実行中かどうかを表しています。<BR>
	 * デコード用スレッドを終了する場合は、返り値がFALSEになるのを待たなければいけません。<BR>
	 * 返り値がTRUEの間にデコードスレッドを終了してしまうと、ハンドルの状態が MVEASY_STATUS_STOP や
	 *  MVEASY_STATUS_PLAYEND に遷移できず、ハンドル破棄が出来なくなります。<BR>
	 * 
	 */
	CriBool ExecuteDecode(CriError &err=CriMv::ErrorContainer);

	/* 再生制御 */
	/*EN
	 * \ingroup MODULE_CONTROL
	 *
	 * \brief       Sets the name of the movie file to play
	 * \param       fname       Name of the movie file
	 * \param       err         Optional error code
	 *
	 * Sets the name of the movie file to play, but does not open the file.  The length of the filename
	 * (including path) is limited to CRIMV_MAX_FILE_NAME characters.  EasyPlayer copies this 
	 * string to internal memory, so a temporary variable can be used.
	 *
	 * If an application plays the same movie repeatedly, it only needs to call this function once.
	 *
	 * Multiple calls to this function will overwrite previous values.  Calling CriMvEasyPlayer::SetData() 
	 * will clear any filename set by this function.
	 *
	 * \remarks
	 * This function can only be called when the handle status is MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND,
	 * or from the file request callback.  See CriMvEasyPlayer::SetFileRequestCallback() for details.
	 *
	 * \remarks
	 * Calling this function does not open the file.  The file is opened by a call to CriMvEasyPlayer::Update().
	 *
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetData()
	 */
	/*JP
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       再生ファイルの指定
	 * \param       fname       ムービファイルパス
	 * \param		err			エラー情報（省略可）
	 *
	 * 再生するムービのファイルパスを設定します。ファイルパスの最大長は CRIMV_MAX_FILE_NAME バイトです。<br>
	 * EasyPlayerは内部でこのファイルパスをコピーするので、引数で渡した文字列は破棄してもかまいません。
	 *
	 * 同じファイルを繰り返し再生する場合は、この関数を再度呼び出す必要はありません。
	 *
	 * この関数を呼び出した直後にもう一度この関数を呼び出すと、前回のファイル情報は新しいファイル情報に上書きされます。
	 * CriMvEasyPlayer::SetData()関数を呼び出した場合は、事前に設定したファイル情報がクリアされます。
	 *
	 * \para 備考1：
	 * この関数はハンドル状態が MVEASY_STATUS_STOP もしくは MVEASY_STATUS_PLAYEND時のみ呼び出し可能です。
	 * またはファイル要求コールバック関数内でもこの関数を呼び出すことができます。詳細は CriMvEasyPlayer::SetFileRequestCallback()関数を
	 * 参照してください。
	 *
	 * \para 備考2：
	 * この関数の内部ではファイルのオープン要求はしません。ファイルのオープン処理は CriMvEasyPlayer::Update() 関数の中で行われます。
	 *
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetData(), CriMvEasyPlayer::SetFileRange()
	 */
	void SetFile(CriChar8 *fname, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Sets in-memory movie data 
	 * \param       dataptr     Pointer to movie data
	 * \param       datasize    Size of data, in bytes
	 * \param       err         Optional error code
	 *
	 * Sets the in-memory data buffer for this EasyPlayer handle to point to the provided buffer,
	 * but does not attempt to read that buffer.  After this call, the memory pointed to by \a dataptr
	 * belongs to CRI Movie and must remain valid until playback is complete.
	 *
	 * If an application plays the same movie data repeatedly, it only needs to call this function once.
	 *
	 * Multiple calls to this function will overwrite previous values.  Calling CriMvEasyPlayer::SetFile() 
	 * will clear any pointer set by this function.
	 *
	 * \remarks
	 * This function can only be called when the handle status is MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND,
	 * or from the file request callback.  See CriMvEasyPlayer::SetFileRequestCallback() for details.
	 *
	 * \remarks
	 * Calling this function does not attempt to reference the memory.  The memory read is initiated by a call 
	 * to CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start().
	 * 
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetFile(), CriMvEasyPlayer::SetFileRange()
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		メモリ上データの指定
	 *  \param		dataptr		データポインタ
	 *  \param		datasize	データサイズ
	 *  \param		err			エラー情報（省略可）
	 * 
	 * この関数はEasyPlayerのハンドル状態がMVEASY_STATUS_STOPかMVEASY_STATUS_PLAYENDの時に呼び出してください。<br>
	 * または、ファイル要求コールバックの内部で呼び出すことができます。<BR>
	 * 
	 * 本関数を繰り返し呼び出した場合は、メモリ情報は上書きされます。<BR>
	 * CriMvEasyPlayer::SetFile()を呼び出した場合は、本関数で指定したメモリ情報はハンドル内から消去されます。
	 * 
	 * 同じハンドルで同じムービデータを繰り返し再生する場合は、本関数の呼び出しは省略することができます。
	 * 
	 * 指定されたメモリ領域に実際にアクセスするのは、 CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Prepare(), 
	 * CriMvEasyPlayer::Start() のいずれかが呼び出された時以降です。<br>
	 * 
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetFile(), CriMvEasyPlayer::SetFileRange()
	 */
	void SetData(CriUint8 *dataptr, CriUint32 datasize, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_CONTROL
	 * \brief		Set a pack file and pass offset and range of movie file to an EasyPlayer handle
	 *  \param		fname	File name string pointer of the pack file
	 *  \param		offset	offset in byte to movie data in the packing file [in bytes]
	 *  \param		range	size of movie data from the offset in the packing file [in bytes]
	 *  \param		err		Optional error code 
	 * 
	 * Sets the name of the pack file that contains the movie file to play.
	 * EasyPlayer tries to read data from 'offset' through 'range' in the pack file as the movie data.
	 * Passing a negative value as range, EasyPlayer reads data until the end of the pack file.
	 * 
	 * The length of file name is limited until CRIMV_MAX_FILE_NAME.
	 * EasyPlayer handle copies the file name string into the handle.
	 * You can use a temporary variable as the file name string.
	 * 
	 * If an application plays the same movie repeatedly, it only needs to call this function once.
	 *
	 * Multiple calls to this function will overwrite previous values.  Calling CriMvEasyPlayer::SetData() 
	 * will clear any filename set by this function.
	 * 
	 * \remarks
	 * This function can only be called when the handle status is MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND,
	 * or from the file request callback.  See CriMvEasyPlayer::SetFileRequestCallback() for details.
	 *
	 * \remarks
	 * Calling this function does not open the file.  The file is opened by a call to CriMvEasyPlayer::Update().
	 * 
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetData(), CriMvEasyPlayer::SetFile()
	 */
	/*JP
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       再生したいムービファイルを含むパックファイルの指定
	 * \param		fname	パックファイル名 (パスを含む)
	 * \param		offset	パックファイル内のムービデータまでのオフセット (単位: バイト) 
	 * \param		range	パックファイル内のムービデータのサイズ (単位：バイト)
	 * \param		err		エラー情報（省略可）
	 *
	 * 再生するムービを含むパックファイルを指定します。引数で指定した offset 位置から range サイズ分までをパックファイル内に含まれるムービデータみなします。
     * rangeに負値を入力するとパックファイルの終端までをムービとして読み込みます。
	 *
	 * パックファイルのファイルパスの最大長は CRIMV_MAX_FILE_NAME バイトです。
	 * EasyPlayerは内部でこのファイルパスをコピーするので、引数で渡した文字列は破棄してもかまいません。
	 *
	 * 同じファイルを繰り返し再生する場合は、この関数を再度呼び出す必要はありません。
	 *
	 * この関数を呼び出した直後に、もう一度この関数を呼び出すと、前回のファイル情報は新しいファイル情報に上書きされます。
	 * CriMvEasyPlayer::SetData()関数を呼び出した場合は、事前に設定したファイル情報がクリアされます。
	 *
	 * \para 備考1：
	 * この関数はハンドル状態が MVEASY_STATUS_STOP もしくは MVEASY_STATUS_PLAYEND時のみ呼び出し可能です。
	 * またはファイル要求コールバック関数内でもこの関数を呼び出すことができます。詳細は CriMvEasyPlayer::SetFileRequestCallback()関数を
	 * 参照してください。
	 *
	 * \para 備考2：
	 * この関数の内部ではファイルのオープン要求はしません。ファイルのオープン処理は CriMvEasyPlayer::Update() 関数の中で行われます。
	 *
	 * \sa CriMvEasyPlayer::SetFileRequestCallback(), CriMvEasyPlayer::SetData(), CriMvEasyPlayer::SetFile()
	 */
	void SetFileRange(CriChar8 *fname, CriUint64 offset, CriSint64 range, CriError &err=CriMv::ErrorContainer);

	/* 前回のムービデータをもう一度登録する（ファイル要求コールバック関数でのみ呼ぶこと） */
	void SetPreviousDataAgain(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Reads movie header and analyzes movie information
	 * \param       err         Optional error code
	 *
	 * This function opens the movie file, reads the header, and starts to analyze the movie data.
	 * It does not start movie playback.
	 *
	 * When this function is called, the handle status changes to MVEASY_STATUS_DECHDR.  Once EasyPlayer
	 * has finished reading the header and analyzing the movie, the status changes to MVEASY_STATUS_WAIT_PREP.
	 * When the status is MVEASY_STATUS_WAIT_PREP, CriMvEasyPlayer::GetMovieInfo() can be successfully called.
	 * 
	 * To continue playback, call CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start() when the handle status 
	 * is MVEASY_STATUS_WAIT_PREP.
	 *
	 * \remarks
	 * This function can only be called when the handle status is MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND.
	 *
	 * \remarks
	 * You must either call CriMvEasyPlayer::SetFile() or CriMvEasyPlayer::SetData(), or provide a file request 
	 * callback with CriMvEasyPlayer::SetFileRequestCallback(), before calling this function.
	 *
	 * \remarks
	 * Once this function has been called, CriMvEasyPlayer::ExecuteDecode() and CriMvEasyPlayer::Update() must be 
	 * called periodically in order for this function to have any effect.  Otherwise, the handle status will 
	 * never change.
	 *
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::Prepare(), CriMvEasyPlayer::Start(), 
	 *     CriMvEasyPlayer::SetFileRequestCallback(),
	 *     CriMvEasyPlayer::ExecuteDecode(), CriMvEasyPlayer::Update()	 
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		ムービヘッダ解析
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービの再生は開始せず、ヘッダ解析のみ行って待機するための関数です。<br>
	 * この関数を使用してヘッダ解析を事前に済ませることにより、再生開始前にムービの解像度やオーディオの情報を
	 * 得ることができます。<br>
	 * 本関数を呼び出すと、EasyPlayerのハンドル状態はMVEASY_STATUS_STOP → MVEASY_STATUS_DECHDR と遷移していき、
	 * ヘッダ解析が完了するとMVEASY_STATUS_WAIT_PREPとなります。<br>
	 * ムービ情報を取得するには、ハンドル状態がMVEASY_STATUS_WAIT_PREPになったあとに CriMvEasyPlayer::GetMovieInfo()
	 * を実行してください。<br>
	 * 
	 * ハンドル状態がMVEASY_STATUS_WAIT_PREPの時に、 CriMvEasyPlayer::Prepare() か CriMvEasyPlayer::Start() を
	 * 呼ぶことで再生処理を続けることができます。<br>
	 * 
	 * 本関数は EasyPlayerのハンドル状態がMVEASY_STATUS_STOPかMVEASY_STATUS_PLAYENDの時に呼び出してください。
	 * 
	 * 本関数を呼び出す前に CriMvEasyPlayer::SetFile() か CriMvEasyPlayer::SetData() でムービデータを指定してください。<br>
	 * ただし、ファイル要求コールバック関数を登録している場合は事前のムービデータ設定は省略することだきます。
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::Prepare(), CriMvEasyPlayer::Start(), CriMvEasyPlayer::SetFileRequestCallback()
	 */
	void DecodeHeader(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Reads movie header, analyzes movie information, and buffers data.
	 * \param       err         Optional error code
	 *
	 * This function opens the movie file and gets it ready for immediate playback, by reading and analyzing
	 * the header and decoding and buffering video data.  It allows a movie to start playing immediately when
	 * CriMvEasyPlayer::Start() is called, without an initial delay.
	 *
	 * When this function is called, the handle status changes to MVEASY_STATUS_DECHDR.  Once EasyPlayer
	 * has finished reading and analyzing the movie data, the status changes to MVEASY_STATUS_PREP and EasyPlayer
	 * begins decoding video data.  When the initial video data has been decoded and buffered, the status changes to
	 * MVEASY_STATUS_READY.
	 *
	 * The amount of data that is buffered, in seconds, is based on the value set by CriMvEasyPlayer::SetBufferingTime().
	 * By default, this is 1 second.
	 *
	 * When the status is MVEASY_STATUS_READY, movie playback will start imediately when CriMvEasyPlayer::Start()
	 * is called.
	 *
	 * \remarks
	 * Unless you  CriMvEasyPlayer::DecodeHeader() first, this function can only be called when the handle status
	 * is MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND.
	 *
	 * \remarks
	 * You must either call CriMvEasyPlayer::SetFile() or CriMvEasyPlayer::SetData(), or provide a file request 
	 * callback with CriMvEasyPlayer::SetFileRequestCallback(), before calling this function.
	 *
	 * \remarks
	 * Once this function has been called, CriMvEasyPlayer::ExecuteDecode() and CriMvEasyPlayer::Update() must be 
	 * called periodically in order for this function to have any effect.  Otherwise, the handle status will 
	 * never change.
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Start(),
	 *     CriMvEasyPlayer::SetBufferingTime(), CriMvInputBufferInfo
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		再生準備（ヘッダ解析とバッファリング）
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービの再生は開始せず、ヘッダ解析と再生準備のみを行って待機するための関数です。<br>
	 * この関数を使用して再生準備を事前に済ませることにより、ムービ再生開始のタイミングを細かく制御することができます。<br>
	 * （再生準備無しで再生開始関数を呼び出した場合は、実際に再生が始まるまでにタイムラグが発生します。）<br>
	 * 本関数を呼び出すと、EasyPlayerのハンドル状態はMVEASY_STATUS_STOP → MVEASY_STATUS_DECHDR → MVEASY_STATUS_PREP と遷移していき、
	 * 再生準備が完了するとMVEASY_STATUS_READYとなります。
	 * 
	 * ハンドル状態がMVEASY_STATUS_READYの時に、 CriMvEasyPlayer::Start() を呼ぶことで再生を開始することができます。
	 * 
	 * CriMvEasyPlayer::DecodeHeader() の呼び出し無しでこの関数を呼び出す場合は、CriMvEasyPlayerのハンドル状態が
	 * MVEASY_STATUS_STOPかMVEASY_STATUS_PLAYEND でなければいけません。
	 * 
	 * 再生開始前には CriMvEasyPlayer::SetFile() か CriMvEasyPlayer::SetData() でムービデータを指定してください。<br>
	 * ただし、ファイル要求コールバック関数を登録している場合は事前のムービデータ設定は省略することだきます。
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Start(),
	 *     CriMvEasyPlayer::SetBufferingTime(), CriMvInputBufferInfo
	 */
	void Prepare(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Starts movie playback
	 * \param       err         Optional error code
	 *
	 * Opens the movie and starts playback.  If CriMvEasyPlayer::Prepare() was not called, there will be a
	 * delay while the library reads the movie header and buffers the initial data.  If CriMvEasyPlayer::Prepare()
	 * was called and the handle status is MVEASY_STATUS_READY, playback will start as soon as this function is called.
	 *
	 * \remarks
	 * If you call this function without calling CriMvEasyPlayer::DecodeHeader() or CriMvEasyPlayer::Prepare() 
	 * first, this function can only be called when the handle status is MVEASY_STATUS_STOP, MVEASY_STATUS_PLAYEND,
	 * or MVEASY_STATUS_READY.
	 *
	 * \remarks
	 * You must either call CriMvEasyPlayer::SetFile() or CriMvEasyPlayer::SetData(), or provide a file request 
	 * callback with CriMvEasyPlayer::SetFileRequestCallback(), before calling this function.
	 *
	 * \remarks
	 * Once this function has been called, CriMvEasyPlayer::ExecuteDecode() and CriMvEasyPlayer::Update() must be 
	 * called periodically in order for this function to have any effect.  Otherwise, the movie will not play and
	 * the handle status will never change.
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Prepare()
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		再生開始
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービの再生を開始します。<br>
	 * CriMvEasyPlayer::Prepare()を呼ばずに、本関数を呼び出した場合は、ムービの解析と再生の準備を行うため、
	 * 実際にムービの再生が始まるまでにタイムラグが発生します。<br>
	 * CriMvEasyPlayer::Prepare()を先に呼び出して、ハンドル状態がMVEASY_STATUS_READYになっていれば、
	 * この関数を呼び出してすぐに再生が始まります。
	 * 
	 * CriMvEasyPlayer::DecodeHeader() または CriMvEasyPlayer::Prepare() の呼び出し無しでこの関数を呼び出す場合は、
	 * CriMvEasyPlayerのハンドル状態が MVEASY_STATUS_STOPかMVEASY_STATUS_PLAYEND でなければいけません。
	 * 
	 * 再生開始前には CriMvEasyPlayer::SetFile() か CriMvEasyPlayer::SetData() でムービデータを指定してください。<br>
	 * ただし、ファイル要求コールバック関数を登録している場合は事前のムービデータ設定は省略することだきます。
	 * 
	 * \sa CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::Prepare()
	 */
	void Start(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Stops movie playback; resets a handle that is in an ERROR state,
	 * \param       err         Optional error code
	 *
	 * Tells the movie to stop playing and returns immediately.  Note that this does not actually stop playback;
	 * an application must continue to call CriMvEasyPlayer::Update() and CriMvEasyPlayer::ExecuteDecode() until
	 * the status changes to MVEASY_STATUS_STOP.
	 *
	 * Once the handle is in the MVEASY_STATUS_STOP state, it can be reused for a different movie.
	 *
	 * If the handle state is MVEASY_STATUS_ERROR, an application must call this function and wait for the status
	 * to change to MVEASY_STATUS_STOP before destroying or reusing the player handle.
	 *
	 * In principle, this function does not reset any EasyPlayer handle parameters that were explicitly set via
	 * any of the settings APIs, so an application can replay the same movie by simply calling CriMvEasyPlayer::Start()
	 * once the state has changed to MVEASY_STATUS_STOP.  However, calling this function will reset the following
	 * parameters:
	 *
	 *  - The pause state (see CriMvEasyPlayer::Pause()) will be reset to OFF (not paused).
	 *  - If a file request callback has been set (via CriMvEasyPlayer::SetFileRequestCallback()), the movie
	 *    data information will be reset.
	 *
	 * See the description of CriMvEasyPlayer::ResetAllParameters() for a comparison of the parameters that are 
	 * affected by that function and by this function.
	 *
	 * \remarks
	 * This function may call CriMvSoundInterface::Stop() and CriMvFileReaderInterface::Close().  For an EasyPlayer 
	 * handle to change to MVEASY_STATUS_STOP, the sound and file interfaces need to transition to their STOP states.
	 * In the case of CriMvSoundInterface, this means that CriMvSoundInterface::GetStatus() will return 
	 * MVEASY_SOUND_STATUS_STOP.  For CriMvFileReaderInterface, this means that CriMvFileReaderInterface::GetCloseStatus()
	 * will return ASYNC_STATUS_COMPLETE.
	 *
	 * \sa CriMvEasyPlayer::Status, CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::ResetAllParameters()
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		再生停止／エラー状態からの復帰
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービ再生停止の要求を出します。本関数は即時復帰関数です。本関数内で全ての停止処理が実行されるわけではありません。<br>
	 * 本関数呼出し後、再生状態が MVEASY_STATUS_STOP なるまでは通常のメインループ処理を動かしてください。<br>
	 * 具体的には CriMvEasyPlayer::Update(), CriMvEasyPlayer::ExecuteDecode() が通常通り呼び出される必要があります。
	 * 
	 * 再生状態が MVEASY_STATUS_ERROR になった場合は、本関数を呼び出して MVEASY_STATUS_STOP を待ってください。<BR>
	 * 
	 * forループなどによるローカルループで状態変更待ちをしても MVEASY_STATUS_STOP にはなりません。<br>
	 * 
	 * 本関数を呼び出しても、アプリケーションが再生ハンドルに設定した各種パラメータは原則としてリセットされません。<BR>
	 * MVEASY_STATUS_STOP 状態になったあと、もう一度再生を開始すると前回と同じパラメータで再生を行うことができます。<BR>
	 * 例外的に本関数でリセットされるパラメータは以下のものがあります。
	 * - CriMvEasyPlayer::Pause() によるポーズ状態は、OFFにリセットされます。
	 * - ファイル要求コールバック関数の登録がある場合、ムービファイル名（またはメモリ）の情報はリセットされます。
	 * 
	 * リセットされるパラメータ一覧は CriMvEasyPlayer::ResetAllParameters() の説明を参照してください。
	 * 
	 * 本関数は必要に応じて CriMvSoundInterface::Stop() および CriMvFileReaderInterface::Close() を呼び出します。<br>
	 * EasyPlayer ハンドルが MVEASY_STATUS_STOP 状態になるためには、各インタフェースが停止状態にならなければいけません。<br>
	 * サウンドインタフェースの場合、 CriMvSoundInterface::GetStatus()が MVEASY_SOUND_STATUS_STOP を返すこと。<br>
	 * ファイル読み込みインタフェースの場合、 CriMvFileReaderInterface::GetCloseStatus() が、ASYNC_STATUS_COMPLETE 
	 * を返さなければいけません。
	 * 
	 * \sa CriMvEasyPlayer::Status, CriMvEasyPlayer::GetStatus(), CriMvEasyPlayer::ResetAllParameters()
	 */
	void Stop(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup     MODULE_CONTROL
	 *
	 * \brief       Pauses or resumes movie playback
	 * \param       sw          Pause state.  ON (1) will pause playback, OFF (0) will resume it.
	 * \param       err         Optional error code
	 *
	 * Pauses or resumes movie playback, depending on the argument.  If \a sw is ON (1), playback will be paused.
	 * If \a sw is OFF (0), playback will be resumed.
	 *
	 * \remarks
	 * This function will call CriMvSoundInterface::Pause() and CriMvSystemTimerInterface::Pause() with the
	 * provided argument.
	 *
	 * \remarks
	 * Calling  CriMvEasyPlayer::Stop() or CriMvEasyPlayer::ResetAllParameters() will set the pause state to OFF.
	 *
	 * \sa CriMvSoundInterface::Pause(), CriMvSystemTimerInterface::Pause() 
	 */
	/*JP
	 * \ingroup MODULE_CONTROL
	 * \brief		再生の一時停止または再開
	 *  \param		sw		ポーズスイッチ。ポーズONの場合は1、ポーズOFF(レジューム)の場合は0を指定します。
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 本関数の動作は引数に依存します。<br>
	 * 引数 sw がON(1)なら、一時停止。引数 sw がOFF(0)なら再生再開です。
	 * 
	 * CriMvEasyPlayer::Stop() または CriMvEasyPlayer::ResetAllParameters を呼び出すとポーズ状態はOFFにリセットされます。
	 * 
	 * この関数は CriMvSoundInterface::Pause() と CriMvSystemTimerInterface::Pause() を同じ引数で呼び出します。
	 * 
	 * \sa CriMvSoundInterface::Pause(), CriMvSystemTimerInterface::Pause() 
	 */
	void Pause(CriBool sw, CriError &err=CriMv::ErrorContainer);

	CriBool IsPaused(CriError &err=CriMv::ErrorContainer);

	/* オプション設定／取得 */
	/*EN
	 * \ingroup MODULE_OPTION
	 * 
	 * \brief       Sets the type of timer used for video synchronization
	 * \param       type        Type of timer to use
	 * \param       err         Optional error code
	 *
	 * In order to display video frames at the proper rate, CRI Movie uses a timer to determine when the
	 * next frame should be shown.  For a movie with an audio track, you would typically use the
	 * MVEASY_TIMER_AUDIO timer type.  For a movie with no audio, the MVEASY_TIMER_SYSTEM type timer
	 * should be used.
	 *
	 * The default is the timer type that was passed to CriMvEasyPlayer::Create().  If this value
	 * was NULL, a system timer (MVEASY_TIMER_SYSTEM) will be used.
	 *
	 * \remarks
	 * If an audio timer (MVEASY_TIMER_AUDIO) is wanted, it must be created and passed to
	 * CriMvEasyPlayer::Create().
	 *
	 * \remarks
	 * If the movie does not have an audio track, CRI Movie will use a MVEASY_TIMER_SYSTEM regardless of
	 * the value set by this function.
	 *
	 * \sa CriMvEasyPlayer::GetMasterTimer(), CriMvEasyPlayer::Create(), TimerType
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		マスタタイマ種別の指定
	 *  \param		type	マスタタイマ種別
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ビデオフレームの時刻管理に使用するタイマ種別を指定します。<br>
	 * デフォルトはハンドル作成時に指定するシステムタイマです。<br>
	 * ビデオフレームの表示タイミングをオーディオの時刻と同期させたいときはオーディオタイマを指定してください。<br>
	 * オーディオタイマを指定した場合でも、再生するムービにオーディオが含まれていない場合はシステムタイマ同期となります。
	 * 
	 * \sa CriMvEasyPlayer::GetMasterTimer(), CriMvEasyPlayer::Create()
	 */
	void SetMasterTimer(TimerType type, CriError &err=CriMv::ErrorContainer);		// default is SYSTEM

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Returns the type of timer currently being used by the EasyPlayer handle
	 * \param       err         Optional error code
	 * \return      The current timer type
	 *
	 * Returns the current type of timer used to synchronize video frames.  This value can be changed
	 * by calling CriMvEasyPlayer::SetMasterTimer().  Otherwise, the timer type is set when the handle 
	 * is created.
	 *
	 * \sa CriMvEasyPlayer::SetMasterTimer(), CriMvEasyPlayer::Create(), TimerType
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		マスタタイマ種別の取得
	 *  \param		err		エラー情報（省略可）
	 *  \return 	現在設定されているマスタタイマ種別
	 * 
	 * 現在設定されているマスタタイマ種別を取得します。
	 * 
	 * \sa CriMvEasyPlayer::SetMasterTimer()
	 */
	TimerType GetMasterTimer(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the number of internal video buffers
	 * \param       npools      The number of buffers to use; must be greater than 0
	 * \param       err         Optional error code
	 *
	 * CRI Movie uses internal memory, or frame pools, to buffer decoded frames before display.
	 * More frame pools can help smooth out playback under high CPU loads.
	 *
	 * \remarks
	 * By default, the number of pools is 1.  To change the value, this function must be called
	 * before starting playback (with either CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start()).
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		内部ビデオバッファ（フレームプール）数の指定
	 *  \param		npools		内部ビデオバッファ数（最低でも１）
	 *  \param		err		エラー情報（省略可）
	 * 
	 * EasyPlayerハンドル内部のビデオバッファ数を指定します。<br>
	 * この内部ビデオバッファはデコード結果を蓄えておくためのもので、フレームプールと呼びます。<br>
	 * フレームプールが多いほど先行してビデオデコードを進めることができるため、デコードの
	 * 負荷変動が大きかったり、デコードに使用できるCPU時間の変動が大きい場合にもスムーズな再生を
	 * 行いやすくなります。<br>
	 * デフォルトのフレームプール数は１です。<br>
	 * フレームプール数を変更したい場合は、再生開始前( CriMvEasyPlayer::Prepare()または CriMvEasyPlayer::Start())に
	 * 本関数を実行してください。
	 */
	void SetNumberOfFramePools(CriUint32 npools, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the amount of movie data that will be buffered, in seconds
	 * \param       sec         Buffering time, in seconds
	 * \param       err         Optional error code
	 *
	 * CRI Movie buffers enough raw data from disk to allow for smooth playback and to reduce disk reads.
	 * The buffer size is based on the bitrate of the movie, and other movie parameters.
	 *
	 * By default, this buffer will be large enough to hold 1 second worth of playback.
	 *
	 * To determine the current buffering time, look at the \a buffering_time field of the
	 * CriMvStreamingParameters structure, which is retrieved by calling CriMvEasyPlayer::GetMovieInfo().
	 *
	 * \remarks
	 * If this function is called, it must be called before starting playback (with either 
	 * CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start()).
	 *
	 * \remarks
	 * Passing \a 0.0 as the value of \a sec will reset the buffering time to the default value.
	 *
	 * \remarks
	 * If an application calls CriMvEasyPlayer::SetStreamingParameters() for a handle, this function
	 * can not be used with that handle.
	 *
	 * \remarks
	 * The value set by this function, along with the value set by CriMvEasyPlayer::SetReloadThresholdTime(),
	 * determine how often data is read from disk.  See the description of 
	 * CriMvEasyPlayer::SetReloadThresholdTime() for details.
	 *
	 * \sa CriMvEasyPlayer::SetReloadThresholdTime(), CriMvEasyPlayer::GetInputBufferInfo(),
	 *     CriMvStreamingParameters
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		入力データのバッファリング時間の指定
	 *  \param		sec		バッファリング時間。単位は秒。
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ストリーミング再生でバッファリングする入力データの量を秒単位の時間で指定します。<br>
	 * EasyPlayerは、バッファリング時間とムービのビットレート等から読み込みバッファのサイズを決定します。
	 * 
	 * デフォルトのバッファリング時間は、再生開始時点でアプリケーションが作成済みのEasyPlayerハンドル数
	 * に依存して決まります。EasyPlayerハンドル１つにつき１秒のバッファリング時間を確保します。もしもアプリ
	 * ケーションが３つのEasyPlayerハンドルを作成していた場合、バッファリング時間は３秒となります。
	 * 
	 * EasyPlayerハンドルが何秒分のバッファリング時間になっているかは CriMvEasyPlayer::GetMovieInfo 
	 * 関数で取得する CriMvStreamingParameters 構造体の変数 buffering_time で確認できます。
	 * 
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Prepare 関数または CriMvEasyPlayer::Start 関数の前までに実行してください。
	 * 
	 * バッファリング時間に 0.0f を指定した場合、バッファリング時間はライブラリのデフォルト値となります。<br>
	 * また、アプリケーションが CriMvEasyPlayer::SetStreamingParameters 関数を呼び出した場合は本関数で
	 * 設定した値よりも、 CriMvEasyPlayer::SetStreamingParameters 関数の指定が優先されます。
	 * 
	 * \sa CriMvEasyPlayer::SetReloadThresholdTime(), CriMvEasyPlayer::GetInputBufferInfo()
	 */
	void SetBufferingTime(CriFloat32 sec, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Determines how often the movie data buffer is refilled from disk
	 * \param       sec         Number of seconds of playback time to buffer
	 * \param       err         Optional error code
	 *
	 * EasyPlayer buffers raw data from disk to allow for smooth playback.  How often it refills 
	 * its buffers is determined by this function.  When the amount of data remaining, in seconds,
	 * falls below this value, EasyPlayer will call into the FileReader module for this handle in 
	 * order to read more data.
	 *
	 * SetBufferingTime() sets a "low water mark" for the data buffer.  For instance, if an application
	 * sets the buffer size to 4 seconds (with CriMvEasyPlayer::SetBufferingTime()), and sets the reload
	 * threshold to 1 second, then CRI Movie will initially fill the buffer with 4 seconds worth of 
	 * data.  After 3 seconds worth of data have been decoded and consumed, there will be less than 
	 * \a reload \a threshold seconds of data remaining, and CRI Movie will refill the buffer.
	 * 
	 * \remarks
	 * The default value for \a sec is 0.8s.
	 *
	 * \remarks
	 * If this function is called, it must be called before starting playback (with either 
	 * CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start()).
	 *
	 * \sa CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::GetInputBufferInfo(), CriMvFileReaderInterface
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		再読み込み閾値の時間指定
	 *  \param		sec		時間指定による再読み込み閾値。単位は秒。
	 *  \param		err		エラー情報（省略可）
	 * 
	 * EasyPlayerハンドルは、入力バッファ内のデータが再読み込み閾値以下になった時に次のデータ読み込みを実行します。
	 * 再読み込み閾値は本関数による指定時間とムービデータのビットレートによって自動的に計算されます。
	 * 再読み込み閾値は時間[秒]で指定します。デフォルト値は0.8秒です。
	 * 
	 * ムービ再生中にデータを裏読みする場合などにシーク回数を減らすために閾値設定を利用することができます。
	 * 例えば、バッファリング時間を2秒、再読み込み閾値を1秒に設定すると、ムービデータの読み込みは約1秒に1回の実行になります。
	 * こうすることで、約1秒の間はデータの読み込みを連続的に行うことができます。
	 * 
	 * ムービを再生しながらユーザデータの読み込みを行う場合、ユーザデータの読み込みは本関数で指定した時間以内に読み込み
	 * 処理が終わるようにしてください。サイズの大きなデータは複数に分割して読み込むなどの対処が必要になります。
	 * 本関数で指定した時間以内にユーザデータの読み込みが終わらなかった場合、ムービデータが枯渇してムービ再生が滞ります。
	 * 
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Prepare 関数または CriMvEasyPlayer::Start 関数の前までに実行してください。
	 * 
	 * ムービ再生中の入力バッファのデータ量や再読み込み閾値のサイズは、CriMvEasyPlayer::GetInputBufferInfo で取得可能です。
	 * 
	 * \sa CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::GetInputBufferInfo()
	 */
	void SetReloadThresholdTime(CriFloat32 sec, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Retrieves input data buffering settings
	 * \param       ibuf_info       Input buffer information structure
	 * \param       err             Optional error code
	 *
	 * Populates the passed CriMvInputBufferInfo structure with the values of the settings
	 * for the raw input buffer size, the reload interval, and the amount of data currently buffered.
	 *
	 * \remarks
	 * This function can be called once the handle status has transitioned to MVEASY_STATUS_WAIT_PREP.
	 *
	 * \sa CriMvInputBufferInfo, CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::SetReloadThresholdTime()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		入力バッファ情報の取得
	 *  \param		ibuf_info	入力バッファ情報
	 *  \param		err			エラー情報（省略可）
	 * 
	 * 入力バッファ情報 CriMvInputBufferInfo を取得します。<br>
	 * 入力バッファ情報はEasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあと取得できます。<br>
	 * 
	 * \sa CriMvInputBufferInfo, CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::SetReloadThresholdTime()
	 */
	void GetInputBufferInfo(CriMvInputBufferInfo &ibuf_info, CriError &err=CriMv::ErrorContainer);

	/*EN
	 *
	 * \brief       Sets the maximum bitrate EasyPlayer will assume for movie data
	 * \param       max_bitrate     Maximum bitrate, in bits/second
	 * \param       err             Optional error code
	 *
	 * EasyPlayer determines the size of its input data buffer by the movie's bitrate, the buffering time,
	 * and other movie parameters.  In normal usage, an application should not need to call this function.
	 * However, it can be useful when doing concatenated playback of several movies sequentially.
	 *
	 * If the bitrate of the first movie is smaller or larger than the next movie, EasyPlayer can choose
	 * a buffer size that will be appropriate for the first movie, but either too small for the next 
	 * (causing excessive disk reads or playback stuttering) or too large (using more memory than necessary).
	 * 
	 * Passing a value of 0 for \a max_bitrate will cause the handle to revert to its default behavior
	 * for determining maximum bitrate.
	 *
	 * \remarks
	 * The value set by this function will not be reflected in the \a max_bitrate field of the 
	 * CriMvStreamingParameters structure, which will contain the actual value as stored in the movie's
	 * header.
	 *
	 * \remarks
	 * If this function is called, it must be called before starting playback (with either 
	 * CriMvEasyPlayer::Prepare() or CriMvEasyPlayer::Start()).
	 *
	 * \remarks
	 * If an application calls CriMvEasyPlayer::SetStreamingParameters() for a handle, this function
	 * can not be used with that handle.
	 *
	 * \remarks
	 * For details about concatenated playback, see the description of CriMvEasyPlayer::SetFileRequestCallback().
	 *
	 * \sa CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::SetReloadThresholdTime(), 
	 *     CriMvEasyPlayer::SetFileRequestCallback()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		最大ビットレートの指定
	 *  \param		max_bitrate		最大ビットレート(bit per second)
	 *  \param		err				エラー情報(省略可)
	 * 
	 * ムービデータの最大ビットレートを指定します。最大ビットレートはストリーム再生用に確保するバッファサイズに影響します。<br>
	 * 
	 * 単純再生時は本関数を呼び出す必要はありません。EasyPlayerハンドルが自動的に最大ビットレートを取得して必要なだけの
	 * 読み込みバッファを確保します。<br>
	 * 
	 * 連結再生時に、先頭のムービファイルのビットレートが後続のムービファイルと比べて極端に小さい場合には、本関数を使用して
	 * 明示的に最大ビットレートを大きく指定してください。<br>
	 * 
	 * 本関数で設定した最大ビットレートは、CriMvEasyPlayer::GetMovieInfo 関数で取得するムービ情報には反映されません。
	 * CriMvEasyPlayer::GetMovieInfo 関数で取得できるのはムービデータの本来の情報です。<br>
	 * 
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Prepare 関数または CriMvEasyPlayer::Start 関数の前までに実行してください。<br>
	 * 
	 * 最大ビットレートに 0を指定した場合、最大ビットレートはムービデータの持つ値となります。<br>
	 * また、アプリケーションが CriMvEasyPlayer::SetStreamingParameters 関数を呼び出した場合は本関数で
	 * 設定した値よりも、 CriMvEasyPlayer::SetStreamingParameters 関数の指定が優先されます。
	 *
	 * \sa CriMvEasyPlayer::SetBufferingTime(), CriMvEasyPlayer::SetReloadThresholdTime(), 
	 *     CriMvEasyPlayer::SetFileRequestCallback()	
	*/
	void SetMaxBitrate(CriUint32 max_bitrate, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the audio playback track
	 * \param       track       Audio track number
	 * \param       err         Optional error code
	 *
	 * If a movie has multiple audio tracks (for instance, English and Spanish versions), this function
	 * will determine which track plays.  By default, the first audio track is used.
	 *
	 * To determine the number of audio tracks in the movie, call CriMvEasyPlayer::GetMovieInfo() and 
	 * look at the \a num_audio field of the CriMvStreamingParameters structure.
	 *
	 * If the movie does not have any audio, this function has no effect.
	 *
	 * \remarks
	 * To use the default setting, set \a track to CRIMV_AUDIO_TRACK_AUTO.
	 *
	 * \remarks
	 * To turn off audio altogether, set \a track to CRIMV_AUDIO_TRACK_OFF.
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		再生するオーディオトラックの指定
	 *  \param		track	再生するオーディオトラック
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービが複数のオーディオトラックを持っている場合に、再生するオーディオを指定します。<br>
	 * 再生開始前( CriMvEasyPlayer::Prepare()または CriMvEasyPlayer::Start())に本関数を実行してください。
	 * 
	 * 本関数を実行しなかった場合は、もっとも若い番号のオーディオトラックを再生します。<br>
	 * CriMvEasyPlayer::DecodeHeader()と CriMvEasyPlayer::GetMovieInfo()を使うことで、どのチャネルに
	 * どんなオーディオが入っているかを再生開始前に知ることができます。
	 * 
	 * データが存在しないトラック番号を指定した場合は、オーディオは再生されません。
	 * 
	 * トラック番号としてCRIMV_AUDIO_TRACK_OFFを指定すると、例えムービにオーディオが含まれていたと
	 * してもオーディオは再生しません。
	 * 
	 * また、デフォルト設定（もっとも若いチャネルのオーディオを再生する）にしたい場合は、
	 * チャネルとしてCRIMV_AUDIO_TRACK_AUTOを指定してください。
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::DecodeHeader()
	 */
	void SetAudioTrack(CriSint32 track, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Turns movie looping on or off
	 * \param       sw          Looping value
	 * \param       err         Optional error code
	 *
	 * If \a sw is 1 (ON), the movie will loop continuously.
	 * If \a sw is 0 (OFF), the movie will play normally.
	 *
	 * In normal usage, a movie will play once, with the handle status changing to MVEASY_STATUS_PLAYEND
	 * when it completes.  When looping is ON, this behavior changes.  When the movie reaches the
	 * end, it will immediately start playing again from the beginning, and the status will continue to
	 * toggle between MVEASY_STATUS_PLAYING and MVEASY_STATUS_PREP.
	 *
	 * If looping is ON, and the playback is from a file, EasyPlayer will call CriMvFileReaderInterface::Seek()
	 * as necessary to reset the file pointer.
	 *
	 * \remarks
	 * If looping is ON, and an application sets it to OFF while the movie is playing, playback might
	 * not stop at the end of the movie.  In that case, playback will end after the next loop iteration.
	 *
	 * \sa CriMvEasyPlayer::GetLoopFlag(), CriMvFileReaderInterface::Seek()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ループ再生フラグの指定
	 *  \param		sw		ループスイッチ。ONの場合はループあり、OFFの場合はループ無しになります。
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ループ再生の有無を設定します。デフォルトはループOFFです。<br>
	 * ループ再生ONにした場合は、ムービの終端まで再生してもハンドル状態はMVEASY_STATUS_PLAYENDにならず、
	 * ムービの先頭から再生を繰り返します。<br>
	 * ファイル名指定で再生している場合は、最後まで読み込んだあと CriMvFileReaderInterface::Seek()を使って
	 * 読み込み位置をファイルの先頭に戻します。
	 * 
	 * ループ再生OFFに設定した場合は、そのとき読み込んでいたムービの終端まで再生すると、
	 * ハンドル状態がMVEASY_STATUS_PLAYENDに遷移します。<br>
	 * 再生中にループOFFにした場合、タイミングによっては、再生中のムービ終端で終わらず、次の繰り返し
	 * 再生まで実行されます。
	 * 
	 * 現在のループ設定を取得するには CriMvEasyPlayer::GetLoopFlag()を使ってください。
	 *
	 * \sa CriMvEasyPlayer::GetLoopFlag(), CriMvFileReaderInterface::Seek()
	 */
	void SetLoopFlag(CriBool sw, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Retrieves the value of the looping flag
	 * \return      The current looping setting
	 *
	 * By default, looping is OFF and playback will stop when it reaches the end.  You can 
	 * change this behavior by calling CriMvEasyPlayer::SetLoopFlag().
	 *
	 * \sa CriMvEasyPlayer::SetLoopFlag()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ループ再生フラグの取得
	 *  \param		err		エラー情報（省略可）
	 *  \return		現在のループ再生設定
	 * 
	 * 現在のループ設定を取得します。
	 * ループ設定は CriMvEasyPlayer::SetLoopFlag() で変更することができます。
	 * 
	 * \sa CriMvEasyPlayer::SetLoopFlag()
	 */
	CriBool GetLoopFlag(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Gets the amount of time that a movie has been playing
	 * \param       count       Number of timer units since the movie started playing
	 * \param       unit        Number of timer units per second
	 * \param       err         Optional error code
	 *
	 * This function retrieves the absolute time that has elapsed since a movie started playing.
	 * The time value is returned in two parts - a counter with an arbitrary interval, and the 
	 * number of timer ticks per second.  To determine the playing time in seconds, divide
	 * \a count by \a unit.
	 *
	 * Before playback has started, and after it has stopped, this function will return a \a count value of 0.
	 *
	 * The value retrieved is the value of the master timer for the handle, not the time of the current frame itself.
	 * To get the video frame time, check the CriMvFrameInfo structure once you have retrieved the frame.
	 *
	 * \remarks
	 * Note that this function provides an absolute playback time - it does <b>not</b> wrap to 0
	 * when the movie loops.
	 *
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		再生時刻の取得
	 *  \param	count	タイマカウント
	 *  \param	unit	１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 *  \param		err		エラー情報（省略可）
	 * 
	 * タイマ時刻を取得します。時刻はcountとunitの二つの変数で表現します。<br>
	 * count ÷ unit で秒単位の時刻となるような値を返します。<br>
	 * 再生開始前（ CriMvSoundInterface::Start()呼び出し前）および
	 * 再生停止後（ CriMvSoundInterface::Stop()呼び出し後）は、時刻０（タイマカウントが０）を返します。<br>
	 * 本関数はマスタタイマで指定されたタイマの時刻を返すだけで、ビデオフレームの時刻を返すものではありません。<br>
	 * 取得したビデオフレームの本来の表示時刻は、ビデオフレーム取得時の CriMvFrameInfo 構造体を参照してください。
	 */
	void GetTime(CriUint64 &count, CriUint64 &unit, CriError &err=CriMv::ErrorContainer);	// only refer time of SyncMasterTimer

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Gets information about the movie
	 * \param       stmprm      Movie information structure
	 * \param       err         Optional error code
	 * \return      TRUE        if movie information was successfully retrieved
	 * \return      FALSE       if the header has not yet been decoded, or if an error occurred
	 *
	 * Populates the passed \a CriMvStreamingParameters structure with information about the current movie,
	 * including the bitrate, resolution, audio track information, subtitle availability, and more.
	 *
	 * This function is available once the handle status has changed to MVEASY_STATUS_WAIT_PREP.  If an application
	 * needs this information before starting playback  (for instance, to set up for playing subtitles, or to
	 * allocate a display surface based on the size of the movie), call CriMvEasyPlayer::DecodeHeader(), then
	 * call GetMovieInfo().
	 *
	 * \remarks
	 * When doing concatenated playback (via CriMvEasyPlayer::SetFileRequestCallback()), GetMovieInfo() will 
	 * return information about the currently playing movie.
	 *
	 * \sa CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::SetFileRequestCallback()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ムービ情報の取得
	 *  \param		stmprm		ムービ情報
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービ情報 CriMvStreamingParameters を取得します。<br>
	 * ムービ情報からは主にビットレートや解像度、オーディオ数などがわかります。<br>
	 * ムービ情報はEasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあと取得できます。<br>
	 * 再生開始前にムービ情報を知りたい場合は、 CriMvEasyPlayer::DecodeHeader()を呼び出してヘッダ解析を行ってください。
	 * 
	 * 連結再生を行った場合、最後に取得したフレームを含むムービファイルについての情報を返します。
	 * 
	 * \sa CriMvEasyPlayer::DecodeHeader()
	 */
	CriBool GetMovieInfo(CriMvStreamingParameters &stmprm, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Changes the parameters for the movie
	 * \param       stmprm      Movie information structure
	 * \param       err         Optional error code
	 *
	 * This is a DEBUG function and should not normally be used by applications.
	 *
	 * This function allows an application to change the streaming parameters for the movie as a whole. 
	 * It is available once the handle status has changed to MVEASY_STATUS_WAIT_PREP.
	 *
	 * To use this function, first call CriMvEasyPlayer::DecodeHeader(), then call CriMvEasyPlayer::GetMovieInfo()
	 * to retrieve the current movie parameters.  Change the fields of the CriMvStreamingParameters structure
	 * as appropriate, then call SetStreamingParameters().
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::DecodeHeader()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ストリーミングパラメータの変更
	 *  \param		stmprm		ストリーミングパラメータ
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 本関数は通常、アプリケーションからは使用しません。デバッグ用の関数です。
	 * 
	 * ムービ再生のためのストリーミングパラメータをEasyPlayerハンドルに設定します。<br>
	 * ストリーミングパラメータが指定できるのは、EasyPlayerハンドル状態がMVEASY_STATUS_WAIT_PREPの時だけです。<br>
	 * この関数は、読み込みバッファサイズなど細かなパラメータを全てアプリケーションで調整したい場合に使います。<br>
	 * CriMvEasyPlayer::DecodeHeader()でヘッダ解析を行ったあと、 CriMvEasyPlayer::GetMovieInfo()で取得できる
	 * ムービ情報がそのままストリーミングパラメータとなりますので、調整したい値を変更して、本関数で設定しなお
	 * してください。
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::DecodeHeader()
	 */
	void SetStreamingParameters(CriMvStreamingParameters *stmprm, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Determine if a new video frame can be displayed
	 *
	 * \param       err         Optional error code
	 *
	 * \return      TRUE        if a frame is ready to be displayed
	 * \return      FALSE       if the movie is paused or not playing, or if a new frame is
	 *                          not yet decoded or ready to be displayed
	 *
	 * Each frame in a movie corresponds to an absolute time, with the first frame being time 0.
	 * The playback time is controlled by the master timer for this handle.
	 * If the time of the next frame is less than or equal to the current playback time, as determined 
	 * by CriMvEasyPlayer::GetTime(), it is ready to be displayed, and this function will return TRUE.
	 *
	 * If an application needs to do some complex or lengthy processing before displaying a frame (such
	 * as locking a texture), it should call this function first.
	 *
	 * \remarks
	 * If the next frame has not yet been decoded, this function will return FALSE regardless of the playback time.
	 *
	 * \sa CriMvEasyPlayer::GetTime()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		次フレームの表示時刻判定
	 *  \param		err		エラー情報（省略可）
	 *  \return 	次のビデオフレームがすでに表示時刻になっている場合はTRUE(1)を返します。<br>
	 * 				次のビデオフレームがまだデコードできない場合はFALSE(0)を返します。
	 * 
	 * 次のビデオフレームがすでに表示時刻になっているかどうかを問い合わせます。<br>
	 * もしもデコードが遅れていて次のビデオフレームがまだデコードできていない場合は、再生時刻に関係
	 * なくFALSEを返します。<br>
	 * つまりこの関数は「次のフレームが GetFrameOnTime関数で取得できるかどうか」を調べます。<br>
	 * ビデオフレームが実際に取得するよりも先にやらなければいけない処理（例えばテクスチャロックなど）
	 * がある場合は、この関数でフレーム取得の成否を判定してから処理してください。
	 *
	 * \sa CriMvEasyPlayer::GetTime()
	 */
	CriBool IsNextFrameOnTime(CriError &err=CriMv::ErrorContainer);

#if !defined(XPT_TGT_EE)
	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Loads video frame data into a buffer, in 32 bit ARGB format
	 * \param       imagebuf        Pointer to output buffer
	 * \param       pitch           Pitch of output buffer, in bytes
	 * \param       bufsize         Size of output buffer, in bytes
	 * \param       frameinfo       Video frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was retrieved
	 * \return      FALSE           otherwise
	 *
	 * Copies the next decoded frame into the 32 bit ARGB image buffer pointed to by \a imagebuf.
	 * If the display time is less than the frame time, or if the frame has not yet been decoded, the
	 * \a frameinfo parameter will be cleared and this function will return FALSE.
	 *
	 * The \a pitch parameter is the width, in bytes, of each row of the frame image, including any padding.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 *
	 * CRI Movie decodes video frames into YUV420 format internally, and stores them in frame pools (see
	 * CriMvEasyPlayer::SetNumberOfFramePools()) for later display.  When this function is called, 
	 * the frame must first converted to ARGB format.  This can be a very CPU-intensive operation,
	 * especially on the PS3 and Xbox360.  For 1280 x 720 video resolution, this can use almost an
	 * entire vsync interval.  On these platforms, we recommend implementing a pixel shader and calling
	 * CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers() instead.
	 *
	 * \remarks <BR>
	 * If an application uses this function, an application need to call CriMv::InitializeFrame32bitARGB() 
	 * after CriMv::Initialize(). If an application calls this function without CriMv::InitializeFrame32bitARGB() 
	 * calling, this function failed and an error callback occurs.
	 * 
	 * \remarks
	 * This function is not available on the PS2.
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime(), CriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers(),
	 *     CriMvEasyPlayer::SetNumberOfFramePools()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		32bit ARGBフォーマットでのデコード結果の取得
	 *  \param		imagebuf		出力バッファポインタ
	 *  \param		pitch			出力バッファのピッチ [byte]
	 *  \param		bufsize			出力バッファのサイズ [byte]
	 *  \param		frameinfo		取得したビデオフレームの情報構造体
	 *  \param		err		エラー情報（省略可）
	 *  \return		フレームが取得できた場合はTRUE(1)、できなかった場合はFALSE(0)を返します。
	 * 
	 * 32bit ARGBフォーマットで、表示時刻になっているビデオフレームを取得します。<br>
	 * この関数を呼び出す場合は、ARGBバッファの実体を確保したうえで呼び出す必要があります。<br>
	 * ビデオフレームは引数imagebufで指定したARGBバッファに書き出されます。<br>
	 * もしも次のビデオフレームの表示時刻になっていなかったり、デコードが終わっていなかった場合は
	 * フレーム取得できず、frameinfoの中身はクリアされます。<br>
	 * 事前にビデオフレームが取得できるかどうかを知りたい場合は CriMvEasyPlayer::IsNextFrameOnTime()
	 * を使用してください。
	 * 
	 * 32bit ARGB の実際のピクセルデータの並びについては、そのプラットフォームで最も標準的な
	 * フォーマットになります。
	 * 
	 * 注意: <BR>
	 * 本関数を使用する場合はフレーム変換の初期化 CriMv::InitializeFrame32bitARGB()の呼び出しが
	 * 事前に必要です。フレーム変換の初期化を行わずに本関数を呼び出した場合はフレーム取得に失敗し、
	 * エラーコールバックが発生します。
	 * 
	 * 注意: <BR>
	 * PS3, Xbox360 でも本関数は使用できますが、とてもCPU負荷の高い関数となります。<BR>
	 * 解像度が 1280x720 のムービを本関数をフレーム取得すると1vsync近い時間がかかります。<BR>
	 * PS3, Xbox360 ではCriMvEasyPlayer::GetFrameOnTimeAsYUVBuffers() 関数と ピクセルシェーダー
	 * の組み合わせによるフレーム変換をおすすめします。<BR>
	 * 
	 * 備考: <BR>
	 * PS2版CRI Movie は本関数に対応していません。
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	CriBool GetFrameOnTimeAs32bitARGB(CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo &frameinfo, 
									CriError &err=CriMv::ErrorContainer);
#endif

#if !defined(XPT_TGT_EE)
	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Loads video frame data into a set of Y,U,V separate buffers
	 * \param       yuvbuffers      Pointer to Y,U,V buffer data structure
	 * \param       frameinfo       Video frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was retrieved
	 * \return      FALSE           otherwise
	 *
	 * Copies the next decoded frame into the Y,U,V image buffers pointed to by \a yuvbuffers, for use
	 * with a pixel shader.  CRI Movie decodes video frames into YUV420 format internally, so this is
	 * a very efficient function.
	 *
	 * If the display time is less than the frame time, or if the frame has not yet been decoded, the
	 * \a frameinfo parameter will be cleared and this function will return FALSE.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 * 
	 * \remarks
	 * If the movie does not have an alpha channel, the alpha buffer fields of the \a CriMvYuvBuffers 
	 * structure are not used.
	 *
	 * \remarks
	 * This function is not available on the PS2.
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		YUV個別バッファへのデコード結果の取得
	 *  \param		yuvbuffers		YUV個別バッファのパラメータ構造体
	 *  \param		frameinfo		取得したビデオフレームの情報構造体
	 *  \param		err		エラー情報（省略可）
	 *  \return		フレームが取得できた場合はTRUE(1)、できなかった場合はFALSE(0)を返します。
	 * 
	 * YUV個別バッファ形式で表示時刻になっているビデオフレームを取得します。<br>
	 * YUV個別バッファ形式はピクセルシェーダーでフレームを描画するための出力フォーマットです。<br>
	 * この関数を呼び出す場合は、YUV個別バッファの実体を確保したうえで呼び出す必要があります。<br>
	 * ビデオフレームは引数yuvbuffersで指定したYUV個別バッファに書き出されます。<br>
	 * もしも次のビデオフレームの表示時刻になっていなかったり、デコードが終わっていなかった場合は
	 * フレーム取得できず、frameinfoの中身はクリアされます。<br>
	 * 事前にビデオフレームが取得できるかどうかを知りたい場合は CriMvEasyPlayer::IsNextFrameOnTime()
	 * を使用してください。<BR>
	 * <BR>
	 * アルファムービ再生を行わない場合は、引数 yuvbuffers のAlphaテクスチャ関連のパラメータは使用しません。<BR>
	 * 
	 * 備考: <BR>
	 * PS2版CRI Movie は本関数に対応していません。
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	CriBool GetFrameOnTimeAsYUVBuffers(CriMvYuvBuffers *yuvbuffers, CriMvFrameInfo &frameinfo, 
										CriError &err=CriMv::ErrorContainer);
#endif

#if defined(XPT_TGT_PC) || defined(XPT_TGT_SH7269) || defined(XPT_TGT_TRGP6K)
	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Loads video frame data into a buffer, in YUV422 format
	 * \param       imagebuf        Pointer to output buffer
	 * \param       pitch           Pitch of output buffer, in bytes
	 * \param       bufsize         Size of output buffer, in bytes
	 * \param       frameinfo       Video frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was retrieved
	 * \return      FALSE           otherwise
	 *
	 * Copies the next decoded frame into the YUV422 texture buffer pointed to by \a imagebuf.
	 * CRI Movie decodes video frames into YUV420 format internally, so there is some internal conversion
	 * required when using this function.
	 *	 
	 * If the display time is less than the frame time, or if the frame has not yet been decoded, the
	 * \a frameinfo parameter will be cleared and this function will return FALSE.
	 *
	 * The \a pitch parameter is the width, in bytes, of each row of the frame image, including any padding.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 *
	 * \remarks
	 * This function is currently only available in the PC version of CRI Movie.
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		YUV422フォーマットでのデコード結果の取得
	 *  \param		imagebuf		出力バッファのポインタ
	 *  \param		pitch			出力バッファのピッチ [byte]
	 *  \param		bufsize			出力バッファサイズ [byte]
	 *  \param		frameinfo		取得したビデオフレームの情報構造体
	 *  \param		err				エラー情報（省略可）
	 *  \return		フレームが取得できた場合はTRUE(1)、できなかった場合はFALSE(0)を返します。
	 * 
	 * YUV422テクスチャフォーマットで、表示時刻になっているビデオフレームを取得します。<br>
	 * この関数を呼び出す場合は、YUVバッファの実体を確保したうえで呼び出す必要があります。<br>
	 * ビデオフレームは引数imagebufで指定したYUVバッファに書き出されます。<br>
	 * もしも次のビデオフレームの表示時刻になっていなかったり、デコードが終わっていなかった場合は
	 * フレーム取得できず、frameinfoの中身はクリアされます。<br>
	 * 事前にビデオフレームが取得できるかどうかを知りたい場合は CriMvEasyPlayer::IsNextFrameOnTime()
	 * を使用してください。
	 * 
	 * 【備考】<br>
	 * 現在は、PC版CRI Movie のみ本関数に対応しています。
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	CriBool GetFrameOnTimeAsYUV422(CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo &frameinfo, 
									CriError &err=CriMv::ErrorContainer);
#endif

#if defined(XPT_TGT_IPHONE) || defined(XPT_TGT_WINMO) || defined(XPT_TGT_ANDROID) || defined(XPT_TGT_CQSH2A) || defined(XPT_TGT_ACRODEA) || defined(XPT_TGT_NACL) || defined(XPT_TGT_SH7269) || defined(XPT_TGT_PC)|| defined(XPT_TGT_TRGP6K)
	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		Get video frame data to 16bit RGB565 format buffer
	 * 
	 * This function is added for a prototype library for iPhone
	 * Please add comments when releasing the SDK.
	 *
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		16bit RGB565フォーマットでのデコード結果の取得
	 *
	 * この関数はiPhone版CRI Movieのプロトタイプ用の関数宣言です。
	 * SDKとしてリリースする際は、コメントを追加して下さい。
	 * 
	 */	
	CriBool GetFrameOnTimeAsRGB565(CriUint8 *imagebuf, CriUint32 pitch, CriUint32 bufsize, CriMvFrameInfo &frameinfo, 
									CriError &err=CriMv::ErrorContainer);
#endif
	
#if defined(XPT_TGT_EE)
	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Gets a reference to EasyPlayer's internal video frame buffer, in 32 bit ARGB format
	 * \param       frameinfo       Video frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was retrieved
	 * \return      FALSE           otherwise
	 *
	 * Locks the internal buffer for the current video frame in memory, and retrieves a pointer to it.  This is
	 * different behavior than the GetFrameXXX()functions, which copy the frame data into a caller-supplied location.
	 *
	 * On successful return from this function, the fields of \a frameinfo will be populated with information
	 * about the frame.  In particular, the \a imageptr field will be set to the image buffer, in 32 bit ARGB format.
	 * After calling this function, an application must copy the video frame into its own buffer or transfer it to
	 * GS local memory via DMA.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 * 
	 * \remarks
	 * After the application has copied the frame data, it must call CriMvEasyPlayer::UnlockFrame().
	 *
	 * \remarks
	 * This function is only available on the PS2.
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime(), CriMvEasyPlayer::UnlockFrame()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		デコード結果領域(ARGB32bit)のロック。PS2専用。
	 *  \param		frameinfo		ロックしたビデオフレームの情報構造体
	 *  \param		err				エラー情報（省略可）
	 * 
	 * 本関数はPS2専用のフレーム取得関数で、他機種の GetFrame 関数に相当します。<BR>
	 * PS2では GetFrame 関数の代わりに本関数と UnlockFrame 関数を使用してフレーム取得を行います。<BR>
	 * GetFrame 関数は出力バッファを指定してそこへデコード結果を取得するのに対し、LockFrame 関数はバッファを指定せず
	 * CriMvEasyPlayerハンドル内部にあるデコード結果バッファのポインタを取得するところが違います。<BR>
	 * 
	 * 本関数はデコード結果のメモリ領域を参照開始するためにロックします。<BR>
	 * この関数でフレームをロックできるのは、そのフレームが表示可能時間になっている場合のみです。<BR>
	 * アプリケーションはフレームをロックしたあと、デコード結果をDMAでテクスチャ領域へ転送するか、
	 * 別バッファへコピーするなどの処理を行います。<BR>
	 * デコード結果の参照が終わった後には、必ず CriMvEasyPlayer::UnlockFrame() 関数を呼び出して参照終了を通知してください。<BR>
	 * 
	 * 備考: <BR>
	 * 本関数はPS2版CRI Movie のみ対応しています。
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime(), CriMvEasyPlayer::UnlockFrame()
	 */
	CriBool LockFrameOnTimeAs32bitARGB_PS2(CriMvFrameInfo &frameinfo, CriError &err=CriMv::ErrorContainer);
#endif

	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Unlocks the video frame
	 * \param       frameinfo       Pointer to locked video frame information
	 * \param       err             Optional error code
	 *
	 * Unlocks the video frame that was locked in memory by a call to one of the LockFrameXXX() functions.
	 *
	 * The \a frameinfo parameter must be the same one that was passed to LockFrameXXX().
	 *
	 * If the frame has been locked, EasyPlayer will not be able to retrieve the next frame until the frame is
	 * unlocked.  An attempt to lock the same frame more than once will fail.
	 *
	 * The LockFrameXXX() functions are CriMvEasyPlayer::LockFrameOnTimeAs32bitARGB_PS2() and 
	 * CriMvEasyPlayer::LockFrameOnTimeAsYUVBuffers().  Depending on the platform, only one or the other of
	 * these functions will be available.
	 *
	 * \sa CriMvEasyPlayer::LockFrameOnTimeAs32bitARGB_PS2(), CriMvEasyPlayer::LockFrameOnTimeAsYUVBuffers()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		ロックフレームで取得したデコード結果をアンロックする
	 *  \param		frameinfo		ロックしたビデオフレームの情報構造体
	 *  \param		err				エラー情報（省略可）
	 * 
	 * 本関数はロックフレーム関数を使ってロックしていたフレームをアンロックし、メモリ参照の終了を通知します。<BR>
	 * ロックフレーム関数には CriMvEasyPlayer::LockFrameOnTimeAs32bitARGB_PS2() と CriMvEasyPlayer::LockFrameOnTimeAsYUVBuffers()
	 * がありますが、どちらの関数を使ってロックした場合も、本関数を使ってアンロックします。<BR>
	 * 本関数の引数には、どのフレームをアンロックするかを指示するために、ロックフレーム関数で取得したフレーム情報構造体を指定します。<BR>
	 * 
	 * 本関数でアンロックしたフレームは、以後、次にビデオフレームのデコード出力バッファとして使用されます。<BR>
	 * １度アンロックしたフレームをもう一度ロックすることは出来ません。<BR>
	 * 
	 * \sa CriMvEasyPlayer::LockFrameOnTimeAs32bitARGB_PS2(), CriMvEasyPlayer::LockFrameOnTimeAsYUVBuffers()
	 */
	CriBool UnlockFrame(CriMvFrameInfo *frameinfo, CriError &err=CriMv::ErrorContainer);

#if !defined(XPT_TGT_EE)
	/*EN
	 *
	 * \brief       Gets a reference to EasyPlayer's internal video frame buffer
	 * \param       yuvbuffers      Y,U,V buffer data structure
	 * \param       frameinfo       Video frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was retrieved
	 * \return      FALSE           otherwise
	 *
	 * Locks the internal buffer for the current video frame in memory, and retrieves a pointer to it.  This is
	 * different behavior than the GetFrameXXX() functions, which copy the frame data into a caller-supplied location.
	 *
	 * On successful return from this function, the fields of \a yuvbuffers will be set to the Y,U,V fields of the
	 * video frame, and the fields of \a frameinfo will be populated with information about the frame. 
	 *
	 * After calling this function, an application must copy the video frame into its own buffer or transfer it to
	 * texture memory.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 * 
	 * \remarks
	 * After the application has copied the frame data, it must call CriMvEasyPlayer::UnlockFrame().
	 *
	 * \remarks
	 * This function is not available on the PS2.
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime(), CriMvEasyPlayer::UnlockFrame()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		デコード結果領域のロック
	 *  \param		yuvbuffers		YUV個別バッファのパラメータ構造体
	 *  \param		frameinfo		ロックしたビデオフレームの情報構造体
	 *  \param		err				エラー情報（省略可）
	 * 
	 * GetFrame とは別の仕様のフレーム取得関数です。<BR>
	 * 本関数は UnlockFrame 関数とセットで使用します。<BR>
	 * GetFrame 関数は出力バッファを指定してそこへデコード結果を取得するのに対し、LockFrame 関数はバッファを指定せず
	 * CriMvEasyPlayerハンドル内部にあるデコード結果バッファのポインタを取得するところが違います。<BR>
	 * 
	 * 本関数はデコード結果のメモリ領域を参照開始するためにロックし、
	 * デコード結果のYUV３種類のバッファについての情報を引数 yuvbuffers に格納します。<BR>
	 * この関数でフレームをロックできるのは、そのフレームが表示可能時間になっている場合のみです。<BR>
	 * アプリケーションはフレームをロックしたあと、デコード結果をテクスチャ領域へコピーするか、
	 * 別バッファへコピーするなどの処理を行います。<BR>
	 * デコード結果の参照が終わった後には、必ず CriMvEasyPlayer::UnlockFrame() 関数を呼び出して参照終了を通知してください。<BR>
	 * 
	 * 備考: <BR>
	 * PS2版CRI Movie は本関数に対応していません。
	 * 
	 * \sa CriMvEasyPlayer::UnlockFrame()
	 */
	CriBool LockFrameOnTimeAsYUVBuffers(CriMvYuvBuffers &yuvbuffers, CriMvFrameInfo &frameinfo, 
										CriError &err=CriMv::ErrorContainer);
#endif

	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Discards the next video frame
	 * \param       frameinfo       Discarded frame information structure
	 * \param       err             Optional error code
	 * \return      TRUE            if a frame was available to discard
	 * \return      FALSE           otherwise
	 * 
	 * Discards the next video frame, if it is available, and populates \a frameinfo with information about
	 * the frame.  Note that the \a imageptr field of that structure will not be available and should not 
	 * be referenced.
	 *
	 * To determine if the next frame is ready to be displayed, call CriMvEasyPlayer::IsNextFrameOnTime().
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		次フレームを取得せずに捨てる
	 *  \param		frameinfo		破棄したビデオフレームの情報構造体
	 *  \param		err		エラー情報（省略可）
	 * 
	 * デコード済みのビデオフレームを捨てたい場合に使用する関数です。<br>
	 * フレーム取得関数と比べると、出力用バッファを準備する必要が無い部分が特徴です。<br>
	 * CriMvEasyPlayer::IsNextFrameOnTime()で次フレームが取得できることを確認した後、本関数を呼び出してください。<br>
	 * 引数frameinfoには参考のために破棄したビデオフレームの情報が格納されますが、デコード結果自体にはアクセスできません。
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	CriBool DiscardNextFrame(CriMvFrameInfo &frameinfo, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the current channel for displaying subtitles
	 *
	 * \param       channel         Subtitle channel number
	 * \param       err             Optional error code
	 *
	 * A movie can contain up to 16 distinct sets of subtitles, each on its own display channel.  This can 
	 * be used, for instance, to provide multilanguage support.
	 *
	 * Subtitle channels are numbered sequentially, starting from 0, but do not need to be contiguous.
	 * For example, a movie can have 3 sets of subtitles, on channels 1, 5, and 7.  The number of channels 
	 * can be determined once CriMvEasyPlayer::GetMovieInfo() has completed successfully by looking at the 
	 * \a num_subtitle field of the \a CriMvStreamingParameters structure passed to that function.
	 *
	 * By default, subtitle playback is off.  To turn off subtitles once they have been turned on, pass
	 * CRIMV_SUBTITLE_CHANNEL_OFF as the value of \a channel.
	 *
	 * If the selected subtitle channel does not exist, subtitles will not be displayed.
	 *
	 * \remarks
	 * If an application turns on subtitle display with this function, it must periodically call
	 * CriMvEasyPlayer::GetSubtitleOnTime(), or else movie playback will stall.
	 *
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::GetSubtitleOnTime()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		取得する字幕チャネルの設定
	 *  \param		channel	字幕チャネル
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 取得する字幕チャネルを設定します。デフォルトは字幕取得無しです。
	 * 
	 * CriMvEasyPlayer::DecodeHeader()と CriMvEasyPlayer::GetMovieInfo()を使うことで、再生するムービが
	 * いくつの字幕を含んでいるかを再生開始前に知ることができます。
	 * 
	 * データが存在しないチャネル番号を指定した場合は、字幕は取得できません。<br>
	 * デフォルト設定（字幕取得無し）にしたい場合は、チャネルとしてCRIMV_SUBTITLE_CHANNEL_OFFを指定してください。
	 * 
	 * この関数で字幕チャネルを指定した場合は、メインループから定期的に CriMvEasyPlayer::GetSubtitleOnTime() を
	 * 実行してください。字幕取得を定期的に行わない場合は、ムービ再生が途中で止まります。
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::DecodeHeader(), CriMvEasyPlayer::GetSubtitleOnTime
	 */
	void SetSubtitleChannel(CriSint32 channel, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_VIDEO_FRAME
	 *
	 * \brief       Retrieves subtitle data for the current frame, if available
	 *
	 * \param       bufptr          Buffer to receive subtitle data
	 * \param       bufsize         Size of buffer, in bytes
	 * \param       err             Optional error code
	 *
	 * \return      The number of bytes copied into \a bufptr
	 *
	 * If the movie contains subtitle data, and there is a subtitle for the current frame, up to \a bufsize
	 * bytes of the subtitle data for the active subtitle channel will be copied into \a bufptr.  Otherwise,
	 * the entire buffer will be filled with zeroes.
	 *
	 * \remarks
	 * If an application turns on subtitle display with CriMvEasyPlayer::SetSubtitleChannel(), it must 
	 * periodically call this function, or else movie playback will stall.
	 * 
	 * \remarks
	 * An application should not make assumptions as to whether the returned data is NUL-terminated.
	 * 
	 * \sa CriMvEasyPlayer::SetSubtitleChannel()
	 */
	/*JP
	 * \ingroup MODULE_VIDEO_FRAME
	 * \brief		字幕データの取得
	 *  \param		bufptr		出力バッファポインタ
	 *  \param		bufsize		出力バッファサイズ [byte]
	 *  \param		err			エラー情報（省略可）
	 *  \return		取得した字幕データのサイズ[byte]を返します。
	 * 
	 * 表示時刻になっている字幕データを取得します。
	 * この関数を呼び出す場合は、字幕用バッファの実体を確保したうえで呼び出してください。<br>
	 * 字幕データは引数 bufptr で指定したバッファに書き出されます。<br>
	 * もし字幕データが bufsize よりも大きい場合は、bufsize に収まる量だけ書き出し、残りは破棄されます。
	 * 
	 * もしも表示時刻の字幕が無い場合は、バッファの中身はクリアされます。
	 * 
	 * CriMvEasyPlayer::SetSubtitleChannel()で存在する字幕チャネルを指定している場合は、
	 * メインループから定期的に本関数を実行してください。<br>
	 * 実行しない場合は、ムービ再生が途中で止まります。
	 * 
	 * \sa CriMvEasyPlayer::SetSubtitleChannel()
	 */
	CriUint32 GetSubtitleOnTime(CriUint8 *bufptr, CriUint32 bufsize, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets a secondary sound interface
	 *
	 * \param       sound       Secondary sound interface to attach to this handle
	 * \param       err         Optional error code
	 *
	 * A subaudio interface allows an application to play two audio tracks at the same time.  This is 
	 * typically used to play a dialog track or sound effects along with background music.  After calling 
	 * this function, an application needs to call CriMvEasyPlayer::SetSubAudioTrack() to choose the secondary 
	 * audio track to play.
	 *
	 * Note that you must create a separate sound interface to pass to this function.  The interface used 
	 * in the CriMvEasyPlayer::Create() call can not be used.
	 *
	 * If a subaudio interface is used, the application must call CriMvEasyPlayer::DetachSubAudioInterface()
	 * when the EasyPlayer handle reaches the MVEASY_STATUS_STOP or MVEASY_STATUS_PLAYEND state before calling
	 * CriMvEasyPlayer::Destroy().
	 *
	 * Calling CriMvEasyPlayer::ResetAllParameters() will not affect the value set by this function.
	 *
	 * \remarks
	 * If this function is called, it must be called before starting playback (with either CriMvEasyPlayer::Prepare() 
	 * or CriMvEasyPlayer::Start()).
	 * 
	 * \remarks
	 * An EasyPlayer handle can not use a subaudio interface and center channel replacement (see 
	 * CriMvEasyPlayer::ReplaceCenterVoice()) at the same time.
	 * 
	 * \sa CriMvEasyPlayer::DetachSubAudioInterface(), CriMvEasyPlayer::SetSubAudioTrack(),
	 *     CriMvEasyPlayer::ReplaceCenterVoice() 
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		サブオーディオ用サウンドインタフェースの設定
	 *  \param		sound		サブオーディオ用サウンドインタフェース
	 *  \param		err			エラー情報（省略可）
	 * 
	 * サブオーディオ（メインオーディオと同時に別のオーディオを再生する機能）のための
	 * サウンドインタフェースを設定します。<BR>
	 * 設定するサウンドインタフェースは、 CriMvEasyPlayer::Create() 時に指定したサウンドインタフェース
	 * とは「別の」インスタンスでなければいけません。<BR>
	 * 
	 * 本関数は、EasyPlayerハンドル作成後、 CriMvEasyPlayer::Start() または CriMvEasyPlayer::Prepare() の
	 * 呼び出しより前に実行しなければいけません。<BR>
	 * 
	 * サブオーディオを再生するには、本関数でサウンドインタフェースを設定したあと、
	 * CriMvEasyPlayer::SetSubAudioTrack() でサブオーディオのトラックを指定してください。<BR>
	 * 
	 * サブオーディオ用サウンドインタフェースを設定したハンドル破棄を破棄する前に、
	 * MVEASY_STATUS_STOP または MVEASY_STATUS_PLAYEND の状態で CriMvEasyPlayer::DetachSubAudioInterface() を呼んでください。
	 * なお、サブオーディオ用サウンドインタフェースは CriMvEasyPlayer::ResetAllParameters() を呼び出してもリセットされません。
	 * 
	 * 注意: <BR>
	 * サブオーディオ機能は、 CriMvEasyPlayer::ReplaceCenterVoice() によるセンターチャネル置き換え機能とは
	 * 同時に使用できません。<BR>
	 * 
	 * \sa CriMvEasyPlayer::DetachSubAudioInterface(), CriMvEasyPlayer::SetSubAudioTrack(),
	 *     CriMvEasyPlayer::ReplaceCenterVoice()
	 */
	void AttachSubAudioInterface(CriMvSoundInterface *sound, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Removes a secondary audio interface
	 *
	 * \param       err         Optional error code
	 *
	 * Removes the secondary sound interface that was set by a call to CriMvEasyPlayer::AttachSubAudioInterface().
	 * 
	 * This function should be called when the EasyPlayer handle's state is either MVEASY_STATUS_STOP or 
	 * MVEASY_STATUS_PLAYEND.
	 * 
	 * \sa CriMvEasyPlayer::AttachSubAudioInterface()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		サブオーディオ用サウンドインタフェースの解除
	 *  \param		err			エラー情報（省略可）
	 * 
	 * 現在設定されているサブオーディオ用サウンドインタフェースを解除します。<BR>
	 * 
	 * 本関数は、EasyPlayerハンドルの状態が CriMvEasyPlayer::MVEASY_STATUS_STOP または
	 * CriMvEasyPlayer::MVEASY_STATUS_PLAYEND の時に呼び出してください。<BR>
	 * 
	 * \sa CriMvEasyPlayer::AttachSubAudioInterface()
	 */
	void DetachSubAudioInterface(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the secondary audio track
	 *
	 * \param       track           Track number
	 * \param       err             Optional error code
	 *
	 * A secondary, or subaudio, track is typically used to play a dialog track or sound effects along 
	 * with a movie.  An application can support several different languages by setting the desired language 
	 * track with this function.  By default, the subaudio track is disabled, even if the interface has been set.
	 *
	 * Subaudio is enabled with CriMvEasyPlayer::AttachSubAudioInterface().  If the interface has not been set,
	 * this function will have no effect.
	 *
	 * The main audio track for the movie is set with CriMvEasyPlayer::SetAudioTrack().  If the same track 
	 * number is used for the main and subaudio, the subaudio will not play.
	 *
	 * To turn off the subaudio track, pass CRIMV_CENTER_VOICE_OFF as the track number.
	 *
	 * \remarks
	 * If this function is called, it must be called before starting playback (with either CriMvEasyPlayer::Prepare() 
	 * or CriMvEasyPlayer::Start()).
	 * 
	 * \remarks
	 * An EasyPlayer handle can not use a subaudio interface and center channel replacement (see 
	 * CriMvEasyPlayer::ReplaceCenterVoice()) at the same time.
	 * 
	 * \sa CriMvEasyPlayer::AttachSubAudioInterface(), CriMvEasyPlayer::ReplaceCenterVoice() 
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		サブオーディオトラックの設定
	 *  \param		track		サブオーディオ再生するトラック番号
	 *  \param		err			エラー情報（省略可）
	 * 
	 * サブオーディオトラックを設定します。デフォルト値は CRIMV_CENTER_VOICE_OFF です。<BR>
	 * 
	 * サブオーディオを再生するには、 CriMvEasyPlayer::AttachSubAudioInterface() でサウンドインタフェースを設定したあと、
	 * 本関数でサブオーディオのトラックを指定してください。<BR>
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Start() または CriMvEasyPlayer::Prepare() の呼び出しより前でなければいけません。<BR>
	 * 
	 * メインオーディオのトラックは CriMvEasyPlayer::SetAudioTrack() で指定します。
	 * サブオーディオトラックとしてメインオーディオと同じトラックを指定した場合は、サブオーディオからは何も再生されません。<BR>
	 * 
	 * サブオーディオトラックには、センターチャネル置き換え機能とは異なりチャネル数の制限はありません。
	 * モノラル、ステレオ、5.1ch のいずれのトラックもサブオーディオとして使用することができます。<BR>
	 * 
	 * 注意: <BR>
	 * サブオーディオ機能は、 CriMvEasyPlayer::ReplaceCenterVoice() によるセンターチャネル置き換え機能とは
	 * 同時に使用できません。<BR>
	 * 
	 * \sa CriMvEasyPlayer::AttachSubAudioInterface()
	 */
	// default value is -1.
	void SetSubAudioTrack(CriSint32 track, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Retrieves the secondary audio track
	 *
	 * \param       err             Optional error code
	 * \return      track number    Current subaudio track number
	 *
     * When you successfully set the subaudio track, this function return the track number
	 * that you specified by CriMvEasyPlayer::SetSubAudioTrack(). Otherwise, it retuns CRIMV_CENTER_VOICE_OFF.
	 *
	 * \sa CriMvEasyPlayer::SetSubAudioTrack()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		サブオーディオトラックの取得
	 * \param		err			エラー情報（省略可）
	 * \return      トラック番号   現在設定されているトラック番号
	 * 
	 * サブオーディオ再生が有効になっていれば、ユーザがCriMvEasyPlayer::SetSubAudioTrack()で設定した
	 * サブオーディオトラック番号を返します。
	 *
	 * サブオーディオ再生が有効でない場合や、サブオーディオトラックを指定していなかった場合は、
	 * CRIMV_CENTER_VOICE_OFFを返します。
	 * 
	 * \sa CriMvEasyPlayer::SetSubAudioTrack()
	 */
	CriSint32 GetSubAudioTrack(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Replaces the center channel of a 5.1ch audio track
	 *
	 * \param       track           Track number to use
	 * \param       err             Optional error code
	 *
	 * If the current audio track for a movie (set with CriMvEasyPlayer::SetAudioTrack()) is in 5.1ch
	 * surround sound, the center channel can be replaced with a different, mono, track.  This does not 
	 * affect any of the other channels in the 5.1ch track.
	 *
	 * If the current audio track is not 5.1ch, or the replacement track is not monaural, this call will
	 * have no effect.
	 *
	 * Passing CRIMV_CENTER_VOICE_OFF as the value of \a track will undo the replacement and revert to
	 * playing the original center channel of th 5.1ch track.
	 *
	 * \remarks
	 * An EasyPlayer handle can not use center channel replacement and a subaudio interface (see 
	 * CriMvEasyPlayer::AttachSubAudioInterface() and CriMvEasyPlayer::SetSubAudioTrack()) at the same time.
	 *
	 * \sa CriMvEasyPlayer::SetAudioTrack(), CriMvEasyPlayer::AttachSubAudioInterface(),
	 *     CriMvEasyPlayer::SetSubAudioTrack()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		センターボイスの設定
	 *  \param		track	ボイストラック番号
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 5.1ch オーディオ再生時に、センターチャネルだけを別のモノラルトラックと置き換えることができます。<br>
	 * 本関数は、置き換え用のモノラルデータが入ったオーディオトラックを設定します。<br>
	 * 5.1ch BGM に対して、ボイスだけを複数種類から差し替えたい場合に使用してください。
	 * 
	 * デフォルトはセンターボイス指定無しです。
	 * 
	 * この関数を使用した場合、メインのオーディオトラックとして再生している5.1chデータのセンターチャネル
	 * は破棄され、代わりにセンターボイスとして指定したデータが入ります。
	 * 
	 * (a) センターボイスとして使用できるのはモノラルのオーディオだけです。<br>
	 * (b) センター置き換えが有効なのはメインのオーディオが5.1chの場合だけです。
	 * 
	 * この二つの条件を満たしていない場合は、本関数で設定した値は無視されます。
	 * 
	 * デフォルト値に戻したい場合は、チャネルとしてCRIMV_CENTER_VOICE_OFFを指定してください。
	 * 
	 * \sa CriMvEasyPlayer::SetAudioTrack(), CriMvEasyPlayer::AttachSubAudioInterface(),
	 *     CriMvEasyPlayer::SetSubAudioTrack()
	 */
	void ReplaceCenterVoice(CriSint32 track, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets a callback function that will set the movie file
	 *
	 * \param       func        Callback function
	 * \param       usrobj      Pointer to user-provided data
	 * \param       err         Optional error code
	 *
	 * Normally, the movie file is provided directly, via CriMvEasyPlayer::SetFile() or 
	 * CriMvEasyPlayer::SetData(), before playback is started.  Setting a file request callback
	 * allows an application to do \a concatenated \a playback, playing multiple movies, one 
	 * after the other.
	 *
	 * If the callback function is set, it will be called when
	 *
	 * - The last data chunk of the current movie has been read by EasyPlayer.
	 * - Any of the playback functions (CriMvEasyPlayer::Start(), CriMvEasyPlayer::Prepare(), 
	 *   or CriMvEasyPlayer::DecodeHeader()) are called and the movie file has not been set.
	 *
	 * The callback should call CriMvEasyPlayer::SetFile() or CriMvEasyPlayer::SetData() if the
	 * application wants to continue playback.  Otherwise, playback will end once the callback returns.
	 *
	 * The \a usrobj parameter specifies a pointer to arbitrary data, that will be passed as the
	 * second parameter to the callback.
	 *
	 * The prototype of the callback function is
	 *
	 *  <b>void callback(CriMvEasyPlayer *mveasy, void *usrobj)</b>
	 *
	 * where
	 *
	 *  <b>mveasy</b>   is the EasyPlayer object
	 *  <b>usrobj</b>   is the pointer to user data that was passed to SetFileRequestCallback().
	 *
	 * \remarks
	 * In order to do concatenated playback, all movies must have the same
	 *
	 * \remarks
	 * - video resolution
	 * - framerate
	 * - video codec
	 * - audio track structure
	 * - subtitle structure
	 *
	 * \remarks
	 * The same audio track structure means that all movies must have the same number of tracks, and the
	 * same track number must be of the same audio type for each movie.  For instance, if the first movie has 
	 * 2 audio tracks, with track 1 being stereo and track 2 being mono, then \a all other movies would have
	 * to have 2 tracks, track 1 stereo and track 2 mono.
	 *
	 * \remarks
	 * Subtitles have to match in the number of channels.  For instance, if the first movie had 3 subtitle
	 * channels, then all other movies would need 3 channels.  It is also important to keep the languages on
	 * the same tracks for each movie, since otherwise the application would get confused.  CRI Movie makes
	 * no assumptions about languages or the interpretation of subtitles; subtitles are simply treated as 
	 * binary data.
	 *
	 * \remarks
	 * Currently, cuepoints are not supported with concatenated playback.
	 *
	 * \sa CriMvEasyPlayer::SetFile(), CriMvEasyPlayer::SetData()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ファイル要求コールバック関数の登録
	 *  \param		func	ファイル要求コールバック関数
	 *  \param		usrobj	ユーザオブジェクト
	 *  \param		err		エラー情報（省略可）
	 * 
	 * ムービの連結再生を行うために、ムービファイルを要求するコールバック関数を登録します。
	 * このコールバック関数は以下のタイミングで発生します。
	 * 
	 * ・ムービファイルを読み込み終了した時。<br>
	 * ・ファイルの指定無しで再生を開始した時。
	 * 
	 * ファイル要求コールバック関数内で CriMvEasyPlayer::SetFile() または CriMvEasyPlayer::SetData() 
	 * を呼び出すことで、連続して次のムービファイルを指定することができます。<br>
	 * SetFile() も SetData() も呼び出さなかった場合は、読み込み済みのムービが終わると
	 * 再生終了になります。
	 * 
	 * ファイル要求コールバック発生時、コールバック関数の第二引数usrobjには、登録時に指定
	 * したユーザオブジェクトが渡されます。登録ファイルリストなどの管理に利用してください。
	 * 
	 * 連結再生できるムービファイルには以下の条件があります。<BR>
	 * - ビデオ解像度が同じ
	 * - ビデオのフレームレートが同じ
	 * - ビデオのコーデックが同じ
	 * - オーディオおよび字幕のトラック構成が同じ
	 * 
	 * \sa CriMvEasyPlayer::SetFile(), CriMvEasyPlayer::SetData()
	 */
	void SetFileRequestCallback(void (*func)(CriMvEasyPlayer *mveasy, void *usrobj), 
								void *usrobj, CriError &err=CriMv::ErrorContainer);

#if defined(XPT_TGT_PC)
	/*EN
	 * \ingroup MODULE_OPTION
	 * \brief		Set processor parameters for decoding on PC
	 *
 	 *  \param		num_threads		Number of additional threads for load distribution in decoding (Maximum 3 threads)
	 *  \param		affinity_mask 	Pointer to an array of thread affinity masks for each thread specified with num_threads.
	 *  \param		priority		Thread priority of the decoding threads for load balancing
	 *  \param		err				Optional error code
	 * 
	 * This function sets the processor parameters for decoding.  Use it when you want to change 
	 * processors or thread priority for decoding load distribution.
	 * 
	 * If this function is called, it must be called before calling any of the playback functions 
	 * (CriMvEasyPlayer::Start(), CriMvEasyPlayer::Prepare(), or CriMvEasyPlayer::DecodeHeader()).
	 * 
	 * On initialization, CRI Movie prepares three worker threads for distributed decoding.<br>
	 *
	 * \a num_threads specifies how many worker threads CRI Movie should use.<br>
	 * \a affinity_mask is an array of affinity masks for the worker threads.  This array must have
	 * \a num_threads many elements.  The format of \a affinity_mask is same as for the value passed to the 
	 * Win32 <b>SetThreadAffinityMask()</b> API function.<br>
	 * \a priority is used as the thread priority for all of the threads specified by \a num_threads.
     *
	 * If this function is called, three distributed decoding threads will run in parallel by default.
	 * Also, processor assignment of the decoding threads is handled by the operating system, and their 
	 * priority will be normal.
	 * 
	 * To reset the parameters, call this function again, passing CRIMV_DEFAULT_AFFNITY_MASK_PC and 
	 * CRIMV_DEFAULT_THREAD_PRIORITY_PC as the affinity masks and thread priority.
	 * 
	 * \sa CRIMV_DEFAULT_AFFNITY_MASK_PC, CRIMV_DEFAULT_THREAD_PRIORITY_PC
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		PCでデコード処理に使う追加プロセッサ設定
	 *  \param		num_threads		負荷分散デコード用に使用する追加スレッドの数 (最大３つ）
	 *  \param		affinity_masks	スレッドアフィニティマスクの配列へのポインタ。num_threadsで指定したスレッドごとのマスク値。
	 *  \param		priority		負荷分散デコードスレッドの優先度
	 *  \param		err				エラー情報（省略可）
	 * 
	 * デコード処理を分散して処理するためのプロセッサを指定できます。
	 * デコード処理に行うプロセッサやスレッド優先度を変更したい場合に使用してください。
	 * 本関数は再生開始(Start, Prepare, DecodeHader)前に呼び出す必要があります。
	 * 
	 * CRI Movieは初期化の際に３つの分散デコード用のワーカースレッドを用意します。
	 * num_threads引数で、そのうちのいくつのスレッドを実際に使用するかを指定できます。
	 * アプリケーションから明示的にプロセッサ割り当てを行いたい場合、個々のスレッドに対して
	 * アフィニティマスクを設定してください。
	 * アフィニティマスクの値は、Win32 APIのSetThreadAffinityMaskの引数と同じ書式です。
     * スレッド優先度は、num_threadsで指定したデコードに使用するスレッドに対して適用されます。
	 * 
	 * この関数を呼ばなかった場合、３つのスレッドで並列デコードを行います。
	 * デコードスレッドのプロセッサは割り当ては全てOS任せで、優先度はスレッド標準になります。
	 * 
	 * 一度本関数で設定を変更した後、状態を戻したい場合は、CRIMV_DEFAULT_AFFNITY_MASK_PC, CRIMV_DEFAULT_THREAD_PRIORITY_PCを
	 * 引数として指定し、再度呼び出してください。
	 *
	 * \sa CRIMV_DEFAULT_AFFNITY_MASK_PC, CRIMV_DEFAULT_THREAD_PRIORITY_PC
	 */
	void SetUsableProcessors_PC(CriSint32 num_threads, const CriUint32 *affinity_mask, CriSint32 priority,
										CriError &err=CriMv::ErrorContainer);

#endif

#if defined(XPT_TGT_XBOX360)
	/*EN
	 * \ingroup MODULE_OPTION
	 * \brief		Set processor parameters for decoding
	 *  \param		processors_param	Processor Parameters
	 *  \param		err					Optional error code 
	 *
	 * \brief		Set processor parameters for decoding
	 *  \param		processors_param	Processor Parameters
	 *  \param		err					Optional error code
	 * 
	 * This function sets the processor parameters for decoding, along the priority of the
	 * decoding threads.
	 * 
	 * If this function is called, it must be called before calling any of the playback functions 
	 * (CriMvEasyPlayer::Start(), CriMvEasyPlayer::Prepare(), or CriMvEasyPlayer::DecodeHeader()).
	 *
	 * If you don't call this function, the EasyPlayer handle uses Processor 3 (Core 0, Thread 0)
	 * and Processor 5 (Core 0, Thread 0).
	 * 
	 * \sa CriMvProcessorParameters_XBOX360
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		Xbox360でデコードに使うプロセッサ設定
	 *  \param		processors_param	使用プロセッサパラメータ
	 *  \param		err					エラー情報（省略可）
	 * 
	 * デコードに使用するプロセッサを指定します。<br>
	 * 本関数は再生開始(Start, Prepare, DecodeHader)前に呼び出す必要があります。
	 * 
	 * また、デコードに使用する内部スレッドの優先度の設定が出来ます。
	 * 
	 * デフォルトのプロセッサ設定では、プロセッサ３(コア1スレッド1)とプロセッサ５
	 * (コア2スレッド1)を使用します。
	 *
	 * \sa CriMvProcessorParameters_XBOX360
	 */
	void SetUsableProcessors_XBOX360(const CriMvProcessorParameters_XBOX360 *processors_param, 
										CriError &err=CriMv::ErrorContainer);
#endif

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets the frame where playback will start
	 *
	 * \param       seek_frame_id       Frame (0-based) to start playing from
	 * \param       err                 Optional error code
	 *
	 * To play a movie from other than the first frame, call this function before starting playback.
	 * To seek to a new frame when the movie is already playing, call CriMvEasyPlayer::Stop() and wait
	 * for the handle to change to the MVEASY_STATUS_STOP state, call this function with the desired
	 * frame number, then call CriMvEasyPlayer::Start() again.
	 *
	 * Valid values for seek_frame_id are from 0 to \a num_frames - 1, where \a num_frames can be found by
	 *
	 * <pre>
	 *
	 *   CriMvStreamingParameters streaming_params;
	 *   CriSint32                num_frames;
	 *
	 *   GetMovieInfo(streaming_params);
	 *   num_frames = streaming_params.video_prm[0].total_frames;
	 *
	 * </pre>
	 *
	 * Refer to \ref usr_mech7 for more information.
	 *
	 * \remarks
	 * If the value of \a seek_frame_id is out of range, playback will start from frame 0.
	 *
	 * \sa CriMvStreamingParameters
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		シーク再生開始位置の設定
	 *  \param		seek_frame_id		シーク再生開始するフレーム番号（０〜）
	 *  \param		err					エラー情報（省略可）
	 * 
	 * シーク再生を開始するフレーム番号を指定します。
	 * 
	 * 再生開始前( CriMvEasyPlayer::Prepare()または CriMvEasyPlayer::Start()呼び出し前)に本関数を実行してください。
	 * また、この関数はムービの再生中に呼び出すことは出来ません。再生中にシークをする場合は、一度再生を停止してから
	 * 本関数を呼び出してください。
	 *
	 * 本関数を実行しなかった場合、またはフレーム番号０を指定した場合はムービの先頭から再生を開始します。
	 * 指定したフレーム番号が、ムービデータの総フレーム数より大きかったり負の値だった場合もムービの先頭から再生します。
	 * 
	 * \ref usr_mech7 もあわせて参照してください。
	 *
	 */
	void SetSeekPosition(CriSint32 seek_frame_id, CriError &err=CriMv::ErrorContainer);

	/*EN
	 *
	 * \brief       Calculates a  frame ID from a frame time
	 *
	 * \param       count           Timer counter
	 * \param       unit            Counter increment per second
	 * \param       err             Optional error code
	 *
	 * \return      Frame ID corresponding to given time
	 *
	 * Each frame of a movie corresponds to a particular display time, based on the framerate.  Given a
	 * time from the start of playback, this function will return the ID of the specific frame that should
	 * be displayed, barring any delays or skipped frames, at that time.
	 *
	 * The time, in seconds, is specified by \a count / \a unit.<br>
	 *
	 * This function can be used for, among other things, jumping to a particular frame when a cuepoint is reached.
	 *
	 * \remarks
	 * This function can be called once the EasyPlayer handle status has reached MVEASY_STATUS_WAIT_PREP.
	 *
	 * \sa CriMvEasyPlayer::CalcTimeFromFrameId();
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		再生時刻からフレーム番号の計算
	 *  \param		count		タイマカウント
	 *  \param		unit		１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 *  \param		err			エラー情報（省略可）
	 *  \return		frame ID
	 * 
	 * 再生時刻からフレーム番号を計算します。
	 * この関数は、EasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあとに使用できます。
	 * 
	 * シーク再生開始位置を、時刻から計算したいときに使用してください。
	 * （例えばキューポイント情報からシーク位置を決定する場合など。）
	 * 
	 * \sa CriMvEasyPlayer::CalcTimeFromFrameId();
	 */
	CriSint32 CalcFrameIdFromTime(CriUint64 count, CriUint64 unit, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Calculates a frame time from a frame ID
	 *
	 * \param       frame_id        Frame ID
	 * \param       unit            Counter increment per second
	 * \param       err             Optional error code
	 *
	 * \return      Timer counter corresponding to given frame ID
	 *
	 * Each frame of a movie corresponds to a particular display time, based on the framerate.  Given a
	 * frame ID and the number of timer intervals per second, this function will return the timer count
	 * of the display time for that frame.
	 *
	 * The display time, in seconds, for this frame is calculated by dividing the timer count by the timer
	 * interval.
	 * 
	 * If you have the movie frame, you do not have to calculate the time.  The \a time and \a tunit fields
	 * of the CriMvFrameInfo structure that is passed the GetFrameOnTimeXXX() and DiscardNextFrame()
	 * functions will contain this information.
	 *
	 * \remarks
	 * This function can be called once the EasyPlayer handle status has reached MVEASY_STATUS_WAIT_PREP.
	 *
	 * \sa CriMvEasyPlayer::CalcFrameIdFromTime(), CriMvFrameInfo
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		フレーム番号から再生時刻の計算
	 *  \param		frame_id	frame ID
	 *  \param		unit		１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 *  \param		err			エラー情報（省略可）
	 *  \return		タイマカウント
	 * 
	 * フレーム番号から再生時刻を計算します。
	 * この関数は、EasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあとに使用できます。
	 * 
	 * 実際にフレーム取得した場合は、計算の必要はありません。フレーム情報構造体の時刻を参照してください。
	 * 
	 * \sa CriMvEasyPlayer::CalcFrameIdFromTime(), CriMvFrameInfo
	 */
	CriUint64 CalcTimeFromFrameId(CriSint32 frame_id, CriUint64 unit, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Find the next event point, if any, after the given time counter
	 *
	 * \param       count           Timer counter
	 * \param       unit            Counter increment per second
	 * \param       type            Type of event point to look for
	 * \param       eventinfo       Returned event point information structure
	 * \param       err             Optional error code
	 *
	 * \return      Frame ID corresponding to given time
	 * 
	 * Event points allow an application to associate arbitrary actions with specific points in a movie.
	 * This function will search forward in the movie for the next event point after the given time
	 * (specifed as \a count / \a unit).  If an event point is found, \a eventinfo will be populated with
	 * the information about the event point, and the function will return the corresponding frame ID.
	 *
	 * \a type is an application-defined value that can be used to categorize event points, and is specified 
	 * when the movie is encoded (see <b>link to event-point-specification-section</b> for more information
	 * about creating event points.).  If -1 is passed as the value of \a type, all event points will be
	 * searched.  Otherwise, only matching event point types will be searched.
	 *
	 * If no event point of the requested type is found, this function will return -1.
	 *
	 * \remarks
	 * This function can be called once the EasyPlayer handle status has reached MVEASY_STATUS_WAIT_PREP.
	 *
	 * \sa  CriMvEasyPlayer::SearchPrevEventPointByTime();
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		指定時刻直後のイベントポイントの検索
	 *  \param		count		タイマカウント
	 *  \param		unit		１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 *  \param		type		検索対象とするイベントポイントのtype値
	 *  \param		eventinfo	発見したイベントポイントの情報
	 *  \param		err			エラー情報（省略可）
	 *  \return		frame ID
	 * 
	 * 指定時刻の次にあるイベントポイントを検索し、イベントポイント情報とフレーム番号を取得します。
	 * この関数は、EasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあとに使用できます。
	 * 
	 * 検索の対象となるのは type で指定した値が一致するイベントポイントです。
	 * type に -1を指定した場合は、全てのイベントポイントが検索対象となります。
	 * 
	 * 検索対象となるイベントポイントが発見できなかった場合は、フレーム番号は-1を返します。
	 * 
	 * \sa  CriMvEasyPlayer::SearchPrevEventPointByTime();
	 */
	CriSint32 SearchNextEventPointByTime(CriUint64 count, CriUint64 unit, CriSint32 type, 
											CriMvEventPoint &eventinfo, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Find the previous event point, if any, before the given time counter
	 *
	 * \param       count           Timer counter
	 * \param       unit            Counter increment per second
	 * \param       type            Type of event point to look for
	 * \param       eventinfo       Returned event point information structure
	 * \param       err             Optional error code
	 *
	 * \return      Frame ID corresponding to given time
	 * 
	 * Event points allow an application to associate arbitrary actions with specific points in a movie.
	 * This function will search backward in the movie (i.e. towards the beginning) for the next event 
	 * point before the given time (specifed as \a count / \a unit).  If an event point is found, 
	 * \a eventinfo will be populated with the information about the event point, and the function will 
	 * return the corresponding frame ID.
	 *
	 * \a type is an application-defined value that can be used to categorize event points, and is specified 
	 * when the movie is encoded (see <b>link to event-point-specification-section</b> for more information
	 * about creating event points.).  If -1 is passed as the value of \a type, all event points will be
	 * searched.  Otherwise, only matching event point types will be searched.
	 *
	 * If no event point of the requested type is found, this function will return -1.
	 *
	 * \remarks
	 * This function can be called once the EasyPlayer handle status has reached MVEASY_STATUS_WAIT_PREP.
	 *
	 * \sa  CriMvEasyPlayer::SearchNextEventPointByTime();
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		指定時刻直前のイベントポイントの検索
	 *  \param		count		タイマカウント
	 *  \param		unit		１秒あたりのタイマカウント値。count ÷ unit で秒単位の時刻となります。
	 *  \param		type		検索対象とするイベントポイントのtype値
	 *  \param		eventinfo	発見したイベントポイントの情報
	 *  \param		err			エラー情報（省略可）
	 *  \return		frame ID
	 * 
	 * 指定時刻の手前にあるイベントポイントを検索し、イベントポイント情報とフレーム番号を取得します。
	 * この関数は、EasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあとに使用できます。
	 * 
	 * 検索の対象となるのは type で指定した値が一致するイベントポイントです。
	 * type に -1を指定した場合は、全てのイベントポイントが検索対象となります。
	 * 
	 * 検索対象となるイベントポイントが発見できなかった場合は、フレーム番号は-1を返します。
	 *
	 * \sa  CriMvEasyPlayer::SearchNextEventPointByTime();
	 */
	CriSint32 SearchPrevEventPointByTime(CriUint64 count, CriUint64 unit, CriSint32 type, 
											CriMvEventPoint &eventinfo, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Gets the list of all event points in the movie
	 *
	 * \param       err         Optional error code
	 *
	 * \return      Pointer to the list of event points, or NULL if there are no event points
	 *
	 * CRI Movie considers a cue point list to be the number of event points in a movie, and a pointer to
	 * an array of event point structures.  Event points allow an application to associate arbitrary actions 
	 * with specific points in a movie.
	 *
	 * The returned pointer points to an area inside of the EasyPlayer's work buffer.  An application should
	 * not attempt to write to it.
	 * 
	 * \remarks
	 * This function can be called once the EasyPlayer handle status has reached MVEASY_STATUS_WAIT_PREP.
	 * The returned information is valid through the MVEASY_STATUS_STOP state.  Once the EasyPlayer handle 
	 * has been destroyed, or the movie has been restarted (by calling CriMvEasyPlayer::Start(),
	 * CriMvEasyPlayer::Prepare(), or CriMvEasyPlayer::DecodeHeader()), the cuepoint information will be
	 * invalid.
	 * 
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		キューポイント情報（イベントポイント一覧）の取得
	 *  \param		err			エラー情報（省略可）
	 *  \return		Cue point info (Event point list)
	 * 
	 * キューポイント情報（イベントポイント一覧）を取得します。
	 * この関数は、EasyPlayerハンドルの状態が MVEASY_STATUS_WAIT_PREP 以降になったあとに使用できます。
	 * 
	 * この関数で取得するキューポイント情報は、再生ハンドルのワークバッファを直接参照しています。<BR>
	 * 再生停止状態での参照は可能ですが、次の再生を開始した後は参照を禁止します。<BR>
	 * このキューポイント情報を別のメモリにコピーした場合もこの条件は変わりません。
	 * 
	 */
	CriMvCuePointInfo* GetCuePointInfo(CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Sets a function that will be called when a cue point is reached
	 *
	 * \param       func        Callback function
	 * \param       usrobj      Pointer to user-provided data
	 * \param       err         Optional error code
	 *
	 * Beginning with version 2.0, CRI Movie allows a movie to specify arbitrary actions to
	 * happen at various points on the timeline.  These are referred to as \a cue \a points, or, 
	 * more generally, as \a event \a points.  If a cue point callback has been installed for the
	 * movie, it will be called whenever a cuepoint has been reached.
	 *
	 * The \a usrobj parameter specifies a pointer to arbitrary data, that will be passed as the
	 * third parameter to the callback.
	 *
	 * The prototype of the callback function is
	 *
	 *  <b>void callback(CriMvEasyPlayer *mveasy, CriMvEventPoint *eventinfo, void *usrobj)</b>
	 *
	 * where
	 *
	 *  <b>mveasy</b>       is the EasyPlayer object<br>
	 *  <b>eventinfo</b>    is the event info structure that was reached<br>
	 *  <b>usrobj</b>       is the pointer to user data that was passed to SetFileRequestCallback().<br>
	 *
	 * \remarks
	 * Do not call any movie playback functions (for example, CriMvEasyPlayer::Stop()) from the callback
	 * function.  If you need to do this, set a flag from the callback and refer to it in your main loop.
	 *
	 * \sa  CriMvEventPoint
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		キューポイントコールバック関数の登録
	 *  \param		func	キューポイントコールバック関数
	 *  \param		usrobj	ユーザオブジェクト
	 *  \param		err		エラー情報（省略可）
	 * 
	 * キューポイントのコールバック関数を登録します。
	 * このコールバック関数は、ムービの再生時刻が各イベントポイントで指定された時刻を経過した時に発生します。
	 * コールバック関数の呼び出し判定は CriMvEasyPlayer::Update() から行われます。
	 * 
	 * キューポイントコールバック発生時、コールバック関数の第２引数 eventinfo にはエベントポイント情報が、
	 * 第３引数usrobjには、登録時に指定したユーザオブジェクトが渡されます。
	 * 
	 * キューポイントコールバック関数内では、ムービ再生をコントロールする関数（例えば CriMvEasyPlayer::Stop()）
	 * を呼び出してはいけません。
	 * 
	 * \sa  CriMvEventPoint
	 */
	void SetCuePointCallback(void (*func)(CriMvEasyPlayer *mveasy, CriMvEventPoint *eventinfo, void *usrobj), 
								void *usrobj, CriError &err=CriMv::ErrorContainer);


	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Resets movie handle parameters to their default values
	 *
	 * \param       err         Optional error code
	 *
	 * This function will reset most parameters back to their default values.  Call this when you are 
	 * reusing an EasyPlayer handle and need to quickly undo changes to a number of parameters.
	 *
	 * Note that this will not remove a subaudio interface (set by CriMvEasyPlayer::AttachSubAudioInterface()).
	 * CriMvEasyPlayer::DetachSubAudioInterface() must be called instead.
	 *
	 * \remarks
	 * This function can be called once the EasyPlayer handle status is either MVEASY_STATUS_STOP or
	 * MVEASY_STATUS_PLAYEND.
	 *
	 * <table>
	 * <TR><TH> Setting API <TH> Reset by ResetAllParameters() <TH> Reset by Stop()
	 * <TR><TD> SetFile                     <TD> YES <TD> (*1)
	 * <TR><TD> SetData                     <TD> YES <TD> (*1)
	 * <TR><TD> Pause                       <TD> YES <TD> YES
	 * <TR><TD> SetMasterTimer              <TD> YES <TD> NO
	 * <TR><TD> SetNumberOfFramePools       <TD> YES <TD> NO
	 * <TR><TD> SetBufferingTime            <TD> YES <TD> NO
	 * <TR><TD> SetReloadThresholdTime      <TD> YES <TD> NO
	 * <TR><TD> SetMaxBitrate               <TD> YES <TD> NO
	 * <TR><TD> SetAudioTrack               <TD> YES <TD> NO
	 * <TR><TD> SetLoopFlag                 <TD> YES <TD> NO
	 * <TR><TD> SetStreamingParameters      <TD> YES <TD> NO
	 * <TR><TD> AttachSubAudioInterface     <TD> NO  <TD> NO
	 * <TR><TD> SetSubAudioTrack            <TD> YES <TD> NO
	 * <TR><TD> ReplaceCenterVoice          <TD> YES <TD> NO
	 * <TR><TD> SetFileRequestCallback      <TD> YES <TD> NO
	 * <TR><TD> SetSeekPosition             <TD> YES <TD> NO
	 * <TR><TD> SetCuePointCallback         <TD> YES <TD> NO
	 * </table>
	 *
	 * (*1) Normally, values set by SetFile() or SetData() are not reset by a call to Stop().  However,
	 * if a file request callback has been set (via CriMvEasyPlayer::SetFileRequestCallback()), this
	 * setting will be reset.
	 * 
	 * \sa CriMvEasyPlayer::Stop(), CriMvEasyPlayer::SetFileRequestCallback()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		パラメータのリセット
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 再生ハンドルに設定されたパラメータ類をリセットします。<BR>
	 * ただしサブオーディオ用インタフェースだけはリセットされませんので、アプリケーションで明示的に
	 * CriMvEasyPlayer::DetachSubAudioInterface() を呼び出してください。
	 * 
	 * 本関数はハンドル状態が MVEASY_STATUS_STOP または MVEASY_STATUS_PLAYEND の時に呼び出してください。
	 * 
	 * <table>
	 * <TR><TH> 設定関数 <TH> ResetAllParametersによる<BR>リセット処理 <TH> Stopによる<BR>リセット処理
	 * <TR><TD> SetFile                     <TD> o <TD> (*1)
	 * <TR><TD> SetData                     <TD> o <TD> (*1)
	 * <TR><TD> Pause                       <TD> o <TD> o
	 * <TR><TD> SetMasterTimer              <TD> o <TD> x
	 * <TR><TD> SetNumberOfFramePools       <TD> o <TD> x
	 * <TR><TD> SetBufferingTime            <TD> o <TD> x
	 * <TR><TD> SetReloadThresholdTime      <TD> o <TD> x
	 * <TR><TD> SetMaxBitrate               <TD> o <TD> x
	 * <TR><TD> SetAudioTrack               <TD> o <TD> x
	 * <TR><TD> SetLoopFlag                 <TD> o <TD> x
	 * <TR><TD> SetStreamingParameters      <TD> o <TD> x
	 * <TR><TD> AttachSubAudioInterface     <TD> x <TD> x
	 * <TR><TD> SetSubAudioTrack            <TD> o <TD> x
	 * <TR><TD> ReplaceCenterVoice          <TD> o <TD> x
	 * <TR><TD> SetFileRequestCallback      <TD> o <TD> x
	 * <TR><TD> SetSeekPosition             <TD> o <TD> x
	 * <TR><TD> SetCuePointCallback         <TD> o <TD> x
	 * </table>
	 * (*1) 通常はリセットされません。ただしファイル要求コールバックが登録されていた場合はリセットされます。
	 * 
	 * \sa CriMvEasyPlayer::Stop()
	 */
	void ResetAllParameters(CriError &err=CriMv::ErrorContainer);

	/* 再生用ワークバッファおよび下位モジュールの解放（明示的な呼び出し用） */
	void ReleasePlaybackWork(CriError &err=CriMv::ErrorContainer);

public:	/* for DEBUG */
	/*//EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief		Sets the maximum movie data read size
	 *
	 * \param		max_chunk_size		The maximum read size, in bytes
	 * \param		err					Optional error code
	 * 
	 * This is a DEBUG function and should not normally need to be used by applications.
	 *
	 * Sets the maximum read size.  CRI Movie will attempt to read this many bytes each time it
	 * needs to load more movie data from disk.
	 * 
	 * If this function is called, it must be called before calling any of the playback functions 
	 * (CriMvEasyPlayer::Start(), CriMvEasyPlayer::Prepare(), or CriMvEasyPlayer::DecodeHeader()).
	 *
	 * To revert to the default behavior, pass 0 as \a max_chunk_size.
	 *
	 * \remarks
	 * If this function is called, the new value for \a max_chunk_size will not be reflected in the
	 * CriMvStreapingParameters structure.  A call to CriMvEasyPlayer::GetMovieInfo() will return the
	 * original value that was set in the movie header.
	 *
	 * \remarks
	 * If you call CriMvEasyPlayer::SetStreamingParameters(), this function can not be used.
	 *
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::SetMinBufferSize()
	 */
	/*//JP
	 * \ingroup MODULE_OPTION
	 * \brief		最大チャンクサイズの指定
	 *  \param		max_chunk_size		最大チャンクサイズ[byte]
	 *  \param		err					エラー情報（省略可）
	 * 
	 * ムービデータの最大チャンクサイズを指定します。<br>
	 * 現在のライブラリでは、本関数はアプリケーションから使用する必要はありません。<br>
	 * 
	 * 本関数で設定した最大チャンクサイズは、CriMvEasyPlayer::GetMovieInfo 関数で取得するムービ情報には反映されません。
	 * CriMvEasyPlayer::GetMovieInfo 関数で取得できるのはムービデータの本来の情報です。<br>
	 * 
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Prepare 関数または CriMvEasyPlayer::Start 関数の前までに実行してください。
	 * 
	 * 最大チャンクサイズに 0を指定した場合、最大チャンクサイズはムービデータの持つ値となります。<br>
	 * また、アプリケーションが CriMvEasyPlayer::SetStreamingParameters 関数を呼び出した場合は本関数で
	 * 設定した値よりも、 CriMvEasyPlayer::SetStreamingParameters 関数の指定が優先されます。
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::SetMinBufferSize()
	 */
	void SetMaxChunkSize(CriUint32 max_chunk_size, CriError &err=CriMv::ErrorContainer);

	/*//EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief		Sets the minimum movie data buffer size
	 *
	 * \param		min_buffer_size		The minimum buffer size, in bytes
	 * \param		err					Optional error code
	 * 
	 * This is a DEBUG function and should not normally need to be used by applications.
	 *
	 * Sets the minimum buffer size used for reading movie data from disk.
	 * 
	 * If this function is called, it must be called before calling any of the playback functions 
	 * (CriMvEasyPlayer::Start(), CriMvEasyPlayer::Prepare(), or CriMvEasyPlayer::DecodeHeader()).
	 *
	 * To revert to the default behavior, pass 0 as \a max_chunk_size.
	 *
	 * \remarks
	 * If this function is called, the new value for \a min_buffer_size will not be reflected in the
	 * CriMvStreapingParameters structure.  A call to CriMvEasyPlayer::GetMovieInfo() will return the
	 * original value that was set in the movie header.
	 *
	 * \remarks
	 * If you call CriMvEasyPlayer::SetStreamingParameters(), this function can not be used.
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::SetMaxChunkSize()
	 */
	/*//JP
	 * \ingroup MODULE_OPTION
	 * \brief		最小バッファサイズの指定
	 *  \param		min_buffer_size		最小バッファサイズ[byte]
	 *  \param		err					エラー情報（省略可）
	 * 
	 * ムービデータの最小バッファサイズを指定します。<br>
	 * 現在のライブラリでは、本関数はアプリケーションから使用する必要はありません。<br>
	 * 
	 * 本関数で設定した最小バッファサイズは、CriMvEasyPlayer::GetMovieInfo 関数で取得するムービ情報には反映されません。
	 * CriMvEasyPlayer::GetMovieInfo 関数で取得できるのはムービデータの本来の情報です。<br>
	 * 
	 * 本関数の呼び出しは、 CriMvEasyPlayer::Prepare 関数または CriMvEasyPlayer::Start 関数の前までに実行してください。
	 * 
	 * 最小バッファサイズに 0を指定した場合、最小バッファサイズはムービデータの持つ値となります。<br>
	 * また、アプリケーションが CriMvEasyPlayer::SetStreamingParameters 関数を呼び出した場合は本関数で
	 * 設定した値よりも、 CriMvEasyPlayer::SetStreamingParameters 関数の指定が優先されます。
	 * 
	 * \sa CriMvEasyPlayer::GetMovieInfo(), CriMvEasyPlayer::SetMaxChunkSize()
	 */
	void SetMinBufferSize(CriUint32 min_buffer_size, CriError &err=CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Retrieves various movie playback statistics
	 *
	 * \param       playinfo        Playback statistics structure
	 * \param       err             Optional error code
	 *
	 * This is a DEBUG function and is not normally needed by applications.
	 *
	 * Retrieves a number of performance statistics dealing with movie playback, including how often
	 * a frame could not be retrieved and how close the plaback framerate is to the movie's expected
	 * playback.
	 *
	 * This information is updated on every call to CriMvEasyPlayer::IsNextFrameOnTime().  In order for
	 * the values to be accurate, an application should call IsNextFrameOnTime() once each time through
	 * its main loop.  Calling IsNextFrameOnTime() too often or not often enough will result in
	 * misleading statistics.
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		ムービ再生情報の取得
	 *  \param		playinfo		ムービ情報（返り値）
	 *  \param		err		エラー情報（省略可）
	 *
	 * 本関数は通常、アプリケーションからは使用しません。デバッグ用の関数です。
	 *
	 * 現在再生しているムービの再生情報 CriMvPlaybackInfo 構造体を取得できます。<br>
	 * この情報からビデオフレームの取得間隔や、ビデオフレームのデコード遅延などを知ることができます。<br>
	 * 
	 * 再生情報はアプリが呼び出す CriMvEasyPlayer::IsNextFrameOnTime() 内で更新します。<br>
	 * アプリケーションが CriMvEasyPlayer::IsNextFrameOnTime()を呼び出さない場合や、メインループで複数回
	 * 呼び出す場合は情報が正しく更新されないので注意してください。<br>
	 * 
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	void GetPlaybackInfo(CriMvPlaybackInfo & playinfo, CriError &  err = CriMv::ErrorContainer);

	/*EN
	 * \ingroup MODULE_OPTION
	 *
	 * \brief       Gets the result of the last attempt to retrieve a video frame
	 *
	 * \param       err         Optional error code
	 *
	 * \return      The result of the last frame retrieval
	 *
	 * Returns one of the following values:
	 *
	 * <table border = 0>
	 * <tr><th>Value</th>   <th>Meaning</th></tr>
	 * <tr><td><b>CRIMV_LASTFRAME_OK</b></td>           <td>The frame was successfully retrieved.</td></tr>
	 * <tr><td><b>CRIMV_LASTFRAME_TIME_EARLY</b></td>   <td>It is too soon to display this frame.  The frame time
	 *                                                      is greater than the current playback time.</td></tr>
	 * <tr><td><b>CRIMV_LASTFRAME_DECODE_DELAY</b></td> <td>The frame has not been decoded yet, or CRI Movie is not 
	 *                                                      done decoding it.</td></tr>
     * <tr><td><b>CRIMV_LASTFRAME_DISCARDED</b></td>    <td>The frame was discarded, by calling 
     *                                                      CriMvEasyPlayer::DiscardNextFrame()</td></tr>
     * </table>
	 * 
	 * \remarks
	 * The result is determined by calling CriMvEasyPlayer::IsNextFrameOnTime(), not the GetFrameOnTimeXXX()
	 * functions.
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime(), CriMvEasyPlayer::DiscardNextFrame()
	 */
	/*JP
	 * \ingroup MODULE_OPTION
	 * \brief		前回のビデオフレーム取得の結果を取得する
	 *  \param		err		エラー情報（省略可）
	 *  \return				フレーム取得結果の列挙値
	 * 
	 * 本関数は通常、アプリケーションからは使用しません。デバッグ用の関数です。
	 *
	 * 前回のビデオフレーム取得の結果を返します。
	 * ビデオフレームのデコードが間に合っているのかどうかをチェックすることが出来ます。
	 * 
	 * 注意: <BR>
	 * ビデオフレーム取得の結果とは、基本的にアプリケーションが呼び出す CriMvEasyPlayer::IsNextFrameOnTime() の結果を元に
	 * 更新します。GetFrameOnTime関数の結果ではありません。
	 *
	 * \sa CriMvEasyPlayer::IsNextFrameOnTime()
	 */
	CriMvLastFrameResult GetLastFrameResult(CriError &  err = CriMv::ErrorContainer);

	/* [Unofficial] */
	/* Set the number of decoded frames to keep internally during the MVEASY_STATUS_PREP status. */
	/* Adjust the number if you need to reduce latesy for the first frame */
	void SetNumberOfFramesForPrep(CriUint32 num_frames, CriError &  err = CriMv::ErrorContainer);

	/* [Unofficial]
	 * \ingroup MODULE_OPTION
	 * \brief		再生終了/停止通知コールバック関数の登録
	 *  \param		func	再生終了/停止通知コールバック関数
	 *  \param		usrobj	ユーザオブジェクト
	 *  \param		err		エラー情報（省略可）
	 * 
	 * 再生終了および再生停止を通知するコールバック関数を登録します。
	 * このコールバック関数は、ヘッダ解析/再生準備/再生状態から再生停止/再生終了状態に
	 * 遷移した直後に一度だけ呼び出されます。
	 * コールバック関数の呼び出しは CriMvEasyPlayer::Update() から行われます。
	 * 
	 * 登録したコールバック関数内では、ムービ再生をコントロールする関数（例えば CriMvEasyPlayer::Stop()）
	 * を呼び出してはいけません。
	 * 
	 * 注意: MVEASY_STATUS_PLAYEND状態からMVEASY_STATUS_STOP状態への遷移時にはコールバック関数は呼び出されません。
	 */
	void SetStopCompleteCallback(void (*func)(CriMvEasyPlayer *mveasy, void *usrobj), 
							void *usrobj, CriError &err=CriMv::ErrorContainer);

	/* For FAST_LATENCY */
	/*************************************************************************************/
	/* コンフィグ指定のハンドル作成関数 */
	static CriMvEasyPlayer* CRIAPI Create(CriHeap heap, 
								   CriMvHandleConfig *config,
								   CriMvFileReaderInterface *freader, 
								   CriMvSystemTimerInterface *stimer, 
								   CriMvSoundInterface *sound, 
								   CriError &err=CriMv::ErrorContainer);

	/* For Sofdec2 */
	/*************************************************************************************/
	enum MetaFlag {
		MVEASY_META_FLAG_CUE  = 0x0001,
		MVEASY_META_FLAG_SEEK = 0x0002,
		MVEASY_META_FLAG_ALL  = MVEASY_META_FLAG_CUE + MVEASY_META_FLAG_SEEK,

		/* Keep enum 4bytes */
		MVEASY_META_FLAG_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	enum ReferFrameResult {
		MVEASY_REFER_FRAME_RESULT_OK = (1),
		MVEASY_REFER_FRAME_RESULT_SHORT_INPUT = (2),
		MVEASY_REFER_FRAME_RESULT_SHORT_CPUTIME = (3),
		MVEASY_REFER_FRAME_RESULT_DEMUX_STUCK = (4),

		/* Keep enum 4bytes */
		MVEASY_REFER_FRAME_RESULT_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	/* ユーザからのワーク領域渡し版、ハンドル作成関数 */
	static CriMvEasyPlayer* CRIAPI Create(void *work, CriSint32 work_size,
								   CriMvHandleConfig *config,
								   CriMvFileReaderInterface *freader, 
								   CriMvSystemTimerInterface *stimer, 
								   CriMvSoundInterface *sound, 
								   CriError &err=CriMv::ErrorContainer);

	/* 再生用ワーク領域サイズの計算 */
	CriSint32 CalcPlaybackWorkSize(const CriMvStreamingParameters *stmprm, CriError &  err = CriMv::ErrorContainer);

	/* 再生用ワーク領域の設定関数 */
	void SetPlaybackWork(void *work, Sint32 work_size, CriError &  err = CriMv::ErrorContainer);

	/* メタデータワーク用コールバック関数 */
	void SetMetaDataWorkAllocator(CriMvMetaDataWorkMallocFunc allocfunc, CriMvMetaDataWorkFreeFunc freefunc,void *usrobj, CriMvMetaFlag meta_flag);

	/* 引数で指定したフレーム情報の表示判定 */
	CriBool IsFrameOnTime(const CriMvFrameInfo *frameinfo, CriError &err=CriMv::ErrorContainer);

	/* フレームの参照　*/
	ReferFrameResult ReferFrame(CriMvFrameInfo &frameinfo, CriError &err=CriMv::ErrorContainer);

	/* YUV個別バッファフォーマットでのバッファ取得 */
	CriBool LockFrameYUVBuffersWithAlpha(CriMvYuvBuffers &yuvbuffers, CriMvFrameInfo &frameinfo, CriMvAlphaFrameInfo &alpha_frameinfo, CriError &err=CriMv::ErrorContainer);

	/* LockFrameYUVBuffersWithAlphaでロックしたフレームの解放 */
	CriBool UnlockFrameBufferWithAlpha(CriMvFrameInfo *frameinfo, CriMvAlphaFrameInfo *alpha_frameinfo, CriError &err=CriMv::ErrorContainer);

	/* 32bitARGBバッファフォーマットへのコピー関数 */
	CriBool CopyFrameToBufferARGB32(CriUint8 *dstbuf, CriUint32 dst_pitch, CriUint32 dst_bufsize, 
			const CriMvYuvBuffers *srcbufs, const CriMvFrameInfo *src_vinf, const CriMvAlphaFrameInfo *src_ainf, CriError &err=CriMv::ErrorContainer);

	/* 32bitARGBバッファフォーマットへαのみのコピー関数 */
	CriBool CopyAlphaToBufferARGB32(
		const CriMvFrameInfo *src_vinf, 
		CriUint8 *dst_buf, 
		CriUint32 dst_pitch, 
		CriUint32 dst_bufsize, 
		CriError &err=CriMv::ErrorContainer
	);

#if defined(XPT_TGT_IPHONE) || defined(XPT_TGT_WINMO) || defined(XPT_TGT_ANDROID) || defined(XPT_TGT_CQSH2A) || defined(XPT_TGT_ACRODEA) || defined(XPT_TGT_NACL) || defined(XPT_TGT_SH7269) || defined(XPT_TGT_PC)|| defined(XPT_TGT_TRGP6K)
	CriBool CopyFrameToBufferRGB565(CriUint8 *dstbuf, CriUint32 dst_pitch, CriUint32 dst_bufsize, 
			const CriMvYuvBuffers *srcbufs, const CriMvFrameInfo *src_vinf, const CriMvAlphaFrameInfo *src_ainf, CriError &err=CriMv::ErrorContainer);
#endif

	/* YUV個別バッファフォーマットのコピー関数 */
	CriBool CopyFrameToBuffersYUV(CriMvYuvBuffers *dstbufs, 
			const CriMvFrameInfo *src_vinf, const CriMvAlphaFrameInfo *src_ainf, CriError &err=CriMv::ErrorContainer);

	/* リードバッファサイズの強制指定 */
	void SetReadBufferSize(CriUint32 buffer_size, CriError &err=CriMv::ErrorContainer);

	CriUint32 GetMinBufferSize(CriError &err=CriMv::ErrorContainer);
	/**************************************************************************************/
	void SetVideoFramerate(CriUint32 framerate_n, CriUint32 framerate_d);

	void SetCompareFrameTimeCallback(
		CriBool (*func)(CriMvEasyPlayer *mveasy, CriMvFrameInfo *frameinfo, CriUint64 count, CriUint64 unit, void *usrobj), 
		void *usrobj, CriError &err=CriMv::ErrorContainer);
	void SetSeekFrameAndOffset(CriSint32 seek_frame_id, CriUint64 seek_byte, CriError &err=CriMv::ErrorContainer);

	enum InputMode {
		MVEASY_INPUT_UNDEFINED,
		MVEASY_INPUT_STREAMING,		/* メモリストリーミング */
		MVEASY_INPUT_MEMORY,		/* メモリ直接参照（ユニＳＪ） */

		/* Keep enum 4bytes */
		MVEASY_INPUT_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	/* 実験中: この関数のために InputMode 定義を暫定でpublicへ移動 */
	void SetMemoryPlaybackType(InputMode memplay_type, CriError &err=CriMv::ErrorContainer);

	/* デコードスキップの自動実行モード */
	//void SetAutoSkipDecode(CriBool sw, CriFloat32 margin_msec, CriError &err=CriMv::ErrorContainer)

	/* ファイル要求の再コールバック要求 */
	void DeferFileRequest(CriError &err=CriMv::ErrorContainer);

	/* for specific use */
	void SetHeaderAndBodyData(const CriUint64Adr header_ptr, CriSint64 header_size, 
			const CriUint64Adr body_ptr, CriSint64 body_size);
	CriSint32 CalcFramepoolWorkSize(const CriMvStreamingParameters *stmprm, CriError &  err = CriMv::ErrorContainer);
	void SetFramepoolWork(void *work, CriSint32 work_size, CriError &  err = CriMv::ErrorContainer);
	void SetFramepoolWorkAllocator(CriMvFramepoolWorkMallocFunc allocfunc, CriMvFramepoolWorkFreeFunc freefunc, void *usrobj);
	void SetCapacityOfPictureSize(CriSint32 video_picsize, CriSint32 alpha_picsize, CriError &err);
	void GetUsrCapacityOfPictureSize(CriSint32 *video_picsize, CriSint32 *alpha_picsize);
	void SetMaxMovieSize(CriUint32 max_width, CriUint32 max_height, CriError &err);
	void GetMaxMovieSize(CriUint32 *max_width, CriUint32 *max_height);
	CriBool PlaybackResourceAllocated() const;
	CriBool CanReusePlaybackResource(const CriMvStreamingParameters *stmprm) const;
	CriBool GetReusePlaybackResourceFlag() const;

private:
	/* CRI internal use only */

	void initializeHandleParameters(void);
	void initializeCompareFrameParameters(void);
	void resetHandleParameters(void);
	CriBool startInputAndDecoding(void);
	CriBool isEndReadFile(void);
	void reinputDataForLooping(void);
	void executeFileCloseServer(void);
	void supplyDataFromStreamer(void);
	void executeFileReadServer(void);
	void executeFileOpenServer(void);
	void executeCuePointServer(void);
	//void executeAutoSkipDecode(void);
	void executeWaitStatusServer(CriError &err = CriMv::ErrorContainer);
	void surveilTerminationInput(void);
	CriBool isAvailableCenterVoice(const CriMvStreamingParameters *stmprm);
	CriBool isAvailableSubAudio(const CriMvStreamingParameters *stmprm);
	CriBool attachCenterVoice(void);
	void detachCenterVoice(void);

	CriBool attachSubAudioHandle(CriHeap heap);
	void detachSubAudioHandle(void);

 	void getAudioTime(CriUint64 &out_count, CriUint64 &out_unit, CriUint64 s_count, CriUint64 s_unit);

	CriBool allocAndCreateModules(void);
	void startModules(void);
	void requestStopModules(void);
	CriBool closeFileIfOpening(void);
	CriBool isCompleteStopModules(void);
	void tryCleanupModules(CriMvPlyStatus mvstat);
	CriBool tryFreeAndDestroySubmodules(void);
	void freeAndDestroyModules(void);

	CriUint32 adjustNumTrackAudioOut(void);

	/* ストリーミング用のパラメータ取得 */
	/* GetMovieInfo()との違いはユーザ指定値がどこまで反映されるか。
	 * 例えば、最大チャンクサイズはこの関数ではユーザ指定値をとるが、GetMovieInfoだとファイルの値。
	 * この関数は、内部で下位モジュール作成およびメモリ確保する時に使う。 */
	CriBool GetStreamingParameters(CriMvStreamingParameters &stmprm, CriError &err=CriMv::ErrorContainer);

	void setNormalErrorStatus(const CriChar8 *errmsg);

	CriBool compareFrameTimeSimple(CriMvEasyPlayer *mveasy, CriMvFrameInfo *frameinfo, CriUint64 count, CriUint64 unit, void *usrobj);
	static CriBool compareFrameTimeFluctuation(CriMvEasyPlayer *mveasy, CriMvFrameInfo *frameinfo, CriUint64 count, CriUint64 unit, void *usrobj);

	Bool isNextFrameOnTime(CriBool update_stats, CriError &err); 
	Bool checkFrameTime(CriMvFrameInfo *frameinfo);
	void updateGetFrameInfo(CriBool time_ready, CriBool acquired_frame, CriBool discard_frame, CriUint64 frame_count, CriUint64 frame_unit);

	/* for DEBUG */
	void crimveasy_SetSeekInfo(void);
	
	void executeUpdate(CriError &err);

private:
	static CriUint32 crimveasy_SupplyPcmDataByFloat32(void *obj, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_SupplyPcmDataBySint16(void *obj, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_CalcAvailableNumSmpls(CriMvEasyPlayer *mveasy);
	static CriUint32 crimveasy_GetWave16(CriMvEasyPlayer *mveasy, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_GetWave32(CriMvEasyPlayer *mveasy, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl);

	static CriUint32 crimveasy_SupplySubAudioDataByFloat32(void *obj, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_GetSubAudioWave32(CriMvEasyPlayer *mveasy, CriUint32 nch, CriFloat32 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_SupplySubAudioDataBySint16(void *obj, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl);
	static CriUint32 crimveasy_GetSubAudioWave16(CriMvEasyPlayer *mveasy, CriUint32 nch, CriSint16 *pcmbuf[], CriUint32 req_nsmpl);

public:
	CriMvPly					mvply;	/* Temporally allowed to access for debug */

private:
	CriHeap						heap;
	CriBool						user_stmprm_flag;
	CriMvStreamingParameters	stmprm;
	CriBool						alloced_submodules_flag;
	CriMvFrameInfo				frameinfo;
	CriMvAlphaFrameInfo			alpha_frame;
	CriMvFileReaderInterface	*freader;
	CriMvSoundInterface			*sndout;
	CriMvSystemTimerInterface	*stimer;
	TimerType	timertype;
	Status		pre_ezstat;
	CriBool		req_decode_header_flag;
	CriBool		req_prepare_flag;
	CriBool		pause_flag;
	CriUint32	npools;
	CriSint32	track_play_audio;
	CriUint32	num_track_audio_data;
	CriUint32	num_track_audio_out;
	CriBool		loop_flag;
	CriUint64	time_count;
	CriUint64	time_unit;
	CriSint64	total_read;
	CriSint64	fsize_byte;
	CriChunk	read_crick;
	CriBool		exe_open;
	CriBool		exe_close;
	CriBool		exe_read;

	CriUint64		time_syslog_count;		/* システムタイマの記録 */
	CriUint64		time_syslog_unit;
	CriUint64		time_ofs_count;			/* オーディオ終了時のシステムタイマ */
	CriUint64		time_ofs_unit;
	CriUint64		time_prev_audio_count;	/* オーディオ時刻変化チェック用 */
	CriUint64		time_prev_audio_unit;

	CriFloat32		user_buffering_sec;		/* 0.0f means AUTO */
	CriUint32		user_max_bitrate;		/* 0 means AUTO */
	CriUint32		user_max_chunk_size;	/* 0 means AUTO */
	CriUint32		user_min_buffer_size;	/* 0 means AUTO */
	CriSint32		user_read_buffer_size;	/* CRIMV_READ_BUFFER_SIZE_AUTO means AUTO */
	CriSint32		user_video_capacity_of_picsize;	/* 0 means AUTO */
	CriSint32		user_alpha_capacity_of_picsize;	/* 0 means AUTO */
	CriUint32		user_max_width;
	CriUint32		user_max_height;

	CriSint32			usr_subtitle_channel;
	CriMvSubtitleInfo	sbtinfo;
	CriFloat32			sbt_start_msec;
	CriFloat32			sbt_end_msec;
	CriUint8			*sbtbuf;

	enum InputSrc {
		MVEASY_INPUT_SRC_UNDEFINED,
		MVEASY_INPUT_SRC_FILE,
		MVEASY_INPUT_SRC_MEMORY,
		/* Keep enum 4bytes */
		MVEASY_INPUT_SRC_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	InputSrc                 input_src;
	CriMvFileReaderInterface *ext_reader;
	CriMvFileReaderInterface *mem_reader;

	enum InputSupplyStatus {
		MVEASY_SUPPLY_STOP,
		MVEASY_SUPPLY_REQ_OPEN,
		MVEASY_SUPPLY_OPENING,
		MVEASY_SUPPLY_READING,
		MVEASY_SUPPLY_REQ_CLOSE,
		MVEASY_SUPPLY_CLOSING,

		/* Keep enum 4bytes */
		MVEASY_SUPPLY_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};

	InputMode			memplay_type;	/* メモリ再生をストリームするかユニＳＪするか */
	InputMode			input_mode;
	InputSupplyStatus	supply_stat;
	CriBool				change_file_mode;
	CriChar8			file_name[CRIMV_MAX_FILE_NAME];
	CriUint64			file_offset;
	CriSint64			file_range;
	CriUint8			*dataptr;
	CriUint32			datasize;
	CriBool				terminate_flag;
	CriSint32			reinput_cnt;

	enum NextEntryState {
		MVEASY_NEXT_ENTRY_NONE,
		MVEASY_NEXT_ENTRY_READY,
		MVEASY_NEXT_ENTRY_DEFER,
		/* Keep enum 4bytes */
		MVEASY_NEXT_ENTRY_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	NextEntryState	next_entry_state;

	CriUint32		center_ch;
	CriSint32		usr_voice_track;			// by SetCenterVoice()
	CriSint32		voice_attached_track;

	// Sub Audio 
	CriMvSoundInterface		*if_subaudio;
	CriSint32				usr_subaudio_track;
	CriSint32				attached_subaudio_track;

	void (*cbfunc_file_request)(CriMvEasyPlayer *mveasy, void *usrobj);
	void *usrobj_file_request;

	CriBool (*cbfunc_compare_ftime)(CriMvEasyPlayer *mveasy, CriMvFrameInfo *frameinfo, CriUint64 count, CriUint64 unit, void *usrobj);
	void *usrobj_compare_ftime;

	/* Seek Playback */
	CriSint32		seek_frame_id;
	CriUint64		seek_byte;

	/* CuePoint */
	void (*cbfunc_cuepoint)(CriMvEasyPlayer *mveasy, CriMvEventPoint *eventinfo, void *usrobj);
	void *usrobj_cuepoint;

	/* Playback Statistics */
	CriBool   start_getfrm;
	CriUint64 last_getfrm_count;
	CriFloat32 sum_diff_time;
	CriMvPlaybackInfo playinfo;
	CriMvLastFrameResult last_frm_result;

	/* Block flag for simultanious calls on multi-threads */
	CriSint32 execute_decode_block_flag;
	CriSint32 update_block_flag;

	/* Stop Completion Notification */
	void (*cbfunc_stopcomplete)(CriMvEasyPlayer *mveasy, void *usrobj);
	void *usrobj_stopcomplete;	

	/* For Sofdec2 */
	void *playback_work;
	CriSint32 playback_work_size;
	void *mvply_work;
	CriHeap	heap_mveasy;
	CriHeap heap_extra_sound;
	CriBool reuse_modules_flag;

	/* For Debug */
	volatile CriUint8 end_sequence_info;

	/* Handle Protection */
	void *cshn;
	void *cs_work;

	/* Skip Decoding */
	//CriBool skip_auto_flag;
	//CriFloat32 margin_msec;

public:
	enum FrameCompareMode {
		MVEASY_COMPARE_MODE_JUST,			/* 正確に時刻比較する     */
		MVEASY_COMPARE_MODE_DELAY_GET,		/* タイマ時刻を前倒し＝フレームはなるべく渡さない */
		MVEASY_COMPARE_MODE_FAST_GET,		/* タイマ時刻を水増し＝フレームはなるべく渡す     */

		/* Keep enum 4bytes */
		MVEASY_COMPARE_MODE_MAKE_ENUM_SINT32 = 0x7FFFFFFF
	};
	FrameCompareMode	compare_mode;
	CriFloat32			accuracy_system_tmr_msec;	/* システム時刻精度   milli sec  */
	CriFloat32			accuracy_audio_tmr_msec;	/* オーディオ時刻精度 milli sec  */
	CriFloat32			fluctuation_system;			/* システム（SyncFrame)の揺らぎ milli sec */	
	CriFloat32			fluctuation_adjust;			/* 揺らぎ補正 */
//	CriFloat32			fluctuation_system_msec;	/* システム時刻ゆらぎ幅   milli sec  */
//	CriFloat32			fluctuation_audio_msec;		/* オーディオ時刻ゆらぎ幅 milli sec  */
//	CriFloat32			fluctuation_adjust;			/* ゆらぎ補正倍率         */
//	CriSint32			fluctuation_system_usec;	/* システム時刻ゆらぎ幅   micro sec */
//	CriSint32			fluctuation_audio_usec;		/* オーディオ時刻ゆらぎ幅 micro sec */
//	CriFloat32			fluctuation_adjust_multi;	/* ゆらぎ補正倍率         */
//	CriSint32			fluctuation_adjust_add;		/* ゆらぎ補正オフセット   */

private:
	CriBool		req_stop_modules;
	CriBool		error_flag;
	CriBool		req_start_modules;

protected:
	CriMvEasyPlayer(CriHeap heap,
					CriMvFileReaderInterface *freader, 
					CriMvSystemTimerInterface *stimer, 
					CriMvSoundInterface *sound, 
					CriError &err=CriMv::ErrorContainer);
	virtual ~CriMvEasyPlayer();

private:
	CriMvEasyPlayer(void);	//disabled
};




#endif	/* CRI_MOVIE_H_INCLUDED */
