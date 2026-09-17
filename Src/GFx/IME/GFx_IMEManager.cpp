
#include "GFx/IME/GFx_IMEManager.h"
#ifndef SF_NO_IME_SUPPORT
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_MovieDef.h"

namespace Scaleform { namespace GFx { 

IMEManagerBase::IMEManagerBase(): State(State_IMEManager) 
{
	pASIMEManager	= 0;
	IMEDisabled		= false;
	pMovie			= 0;
	pTextField		= 0;
	CursorPosition	= 0;
};

IMEManagerBase::~IMEManagerBase()
{
	pASIMEManager = 0;
};

const wchar_t* IMEManagerBase::GetCompositionString()
{
	if (pMovie)
	{
		Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
		if (pfocusedCh && pfocusedCh->GetType() == TextFieldDef::TextField)
		{
			Ptr<TextField> ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
			return ptextFld->GetCompositionString();
		}
	}
	return NULL;
}

void IMEManagerBase::StartComposition()
{
	// we need to save the current position in text field
	if (pMovie)
	{
		Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
		if (pfocusedCh && pfocusedCh->GetType() == TextFieldDef::TextField)
		{
			Ptr<TextField> ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
			if (ptextFld->IsIMEEnabled())
			{
				pTextField = ptextFld;
				GetASIMEManager()->SetTextFieldPtr(pTextField);
				// first of all, if we have an active selection - remove it
				UPInt begin = pTextField->GetBeginIndex();
				UPInt end	= pTextField->GetEndIndex();
				pTextField->ReplaceText(L"", begin, end, end-begin); 

				CursorPosition = begin;
				pTextField->SetSelection(begin, begin);
				pTextField->CreateCompositionString();
			}
		}
	}
}

void IMEManagerBase::FinalizeComposition(const wchar_t* pstr, UPInt len)
{
	if (pTextField)
	{
		pTextField->CommitCompositionString(pstr, len);
		//pTextField->SetCompositionStringHighlighting(true);
	}
	else
	{
		// a special case, used for Chinese New ChangJie, if typing in English:
		// in this case, no start composition event is sent and whole
		// text is being sent through ime_finalize message. If we don't
		// have pTextField at this time we need to find currently focused
		// one and use it.
		if (pMovie)
		{
			Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pMovie))->GetFocusedCharacter(0);
			if (pfocusedCh && pfocusedCh->GetType() == TextFieldDef::TextField)
			{
				TextField* ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
				ptextFld->ReplaceText(pstr, ptextFld->GetBeginIndex(), 
					ptextFld->GetEndIndex(), len);

			}
		}
	}
}

void IMEManagerBase::ClearComposition()
{
	if (pTextField)
	{
		pTextField->ClearCompositionString();
		//pTextField->SetCompositionStringHighlighting(true);
	}
}

void IMEManagerBase::ReleaseComposition()
{
	if (pTextField)
	{
		pTextField->ReleaseCompositionString();
		//pTextField->SetCompositionStringHighlighting(true);
	}
}

void IMEManagerBase::SetCompositionText(const wchar_t* pstr, UPInt len)
{
	if (pTextField)
	{
		pTextField->SetCompositionStringText(pstr, len);
	}
}

void IMEManagerBase::SetCompositionPosition()
{
	if (pTextField)
	{
		UPInt pos = pTextField->GetCaretIndex();
		pTextField->SetCompositionStringPosition(pos);
	}
}

void IMEManagerBase::SetCursorInComposition(UPInt pos)
{
	if (pTextField)
	{
		pTextField->SetCursorInCompositionString(pos);
	}
}

void IMEManagerBase::SetWideCursor(bool ow)
{
	if (pTextField)
	{
		pTextField->SetWideCursor(ow);
		//pTextField->SetCompositionStringHighlighting(!ow);
	}
}

void IMEManagerBase::GetMetrics(RectF* pviewRect, RectF* pcursorRect, int cursorOffset)
{
	SF_UNUSED(pcursorRect);
	if (pTextField)
	{
		Render::Matrix2F wm = pTextField->GetWorldMatrix();
		RectF vr = pTextField->GetBounds(wm);
		if (pviewRect)
			*pviewRect = TwipsToPixels(vr);

		UPInt curspos = pTextField->GetCompositionStringPosition();
		if (curspos == SF_MAX_UPINT)
			curspos = pTextField->GetCaretIndex();
		else
			curspos += pTextField->GetCompositionStringLength();
		curspos += cursorOffset;
		if ((SPInt)curspos < 0)
			curspos = 0;

		RectF cr = pTextField->GetCursorBounds(curspos, 0, 0);

		cr = wm.EncloseTransform(cr);
		if (pcursorRect)
			*pcursorRect = TwipsToPixels(cr);
	}
}

void IMEManagerBase::HighlightText(UPInt pos, UPInt len, TextHighlightStyle style, bool clause)
{
    SF_UNUSED2(style, clause);
    if (pTextField)
    {
        /*if (clause)
        {
            // clause flag is set in order to highlight whole composition string
            // by solid single underline (used for Japanese).
            pTextField->HighlightCompositionString(Text::IMEStyle::SC_ConvertedSegment);
        }
        else
            pTextField->HighlightCompositionString(Text::IMEStyle::SC_CompositionSegment);
            */
        Text::IMEStyle::Category cat;
        switch (style)
        {
        case IMEManagerBase::THS_CompositionSegment: cat = Text::IMEStyle::SC_CompositionSegment; break;
        case IMEManagerBase::THS_ClauseSegment:      cat = Text::IMEStyle::SC_ClauseSegment; break;
        case IMEManagerBase::THS_ConvertedSegment:   cat = Text::IMEStyle::SC_ConvertedSegment; break;
        case IMEManagerBase::THS_PhraseLengthAdj:    cat = Text::IMEStyle::SC_PhraseLengthAdj; break;
        case IMEManagerBase::THS_LowConfSegment:     cat = Text::IMEStyle::SC_LowConfSegment; break;
        default: SF_ASSERT(0); cat = Text::IMEStyle::SC_CompositionSegment; // avoid warning
        }
        pTextField->HighlightCompositionStringText(pos, len, cat);
    }
}

void IMEManagerBase::SetActiveMovie(Movie* pmovie)
{
	if (pmovie != pMovie)
	{
        ClearActiveMovie();
        DoFinalize();
		if (pmovie)
		{
			// check if this pmovie was already hashed
			MovieASIMEManagerHashDef::Iterator it = MovieASIMEManagerHash.Find(pmovie);
			if (it != MovieASIMEManagerHash.End())
			{
				pASIMEManager = it->Second; 
			}
			else
			{
				ClearCandidateListPath();
				ASSupport* pASSupport = pmovie->pASMovieRoot->GetASSupport();
				if (pASSupport != NULL)
				{
					pASIMEManager = pASSupport->CreateASIMEManager();
					if (pASIMEManager)
					{
						pASIMEManager->SetIMEManager(this);
						pASIMEManager->SetIMEMoviePath(CandidateSwfPath);
						// Create function object to intercept IME notifications (candidate list and language bar)
						pASIMEManager->CustomFuncCandList = *SF_HEAP_NEW(Memory::GetGlobalHeap()) ASIMEManager::IMEFuncHandler(this);
						pASIMEManager->CustomFuncLanguageBar = *SF_HEAP_NEW(Memory::GetGlobalHeap()) ASIMEManager::IMEFuncHandler(this);
						pASIMEManager->SetActiveMovie(pmovie);
						// Put in hash table
						MovieASIMEManagerHash.Add(pmovie, pASIMEManager);
					}
				}				
			}
		}
		Reset();
		pMovie = pmovie;
	}
}

void IMEManagerBase::ClearActiveMovie()
{
	if (pMovie)
	{
		// Remove from Movie-AsIMEManager Hash
		MovieASIMEManagerHash.Remove(pMovie);
		if (pASIMEManager)
			pASIMEManager->ShutDown();

		Reset();
		// finalize (or, cancel?)
		OnShutdown();
	}
}

bool IMEManagerBase::IsMovieActive(Movie* pmovie) const
{
	return pMovie == pmovie;
}

Movie* IMEManagerBase::GetActiveMovie() const
{
	return pMovie;   
}

// callback, invoked when mouse button is down. buttonsState is a mask:
//   bit 0 - right button is pressed,
//   bit 1 - left
//   bit 2 - middle
void IMEManagerBase::OnMouseDown(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr)
{
	SF_UNUSED(buttonsState);
	if (IsMovieActive(pmovie) && pTextField && pitemUnderMousePtr == pTextField)
	{
		// if mouse clicked on the same active text field then finalize.
		// Otherwise let HandleFocus to handle the stuff.
		DoFinalize();
	}
}

void IMEManagerBase::OnMouseUp(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr)
{
	SF_UNUSED3(pmovie, buttonsState, pitemUnderMousePtr);
}

unsigned IMEManagerBase::HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent)
{
	SF_UNUSED2(pmovie, imeEvent);
	return Movie::HE_NotHandled;
}

void IMEManagerBase::OnCandidateListStyleRequest(IMECandidateListStyle* pstyle) const
{
	if (pMovie)
	{
		((MovieImpl*)pMovie)->GetIMECandidateListStyle(pstyle);
	}
}

bool IMEManagerBase::SetCandidateListStyle(const IMECandidateListStyle& st)
{
	if (pMovie)
		((MovieImpl*)(pMovie))->SetIMECandidateListStyle(st);
	if (IsCandidateListLoaded())
	{
		OnCandidateListStyleChanged(st);
		return true;
	}
	return false;
}

bool IMEManagerBase::GetCandidateListStyle(IMECandidateListStyle* pst) const
{

	if (pMovie)
	{
		((MovieImpl*)(pMovie))->GetIMECandidateListStyle(pst);
		return true;
	}
	
	return false;
}

bool IMEManagerBase::GetCandidateListErrorMsg(String* pdest)
{
	if (pdest)
		*pdest = GetASIMEManager()->CandidateSwfErrorMsg;
	return (GetASIMEManager()->CandidateSwfErrorMsg.GetLength() > 0);
}

void IMEManagerBase::DoFinalize()
{
	//    if (pTextField)
	{
		OnFinalize();
		pTextField = NULL;
	}
}

void IMEManagerBase::EnableIME(bool enable)
{
	if (IMEDisabled == enable)
	{
		IMEDisabled = !IMEDisabled;
		OnEnableIME(enable);
	}
}

void ASIMEManager::IMEFuncHandler::Call(const FunctionHandler::Params& params)
{
	pimeManager->ProcessIMEMessage(params);
}

}}

#endif //SF_NO_IME_SUPPORT
