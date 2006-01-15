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

#pragma once

class CTwiBootProgDoc : public CDocument
{
public:
    bool GetData(BYTE* pBuffer, unsigned int Length, unsigned int Start);
    bool SetData(BYTE* pBuffer, unsigned int Length, unsigned int Start);
    void SetChangeEnabled(bool bEnabled){m_bChangeEnabled = bEnabled;};
    bool IsChangeEnabled(){return m_bChangeEnabled;}
    unsigned int GetBufferLength() {return m_BufferLength;};
    BOOL LoadFile(CString FilePath, eFileType Type);
    BYTE* GetBuffer(){return m_pBuffer;}
    static const char*  m_strFileTypeFilter;
    unsigned int m_CaretAddr;
    unsigned int m_CaretByteOffset;
private:
    bool SaveFile();
    bool SaveFileAs();
    bool SaveBinary(CString FilePath);
    bool SaveIntelHex(CString FilePath);
    bool SaveIntelHexBlock(CStdioFile *pFile, ULONG Size, ULONG StartAddr);
    BOOL LoadBinary(CString FilePath);
    BOOL LoadIntelHex(CString FilePath);
    BOOL CalcIntelHexSize(CStdioFile* pFile, ULONG *pSize);

    eFileType m_FileType;
    BYTE *m_pBuffer;
    unsigned int m_BufferLength;
    bool m_bChangeEnabled;

protected: // create from serialization only
    CTwiBootProgDoc();
    DECLARE_DYNCREATE(CTwiBootProgDoc)

// Attributes
public:

// Operations
public:

// Overrides
    public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

// Implementation
public:
    virtual ~CTwiBootProgDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    DECLARE_MESSAGE_MAP()
public:
protected:
    virtual BOOL SaveModified();
public:
//  afx_msg void OnFileOpen();
    afx_msg void OnFileSave();
    afx_msg void OnFileSaveAs();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    afx_msg void OnEditFill();
};

