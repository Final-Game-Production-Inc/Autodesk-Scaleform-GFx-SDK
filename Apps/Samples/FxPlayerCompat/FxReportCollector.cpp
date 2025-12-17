/**************************************************************************

Filename    :   FxReportCollector.cpp
Content     :   
Created     :   September 23, 2008
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "FxReportCollector.h"

static const char spaces[] = "                              ";
enum {sizeOfSpaces = sizeof(spaces) - 1};

void FxReportCollector::RenderRow(SF::StringBuffer *pbuffer, unsigned row)
{
    unsigned    column = 0;
    const char* ptext = RowColumns[row].ToCStr();
    UInt32      ch;

    // Count the number of columns and their alignment.
    ch = UTF8Util::DecodeNextChar(&ptext);
    while (ch != 0)
    {
        if (ch == '|')
        {
            ColumnAlign align = Columns[column].Align;
            const char* ptextStart = ptext;
            unsigned    size = 0;

            ParseInt(&ptext); // Skip integer.
            ch = UTF8Util::DecodeNextChar(&ptext);
            //  if (ch == '|')
            //      continue;

            if (ch == '>' || ch == '<')
            {
                align = (ch == '>') ? Align_Right : Align_Left;
                ptextStart = ptext;
                ch = UTF8Util::DecodeNextChar(&ptext);
            }
            else
            {
                // If no '<' or '>' character was after the number,
                // backtrack and re-parse to output it properly.
                ptext = ptextStart;
                ch = UTF8Util::DecodeNextChar(&ptext);
            }

            // Track the word length
            while(ch && ch != '|')
            {
                size++;
                ch = UTF8Util::DecodeNextChar(&ptext);
            }

            // Concatenate the word and extra white-spaces if necessary.
            if (align == Align_Left)
            {
                pbuffer->AppendString(ptextStart, size);

                while (size < Columns[column].Size)
                {
                    pbuffer->AppendChar(' ');
                    size++;
                }
            }
            else
            {
                unsigned sizeSave = size;
                while (size < Columns[column].Size)
                {
                    pbuffer->AppendChar(' ');
                    size++;
                }

                pbuffer->AppendString(ptextStart, sizeSave);
            }

            // Move to next column
            column++;
            if (column == MaxColumns)
                break;
        }
        else
        {
            break;
        }
    }

}

void FxReportCollector::RenderContent()
{
    SF::StringBuffer sbuffer(1024);

    // Emit any accumulated column rows and clear them.
    for (unsigned i = 0; i < RowNames.GetSize(); i++)
    {
        sbuffer.Clear();

        RenderRow(&sbuffer, i);
        //printf("%s %s\n", RowNames[i].ToCStr(), sbuffer.ToCStr());
        pOutput->AddLine(RowNames[i].ToCStr(), sbuffer.ToCStr());
    }

    ColumnCount = 0;
    RowColumns.Clear();
    RowNames.Clear();
}


void FxReportCollector::AddItem(const String& name, const String& text)
{
    ColumnInfo  newColumns[MaxColumns];
    unsigned    newColumnCount = 0;

    const char* ptext = text.ToCStr();
    UInt32      ch;

    // Count the number of columns and their alignment.
    ch = UTF8Util::DecodeNextChar(&ptext);
    while (ch != 0)
    {
        if (ch == '|')
        {            
            newColumns[newColumnCount].Align = Align_Left;
            newColumns[newColumnCount].Size  = 0;

            // We can have a number digit before '<' or '>' sign
            // to force minimum column size.
            const char* ptextStart = ptext;
            unsigned    minSize    = ParseInt(&ptext);

            ch = UTF8Util::DecodeNextChar(&ptext);            
            if (ch == '>' || ch == '<')
            {
                if (ch == '>')
                    newColumns[newColumnCount].Align = Align_Right;
                ch = UTF8Util::DecodeNextChar(&ptext);
            }
            else
            {                
                ptext   = ptextStart;
                minSize = 0;
                ch      = UTF8Util::DecodeNextChar(&ptext);                
            }

            // Count user characters for a minimum size
            while(ch && ch != '|')
            {
                newColumns[newColumnCount].Size++;
                ch = UTF8Util::DecodeNextChar(&ptext);
            }

            if (minSize > newColumns[newColumnCount].Size)
                newColumns[newColumnCount].Size = minSize;

            newColumnCount++;
            if (newColumnCount == MaxColumns)
                break;
        }
        else
        {
            break;
        }
    }

    // See if column number changed; if so render old columns
    // and record new ones. If no change, update column sizes.

    if (newColumnCount != ColumnCount)
    {
        RenderContent();

        memcpy(Columns, newColumns, newColumnCount * sizeof(ColumnInfo) );
        ColumnCount = newColumnCount;
    }
    else
    {
        for (UPInt i=0; i< newColumnCount; i++)
            Columns[i].Size = (Columns[i].Size > newColumns[i].Size) ? Columns[i].Size : newColumns[i].Size;

        //Alg::Max()
    }

    if (newColumnCount)
    {
        // Store data
        RowNames.PushBackAlt(name);
        RowColumns.PushBackAlt(text);
    }
    else
    {
        // Display line
        //printf("%s %s\n", name.ToCStr(), text.ToCStr());
        pOutput->AddLine(name.ToCStr(), text.ToCStr());
    }

}

void FxReportCollector::AddItem( FxStatItem* pitem, UInt8 identSize )
{
    String name, text;
    pitem->GetName(&name);
    pitem->GetText(&text);

    const char* psign;
    if (pitem->GetChildCount() == 0)
        psign = " ";
    else if (pitem->IsExpanded())
        psign = "-";
    else psign = "+";

    if (identSize > sizeOfSpaces) 
        identSize = sizeOfSpaces;
    String nameLine(spaces + sizeOfSpaces - identSize, psign, name.ToCStr());

    if (pitem->IsChanged())
        nameLine+="*";
    // Columns?
    if (text.GetSize() && text[0] == '|')
    {
        if (nameLine.GetLength() < sizeOfSpaces)
            nameLine += (spaces + nameLine.GetLength());
        else if (nameLine.GetLength() > sizeOfSpaces)
            nameLine = nameLine.Substring(0, sizeOfSpaces);
    }
    AddItem(nameLine, text);
}
// Reports an individual item content, recursing into children as necessary.
// An proper indent is built up to generate tree look.

void FxReportCollector::ReportItemsImpl(FxStatItem* pitem, const String& indent, bool reportAll /*= false*/)
{
    if(pitem->IsHidden())
        return;
    //SF_ASSERT(pitem->IsShown()); //REMOVE IT!!!

    String name,text;
    pitem->GetName(&name);
    pitem->GetText(&text);

    UPInt childCount = pitem->GetChildCount();    

    //const char* sign = (childCount == 0) ? " " : "-";

    const char* psign;
    if (pitem->GetChildCount() == 0)
        psign = " ";
    else if (pitem->IsExpanded() || reportAll)
        psign = "-";
    else psign = "+";


    // Format and display our line.    
    String nameLine(indent.ToCStr(), psign, name.ToCStr());

    if (pitem->IsChanged())
        nameLine+="*";

    // Columns?
    if (text.GetSize() && text[0] == '|')
    {
        if (nameLine.GetLength() < sizeOfSpaces)
            nameLine += (spaces + nameLine.GetLength());
        else if (nameLine.GetLength() > sizeOfSpaces)
            nameLine = nameLine.Substring(0, sizeOfSpaces);
    }

    // Display the line.
    AddItem(nameLine, text);


    // Display children
    if (childCount && (pitem->IsExpanded() || reportAll))
    {
        String newIndent = indent;
        newIndent += "  ";

        for (UPInt i=0; i< childCount; i++)
            ReportItemsImpl(pitem->GetChild(i), newIndent, reportAll);
    } 
}
