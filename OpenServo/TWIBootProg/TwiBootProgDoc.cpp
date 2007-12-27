// Copyright (C)2004 Dimax ( http://www.xdimax.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "stdafx.h"
#include "TwiBootProg.h"
#include "TwiBootProgDoc.h"
#include "IntelHexRec.h"
#include "OpenTypeDlg.h"
#include "BufferSizeDlg.h"
#include "FillDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int erInvalidHexFormat = 1;

// CTwiBootProgDoc

IMPLEMENT_DYNCREATE(CTwiBootProgDoc, CDocument)

BEGIN_MESSAGE_MAP(CTwiBootProgDoc, CDocument)
//  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    ON_COMMAND(ID_EDIT_FILL, OnEditFill)
END_MESSAGE_MAP()

// CTwiBootProgDoc construction/destruction

const char* CTwiBootProgDoc::m_strFileTypeFilter = "Binary data|*.*|Intel Hex|*.*||";

CTwiBootProgDoc::CTwiBootProgDoc()
: m_BufferLength(0), m_pBuffer(NULL), m_FileType(ftUnknown), m_bChangeEnabled(true)
{
    // TODO: add one-time construction code here

}

CTwiBootProgDoc::~CTwiBootProgDoc()
{
    if (m_pBuffer != NULL)
        delete[] m_pBuffer;
}

BOOL CTwiBootProgDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    CBufferSizeDlg dlg;
    if (IDCANCEL == dlg.DoModal())
        return FALSE;
    m_BufferLength = dlg.GetBufferSize();
    m_pBuffer = new BYTE[m_BufferLength];
    FillMemory(m_pBuffer, m_BufferLength, dlg.GetFillVal());
    m_CaretAddr = 0x0;
    m_CaretByteOffset = 0;
    return TRUE;
}

bool CTwiBootProgDoc::GetData(BYTE* pBuffer, unsigned int Length, unsigned int Start)
{
    if (Start+Length > m_BufferLength)
    {
        ASSERT(FALSE);
        return false;
    }
    CopyMemory(pBuffer, m_pBuffer+Start, Length);
    return true;
}

bool CTwiBootProgDoc::SetData(BYTE* pBuffer, unsigned int Length, unsigned int Start)
{
    if (Start+Length > m_BufferLength)
    {
        ASSERT(FALSE);
        return false;
    }
    CopyMemory(m_pBuffer+Start, pBuffer, Length);
    UpdateAllViews(NULL);
    SetModifiedFlag(TRUE);
    return true;
}


// CTwiBootProgDoc serialization

void CTwiBootProgDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: add storing code here
    }
    else
    {
        // TODO: add loading code here
    }
}


// CTwiBootProgDoc diagnostics

#ifdef _DEBUG
void CTwiBootProgDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CTwiBootProgDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

// CTwiBootProgDoc commands

BOOL CTwiBootProgDoc::SaveModified()
{
    // TODO: Add your specialized code here and/or call the base class
    if (!IsModified())
        return TRUE;
    switch (AfxMessageBox(IDS_SAVE_MODIFIED, MB_YESNOCANCEL))
    {
    case IDCANCEL:
        return FALSE;
    case IDYES:
        if (!SaveFile())
            return FALSE;
        else
            return TRUE;
    case IDNO:
        return TRUE;
    default:
        ASSERT(FALSE);
        return FALSE;
    }
}

bool CTwiBootProgDoc::SaveFile()
{
    CString FilePath = GetPathName();
    CString FileName = GetTitle();
    if ((m_FileType == ftUnknown) || FileName.IsEmpty() || FileName.IsEmpty())
        return SaveFileAs();
    bool rVal = false;
    switch (m_FileType)
    {
    case ftBinary:
        rVal = SaveBinary(FilePath);
        break;
    case ftIntelHex:
        rVal = SaveIntelHex(FilePath);
        break;
    default:
        ASSERT(FALSE);
        rVal = false;
        break;
    }
    if (rVal)
        SetModifiedFlag(FALSE);
    return rVal;
}

bool CTwiBootProgDoc::SaveBinary(CString FilePath)
{
    try
    {
        CFile File(FilePath, CFile::modeCreate|CFile::modeWrite);
        File.Write(m_pBuffer, m_BufferLength);
        File.Close();
        return true;
    }
    catch (CFileException* ex)
    {
        CString strErr;
        strErr.Format(IDS_SAVE_OPEN_FAILED, ex->m_cause);
        AfxMessageBox(strErr);
        ex->Delete();
        return false;
    }
}

bool CTwiBootProgDoc::SaveIntelHexBlock(CStdioFile *pFile, ULONG Size, ULONG StartAddr)
{
    ASSERT(LOWORD(StartAddr) + Size <= 0x10000);
    ULONG RecSize;
    CIntelHexRec Rec;
    CString str;
    for (ULONG Addr = StartAddr; Addr < StartAddr + Size; Addr += RecSize)
    {
        if (StartAddr + Size - Addr < 0x10)
            RecSize = StartAddr + Size - Addr;
        else
            RecSize = 0x10;
        if (!Rec.InitData((BYTE)RecSize, LOWORD(Addr),m_pBuffer+Addr))
            return false;
        pFile->WriteString(Rec.GetString());
    }
    return true;
}

bool CTwiBootProgDoc::SaveIntelHex(CString FilePath)
{
    try
    {
        CStdioFile File(FilePath, CFile::modeCreate|CFile::modeWrite|CFile::typeText);
        CIntelHexRec Rec;
        CString str;
        unsigned long Addr, BlockSize;
        for (Addr = 0; Addr < m_BufferLength; Addr+=BlockSize)
        {
            if (!Rec.InitExtAddr(HIWORD(Addr)))
                return false;
            File.WriteString(Rec.GetString());
            if (Addr + 0x10000 < m_BufferLength)
                BlockSize = 0x10000;
            else
                BlockSize = m_BufferLength - Addr;
            if (!SaveIntelHexBlock(&File, BlockSize, Addr))
                return false;
        }
        if (!Rec.InitEndOfData())
            return false;
        File.WriteString(Rec.GetString());
        File.Close();
        return true;
    }
    catch (CFileException* ex)
    {
        CString strErr;
        strErr.Format(IDS_SAVE_FAILED_WITH_ERRNO, ex->m_cause);
        AfxMessageBox(strErr);
        ex->Delete();
        return false;
    }
}

bool CTwiBootProgDoc::SaveFileAs()
{
    CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, m_strFileTypeFilter, NULL);
    CString FilePath = GetPathName();
    CString FileName = GetTitle();
    if (FilePath.IsEmpty())
        FilePath = FileName;
    dlg.m_ofn.lpstrTitle = FileName;
    dlg.m_ofn.lpstrFile = FilePath.GetBuffer(_MAX_PATH);
    INT_PTR res = dlg.DoModal();
    FilePath.ReleaseBuffer();
    if (res == IDCANCEL)
        return false;
    CString FileExt = dlg.GetFileExt();

    switch(dlg.m_ofn.nFilterIndex)
    {
    case 1:
        m_FileType = ftBinary;
        if (FileExt.IsEmpty())
            FilePath += ".bin";
        break;
    case 2:
        m_FileType = ftIntelHex;
        if (FileExt.IsEmpty())
            FilePath += ".hex";
        break;
    default:
        ASSERT(FALSE);
        return false;
    }
    SetPathName(FilePath);
    SetTitle(FilePath);
    return SaveFile();
}


//void CTwiBootProgDoc::OnFileOpen()
//{
//  // TODO: Add your command handler code here
//}

void CTwiBootProgDoc::OnFileSave()
{
    // TODO: Add your command handler code here
    SaveFile();
}

void CTwiBootProgDoc::OnFileSaveAs()
{
    // TODO: Add your command handler code here
    SaveFileAs();
}

BOOL CTwiBootProgDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    COpenTypeDlg dlg;
    dlg.DoModal();
    return LoadFile(lpszPathName, dlg.GetType());
}

BOOL CTwiBootProgDoc::LoadFile(CString FilePath, eFileType Type)
{
    ASSERT(!FilePath.IsEmpty());
    BOOL Res = FALSE;
    switch(Type)
    {
    case ftBinary:
        Res = LoadBinary(FilePath);
        break;
    case ftIntelHex:
        Res = LoadIntelHex(FilePath);
        break;
    default:
        ASSERT(FALSE);
        Res = FALSE;
        break;
    }
    return Res;
}

BOOL CTwiBootProgDoc::LoadBinary(CString FilePath)
{
    ASSERT (m_pBuffer == NULL);
    ASSERT(m_BufferLength == 0);
    try
    {
        CFile File(FilePath, CFile::modeRead);
        ULONGLONG FileLen = File.GetLength();
        if (FileLen > MAX_BUFFER_SIZE)
            return FALSE;
        m_BufferLength = (ULONG)FileLen;
        m_pBuffer = new BYTE[m_BufferLength];
        File.Read(m_pBuffer, m_BufferLength);
        File.Close();
        return TRUE;
    }
    catch (CFileException* ex)
    {
        CString strErr;
        strErr.Format(IDS_SAVE_OPEN_FAILED, ex->m_cause);
        AfxMessageBox(strErr);
        ex->Delete();
        return FALSE;
    }
}

BOOL CTwiBootProgDoc::CalcIntelHexSize(CStdioFile* pFile, ULONG *pSize)
{
    CIntelHexRec Rec;
    CString str;
    ULONG Size = 0, CurAddr = 0;
    while(pFile->ReadString(str))
    {
        if (!Rec.InitFromString(str))
            return FALSE;
        switch(Rec.m_Type)
        {
        case cHexTypeData:
            CurAddr &= 0xFFFF0000;
            CurAddr |= Rec.m_Addr;
            CurAddr += Rec.m_Size;
            if (CurAddr > Size) Size = CurAddr;
            break;
        case cHexTypeEndOfData:
            break;
        case cHexTypeExtLinearAddr:
            CurAddr = Rec.GetExtAddr();
            break;
        default:
            ASSERT(FALSE);
            return FALSE;
        }
    }
    if (Size != 0)
    {
        *pSize = Size;
        return TRUE;
    }
    else
        return FALSE;
}

BOOL CTwiBootProgDoc::LoadIntelHex(CString FilePath)
{
    ASSERT (m_pBuffer == NULL);
    ASSERT(m_BufferLength == 0);
    try
    {
        CStdioFile File(FilePath, CFile::modeRead|CFile::typeText);
        CIntelHexRec Rec;
        CString str;
        ULONG len = 0, Addr = 0;
        if (!CalcIntelHexSize(&File, &len))
            throw erInvalidHexFormat;
        m_BufferLength = len;
        m_pBuffer = new BYTE[len];
        FillMemory(m_pBuffer, m_BufferLength, 0xFF);
        File.SeekToBegin();
        while(File.ReadString(str))
        {
            if (!Rec.InitFromString(str))
                throw erInvalidHexFormat;
            switch(Rec.m_Type)
            {
            case cHexTypeData:
                Addr &= 0xFFFF0000;
                Addr |= Rec.m_Addr;
                CopyMemory(m_pBuffer + Addr, Rec.m_Data, Rec.m_Size);
                break;
            case cHexTypeEndOfData:
                break;
            case cHexTypeExtLinearAddr:
                Addr = Rec.GetExtAddr();
                break;
            default:
                ASSERT(FALSE);
                throw erInvalidHexFormat;
            }
        }
        File.Close();
        return TRUE;
    }
    catch (CFileException* ex)
    {
        CString strErr;
        strErr.Format(IDS_OPEN_FAILED_WITH_ERRNO, ex->m_cause);
        AfxMessageBox(strErr);
        ex->Delete();
        return FALSE;
    }
    catch (int Err)
    {
        switch (Err)
        {
        case erInvalidHexFormat:
            AfxMessageBox(IDS_INVALID_HEX_FORMAT);
            break;
        default:
            ASSERT(FALSE);
            AfxMessageBox(IDS_OPEN_FAILED);
            break;
        }
        return FALSE;
    }

}

void CTwiBootProgDoc::OnEditFill()
{
    CFillDlg dlg;
    dlg.SetMaxAddr(m_BufferLength-1);
    if (IDCANCEL == dlg.DoModal())
        return;
    FillMemory(m_pBuffer+dlg.GetFromValue(),
        dlg.GetToValue() - dlg.GetFromValue() + 1, dlg.GetFillValue());
    UpdateAllViews(NULL);
}
