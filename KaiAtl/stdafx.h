// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

using namespace ATL;

#include "Error.h"

/*
#define ERROR_LOG (sMsg) wstringstream io__; \
            io__ << __LINE__; \
            CError::HandleError (sMsg, \
                                 std::wstring (_T(__FILE__)) + \
                                 std::wstring (_T("\t")) + \
                                 io__.str() + \
                                 std::wstring (_T("\t")) + \
                                 std::wstring (_T(__FUNCTION__)));
*/
