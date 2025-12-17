/**************************************************************************

Filename    :   FxReportCollector.h
Content     :   
Created     :   September 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_FXREPORTCOLLECTOR_H
#define INC_FXREPORTCOLLECTOR_H
#include "FxStatItem.h"
// **** Stat Report Collector

// FxReportCollector creates a table of items consisting of multiple
// evenly formatted columns. The table text is added with AddItem and
// is output based on the item 'Text' formatting tags, which can
// cause creation of columns. Columns in consecutive rows are automatically
// aligned to each other during formatting; this formatting proceeds
// until a line with a different number of columns is encountered.
//


//Output interface for FxReportCollector
class FxReportOutput : public RefCountBase<FxReportOutput, Stat_DebugTracker_Mem>
{
public:
    virtual void AddLine(const char* pname, const char* ptext) = 0;
    virtual void Clear() {}
};

// Default Implementation of FxReportOutput
class FxStdReportOutput : public FxReportOutput
{
public:
    virtual void AddLine(const char* pname, const char* ptext)
    {
        printf("%s %s\n", pname, ptext);
    }

};

class FxReportCollector
{
    enum ColumnAlign
    { 
        Align_Left,
        Align_Right
    };  

    enum { MaxColumns = 10 };

    struct ColumnInfo
    {
        ColumnAlign Align;
        unsigned    Size;
    };

    Ptr<FxReportOutput> pOutput;

    // Column header information, if columns are in use.
    ColumnInfo          Columns[MaxColumns];
    unsigned            ColumnCount;
    // Column data.
    Array<String> RowNames;
    Array<String> RowColumns;


    void    RenderRow(SF::StringBuffer *pbuffer, unsigned row);   
    void    RenderContent();
    void    ReportItemsImpl(FxStatItem* pitem, const String& indent, bool reportAll = false);

    // Parse integer and advance pointer.
    unsigned    ParseInt(const char** pptext)
    {
        unsigned i = 0;
        while (**pptext >= '0' && **pptext <= '9')
        {
            i = i*10 + (**pptext - '0');
            (*pptext)++;
        }
        return i;
    }

public:

    FxReportCollector(FxReportOutput* poutput = NULL)
    {
        ColumnCount = 0;
        if (poutput)
        {
            pOutput = poutput;
        }
        else
        {
            pOutput = *new FxStdReportOutput;
        }

    }

    // Add an item for display. If item starts with a column character '|',
    // it will begin a set of columns.  
    void    AddItem(const String& name, const String& text);

    void    AddItem(FxStatItem* pitem, UInt8 ident);

    // Call this function after the last item is added
    void    FinalizeItems()
    {
        RenderContent();
    }

    // Generate a report of the item and its children.
    void ReportItems(FxStatItem* pitem, bool reportAll = false)
    {
        pOutput->Clear();
        //for (UPInt i = 0; i < pitem->GetChildCount(); i++)
        //    ReportItemsImpl(pitem->GetChild(i), String()); 
        ReportItemsImpl(pitem, String(), reportAll); 
        FinalizeItems();
    }
};



#endif // INC_FXREPORTCOLLECTOR_H
