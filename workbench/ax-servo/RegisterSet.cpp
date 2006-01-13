// RegisterSet.cpp : Implementation of CRegisterSet

#include "stdafx.h"
#include "RegisterSet.h"
#include ".\registerset.h"


// CRegisterSet
HRESULT CRegisterSet::FinalConstruct() 
{
	return S_OK;
}

void CRegisterSet::FinalRelease() 
{
	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++)
		r->second->Release();
}


STDMETHODIMP CRegisterSet::get_byName(BSTR name, IRegister** pVal)
{
	IRegister* pregister = m_registers[ name ];
	if(pregister) {
		*pVal = pregister;
		pregister->AddRef();
		return S_OK;
	} else
		return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::put_byName(BSTR name, IRegister* newVal)
{
	IRegister* pregister = m_registers[ name ];
	if(pregister) {
		pregister->Release();
		m_registers[ name ] = newVal;
		newVal->AddRef();
		return S_OK;
	} else
		return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::get_byIndex(LONG idx, IRegister** pVal)
{
	int i=0;
	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++)
		if(i++ == idx) {
			*pVal = r->second;
			(*pVal)->AddRef();
			return S_OK;
		}
	return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::put_byIndex(LONG idx, IRegister* newVal)
{
	int i=0;
	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++)
		if(i++ == idx) {
			BSTR name;
			if( SUCCEEDED( newVal->get_name(&name) ) ) {
				r->second->Release();
				m_registers[ name ] = newVal;
				newVal->AddRef();
				SysFreeString(name);
				return S_OK;
			}
		}
	return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::get_byAddress(LONG address, IRegister** pVal)
{
	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++) {
		LONG raddr;
		if( SUCCEEDED(r->second->get_address(&raddr)) && (raddr==address) ) {
			*pVal = r->second;
			(*pVal)->AddRef();
			return S_OK;
		}
	}
	return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::put_byAddress(LONG address, IRegister* newVal)
{
	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++) {
		LONG raddr;
		if( SUCCEEDED(r->second->get_address(&raddr)) && (raddr==address) ) {
			r->second->Release();
			r->second = newVal;
			newVal->AddRef();
			return S_OK;
		}
	}
	return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::Add(IRegister* reg)
{
	BSTR name;
	if( SUCCEEDED(reg->get_name(&name)) ) {
        reg->AddRef();
		m_registers[ name ] = reg;
		SysFreeString(name);
		return S_OK;
	} else
		return E_INVALIDARG;
}

STDMETHODIMP CRegisterSet::get_ByCategory(BSTR catname, IRegisterSet** pVal)
{
	IRegisterSet* pset;
	if(FAILED( CoCreateInstance(__uuidof(CRegisterSet), NULL, CLSCTX_ALL, __uuidof(IRegisterSet), (void**)&pset) ))
		return E_FAIL;

	for(RegisterMap::iterator r=m_registers.begin(), _r=m_registers.end(); r!=_r; r++) {
		BSTR category;
		if( SUCCEEDED(r->second->get_category(&category)) ) {
			if( _wcsicmp(category, catname)==0 )
				pset->Add( r->second );
			SysFreeString(category);
		}
	}

	return S_OK;
}

STDMETHODIMP CRegisterSet::AddRegister(LONG address, BSTR name, USHORT direction, BSTR category, BSTR description)
{
	IRegister* pregister;
	if(FAILED( CoCreateInstance(__uuidof(CRegister), NULL, CLSCTX_ALL, __uuidof(IRegister), (void**)&pregister) ))
		return E_FAIL;

	CRegister* pcreg = (CRegister*)pregister;
	pcreg->m_address = address;
	pcreg->m_name = name;
	pcreg->m_direction = (RegisterDirection)direction;
	pcreg->m_category = category;
	pcreg->m_description = description;

	m_registers[ pcreg->m_name ] = pregister;

	return S_OK;
}

STDMETHODIMP CRegisterSet::get_count(LONG* pVal)
{
	*pVal = (LONG)m_registers.size();
	return S_OK;
}
