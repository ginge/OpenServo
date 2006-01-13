// Register.cpp : Implementation of CRegister

#include "stdafx.h"
#include "Register.h"
#include ".\register.h"


// CRegister


STDMETHODIMP CRegister::get_address(LONG* pVal)
{
	*pVal = m_address;
	return S_OK;
}

STDMETHODIMP CRegister::get_name(BSTR* pVal)
{
	_bstr_t name(m_name, true);
	*pVal = name.Detach();
	return S_OK;
}

STDMETHODIMP CRegister::get_value(VARIANT* pVal)
{
	_variant_t value(m_value, true);
	*pVal = value.Detach();
	return S_OK;
}

STDMETHODIMP CRegister::put_value(VARIANT newVal)
{
	m_value = newVal;
	m_isDirty = true;
	return S_OK;
}

STDMETHODIMP CRegister::get_isDirty(VARIANT_BOOL* pVal)
{
	*pVal = m_isDirty ? 1 : 0;
	return S_OK;
}

STDMETHODIMP CRegister::put_isDirty(VARIANT_BOOL newVal)
{
	m_isDirty = newVal > 0;
	return S_OK;
}

STDMETHODIMP CRegister::get_direction(USHORT* pVal)
{
	*pVal = (USHORT)m_direction;
	return S_OK;
}

STDMETHODIMP CRegister::get_category(BSTR* pVal)
{
	_bstr_t category(m_category, true);
	*pVal = category.Detach();
	return S_OK;
}

STDMETHODIMP CRegister::get_description(BSTR* pVal)
{
	_bstr_t desc(m_description, true);
	*pVal = desc.Detach();
	return S_OK;
}
