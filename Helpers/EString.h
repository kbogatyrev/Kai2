#pragma once

#include "stdafx.h"

#include "tchar.h"
#include <Regex>
#include <string>
#include "Exception.h"

using namespace std;

static wchar_t * szBreakChars_ = _T(" \n");
static wchar_t * szTabs_ = _T("\t");
static wchar_t * szPunctuation_ = _T(".,;:/?<>[]{}~!()-_\'\"\\");
static wchar_t chrEscape_ = L'\27';
static unsigned int uiAllocationUnit_ = 1000;
static const unsigned int cuiMaxSize_ = 100000;

class  CEString
{

public:

    //
    // Helper objects
    //
    enum ERelation
    {
        ecRelFront      = 20001,
        ecEqual         = 20002,
        ecLess          = 20003,
        ecGreater       = 20004,
        ecRelBack       = ecGreater + 1
    };

    enum ETokenType
    {
        ecTokenTypeFront		= 10001,
        ecTokenText				= 10002,
        ecTokenSpace			= 10003,    // sequence of break chars
        ecTokenBreakChars		= 10003,    // alias
        ecTokenTab				= 10004,    // sequence of tabs
        ecTokenDiacritics       = 10005,
		ecTokenMeta				= 10006,	// e.g., XML tag
        ecTokenPunctuation		= 10007,    // sequence of punct. marks i.e. ; or ?!...
        ecTokenRegexMatch		= 10008,
        ecTokenTypeBack			= ecTokenRegexMatch + 1
    };

    struct StToken
    {
        ETokenType eType;
        unsigned int uiOffset;
        unsigned int uiLength;
        unsigned int uiPosition;

        StToken()
        {
            eType = ecTokenTypeFront;
            uiOffset = 0;
            uiLength = 0;
        }

        //
        // These operators are needed so we can sort tokens:
        //
        bool operator < (const StToken& stToken) const
        {
            return uiOffset < stToken.uiOffset;
        }

        bool operator == (const StToken& stToken) const
        {
            bool bType = (eType == stToken.eType);
            bool bOffset = (uiOffset == stToken.uiOffset);
            bool bLength = (uiLength == stToken.uiLength);
    
            return (bType && bOffset && bLength);
        }

        bool bIsLinearText() const
        {
            if (ecTokenTypeFront == eType ||
                ecTokenDiacritics == eType ||
                ecTokenMeta == eType ||
                ecTokenTypeBack == eType)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

    };	// struct StToken


private:

    wchar_t * m_szData;

    unsigned int m_uiLength;
    unsigned int m_uiBlocksAllocated;

    wchar_t m_szRegex[1000];
	wchar_t m_szVowels[100];

    vector<StToken> m_vecTokens;
    vector<StToken> m_vecRegexMatches;

    bool m_bInvalid;

public:

    CEString() : m_uiLength(0), m_uiBlocksAllocated (1), m_bInvalid (true)
    {
        m_szData = new wchar_t[uiAllocationUnit_];
        m_szData[0] = L'\0';
    }

    CEString (unsigned int uiBlocks) : m_uiLength(0), m_uiBlocksAllocated (uiBlocks), m_bInvalid (true)
    {
        unsigned int uiSize = m_uiBlocksAllocated * uiAllocationUnit_;
        if (uiSize > cuiMaxSize_)
        {
            wstring sMsg (L"Requested size exceeds maximum allowed.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        m_szData = new wchar_t[m_uiBlocksAllocated * uiAllocationUnit_];
        m_szData[0] = L'\0';
    }

    CEString (const CEString& coSource) : 
        m_szData (NULL), 
        m_uiLength (coSource.m_uiLength), 
        m_uiBlocksAllocated (coSource.m_uiBlocksAllocated),
        m_bInvalid (coSource.m_bInvalid)
    {
        if (coSource.m_uiLength > cuiMaxSize_ || 
            coSource.m_uiLength >= coSource.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Source string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        m_szData = new wchar_t[m_uiBlocksAllocated * uiAllocationUnit_];
        errno_t error = wmemmove_s (m_szData, m_uiLength, coSource.m_szData, m_uiLength); 
	    if (error)
	    {
            wstring sMsg (L"wmemmove_s error.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
	    }

        m_szData[m_uiLength] = L'\0';

        if (!m_bInvalid)
        {
            m_vecTokens = coSource.m_vecTokens;
        }

        m_vecRegexMatches = coSource.m_vecRegexMatches;
    
    }   //  Copy ctor

    CEString (const wchar_t * szSource) : m_szData(NULL), m_uiBlocksAllocated(0), m_bInvalid (true)
    {
        m_uiLength = wcslen (szSource);
        if (m_uiLength > cuiMaxSize_)
        {
            wstring sMsg (L"Source string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        Assign (szSource, m_uiLength);
    
    }   //  CEString (const wchar_t *)

    virtual ~CEString()
    {
        delete[] m_szData;
    }

public:
    static ERelation eCompare (const wchar_t * szLeft, const wchar_t * szRight)
    {
        if (0 == szLeft && 0 == szRight)
        {
            return ecEqual;
        }

        if (0 == szLeft)
        {
            ecLess;
        }

        if (0 == szRight)
        {
            return ecGreater;
        }

        for (int iAt = 0; iAt < cuiMaxSize_; ++iAt)
        {
            if (L'\0' == szLeft[iAt] && L'\0' == szLeft[iAt])
            {
                return ecEqual;
            }
            if (L'\0' == szLeft[iAt])
            {
                return ecLess;
            }

            if (L'\0' == szRight[iAt])
            {
                return ecGreater;
            }

            if (szLeft[iAt] < szRight[iAt])
            {
                return ecLess;
            }
            if (szLeft[iAt] > szRight[iAt])
            {
                return ecGreater;
            }
        }

        wstring sMsg (L"Arguments too long.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);

    }       // eCompare (...)

    static ERelation eCompare (const CEString& sLhs, const wchar_t * szRhs)
    {
        return eCompare (sLhs.m_szData, szRhs);

    }       // eCompare (...)

    static ERelation eCompare (const wchar_t * szLhs, const CEString& sRhs)
    {
        return eCompare (szLhs, sRhs.m_szData);

    }       // eCompare (...)

    static ERelation eCompare (const CEString& sLhs, const CEString& sRhs)
    {
        return eCompare (sLhs.m_szData, sRhs.m_szData);

    }       // eCompare (...)


    //
    // Operators
    //

/*
    bool operator== (const CEString& coRhs)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_
            || coRhs.m_uiLength >= coRhs.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (m_uiLength != coRhs.m_uiLength || m_uiBlocksAllocated != coRhs.m_uiBlocksAllocated)
        {
            return false;
        }

        return (ecEqual == eCompare (m_szData, coRhs.m_szData));
    
    }   //  operator==

    bool operator< (const CEString& coRhs)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_ 
            || coRhs.m_uiLength >= coRhs.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        return (ecLess == eCompare (m_szData, coRhs.m_szData));

    }   //  operator<

    bool operator> (const CEString& coRhs)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_ 
            || coRhs.m_uiLength >= coRhs.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        return (ecGreater == eCompare (m_szData, coRhs.m_szData));

    }   //  operator>

    bool operator<= (const CEString& coRhs)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_ 
            || coRhs.m_uiLength >= coRhs.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        ERelation eRel = eCompare (m_szData, coRhs.m_szData);
        return (ecLess == eRel || ecEqual == eRel);

    }   //  operator<=

    bool operator>= (const CEString& coRhs)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_ 
            || coRhs.m_uiLength >= coRhs.m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        ERelation eRel = eCompare (m_szData, coRhs.m_szData);
        return (ecGreater == eRel || ecEqual == eRel);

    }   //  operator>=
*/

    CEString& operator= (const wchar_t * szRhs)
    {
        unsigned int uiSourceLength = wcslen (szRhs);
        Assign (szRhs, uiSourceLength);

        return *this;
    
    }   //  operator= 

    CEString& operator= (const CEString& sRhs)
    {
        if (&sRhs == this)
        {
            return *this;
        }

        m_uiBlocksAllocated = sRhs.m_uiBlocksAllocated;
        m_uiLength = sRhs.m_uiLength;
        Assign (sRhs.m_szData, m_uiLength);
        
        return *this;
    
    }   //  operator= 

    CEString& operator+= (const wchar_t * szRhs)
    {
        unsigned int uiRhsLength = wcslen (szRhs);
        unsigned int uiNewSize = uiRhsLength + m_uiLength;
        if (uiNewSize > cuiMaxSize_)
        {
            wstring sMsg (L"Right-hand side string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        Concatenate (szRhs, uiRhsLength);

        return *this;

    }   //  operator+=

    CEString& operator+= (const CEString& sRhs)
    {
        unsigned int uiNewSize = sRhs.m_uiLength + m_uiLength;
        if (uiNewSize > cuiMaxSize_)
        {
            wstring sMsg (L"Right-hand side string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        Concatenate (sRhs.m_szData, sRhs.m_uiLength);

        return *this;

    }   //  operator+=

    //
    // Methods
    //

    unsigned int uiLength()
    {
        return m_uiLength;
    }

    void TrimLeft (wchar_t * szCharsToTrim)
    {
        if (m_uiLength >= (m_uiBlocksAllocated * uiAllocationUnit_))
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (L'\0' != m_szData[m_uiLength])
        {
            wstring sMsg (L"Malformed string.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        unsigned int uiAt = 0;
        for (; uiAt < m_uiLength; ++uiAt)
        {
            if (L'\0' == m_szData[uiAt])
            {
                wstring sMsg (L"Malformed string.");
                ERROR_LOG (sMsg);
                throw CException (E_UNEXPECTED, sMsg);
            }

            if (!bIn (m_szData[uiAt], szCharsToTrim))
            {
                break;
            }
        }

        if (0 == uiAt)
        {
            return;
        }

        if (uiAt >= m_uiLength)
        {
            m_szData[0] = L'\0';
            m_uiLength = 0;
            Shrink();
            return;
        }

        m_uiLength -= uiAt;
        errno_t error = wmemmove_s (m_szData, m_uiLength, &m_szData[uiAt], m_uiLength); 
        if (error)
        {
            wstring sMsg (L"wmemmove_s error.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
        }
        m_szData[m_uiLength] = L'\0';
    
    }       // TrimLeft (...)

    void TrimLeft()
    {
        TrimLeft (szBreakChars_);
    }

    void TrimRight (wchar_t * szCharsToTrim)
    {
        if (m_uiLength >= m_uiBlocksAllocated * uiAllocationUnit_)
        {
            wstring sMsg (L"Illegal length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (L'\0' != m_szData[m_uiLength])
        {
            wstring sMsg (L"Malformed string.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        int iAt = (int)m_uiLength-1;
        for (; iAt >= 0; --iAt)
        {
            if (L'\0' == m_szData[iAt])
            {
                wstring sMsg (L"Malformed string.");
                ERROR_LOG (sMsg);
                throw CException (E_UNEXPECTED, sMsg);
            }

            if (!bIn (m_szData[iAt], szCharsToTrim))
            {
                break;
            }
        }

        if (iAt == (int)m_uiLength-1)
        {
            return;
        }

        m_uiLength = (int)iAt+1;
        m_szData[m_uiLength] = L'\0';
        
        Shrink();

    }   //  void TrimRight()

    void TrimRight()
    {
        TrimRight (szBreakChars_);
    }

    void Trim (wchar_t * szCharsToTrim)
    {
        TrimLeft (szCharsToTrim);
        TrimRight (szCharsToTrim);
    }

    void Trim()
    {
        TrimLeft (szBreakChars_);
        TrimRight (szBreakChars_);
    }

    bool bIn (const wchar_t chr, wchar_t * szSearchSet)
    {
        if (m_uiLength < 1)
        {
            return false;
        }

        for (int iAt = 0; iAt < 1000; ++iAt)
        {
            if (chr == szSearchSet[iAt])
            {
                return true;
            }

            if (L'\0' == szSearchSet[iAt])
            {
                return false;
            }
        }

        wstring sMsg (L"Search set too long.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    
    }   //  bIn

    CEString sSubstr (unsigned int uiOffset, unsigned int uiLength)
    {
        if (uiLength >= cuiMaxSize_)
        {
            wstring sMsg (L"Source string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        if (L'\0' != m_szData[m_uiLength])
        {
            wstring sMsg (L"Source string not null-terminated.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (uiOffset >= m_uiLength)
        {
            wstring sMsg (L"Invalid offset.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (uiOffset + uiLength > m_uiLength)
        {
            wstring sMsg (L"Invalid length.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        unsigned int uiBlocksToAllocate = ((uiLength+1)/uiAllocationUnit_) + 1;

        CEString sResult (uiBlocksToAllocate);
        errno_t error = wmemmove_s (sResult.m_szData, uiLength, &m_szData[uiOffset], uiLength); 
	    if (error)
	    {
            wstring sMsg (L"wmemmove_s error.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
	    }
        
        sResult.m_szData[m_uiLength] = L'\0';
        sResult.m_uiLength = uiLength;

        return sResult;

    }   //  CEString sSubstr (...)

//    int i_CompareNoCase (const std::wstring& str) const;

    CEString sGetField (int iAt, ETokenType eType = ecTokenText)
    {
// TODO exc handling
        vector<StToken>::iterator itToken = itFindToken (iAt, eType);
        if (m_vecTokens.end() == itToken)
	    {
            wstring sMsg (L"Failed to find token.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
	    }

        return sSubstr ((*itToken).uiOffset, (*itToken).uiLength);
    
    }   // sGetField (...)


    StToken stGetField (int iAt, ETokenType eType = ecTokenText)
    {
// TODO exc handling
        vector<StToken>::iterator itToken = itFindToken (iAt, eType);
        if (m_vecTokens.end() == itToken)
	    {
            wstring sMsg (L"Failed to find token.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
	    }

        return *itToken;
    
    }   //  stGetField (...)

//    ST_Token st_GetFieldFromOffset (int i_offset,
//                                    et_TokenType eo_type = ec_TokenText);

    StToken stGetTokenFromOffset (int iOffset)
    {
        vector<StToken>::iterator it_ = itTokenFromOffset (iOffset);
        return (*it_);
    }

    ETokenType eGetTokenType (unsigned int uiAt)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return m_vecTokens[uiAt].eType;
    
    }   //  eGetTokenType (...)

//    et_TokenType eo_GetTokenType (int i_offset, int i_at);

    StToken stGetToken (unsigned int uiAt)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return m_vecTokens[uiAt];
    
    }   //  stGetToken (...)

    const StToken& rstGetToken (unsigned int uiAt)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return m_vecTokens[uiAt];
    
    }   //  const StToken& rstGetToken (...)

    CEString sGetToken (unsigned int uiAt)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        StToken stToken = m_vecTokens[uiAt];
        return CEString (sSubstr (stToken.uiOffset, stToken.uiLength));
    
    }   //  CEString sGetToken (...)

    bool bGetNextToken (StToken& stToken)
    {
        Tokenize();

        vector<StToken>::iterator it_ = find (m_vecTokens.begin(), m_vecTokens.end(), stToken);
        if (m_vecTokens.end() == it_)
        {
            wstring sMsg (L"Token not found.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        ++it_;
        if (m_vecTokens.end() == it_)
        {
            return false;
        }
        else
        {
            stToken = *it_;
        }

        return true;

    }    //  bool bGetNextToken (...)

    bool bGetPrevToken (StToken& stToken)
    {
        Tokenize();

        vector<StToken>::iterator it_ = find (m_vecTokens.begin(), m_vecTokens.end(), stToken);
        if (m_vecTokens.end() == it_)
        {
            wstring sMsg (L"Token not found.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        if (m_vecTokens.begin() == it_)
        {
            return false;
        }

        --it_;
        stToken = *it_;

        return true;

    }    //  bool bGetPrevToken (...)

    unsigned int uiGetTokenNum (const StToken& stToken)
    {
        Tokenize();

        vector<StToken>::iterator it_ = find (m_vecTokens.begin(), m_vecTokens.end(), stToken);
        if (it_ == m_vecTokens.end())
        {
            wstring sMsg (L"Token not found.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return it_ - m_vecTokens.begin();
    
    }   //  unsigned int uiGetTokenNum (...)

    unsigned int uiGetNumOfFields (ETokenType eType = ecTokenText)
    {
        Tokenize();

        int iTokens = 0;
        vector<StToken>::iterator it_ = m_vecTokens.begin();
        for (; it_ != m_vecTokens.end(); ++it_)
        {
            if ((*it_).eType == eType)
            {
                ++iTokens;
            }
        }
        
        return iTokens;
    
    }   //  uiGetNumOfFields (...)

/*
    unsigned int uiGetNumOfFields (unsigned int uiOffset, 
                                   unsigned int uiLength,
                                   ETokenType eType = ecTokenText)
    {
        Tokenize();

        vector<StToken>::iterator it_ = m_vecTokens.begin();
        for (; it_ < m_vecTokens.end(); ++it_)
        {
            if ((*it_).uiOffset >= uiOffset)
            {
                break;
            }
        }
        
        int iTokens = 0;
        for (; it_ != m_vecTokens.end(); ++it_)
        {
            if ((*it_).uiOffset >= uiOffset + uiLength)
            {
                break;
            }

            if ((*it_).eType == eType)
            {
                ++iTokens;
            }
        }
        
        if (m_vecTokens.end() == it_)
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return iTokens;
    
    }   //  unsigned int uiGetNumOfFields (unsigned int uiOffset, ...)
*/

    unsigned int uiNFields (ETokenType eType = ecTokenText)
    {
        return uiGetNumOfFields (eType);
    }

//    unsigned int uiNFields (unsigned int uiOffset, 
//                            unsigned int uiLength, 
//                            ETokenType eType = ecTokenText)
//    {
//        return uiGetNumOfFields (uiOffset, uiLength, eType);
//    }

    unsigned int uiGetNumOfTokens()
    {
        Tokenize();
        return m_vecTokens.size();
    }

    unsigned int uiNTokens()
    {
        Tokenize();
        return m_vecTokens.size();
    }

    unsigned int uiGetVisibleLength()
    {
        Tokenize();

        unsigned int uiVlength = 0;
        vector<StToken>::iterator it_ = m_vecTokens.begin();
        for (; it_ != m_vecTokens.end(); ++it_)    
        {
            if ((*it_).bIsLinearText())
            {
                uiVlength += (*it_).uiLength;
            }
        }
        return uiVlength;
    
    }   //  uiGetVisibleLength()

    unsigned int uiGetFieldLength (unsigned int uiAt, ETokenType eType = ecTokenText)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token index out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        vector<StToken>::iterator it = m_vecTokens.begin();
        for (; it != m_vecTokens.end(); ++it)
        {
            if ((*it).eType == eType)
            {
                if (distance (m_vecTokens.begin(), it) >= (int)uiAt)
                {
                    break;
                }
            }
        }
        
        if (m_vecTokens.end() == it)
        {
            wstring sMsg (L"Token not found.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return (*it).uiLength;

    }   //  uiGetFieldLength (...)

    //unsigned int uiGetFieldLength (unsigned int uiOffset, 
    //                               unsigned int uiAt,
    //                               ETokenType eType = ecTokenText)
    //{
    //}

////////////////////////////////////////////////////////////////////////////////////////////


private:

    void Grow (unsigned int uiCharsToAdd)
    {
        wchar_t * szNewBuffer = new wchar_t[m_uiLength + uiCharsToAdd];
	    errno_t error = wmemmove_s (szNewBuffer, m_uiLength, m_szData, m_uiLength); 
	    if (error)
	    {
            wstring sMsg (L"wmemmove_s failed.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
	    }
        szNewBuffer[m_uiLength] = L'\0';

        delete[] m_szData;
        m_szData = szNewBuffer;
    
    }   //  Grow()

    void Shrink()
    {
        unsigned int uiBlocksToFree = ((m_uiBlocksAllocated * uiAllocationUnit_) - m_uiLength) / uiAllocationUnit_;
        if (uiBlocksToFree < 1)
        {
            return;
        }

        m_uiBlocksAllocated -= uiBlocksToFree;
        wchar_t * szNewBuffer = new wchar_t[m_uiBlocksAllocated * uiAllocationUnit_];
	    errno_t error = wmemmove_s (szNewBuffer, m_uiLength, m_szData, m_uiLength); 
	    if (error)
	    {
            wstring sMsg (L"wmemmove_s failed.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
	    }
        szNewBuffer[m_uiLength] = L'\0';

        delete[] m_szData;
        m_szData = szNewBuffer;
    
    }   //  Grow()

    void Concatenate (const wchar_t * szRhs, unsigned int uiRhsLength)
    {
        unsigned int uiNewLength = m_uiLength + uiRhsLength;
        if (uiNewLength > cuiMaxSize_)
        {
            wstring sMsg (L"Right-hand side string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        if (uiNewLength >= m_uiBlocksAllocated * uiAllocationUnit_)
        {
            m_uiBlocksAllocated = (uiNewLength/uiAllocationUnit_) + 1;
            wchar_t * szNewData = new wchar_t[m_uiBlocksAllocated * uiAllocationUnit_];
            errno_t error = wmemmove_s (szNewData, m_uiLength, m_szData, m_uiLength); 
	        if (error)
	        {
                wstring sMsg (L"wmemmove_s error.");
                ERROR_LOG (sMsg);
                throw CException (error, sMsg);
	        }
            delete[] m_szData;
            m_szData = szNewData;
        }

        errno_t error = wmemmove_s (&m_szData[m_uiLength], uiRhsLength, szRhs, uiRhsLength); 
        if (error)
        {
            wstring sMsg (L"wmemmove_s error.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
        }

        m_uiLength = uiNewLength;
        m_szData[m_uiLength] = L'\0';

    }   //  void Concatenate (...)

    void Tokenize()
    {
        if (!m_bInvalid)
        {
            return;
        }

        m_bInvalid = false;
        m_vecTokens.clear();

        StToken stToken;
        for (unsigned int uiAt = 0; uiAt < m_uiBlocksAllocated * uiAllocationUnit_; ++uiAt)
        {
            wchar_t chrCurrent = m_szData[uiAt];

            if (L'\0' == chrCurrent)
            {
                if (uiAt != m_uiLength)
                {
                    m_bInvalid = true;
                    wstring sMsg (L"Unexpected NULL character.");
                    ERROR_LOG (sMsg);
                    throw CException (E_UNEXPECTED, sMsg);
                }

                if (0 == uiAt)
                {
                    return;
                }

                if (ecTokenTypeFront == stToken.eType)
                {
                    m_bInvalid = true;
                    wstring sMsg (L"Illegal token type.");
                    ERROR_LOG (sMsg);
                    throw CException (E_UNEXPECTED, sMsg);
                }

                m_vecTokens.push_back (stToken);
                return;
            }

            if (bIn (chrCurrent, szBreakChars_))
            {
                Advance (ecTokenBreakChars, uiAt, stToken);
                continue;
            }

            if (bIn (chrCurrent, szTabs_))
            {
                Advance (ecTokenTab, uiAt, stToken);
                continue;
            }

            if (bIn (chrCurrent, szPunctuation_))
            {
                Advance (ecTokenPunctuation, uiAt, stToken);
                continue;
            }

            if (chrEscape_ == chrCurrent)
            {

                ATLASSERT(m_uiLength > 0);
                if (uiAt >= m_uiLength-1)
                {
                    m_bInvalid = true;
                    wstring sMsg (L"Unexpected escape character.");
                    ERROR_LOG (sMsg);
                    throw CException (E_UNEXPECTED, sMsg);
                }

                bool bDoubleEscape = false;
                if (chrEscape_ == m_szData[uiAt+1])
                {
                    bDoubleEscape = true;
                }
                else if (uiAt > 0 && (chrEscape_ == m_szData[uiAt-1]))
                {
                    bDoubleEscape = true;
                }

                if (!bDoubleEscape)
                {
                    AddTag (uiAt, stToken);
                    continue;
                }

            }   //  for (int uiAt = 0; ...)

            Advance (ecTokenText, uiAt, stToken);
            
        }   //  for ...

        m_bInvalid = true;
        wstring sMsg (L"Tokenizer failed.");
        ERROR_LOG (sMsg);
        throw CException (E_UNEXPECTED, sMsg);

    }   //  Tokenize_ (...)

    void Advance (ETokenType eType, unsigned int uiOffset, StToken& stToken)
    {
        if (eType <= ecTokenTypeFront || eType > ecTokenTypeBack)
        {
            wstring sMsg (L"Unexpected token state.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (eType != stToken.eType)
        {
            if (ecTokenTypeFront != stToken.eType)
            {
                m_vecTokens.push_back (stToken);
            }

            stToken.eType = eType;
            stToken.uiOffset = uiOffset;
            stToken.uiLength = 1;
        }
        else
        {
            ++stToken.uiLength;
        }
    
    }   // void Advance (...)

    void AddTag (unsigned int uiOffset, StToken& stToken)
    {
        if (ecTokenMeta == stToken.eType || uiOffset < 0)
        {
            wstring sMsg (L"Unexpected token state.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (ecTokenTypeFront != stToken.eType)
        {
            m_vecTokens.push_back (stToken);
        }

        stToken.eType = ecTokenMeta;
        stToken.uiOffset = uiOffset;
        stToken.uiLength = 0;

        for (int iAt = uiOffset + 1; iAt < (int)(m_uiBlocksAllocated * uiAllocationUnit_); ++iAt)
        {
            if (L'\0' == m_szData[iAt])
            {
                wstring sMsg (L"Unterminated escape sequence.");
                ERROR_LOG (sMsg);
                throw CException (E_UNEXPECTED, sMsg);
            }

            if (chrEscape_ == m_szData[iAt])
            {
                if (uiOffset+1 == iAt)
                {
                    wstring sMsg (L"Empty escape sequence.");
                    ERROR_LOG (sMsg);
                    throw CException (E_UNEXPECTED, sMsg);
                }
                stToken.uiLength = iAt - uiOffset + 1;
                m_vecTokens.push_back (stToken);
                break;
            }
        }
    }       //  AddTag (...)

    // Assign null-terminated string erasing current contents
    // uiSourceLength does not count file '0'
    void Assign (const wchar_t * szSource, unsigned int uiSourceLength)
    {
        if (uiSourceLength >= cuiMaxSize_)
        {
            wstring sMsg (L"Source string too long.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }

        if (L'\0' != szSource[uiSourceLength])
        {
            wstring sMsg (L"Source string not null-terminated.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (uiSourceLength >= (m_uiBlocksAllocated * uiAllocationUnit_))
        {
            m_uiBlocksAllocated = ((uiSourceLength+1)/uiAllocationUnit_) + 1;
            if (m_szData)
            {
                delete[] m_szData;
            }
            m_szData = new wchar_t[uiAllocationUnit_ * m_uiBlocksAllocated];
        }

        errno_t error = wmemmove_s (m_szData, uiSourceLength, szSource, uiSourceLength); 
	    if (error)
	    {
            wstring sMsg (L"wmemmove_s error.");
            ERROR_LOG (sMsg);
            throw CException (error, sMsg);
	    }

        m_szData[uiSourceLength] = L'\0';
        m_uiLength = uiSourceLength;
    
    }   //  Assign (...)

    vector<StToken>::iterator itFindToken (unsigned int uiAt, ETokenType eType = ecTokenText)
    {
        Tokenize();

        if (uiAt >= m_vecTokens.size())
        {
            wstring sMsg (L"Token position out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        unsigned int uiField = 0;
        vector<StToken>::iterator it_ = m_vecTokens.begin();
        for (; it_ != m_vecTokens.end(); ++it_)
        {
            if (eType == (*it_).eType)
            {
                if (uiAt == uiField)
                {
                    break;
                }
                ++uiField;
            }
        }

        if (m_vecTokens.end() == it_)
        {
            wstring sMsg (L"Token position out of range.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        return it_;

    }   //  uiFindToken (...)

    vector<StToken>::iterator itTokenFromOffset (unsigned int uiOffset)
    {
        Tokenize();

        vector<StToken>::iterator it_ = m_vecTokens.begin();
        for (; it_ != m_vecTokens.end(); ++it_)
        {
            if ((*it_).uiOffset > uiOffset)
            {
                break;
            }
        }

        if (m_vecTokens.begin() == it_)
        {
            wstring sMsg (L"Failed to find token.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }

        return --it_;

    }   //  itTokenFromOffset (...)


};   //  class  CEString

const CEString operator+ (const CEString& sLhs, const wchar_t * szRhs)
{
    return CEString (sLhs) += szRhs;
}

const CEString operator+ (const wchar_t * szLhs, const CEString& sRhs)
{
    return CEString (szLhs) += sRhs;
}

const CEString operator+ (const CEString& sLhs, const CEString& sRhs)
{
    return CEString (sLhs) += sRhs;
}

bool operator== (const CEString& sLhs, const wchar_t * szRhs)
{
    return CEString::ecEqual == CEString::eCompare (sLhs, szRhs);
}

bool operator== (const wchar_t * szLhs, const CEString& sRhs)
{
    return CEString::ecEqual == CEString::eCompare (szLhs, sRhs);
}

bool operator== (const CEString& sLhs, const CEString& sRhs)
{
    return CEString::ecEqual == CEString::eCompare (sLhs, sRhs);
}

bool operator!= (const CEString& sLhs, const wchar_t * szRhs)
{
    return CEString::ecEqual != CEString::eCompare (sLhs, szRhs);
}

bool operator!= (const wchar_t * szLhs, const CEString& sRhs)
{
    return CEString::ecEqual != CEString::eCompare (szLhs, sRhs);
}

bool operator!= (const CEString& sLhs, const CEString& sRhs)
{
    return CEString::ecEqual != CEString::eCompare (sLhs, sRhs);
}

bool operator< (const CEString& sLhs, const wchar_t * szRhs)
{
    return CEString::ecLess == CEString::eCompare (sLhs, szRhs);
}

bool operator< (const wchar_t * szLhs, const CEString& sRhs)
{
    return CEString::ecLess == CEString::eCompare (szLhs, sRhs);
}

bool operator< (const CEString& sLhs, const CEString& sRhs)
{
    return CEString::ecLess == CEString::eCompare (sLhs, sRhs);
}

bool operator> (const CEString& sLhs, const wchar_t * szRhs)
{
    return CEString::ecGreater == CEString::eCompare (sLhs, szRhs);
}

bool operator> (const wchar_t * szLhs, const CEString& sRhs)
{
    return CEString::ecGreater == CEString::eCompare (szLhs, sRhs);
}

bool operator> (const CEString& sLhs, const CEString& sRhs)
{
    return CEString::ecLess == CEString::eCompare (sLhs, sRhs);
}

bool operator<= (const CEString& sLhs, const wchar_t * szRhs)
{
    CEString::ERelation eRet = CEString::eCompare (sLhs, szRhs);
    return (CEString::ecLess == eRet || CEString::ecEqual == eRet);
}

bool operator<= (const wchar_t * szLhs, const CEString& sRhs)
{
    CEString::ERelation eRet = CEString::eCompare (szLhs, sRhs);
    return (CEString::ecLess == eRet || CEString::ecEqual == eRet);
}

bool operator<= (const CEString& sLhs, const CEString& sRhs)
{
    CEString::ERelation eRet = CEString::eCompare (sLhs, sRhs);
    return (CEString::ecLess == eRet || CEString::ecEqual == eRet);
}

bool operator>= (const CEString& sLhs, const wchar_t * szRhs)
{
    CEString::ERelation eRet = CEString::eCompare (sLhs, szRhs);
    return (CEString::ecGreater == eRet || CEString::ecEqual == eRet);
}

bool operator>= (const wchar_t * szLhs, const CEString& sRhs)
{
    CEString::ERelation eRet = CEString::eCompare (szLhs, sRhs);
    return (CEString::ecGreater == eRet || CEString::ecEqual == eRet);
}

bool operator>= (const CEString& sLhs, const CEString& sRhs)
{
    CEString::ERelation eRet = CEString::eCompare (sLhs, sRhs);
    return (CEString::ecGreater == eRet || CEString::ecEqual == eRet);
}
