
#ifndef GFX_TSFUIELEMENTDEF_H
#define GFX_TSFUIELEMENTDEF_H

EXTERN_C const IID IID_ITfUIElementMgr_GFx;
EXTERN_C const IID IID_ITfUIElementSink_GFx;
EXTERN_C const IID IID_ITfCandidateListUIElementBehavior_GFx;
EXTERN_C const IID IID_ITfThreadMgrEx_GFx;
EXTERN_C const IID IID_ITfInputProcessorProfileActivationSink_GFx;
EXTERN_C const IID IID_ITfInputProcessorProfileMgr_GFx;

/*
This file contains the additional interface definitions that are needed to compile TSF code. 
Visual Studio 2003 already contains a msctf.h that contains most of the TSF related 
definitions. This file provides the rest of the definitions. If you have downloaded platform
SDK, the msctf.h file should contain all the interface definitions and you don't need this 
file.
*/
#ifndef __IEnumTfInputProcessorProfiles_FWD_DEFINED__
// same header file that defines the above symbol also creates this struct definition.
#ifndef __TF_INPUTPROCESSORPROFILE_GFX
#define __TF_INPUTPROCESSORPROFILE_GFX

typedef /* [uuid] */  DECLSPEC_UUID("44d2825a-10e5-43b2-877f-6cb2f43b7e7e") struct TF_INPUTPROCESSORPROFILE
{
    DWORD dwProfileType;
    LANGID langid;
    CLSID clsid;
    GUID guidProfile;
    GUID catid;
    HKL hklSubstitute;
    DWORD dwCaps;
    HKL hkl;
    DWORD dwFlags;
}   TF_INPUTPROCESSORPROFILE;
#endif
#endif



#define TF_PROFILETYPE_INPUTPROCESSOR       0x0001
#define TF_PROFILETYPE_KEYBOARDLAYOUT       0x0002

#define TF_CONVERSIONMODE_ALPHANUMERIC      0x0000
#define TF_CONVERSIONMODE_NATIVE            0x0001
#define TF_CONVERSIONMODE_KATAKANA          0x0002
#define TF_CONVERSIONMODE_FULLSHAPE         0x0008
#define TF_CONVERSIONMODE_ROMAN             0x0010
#define TF_CONVERSIONMODE_CHARCODE          0x0020
#define TF_CONVERSIONMODE_SOFTKEYBOARD      0x0080
#define TF_CONVERSIONMODE_NOCONVERSION      0x0100
#define TF_CONVERSIONMODE_EUDC              0x0200
#define TF_CONVERSIONMODE_SYMBOL            0x0400
#define TF_CONVERSIONMODE_FIXED             0x0800
#define TF_IPPMF_FORPROCESS                 0x10000000
#define TF_IPP_FLAG_ACTIVE                  0x00000001
#define TF_IPP_FLAG_ENABLED                 0x00000002

#define LOCALE_NAME_USER_DEFAULT            NULL

#ifndef __ITfUIElementMgr_FWD_DEFINED__
#define __ITfUIElementMgr_FWD_DEFINED__
typedef interface ITfUIElementMgr ITfUIElementMgr;
#endif  /* __ITfUIElementMgr_FWD_DEFINED__ */


#ifndef __IEnumTfUIElements_FWD_DEFINED__
#define __IEnumTfUIElements_FWD_DEFINED__
typedef interface IEnumTfUIElements IEnumTfUIElements;
#endif  /* __IEnumTfUIElements_FWD_DEFINED__ */


#ifndef __ITfUIElementSink_FWD_DEFINED__
//#define __ITfUIElementSink_FWD_DEFINED__
typedef interface ITfUIElementSink ITfUIElementSink;
#endif  /* __ITfUIElementSink_FWD_DEFINED__ */


#ifndef __ITfUIElement_FWD_DEFINED__
#define __ITfUIElement_FWD_DEFINED__
typedef interface ITfUIElement ITfUIElement;
#endif  /* __ITfUIElement_FWD_DEFINED__ */


#ifndef __ITfCandidateListUIElement_FWD_DEFINED__
#define __ITfCandidateListUIElement_FWD_DEFINED__
typedef interface ITfCandidateListUIElement ITfCandidateListUIElement;
#endif  /* __ITfCandidateListUIElement_FWD_DEFINED__ */


#ifndef __ITfCandidateListUIElementBehavior_FWD_DEFINED__
#define __ITfCandidateListUIElementBehavior_FWD_DEFINED__
typedef interface ITfCandidateListUIElementBehavior ITfCandidateListUIElementBehavior;
#endif  /* __ITfCandidateListUIElementBehavior_FWD_DEFINED__ */


#ifndef __ITfReadingInformationUIElement_FWD_DEFINED__
#define __ITfReadingInformationUIElement_FWD_DEFINED__
typedef interface ITfReadingInformationUIElement ITfReadingInformationUIElement;
#endif  /* __ITfReadingInformationUIElement_FWD_DEFINED__ */

#ifndef __ITfThreadMgrEx_FWD_DEFINED__
#define __ITfThreadMgrEx_FWD_DEFINED__
typedef interface ITfThreadMgrEx ITfThreadMgrEx;
#endif  /* __ITfThreadMgrEx_FWD_DEFINED__ */


#ifndef __ITfThreadMgrEventSink_FWD_DEFINED__
#define __ITfThreadMgrEventSink_FWD_DEFINED__
typedef interface ITfThreadMgrEventSink ITfThreadMgrEventSink;
#endif  /* __ITfThreadMgrEventSink_FWD_DEFINED__ */


#ifndef __ITfUIElementMgr_INTERFACE_DEFINED__
#define __ITfUIElementMgr_INTERFACE_DEFINED__

/* interface ITfUIElementMgr */
/* [unique][uuid][local][object] */ 




#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea1ea135-19df-11d7-a6d2-00065b84435c")
    ITfUIElementMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginUIElement( 
            /* [in] */ ITfUIElement *pElement,
            /* [out][in] */ BOOL *pbShow,
            /* [out] */ DWORD *pdwUIElementId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateUIElement( 
            /* [in] */ DWORD dwUIElementId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndUIElement( 
            /* [in] */ DWORD dwUIElementId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetUIElement( 
            /* [in] */ DWORD dwUIELementId,
            /* [out] */ ITfUIElement **ppElement) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumUIElements( 
            /* [out] */ IEnumTfUIElements **ppEnum) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfUIElementMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfUIElementMgr * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfUIElementMgr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfUIElementMgr * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginUIElement )( 
            ITfUIElementMgr * This,
            /* [in] */ ITfUIElement *pElement,
            /* [out][in] */ BOOL *pbShow,
            /* [out] */ DWORD *pdwUIElementId);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateUIElement )( 
            ITfUIElementMgr * This,
            /* [in] */ DWORD dwUIElementId);
        
        HRESULT ( STDMETHODCALLTYPE *EndUIElement )( 
            ITfUIElementMgr * This,
            /* [in] */ DWORD dwUIElementId);
        
        HRESULT ( STDMETHODCALLTYPE *GetUIElement )( 
            ITfUIElementMgr * This,
            /* [in] */ DWORD dwUIELementId,
            /* [out] */ ITfUIElement **ppElement);
        
        HRESULT ( STDMETHODCALLTYPE *EnumUIElements )( 
            ITfUIElementMgr * This,
            /* [out] */ IEnumTfUIElements **ppEnum);
        
        END_INTERFACE
    } ITfUIElementMgrVtbl;

    interface ITfUIElementMgr
    {
        CONST_VTBL struct ITfUIElementMgrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfUIElementMgr_QueryInterface(This,riid,ppvObject) \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfUIElementMgr_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfUIElementMgr_Release(This)   \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfUIElementMgr_BeginUIElement(This,pElement,pbShow,pdwUIElementId) \
    ( (This)->lpVtbl -> BeginUIElement(This,pElement,pbShow,pdwUIElementId) ) 

#define ITfUIElementMgr_UpdateUIElement(This,dwUIElementId) \
    ( (This)->lpVtbl -> UpdateUIElement(This,dwUIElementId) ) 

#define ITfUIElementMgr_EndUIElement(This,dwUIElementId)    \
    ( (This)->lpVtbl -> EndUIElement(This,dwUIElementId) ) 

#define ITfUIElementMgr_GetUIElement(This,dwUIELementId,ppElement)  \
    ( (This)->lpVtbl -> GetUIElement(This,dwUIELementId,ppElement) ) 

#define ITfUIElementMgr_EnumUIElements(This,ppEnum) \
    ( (This)->lpVtbl -> EnumUIElements(This,ppEnum) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfUIElementMgr_INTERFACE_DEFINED__ */


#ifndef __IEnumTfUIElements_INTERFACE_DEFINED__
#define __IEnumTfUIElements_INTERFACE_DEFINED__

/* interface IEnumTfUIElements */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_IEnumTfUIElements;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("887aa91e-acba-4931-84da-3c5208cf543f")
    IEnumTfUIElements : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ IEnumTfUIElements **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [length_is][size_is][out] */ ITfUIElement **ppElement,
            /* [out] */ ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulCount) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct IEnumTfUIElementsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTfUIElements * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTfUIElements * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTfUIElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTfUIElements * This,
            /* [out] */ IEnumTfUIElements **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTfUIElements * This,
            /* [in] */ ULONG ulCount,
            /* [length_is][size_is][out] */ ITfUIElement **ppElement,
            /* [out] */ ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTfUIElements * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTfUIElements * This,
            /* [in] */ ULONG ulCount);
        
        END_INTERFACE
    } IEnumTfUIElementsVtbl;

    interface IEnumTfUIElements
    {
        CONST_VTBL struct IEnumTfUIElementsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTfUIElements_QueryInterface(This,riid,ppvObject)   \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEnumTfUIElements_AddRef(This)  \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEnumTfUIElements_Release(This) \
    ( (This)->lpVtbl -> Release(This) ) 


#define IEnumTfUIElements_Clone(This,ppEnum)    \
    ( (This)->lpVtbl -> Clone(This,ppEnum) ) 

#define IEnumTfUIElements_Next(This,ulCount,ppElement,pcFetched)    \
    ( (This)->lpVtbl -> Next(This,ulCount,ppElement,pcFetched) ) 

#define IEnumTfUIElements_Reset(This)   \
    ( (This)->lpVtbl -> Reset(This) ) 

#define IEnumTfUIElements_Skip(This,ulCount)    \
    ( (This)->lpVtbl -> Skip(This,ulCount) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __IEnumTfUIElements_INTERFACE_DEFINED__ */


#ifndef __ITfUIElementSink_INTERFACE_DEFINED__
#define __ITfUIElementSink_INTERFACE_DEFINED__

/* interface ITfUIElementSink */
/* [unique][uuid][local][object] */ 




#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea1ea136-19df-11d7-a6d2-00065b84435c")
    ITfUIElementSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE BeginUIElement( 
            /* [in] */ DWORD dwUIElementId,
            /* [out][in] */ BOOL *pbShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateUIElement( 
            /* [in] */ DWORD dwUIElementId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndUIElement( 
            /* [in] */ DWORD dwUIElementId) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfUIElementSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfUIElementSink * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfUIElementSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfUIElementSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginUIElement )( 
            ITfUIElementSink * This,
            /* [in] */ DWORD dwUIElementId,
            /* [out][in] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateUIElement )( 
            ITfUIElementSink * This,
            /* [in] */ DWORD dwUIElementId);
        
        HRESULT ( STDMETHODCALLTYPE *EndUIElement )( 
            ITfUIElementSink * This,
            /* [in] */ DWORD dwUIElementId);
        
        END_INTERFACE
    } ITfUIElementSinkVtbl;

    interface ITfUIElementSink
    {
        CONST_VTBL struct ITfUIElementSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfUIElementSink_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfUIElementSink_AddRef(This)   \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfUIElementSink_Release(This)  \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfUIElementSink_BeginUIElement(This,dwUIElementId,pbShow)  \
    ( (This)->lpVtbl -> BeginUIElement(This,dwUIElementId,pbShow) ) 

#define ITfUIElementSink_UpdateUIElement(This,dwUIElementId)    \
    ( (This)->lpVtbl -> UpdateUIElement(This,dwUIElementId) ) 

#define ITfUIElementSink_EndUIElement(This,dwUIElementId)   \
    ( (This)->lpVtbl -> EndUIElement(This,dwUIElementId) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfUIElementSink_INTERFACE_DEFINED__ */


#ifndef __ITfUIElement_INTERFACE_DEFINED__
#define __ITfUIElement_INTERFACE_DEFINED__

/* interface ITfUIElement */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfUIElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea1ea137-19df-11d7-a6d2-00065b84435c")
    ITfUIElement : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [out] */ BSTR *pbstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGUID( 
            /* [out] */ GUID *pguid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Show( 
            /* [in] */ BOOL bShow) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsShown( 
            /* [out] */ BOOL *pbShow) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfUIElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfUIElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfUIElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfUIElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfUIElement * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfUIElement * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfUIElement * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfUIElement * This,
            /* [out] */ BOOL *pbShow);
        
        END_INTERFACE
    } ITfUIElementVtbl;

    interface ITfUIElement
    {
        CONST_VTBL struct ITfUIElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfUIElement_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfUIElement_AddRef(This)   \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfUIElement_Release(This)  \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfUIElement_GetDescription(This,pbstrDescription)  \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfUIElement_GetGUID(This,pguid)    \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfUIElement_Show(This,bShow)   \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfUIElement_IsShown(This,pbShow)   \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfUIElement_INTERFACE_DEFINED__ */


#ifndef __ITfCandidateListUIElement_INTERFACE_DEFINED__
#define __ITfCandidateListUIElement_INTERFACE_DEFINED__

/* interface ITfCandidateListUIElement */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfCandidateListUIElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea1ea138-19df-11d7-a6d2-00065b84435c")
    ITfCandidateListUIElement : public ITfUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUpdatedFlags( 
            /* [out] */ DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDocumentMgr( 
            /* [out] */ ITfDocumentMgr **ppdim) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCount( 
            /* [out] */ UINT *puCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSelection( 
            /* [out] */ UINT *puIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetString( 
            /* [in] */ UINT uIndex,
            /* [out] */ BSTR *pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPageIndex( 
            /* [length_is][size_is][out] */ UINT *pIndex,
            /* [in] */ UINT uSize,
            /* [out] */ UINT *puPageCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPageIndex( 
            /* [size_is][in] */ UINT *pIndex,
            /* [in] */ UINT uPageCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentPage( 
            /* [out] */ UINT *puPage) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfCandidateListUIElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfCandidateListUIElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfCandidateListUIElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfCandidateListUIElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfCandidateListUIElement * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfCandidateListUIElement * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfCandidateListUIElement * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfCandidateListUIElement * This,
            /* [out] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetUpdatedFlags )( 
            ITfCandidateListUIElement * This,
            /* [out] */ DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentMgr )( 
            ITfCandidateListUIElement * This,
            /* [out] */ ITfDocumentMgr **ppdim);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ITfCandidateListUIElement * This,
            /* [out] */ UINT *puCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelection )( 
            ITfCandidateListUIElement * This,
            /* [out] */ UINT *puIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetString )( 
            ITfCandidateListUIElement * This,
            /* [in] */ UINT uIndex,
            /* [out] */ BSTR *pstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageIndex )( 
            ITfCandidateListUIElement * This,
            /* [length_is][size_is][out] */ UINT *pIndex,
            /* [in] */ UINT uSize,
            /* [out] */ UINT *puPageCnt);
        
        HRESULT ( STDMETHODCALLTYPE *SetPageIndex )( 
            ITfCandidateListUIElement * This,
            /* [size_is][in] */ UINT *pIndex,
            /* [in] */ UINT uPageCnt);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentPage )( 
            ITfCandidateListUIElement * This,
            /* [out] */ UINT *puPage);
        
        END_INTERFACE
    } ITfCandidateListUIElementVtbl;

    interface ITfCandidateListUIElement
    {
        CONST_VTBL struct ITfCandidateListUIElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfCandidateListUIElement_QueryInterface(This,riid,ppvObject)   \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfCandidateListUIElement_AddRef(This)  \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfCandidateListUIElement_Release(This) \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfCandidateListUIElement_GetDescription(This,pbstrDescription) \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfCandidateListUIElement_GetGUID(This,pguid)   \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfCandidateListUIElement_Show(This,bShow)  \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfCandidateListUIElement_IsShown(This,pbShow)  \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 


#define ITfCandidateListUIElement_GetUpdatedFlags(This,pdwFlags)    \
    ( (This)->lpVtbl -> GetUpdatedFlags(This,pdwFlags) ) 

#define ITfCandidateListUIElement_GetDocumentMgr(This,ppdim)    \
    ( (This)->lpVtbl -> GetDocumentMgr(This,ppdim) ) 

#define ITfCandidateListUIElement_GetCount(This,puCount)    \
    ( (This)->lpVtbl -> GetCount(This,puCount) ) 

#define ITfCandidateListUIElement_GetSelection(This,puIndex)    \
    ( (This)->lpVtbl -> GetSelection(This,puIndex) ) 

#define ITfCandidateListUIElement_GetString(This,uIndex,pstr)   \
    ( (This)->lpVtbl -> GetString(This,uIndex,pstr) ) 

#define ITfCandidateListUIElement_GetPageIndex(This,pIndex,uSize,puPageCnt) \
    ( (This)->lpVtbl -> GetPageIndex(This,pIndex,uSize,puPageCnt) ) 

#define ITfCandidateListUIElement_SetPageIndex(This,pIndex,uPageCnt)    \
    ( (This)->lpVtbl -> SetPageIndex(This,pIndex,uPageCnt) ) 

#define ITfCandidateListUIElement_GetCurrentPage(This,puPage)   \
    ( (This)->lpVtbl -> GetCurrentPage(This,puPage) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfCandidateListUIElement_INTERFACE_DEFINED__ */


#ifndef __ITfCandidateListUIElementBehavior_INTERFACE_DEFINED__
#define __ITfCandidateListUIElementBehavior_INTERFACE_DEFINED__

/* interface ITfCandidateListUIElementBehavior */
/* [unique][uuid][local][object] */ 


#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("85fad185-58ce-497a-9460-355366b64b9a")
    ITfCandidateListUIElementBehavior : public ITfCandidateListUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetSelection( 
            /* [in] */ UINT nIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Finalize( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfCandidateListUIElementBehaviorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfCandidateListUIElementBehavior * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfCandidateListUIElementBehavior * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfCandidateListUIElementBehavior * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfCandidateListUIElementBehavior * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetUpdatedFlags )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentMgr )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ ITfDocumentMgr **ppdim);
        
        HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ UINT *puCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetSelection )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ UINT *puIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetString )( 
            ITfCandidateListUIElementBehavior * This,
            /* [in] */ UINT uIndex,
            /* [out] */ BSTR *pstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageIndex )( 
            ITfCandidateListUIElementBehavior * This,
            /* [length_is][size_is][out] */ UINT *pIndex,
            /* [in] */ UINT uSize,
            /* [out] */ UINT *puPageCnt);
        
        HRESULT ( STDMETHODCALLTYPE *SetPageIndex )( 
            ITfCandidateListUIElementBehavior * This,
            /* [size_is][in] */ UINT *pIndex,
            /* [in] */ UINT uPageCnt);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentPage )( 
            ITfCandidateListUIElementBehavior * This,
            /* [out] */ UINT *puPage);
        
        HRESULT ( STDMETHODCALLTYPE *SetSelection )( 
            ITfCandidateListUIElementBehavior * This,
            /* [in] */ UINT nIndex);
        
        HRESULT ( STDMETHODCALLTYPE *Finalize )( 
            ITfCandidateListUIElementBehavior * This);
        
        HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ITfCandidateListUIElementBehavior * This);
        
        END_INTERFACE
    } ITfCandidateListUIElementBehaviorVtbl;

    interface ITfCandidateListUIElementBehavior
    {
        CONST_VTBL struct ITfCandidateListUIElementBehaviorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfCandidateListUIElementBehavior_QueryInterface(This,riid,ppvObject)   \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfCandidateListUIElementBehavior_AddRef(This)  \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfCandidateListUIElementBehavior_Release(This) \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfCandidateListUIElementBehavior_GetDescription(This,pbstrDescription) \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfCandidateListUIElementBehavior_GetGUID(This,pguid)   \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfCandidateListUIElementBehavior_Show(This,bShow)  \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfCandidateListUIElementBehavior_IsShown(This,pbShow)  \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 


#define ITfCandidateListUIElementBehavior_GetUpdatedFlags(This,pdwFlags)    \
    ( (This)->lpVtbl -> GetUpdatedFlags(This,pdwFlags) ) 

#define ITfCandidateListUIElementBehavior_GetDocumentMgr(This,ppdim)    \
    ( (This)->lpVtbl -> GetDocumentMgr(This,ppdim) ) 

#define ITfCandidateListUIElementBehavior_GetCount(This,puCount)    \
    ( (This)->lpVtbl -> GetCount(This,puCount) ) 

#define ITfCandidateListUIElementBehavior_GetSelection(This,puIndex)    \
    ( (This)->lpVtbl -> GetSelection(This,puIndex) ) 

#define ITfCandidateListUIElementBehavior_GetString(This,uIndex,pstr)   \
    ( (This)->lpVtbl -> GetString(This,uIndex,pstr) ) 

#define ITfCandidateListUIElementBehavior_GetPageIndex(This,pIndex,uSize,puPageCnt) \
    ( (This)->lpVtbl -> GetPageIndex(This,pIndex,uSize,puPageCnt) ) 

#define ITfCandidateListUIElementBehavior_SetPageIndex(This,pIndex,uPageCnt)    \
    ( (This)->lpVtbl -> SetPageIndex(This,pIndex,uPageCnt) ) 

#define ITfCandidateListUIElementBehavior_GetCurrentPage(This,puPage)   \
    ( (This)->lpVtbl -> GetCurrentPage(This,puPage) ) 


#define ITfCandidateListUIElementBehavior_SetSelection(This,nIndex) \
    ( (This)->lpVtbl -> SetSelection(This,nIndex) ) 

#define ITfCandidateListUIElementBehavior_Finalize(This)    \
    ( (This)->lpVtbl -> Finalize(This) ) 

#define ITfCandidateListUIElementBehavior_Abort(This)   \
    ( (This)->lpVtbl -> Abort(This) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfCandidateListUIElementBehavior_INTERFACE_DEFINED__ */


#ifndef __ITfReadingInformationUIElement_INTERFACE_DEFINED__
#define __ITfReadingInformationUIElement_INTERFACE_DEFINED__

/* interface ITfReadingInformationUIElement */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfReadingInformationUIElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ea1ea139-19df-11d7-a6d2-00065b84435c")
    ITfReadingInformationUIElement : public ITfUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUpdatedFlags( 
            /* [out] */ DWORD *pdwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContext( 
            /* [out] */ ITfContext **ppic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetString( 
            /* [out] */ BSTR *pstr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxReadingStringLength( 
            /* [out] */ UINT *pcchMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorIndex( 
            /* [out] */ UINT *pErrorIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsVerticalOrderPreferred( 
            /* [out] */ BOOL *pfVertical) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfReadingInformationUIElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfReadingInformationUIElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfReadingInformationUIElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfReadingInformationUIElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfReadingInformationUIElement * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetUpdatedFlags )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ DWORD *pdwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ ITfContext **ppic);
        
        HRESULT ( STDMETHODCALLTYPE *GetString )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ BSTR *pstr);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxReadingStringLength )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ UINT *pcchMax);
        
        HRESULT ( STDMETHODCALLTYPE *GetErrorIndex )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ UINT *pErrorIndex);
        
        HRESULT ( STDMETHODCALLTYPE *IsVerticalOrderPreferred )( 
            ITfReadingInformationUIElement * This,
            /* [out] */ BOOL *pfVertical);
        
        END_INTERFACE
    } ITfReadingInformationUIElementVtbl;

    interface ITfReadingInformationUIElement
    {
        CONST_VTBL struct ITfReadingInformationUIElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfReadingInformationUIElement_QueryInterface(This,riid,ppvObject)  \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfReadingInformationUIElement_AddRef(This) \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfReadingInformationUIElement_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfReadingInformationUIElement_GetDescription(This,pbstrDescription)    \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfReadingInformationUIElement_GetGUID(This,pguid)  \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfReadingInformationUIElement_Show(This,bShow) \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfReadingInformationUIElement_IsShown(This,pbShow) \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 


#define ITfReadingInformationUIElement_GetUpdatedFlags(This,pdwFlags)   \
    ( (This)->lpVtbl -> GetUpdatedFlags(This,pdwFlags) ) 

#define ITfReadingInformationUIElement_GetContext(This,ppic)    \
    ( (This)->lpVtbl -> GetContext(This,ppic) ) 

#define ITfReadingInformationUIElement_GetString(This,pstr) \
    ( (This)->lpVtbl -> GetString(This,pstr) ) 

#define ITfReadingInformationUIElement_GetMaxReadingStringLength(This,pcchMax)  \
    ( (This)->lpVtbl -> GetMaxReadingStringLength(This,pcchMax) ) 

#define ITfReadingInformationUIElement_GetErrorIndex(This,pErrorIndex)  \
    ( (This)->lpVtbl -> GetErrorIndex(This,pErrorIndex) ) 

#define ITfReadingInformationUIElement_IsVerticalOrderPreferred(This,pfVertical)    \
    ( (This)->lpVtbl -> IsVerticalOrderPreferred(This,pfVertical) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfReadingInformationUIElement_INTERFACE_DEFINED__ */


#ifndef __ITfTransitoryExtensionUIElement_INTERFACE_DEFINED__
#define __ITfTransitoryExtensionUIElement_INTERFACE_DEFINED__

/* interface ITfTransitoryExtensionUIElement */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfTransitoryExtensionUIElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("858f956a-972f-42a2-a2f2-0321e1abe209")
    ITfTransitoryExtensionUIElement : public ITfUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDocumentMgr( 
            /* [out] */ ITfDocumentMgr **ppdim) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfTransitoryExtensionUIElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfTransitoryExtensionUIElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfTransitoryExtensionUIElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfTransitoryExtensionUIElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfTransitoryExtensionUIElement * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfTransitoryExtensionUIElement * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfTransitoryExtensionUIElement * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfTransitoryExtensionUIElement * This,
            /* [out] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetDocumentMgr )( 
            ITfTransitoryExtensionUIElement * This,
            /* [out] */ ITfDocumentMgr **ppdim);
        
        END_INTERFACE
    } ITfTransitoryExtensionUIElementVtbl;

    interface ITfTransitoryExtensionUIElement
    {
        CONST_VTBL struct ITfTransitoryExtensionUIElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfTransitoryExtensionUIElement_QueryInterface(This,riid,ppvObject) \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfTransitoryExtensionUIElement_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfTransitoryExtensionUIElement_Release(This)   \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfTransitoryExtensionUIElement_GetDescription(This,pbstrDescription)   \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfTransitoryExtensionUIElement_GetGUID(This,pguid) \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfTransitoryExtensionUIElement_Show(This,bShow)    \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfTransitoryExtensionUIElement_IsShown(This,pbShow)    \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 


#define ITfTransitoryExtensionUIElement_GetDocumentMgr(This,ppdim)  \
    ( (This)->lpVtbl -> GetDocumentMgr(This,ppdim) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfTransitoryExtensionUIElement_INTERFACE_DEFINED__ */


#ifndef __ITfTransitoryExtensionSink_INTERFACE_DEFINED__
#define __ITfTransitoryExtensionSink_INTERFACE_DEFINED__

/* interface ITfTransitoryExtensionSink */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfTransitoryExtensionSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a615096f-1c57-4813-8a15-55ee6e5a839c")
    ITfTransitoryExtensionSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnTransitoryExtensionUpdated( 
            /* [in] */ ITfContext *pic,
            /* [in] */ TfEditCookie ecReadOnly,
            /* [in] */ ITfRange *pResultRange,
            /* [in] */ ITfRange *pCompositionRange,
            /* [out] */ BOOL *pfDeleteResultRange) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfTransitoryExtensionSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfTransitoryExtensionSink * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfTransitoryExtensionSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfTransitoryExtensionSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransitoryExtensionUpdated )( 
            ITfTransitoryExtensionSink * This,
            /* [in] */ ITfContext *pic,
            /* [in] */ TfEditCookie ecReadOnly,
            /* [in] */ ITfRange *pResultRange,
            /* [in] */ ITfRange *pCompositionRange,
            /* [out] */ BOOL *pfDeleteResultRange);
        
        END_INTERFACE
    } ITfTransitoryExtensionSinkVtbl;

    interface ITfTransitoryExtensionSink
    {
        CONST_VTBL struct ITfTransitoryExtensionSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfTransitoryExtensionSink_QueryInterface(This,riid,ppvObject)  \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfTransitoryExtensionSink_AddRef(This) \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfTransitoryExtensionSink_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfTransitoryExtensionSink_OnTransitoryExtensionUpdated(This,pic,ecReadOnly,pResultRange,pCompositionRange,pfDeleteResultRange) \
    ( (This)->lpVtbl -> OnTransitoryExtensionUpdated(This,pic,ecReadOnly,pResultRange,pCompositionRange,pfDeleteResultRange) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfTransitoryExtensionSink_INTERFACE_DEFINED__ */


#ifndef __ITfToolTipUIElement_INTERFACE_DEFINED__
#define __ITfToolTipUIElement_INTERFACE_DEFINED__

/* interface ITfToolTipUIElement */
/* [unique][uuid][local][object] */ 


EXTERN_C const IID IID_ITfToolTipUIElement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("52b18b5c-555d-46b2-b00a-fa680144fbdb")
    ITfToolTipUIElement : public ITfUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetString( 
            /* [out] */ BSTR *pstr) = 0;
        
    };
    
#else   /* C style interface */

    typedef struct ITfToolTipUIElementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfToolTipUIElement * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITfToolTipUIElement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITfToolTipUIElement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            ITfToolTipUIElement * This,
            /* [out] */ BSTR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            ITfToolTipUIElement * This,
            /* [out] */ GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *Show )( 
            ITfToolTipUIElement * This,
            /* [in] */ BOOL bShow);
        
        HRESULT ( STDMETHODCALLTYPE *IsShown )( 
            ITfToolTipUIElement * This,
            /* [out] */ BOOL *pbShow);
        
        HRESULT ( STDMETHODCALLTYPE *GetString )( 
            ITfToolTipUIElement * This,
            /* [out] */ BSTR *pstr);
        
        END_INTERFACE
    } ITfToolTipUIElementVtbl;

    interface ITfToolTipUIElement
    {
        CONST_VTBL struct ITfToolTipUIElementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITfToolTipUIElement_QueryInterface(This,riid,ppvObject) \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfToolTipUIElement_AddRef(This)    \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfToolTipUIElement_Release(This)   \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfToolTipUIElement_GetDescription(This,pbstrDescription)   \
    ( (This)->lpVtbl -> GetDescription(This,pbstrDescription) ) 

#define ITfToolTipUIElement_GetGUID(This,pguid) \
    ( (This)->lpVtbl -> GetGUID(This,pguid) ) 

#define ITfToolTipUIElement_Show(This,bShow)    \
    ( (This)->lpVtbl -> Show(This,bShow) ) 

#define ITfToolTipUIElement_IsShown(This,pbShow)    \
    ( (This)->lpVtbl -> IsShown(This,pbShow) ) 


#define ITfToolTipUIElement_GetString(This,pstr)    \
    ( (This)->lpVtbl -> GetString(This,pstr) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfToolTipUIElement_INTERFACE_DEFINED__ */


#ifndef __ITfThreadMgrEx_INTERFACE_DEFINED__
#define __ITfThreadMgrEx_INTERFACE_DEFINED__

    /* interface ITfThreadMgrEx */
    /* [unique][uuid][object] */ 

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3e90ade3-7594-4cb0-bb58-69628f5f458c")
ITfThreadMgrEx : public ITfThreadMgr
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ActivateEx( 
            /* [out] */ /* __RPC__out */ TfClientId *ptid,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetActiveFlags( 
            /* [out] */ /* __RPC__out */ DWORD *lpdwFlags) = 0;

    };

#else   /* C style interface */

    typedef struct ITfThreadMgrExVtbl
    {
        BEGIN_INTERFACE

            HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfThreadMgrEx * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);

            ULONG ( STDMETHODCALLTYPE *AddRef )( 
                ITfThreadMgrEx * This);

            ULONG ( STDMETHODCALLTYPE *Release )( 
                ITfThreadMgrEx * This);

            HRESULT ( STDMETHODCALLTYPE *Activate )( 
                ITfThreadMgrEx * This,
                /* [out] */ /* __RPC__out */ TfClientId *ptid);

            HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
                ITfThreadMgrEx * This);

            HRESULT ( STDMETHODCALLTYPE *CreateDocumentMgr )( 
                ITfThreadMgrEx * This,
                /* [out] */ __RPC__deref_out_opt ITfDocumentMgr **ppdim);

            HRESULT ( STDMETHODCALLTYPE *EnumDocumentMgrs )( 
                ITfThreadMgrEx * This,
                /* [out] */ __RPC__deref_out_opt IEnumTfDocumentMgrs **ppEnum);

            HRESULT ( STDMETHODCALLTYPE *GetFocus )( 
                ITfThreadMgrEx * This,
                /* [out] */ __RPC__deref_out_opt ITfDocumentMgr **ppdimFocus);

            HRESULT ( STDMETHODCALLTYPE *SetFocus )( 
                ITfThreadMgrEx * This,
                /* [in] */ __RPC__in_opt ITfDocumentMgr *pdimFocus);

            HRESULT ( STDMETHODCALLTYPE *AssociateFocus )( 
                ITfThreadMgrEx * This,
                /* [in] */ __RPC__in HWND hwnd,
                /* [unique][in] */ __RPC__in_opt ITfDocumentMgr *pdimNew,
                /* [out] */ __RPC__deref_out_opt ITfDocumentMgr **ppdimPrev);

            HRESULT ( STDMETHODCALLTYPE *IsThreadFocus )( 
                ITfThreadMgrEx * This,
                /* [out] */ /* __RPC__out */ BOOL *pfThreadFocus);

            HRESULT ( STDMETHODCALLTYPE *GetFunctionProvider )( 
                ITfThreadMgrEx * This,
                /* [in] */ __RPC__in REFCLSID clsid,
                /* [out] */ __RPC__deref_out_opt ITfFunctionProvider **ppFuncProv);

            HRESULT ( STDMETHODCALLTYPE *EnumFunctionProviders )( 
                ITfThreadMgrEx * This,
                /* [out] */ __RPC__deref_out_opt IEnumTfFunctionProviders **ppEnum);

            HRESULT ( STDMETHODCALLTYPE *GetGlobalCompartment )( 
                ITfThreadMgrEx * This,
                /* [out] */ __RPC__deref_out_opt ITfCompartmentMgr **ppCompMgr);

            HRESULT ( STDMETHODCALLTYPE *ActivateEx )( 
                ITfThreadMgrEx * This,
                /* [out] */ /* __RPC__out */ TfClientId *ptid,
                /* [in] */ DWORD dwFlags);

            HRESULT ( STDMETHODCALLTYPE *GetActiveFlags )( 
                ITfThreadMgrEx * This,
                /* [out] */ /* __RPC__out */ DWORD *lpdwFlags);

        END_INTERFACE
    } ITfThreadMgrExVtbl;

    interface ITfThreadMgrEx
    {
        CONST_VTBL struct ITfThreadMgrExVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define ITfThreadMgrEx_QueryInterface(This,riid,ppvObject)  \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfThreadMgrEx_AddRef(This) \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfThreadMgrEx_Release(This)    \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfThreadMgrEx_Activate(This,ptid)  \
    ( (This)->lpVtbl -> Activate(This,ptid) ) 

#define ITfThreadMgrEx_Deactivate(This) \
    ( (This)->lpVtbl -> Deactivate(This) ) 

#define ITfThreadMgrEx_CreateDocumentMgr(This,ppdim)    \
    ( (This)->lpVtbl -> CreateDocumentMgr(This,ppdim) ) 

#define ITfThreadMgrEx_EnumDocumentMgrs(This,ppEnum)    \
    ( (This)->lpVtbl -> EnumDocumentMgrs(This,ppEnum) ) 

#define ITfThreadMgrEx_GetFocus(This,ppdimFocus)    \
    ( (This)->lpVtbl -> GetFocus(This,ppdimFocus) ) 

#define ITfThreadMgrEx_SetFocus(This,pdimFocus) \
    ( (This)->lpVtbl -> SetFocus(This,pdimFocus) ) 

#define ITfThreadMgrEx_AssociateFocus(This,hwnd,pdimNew,ppdimPrev)  \
    ( (This)->lpVtbl -> AssociateFocus(This,hwnd,pdimNew,ppdimPrev) ) 

#define ITfThreadMgrEx_IsThreadFocus(This,pfThreadFocus)    \
    ( (This)->lpVtbl -> IsThreadFocus(This,pfThreadFocus) ) 

#define ITfThreadMgrEx_GetFunctionProvider(This,clsid,ppFuncProv)   \
    ( (This)->lpVtbl -> GetFunctionProvider(This,clsid,ppFuncProv) ) 

#define ITfThreadMgrEx_EnumFunctionProviders(This,ppEnum)   \
    ( (This)->lpVtbl -> EnumFunctionProviders(This,ppEnum) ) 

#define ITfThreadMgrEx_GetGlobalCompartment(This,ppCompMgr) \
    ( (This)->lpVtbl -> GetGlobalCompartment(This,ppCompMgr) ) 


#define ITfThreadMgrEx_ActivateEx(This,ptid,dwFlags)    \
    ( (This)->lpVtbl -> ActivateEx(This,ptid,dwFlags) ) 

#define ITfThreadMgrEx_GetActiveFlags(This,lpdwFlags)   \
    ( (This)->lpVtbl -> GetActiveFlags(This,lpdwFlags) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */




#endif  /* __ITfThreadMgrEx_INTERFACE_DEFINED__ */


#ifndef __ITfThreadMgrEventSink_INTERFACE_DEFINED__
#define __ITfThreadMgrEventSink_INTERFACE_DEFINED__

    /* interface ITfThreadMgrEventSink */
    /* [unique][uuid][object] */ 


    EXTERN_C const IID IID_ITfThreadMgrEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("aa80e80e-2021-11d2-93e0-0060b067b86e")
ITfThreadMgrEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnInitDocumentMgr( 
            /* [in] */ __RPC__in_opt ITfDocumentMgr *pdim) = 0;

        virtual HRESULT STDMETHODCALLTYPE OnUninitDocumentMgr( 
            /* [in] */ __RPC__in_opt ITfDocumentMgr *pdim) = 0;

        virtual HRESULT STDMETHODCALLTYPE OnSetFocus( 
            /* [in] */ __RPC__in_opt ITfDocumentMgr *pdimFocus,
            /* [in] */ __RPC__in_opt ITfDocumentMgr *pdimPrevFocus) = 0;

        virtual HRESULT STDMETHODCALLTYPE OnPushContext( 
            /* [in] */ __RPC__in_opt ITfContext *pic) = 0;

        virtual HRESULT STDMETHODCALLTYPE OnPopContext( 
            /* [in] */ __RPC__in_opt ITfContext *pic) = 0;

    };

#else   /* C style interface */

    typedef struct ITfThreadMgrEventSinkVtbl
    {
        BEGIN_INTERFACE

            HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITfThreadMgrEventSink * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);

            ULONG ( STDMETHODCALLTYPE *AddRef )( 
                ITfThreadMgrEventSink * This);

            ULONG ( STDMETHODCALLTYPE *Release )( 
                ITfThreadMgrEventSink * This);

            HRESULT ( STDMETHODCALLTYPE *OnInitDocumentMgr )( 
                ITfThreadMgrEventSink * This,
                /* [in] */ __RPC__in_opt ITfDocumentMgr *pdim);

            HRESULT ( STDMETHODCALLTYPE *OnUninitDocumentMgr )( 
                ITfThreadMgrEventSink * This,
                /* [in] */ __RPC__in_opt ITfDocumentMgr *pdim);

            HRESULT ( STDMETHODCALLTYPE *OnSetFocus )( 
                ITfThreadMgrEventSink * This,
                /* [in] */ __RPC__in_opt ITfDocumentMgr *pdimFocus,
                /* [in] */ __RPC__in_opt ITfDocumentMgr *pdimPrevFocus);

            HRESULT ( STDMETHODCALLTYPE *OnPushContext )( 
                ITfThreadMgrEventSink * This,
                /* [in] */ __RPC__in_opt ITfContext *pic);

            HRESULT ( STDMETHODCALLTYPE *OnPopContext )( 
                ITfThreadMgrEventSink * This,
                /* [in] */ __RPC__in_opt ITfContext *pic);

        END_INTERFACE
    } ITfThreadMgrEventSinkVtbl;

    interface ITfThreadMgrEventSink
    {
        CONST_VTBL struct ITfThreadMgrEventSinkVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define ITfThreadMgrEventSink_QueryInterface(This,riid,ppvObject)   \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITfThreadMgrEventSink_AddRef(This)  \
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITfThreadMgrEventSink_Release(This) \
    ( (This)->lpVtbl -> Release(This) ) 


#define ITfThreadMgrEventSink_OnInitDocumentMgr(This,pdim)  \
    ( (This)->lpVtbl -> OnInitDocumentMgr(This,pdim) ) 

#define ITfThreadMgrEventSink_OnUninitDocumentMgr(This,pdim)    \
    ( (This)->lpVtbl -> OnUninitDocumentMgr(This,pdim) ) 

#define ITfThreadMgrEventSink_OnSetFocus(This,pdimFocus,pdimPrevFocus)  \
    ( (This)->lpVtbl -> OnSetFocus(This,pdimFocus,pdimPrevFocus) ) 

#define ITfThreadMgrEventSink_OnPushContext(This,pic)   \
    ( (This)->lpVtbl -> OnPushContext(This,pic) ) 

#define ITfThreadMgrEventSink_OnPopContext(This,pic)    \
    ( (This)->lpVtbl -> OnPopContext(This,pic) ) 

#endif /* COBJMACROS */


#endif  /* C style interface */

#endif

#ifndef __IEnumTfInputProcessorProfiles_FWD_DEFINED__
#define __IEnumTfInputProcessorProfiles_FWD_DEFINED__
    typedef interface IEnumTfInputProcessorProfiles IEnumTfInputProcessorProfiles;
#endif  /* __IEnumTfInputProcessorProfiles_FWD_DEFINED__ */

#ifndef __IEnumTfInputProcessorProfiles_INTERFACE_DEFINED__
#define __IEnumTfInputProcessorProfiles_INTERFACE_DEFINED__

    /* interface IEnumTfInputProcessorProfiles */
    /* [unique][uuid][object] */ 


    EXTERN_C const IID IID_IEnumTfInputProcessorProfiles;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("71c6e74d-0f28-11d8-a82a-00065b84435c")
IEnumTfInputProcessorProfiles : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */  IEnumTfInputProcessorProfiles **ppEnum) = 0;

        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [length_is][size_is][out] */  TF_INPUTPROCESSORPROFILE *pProfile,
            /* [out] */  ULONG *pcFetch) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulCount) = 0;

    };

#endif  /* C style interface */
#endif


#ifndef __ITfInputProcessorProfileActivationSink_FWD_DEFINED__
#define __ITfInputProcessorProfileActivationSink_FWD_DEFINED__
    typedef interface ITfInputProcessorProfileActivationSink ITfInputProcessorProfileActivationSink;
#endif  /* __ITfInputProcessorProfileActivationSink_FWD_DEFINED__ */

#ifndef __ITfInputProcessorProfileActivationSink_INTERFACE_DEFINED__
#define __ITfInputProcessorProfileActivationSink_INTERFACE_DEFINED__

    /* interface ITfInputProcessorProfileActivationSink */
    /* [unique][uuid][object] */ 

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("71c6e74e-0f28-11d8-a82a-00065b84435c")
ITfInputProcessorProfileActivationSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnActivated( 
            /* [in] */ DWORD dwProfileType,
            /* [in] */ LANGID langid,
            /* [in] */ REFCLSID clsid,
            /* [in] */ REFGUID catid,
            /* [in] */ REFGUID guidProfile,
            /* [in] */ HKL hkl,
            /* [in] */ DWORD dwFlags) = 0;

    };

#endif  /* C style interface */
#endif

#ifndef __ITfInputProcessorProfileMgr_FWD_DEFINED__
#define __ITfInputProcessorProfileMgr_FWD_DEFINED__
    typedef interface ITfInputProcessorProfileMgr ITfInputProcessorProfileMgr;
#endif  /* __ITfInputProcessorProfileMgr_FWD_DEFINED__ */

#ifndef __ITfInputProcessorProfileMgr_INTERFACE_DEFINED__
#define __ITfInputProcessorProfileMgr_INTERFACE_DEFINED__

    /* interface ITfInputProcessorProfileMgr */
    /* [unique][uuid][object] */ 

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("71c6e74c-0f28-11d8-a82a-00065b84435c")
ITfInputProcessorProfileMgr : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ActivateProfile( 
            /* [in] */ DWORD dwProfileType,
            /* [in] */ LANGID langid,
            /* [in] */  REFCLSID clsid,
            /* [in] */  REFGUID guidProfile,
            /* [in] */ HKL hkl,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE DeactivateProfile( 
            /* [in] */ DWORD dwProfileType,
            /* [in] */ LANGID langid,
            /* [in] */  REFCLSID clsid,
            /* [in] */  REFGUID guidProfile,
            /* [in] */ HKL hkl,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetProfile( 
            /* [in] */ DWORD dwProfileType,
            /* [in] */ LANGID langid,
            /* [in] */  REFCLSID clsid,
            /* [in] */  REFGUID guidProfile,
            /* [in] */ HKL hkl,
            /* [out] */  TF_INPUTPROCESSORPROFILE *pProfile) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumProfiles( 
            /* [in] */ LANGID langid,
            /* [out] */IEnumTfInputProcessorProfiles **ppEnum) = 0;

        virtual HRESULT STDMETHODCALLTYPE ReleaseInputProcessor( 
            /* [in] */  REFCLSID rclsid,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE RegisterProfile( 
            /* [in] */  REFCLSID rclsid,
            /* [in] */ LANGID langid,
            /* [in] */  REFGUID guidProfile,
            /* [size_is][in] */ const WCHAR *pchDesc,
            /* [in] */ ULONG cchDesc,
            /* [size_is][in] */ const WCHAR *pchIconFile,
            /* [in] */ ULONG cchFile,
            /* [in] */ ULONG uIconIndex,
            /* [in] */ HKL hklsubstitute,
            /* [in] */ DWORD dwPreferredLayout,
            /* [in] */ BOOL bEnabledByDefault,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE UnregisterProfile( 
            /* [in] */  REFCLSID rclsid,
            /* [in] */ LANGID langid,
            /* [in] */  REFGUID guidProfile,
            /* [in] */ DWORD dwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetActiveProfile( 
            /* [in] */  REFGUID catid,
            /* [out] */  TF_INPUTPROCESSORPROFILE *pProfile) = 0;

    };

#endif  /* __ITfInputProcessorProfileMgr_INTERFACE_DEFINED__ */

#endif

#endif // GFX_TSFUIELEMENTDEF_H
