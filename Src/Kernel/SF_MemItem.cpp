#include "SF_MemItem.h"
#include "SF_HeapNew.h"
#include "SF_MsgFormat.h"

namespace Scaleform {

///////////////////////////////////////

void MemItem::SetValue(UInt32 memValue)
{
    Value = memValue;
    HasValue = true;
}

MemItem* MemItem::AddChild(UInt32 id, const char* name)
{
    MemItem* child = SF_HEAP_AUTO_NEW(this) MemItem(id);
    child->Name = name;
    Children.PushBack(*child);
    return child;
}

MemItem* MemItem::AddChild(UInt32 id, const char* name, UInt32 memValue)
{
    MemItem* child = SF_HEAP_AUTO_NEW(this) MemItem(id);
    child->Name = name;
    child->SetValue(memValue);
    Children.PushBack(*child);
    return child;
}

// Depth-first search for matching name
MemItem* MemItem::SearchForName(const char* name)
{
    if (Name == name)
    {
        return this;
    }
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        MemItem* childFound = Children[i]->SearchForName(name);
        if (childFound != NULL)
        {
            return childFound;
        }
    }
    return NULL;
}

UInt32 MemItem::SumValues(const char* name) const
{
    if (Name == name)
    {
        return Value;
    }

    UInt32 sum = 0;
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        sum += Children[i]->SumValues(name);
    }
    return sum;
}

UInt32 MemItem::GetValue(const char* name) const
{
    if (Name == name)
    {
        return Value;
    }

    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        UInt32 childValue = Children[i]->GetValue(name);
        if (childValue != 0)
        {
            return childValue;
        }
    }
    return 0;
}


// Returns the greatest ID for this item and its children, 
// so we can add more items with unique IDs
UInt32 MemItem::GetMaxId() const
{
    UInt32 maxId = ID;
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        maxId = Alg::Max(maxId, Children[i]->GetMaxId());
    }
    return maxId;
}

MemItem& MemItem::operator=(const MemItem& rhs)
{
    Name = rhs.Name;
    Value = rhs.Value;
    HasValue = rhs.HasValue;
    StartExpanded = rhs.StartExpanded;
    ID = rhs.ID;
    if (rhs.ImageExtraData)
    {
        ImageExtraData = *SF_HEAP_AUTO_NEW(this) MemItemExtra(0);
        *ImageExtraData = *rhs.ImageExtraData;
    }
    else
    {
        ImageExtraData = NULL;
    }
    Children.Clear();
    for (UPInt i = 0; i < rhs.Children.GetSize(); ++i)
    {
        MemItem* newChild = SF_HEAP_AUTO_NEW(this) MemItem(rhs.Children[i]->ID);
        *newChild = *rhs.Children[i];
        Children.PushBack(*newChild);
    }
    return *this;
}


MemItem& MemItem::operator/=(unsigned numFrames)
{
    Value /= numFrames;
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        *Children[i] /= numFrames;
    }
    return *this;
}

MemItem& MemItem::operator*=(unsigned num)
{
    Value *= num;
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        *Children[i] *= num;
    }
    return *this;
}

bool MemItem::Merge(const MemItem& other)
{
    if (!HasValue && Name.IsEmpty() && Children.GetSize() == 0)
    {
        *this = other;
        return true;
    }

    if (other.Name != Name)
    {
        return false;
    }

    Value += other.Value;
    HasValue = (HasValue || other.HasValue);
    for (UPInt i = 0; i < other.Children.GetSize(); ++i)
    {
        bool merged = false;
        for (UPInt j = 0; j < Children.GetSize(); ++j)
        {
            if (Children[j]->Merge(*other.Children[i]))
            {
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            MemItem* newItem = SF_HEAP_AUTO_NEW(this) MemItem(other.Children[i]->ID);
            *newItem = *other.Children[i];
            Children.PushBack(*newItem);
        }
    }
    return true;
}


// Serialization
void MemItem::Read(File& str, UInt32 version)
{
    UInt32 stringLength = str.ReadUInt32();
    for (UInt32 i = 0; i < stringLength; ++i)
    {
        Name.AppendChar(str.ReadSByte());
    }

    HasValue = (str.ReadUByte() != 0);
    StartExpanded = (str.ReadUByte() != 0);
    Value = str.ReadUInt32();
    ID = str.ReadUInt32();
    if (version > 11)
    {
        UByte hasData = str.ReadUByte();
        if (hasData != 0)
        {
            ImageExtraData = *SF_HEAP_AUTO_NEW(this) MemItemExtra(0);
            ImageExtraData->Read(str, version);
        }
    }
    else
    {
        UInt32 imageId = str.ReadUInt32();
        if (imageId != 0)
        {
            ImageExtraData = *SF_HEAP_AUTO_NEW(this) MemItemExtra(imageId);
        }
    }
    UInt32 numChildren = str.ReadUInt32();
    Children.Resize(numChildren);
    for (UInt32 i = 0; i < numChildren; ++i)
    {
        Children[i] = *SF_HEAP_AUTO_NEW(this) MemItem(0);
        Children[i]->Read(str, version);
    }
}

void MemItem::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(static_cast<UInt32>(Name.GetLength()));
    for (UPInt i = 0; i < Name.GetLength(); ++i)
    {
        str.WriteSByte(Name[i]);
    }
    str.WriteUByte(HasValue ? 1 : 0);
    str.WriteUByte(StartExpanded ? 1 : 0);
    str.WriteUInt32(Value);
    str.WriteUInt32(ID);
    if (version > 11)
    {
        if (ImageExtraData)
        {
            str.WriteUByte(1);
            ImageExtraData->Write(str, version);
        }
        else
        {
            str.WriteUByte(0);
        }
    }
    else
    {
        if (ImageExtraData)
        {
            str.WriteUInt32(ImageExtraData->ImageId);
        }
        else
        {
            str.WriteUInt32(0);
        }
    }
    str.WriteUInt32(static_cast<UInt32>(Children.GetSize()));
    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->Write(str, version);
    }
}

void MemItem::ToString(StringBuffer* report, UByte indent) const
{
    Array<char> sft(indent + 1);
    memset(&sft[0], ' ', indent);
    sft[indent] = '\0';
    report->AppendString(&sft[0]);

    report->AppendString(Name.ToCStr());
    if (HasValue)
    {
        const unsigned tabSize = 50;
        unsigned numSpaces = tabSize - Alg::Min(static_cast<unsigned>(Name.GetLength()) + indent, tabSize);
        sft.Resize(numSpaces + 1);
        memset(&sft[0], ' ', numSpaces);
        sft[numSpaces] = '\0';
        report->AppendString(&sft[0]);
        Format(*report, " {0:sep:,}", Value);
    }
    report->AppendChar('\n');

    for (UPInt i = 0; i < Children.GetSize(); ++i)
    {
        Children[i]->ToString(report, indent + 4);
    }
}

void MemItem::ToXml(StringBuffer* report, UByte indent) const
{
    Array<char> sft(indent + 1);
    memset(&sft[0], ' ', indent);
    sft[indent] = '\0';
    char* indStr = &sft[0];

    if (HasValue)
    {
        Format(*report, "{0}<MemItem Name=\"{1}\" Value={2}>", indStr, Name, Value);
    }
    else
    {
        Format(*report, "{0}<MemItem Name=\"{1}\">", indStr, Name);
    }
    if (Children.GetSize() > 0)
    {
        report->AppendChar('\n');
        for (UPInt i = 0; i < Children.GetSize(); ++i)
        {
            Children[i]->ToXml(report, indent + 4);
        }
        Format(*report, "{0}</MemItem>\n", indStr);
    }
    else
    {
        report->AppendString("</MemItem>\n");
    }

}


//////////////////////////////////////////////////////////

void MemItemExtra::Read(File& str, UInt32 version)
{
    ImageId = str.ReadUInt32();
    if (version > 11)
    {
        AtlasId = str.ReadUInt32();
        AtlasRectLeft = str.ReadSInt32();
        AtlasRectTop = str.ReadSInt32();
        AtlasRectRight = str.ReadSInt32();
        AtlasRectBottom = str.ReadSInt32();
    }
}

void MemItemExtra::Write(File& str, UInt32 version) const
{
    str.WriteUInt32(ImageId);
    if (version > 11)
    {
        str.WriteUInt32(AtlasId);
        str.WriteSInt32(AtlasRectLeft);
        str.WriteSInt32(AtlasRectTop);
        str.WriteSInt32(AtlasRectRight);
        str.WriteSInt32(AtlasRectBottom);
    }
}


} // namespace Scaleform
