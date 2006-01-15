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

template <class _Class, class _Context>
class CContextThreadClass
{
public:
    CContextThreadClass(_Class* pObj, unsigned (_Class::*pFunc)(_Context*), _Context *pContext)
        :m_pObj(pObj), m_pFunc(pFunc), m_pContext(pContext)
    {};
    static unsigned __stdcall ThreadFunc(void* pParam)
    { 
        CContextThreadClass* pInst = (CContextThreadClass*)pParam;
        unsigned rVal = (pInst->m_pObj->*pInst->m_pFunc)(pInst->m_pContext);
        delete pInst;
        return rVal;
    };
private:
    _Class *m_pObj;
    unsigned (_Class::*m_pFunc)(_Context*);
    _Context *m_pContext;
};

template <class _Class, class _Context> inline HANDLE BeginThreadEx (_Class* pObj, unsigned (_Class::*pFunc)(_Context*), _Context* pContext, DWORD *pThreadId)
{
    CContextThreadClass<_Class, _Context> *pThreadClass = new CContextThreadClass<_Class, _Context>(pObj, pFunc, pContext);
    return (HANDLE)_beginthreadex(NULL, 0, CContextThreadClass<_Class, _Context>::ThreadFunc, pThreadClass, 0, (unsigned int*)pThreadId);
}

template <class _Class, class _Context> inline HANDLE BeginThread (_Class* pObj, unsigned (_Class::*pFunc)(_Context*), _Context* pContext)
{
    CContextThreadClass<_Class, _Context> *pThreadClass = new CContextThreadClass<_Class, _Context>(pObj, pFunc, pContext);
    unsigned addr;
    return (HANDLE)_beginthreadex(NULL, 0, CContextThreadClass<_Class, _Context>::ThreadFunc, pThreadClass, 0, &addr);
}





template <class _Class>
class CThreadClass
{
public:
    CThreadClass(_Class* pObj, unsigned (_Class::*pFunc)())
        :m_pObj(pObj), m_pFunc(pFunc)
    {};
    static unsigned __stdcall ThreadFunc(void* pParam)
    { 
        CThreadClass* pInst = (CThreadClass*)pParam;
        unsigned rVal = (pInst->m_pObj->*pInst->m_pFunc)();
        delete pInst;
        return rVal;
    };
private:
    _Class *m_pObj;
    unsigned (_Class::*m_pFunc)();
};

template <class _Class> inline HANDLE BeginThreadEx (_Class* pObj, unsigned (_Class::*pFunc)(), DWORD *pThreadId)
{
    CThreadClass<_Class> *pThreadClass = new CThreadClass<_Class>(pObj, pFunc);
    return (HANDLE)_beginthreadex(NULL, 0, CThreadClass<_Class>::ThreadFunc, pThreadClass, 0, (unsigned int*)pThreadId);
}

template <class _Class> inline HANDLE BeginThread (_Class* pObj, unsigned (_Class::*pFunc)())
{
    CThreadClass<_Class> *pThreadClass = new CThreadClass<_Class>(pObj, pFunc);
    unsigned addr;
    return (HANDLE)_beginthreadex(NULL, 0, CThreadClass<_Class>::ThreadFunc, pThreadClass, 0, &addr);
}

