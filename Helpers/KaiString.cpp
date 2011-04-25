//==============================================================================
//
//    Copyright (c) Konstantin Bogatyrev, 2001
//    
//    Facility: Kai 1.2
//
//    Module description:    implementation of an STL-based string class.
//
//    $Id: KaiString.cpp,v 1.81 2007-12-23 19:58:16 kostya Exp $
//    
//==============================================================================

//
// Disable STL-related compiler warnings
//
#pragma warning (disable: 4786)
#pragma warning (disable: 4231)

#include "KaiGeneric.h"
#include "KaiException.h"
#include "KaiTypes.h"
#include "KaiString.h"

using namespace KaiLibMFC;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
// Helper struct
//
ST_Token::ST_Token()
{
//    v_Null();
}

ST_Token::ST_Token (et_TokenType eo_type, 
                    int i_offset, 
                    int i_diacrOffset, 
                    unsigned int ui_font, 
                    int i_length) :
  eo_TokenType (eo_type),
  i_Offset (i_offset),
  i_DiacrOffset (i_diacrOffset),
  ui_FontDescriptor (ui_font),
  i_Length (i_length)
{
    b_Measured = false;
    i_ScreenLength = -1;
    i_ScreenHeight = -1;
    i_ExtraSpace = -1;
    i_PrintLength = -1;
    i_ExtraPrintSpace = -1;
}

ST_Token::ST_Token (const ST_Token& st_rhs) :
  eo_TokenType (st_rhs.eo_TokenType),
  i_Offset (st_rhs.i_Offset),
  i_DiacrOffset (st_rhs.i_DiacrOffset),
  ui_FontDescriptor (st_rhs.ui_FontDescriptor),
  i_Length (st_rhs.i_Length),
  b_Measured (st_rhs.b_Measured),
  i_ScreenLength (st_rhs.i_ScreenLength),
  i_ScreenHeight (st_rhs.i_ScreenHeight),
  i_ExtraSpace (st_rhs.i_ExtraSpace),
  i_PrintLength (st_rhs.i_PrintLength),
  i_ExtraPrintSpace (st_rhs.i_ExtraPrintSpace)
{}

/*
void ST_Token::v_Null()
{
    eo_TokenType = ec_TokenTypeFront;
    i_Offset = -1;
    i_DiacrOffset = -1;
    ui_FontDescriptor = 0;
    i_Length = -1;
    b_Measured = false;
    i_ScreenLength = -1;
    i_ScreenHeight = -1;
    i_ExtraSpace = -1;
    i_ExtraSpace = -1;
    i_PrintLength = -1;
    i_ExtraPrintSpace = -1;
}
*/

ST_Token& ST_Token::operator = (const ST_Token& rst_rhs)
{
    eo_TokenType = rst_rhs.eo_TokenType;
    i_Offset = rst_rhs.i_Offset;
    i_DiacrOffset = rst_rhs.i_DiacrOffset;
    ui_FontDescriptor = rst_rhs.ui_FontDescriptor;
    i_Length = rst_rhs.i_Length;
    b_Measured = rst_rhs.b_Measured;
    i_ScreenLength = rst_rhs.i_ScreenLength;
    i_ScreenHeight = rst_rhs.i_ScreenHeight;
    i_ExtraSpace = rst_rhs.i_ExtraSpace;
    i_PrintLength = rst_rhs.i_PrintLength;
    i_ExtraPrintSpace = rst_rhs.i_ExtraPrintSpace;

    return *this;
}

//
// These operators are needed so we can sort tokens:
//
bool ST_Token::operator < (const ST_Token& st_token) const
{
    return i_Offset < st_token.i_Offset;
}

bool ST_Token::operator == (const ST_Token& st_token) const
{
    bool b_type = (eo_TokenType == st_token.eo_TokenType);
    bool b_offset = (i_Offset == st_token.i_Offset);
    bool b_dOffset = (i_DiacrOffset == st_token.i_DiacrOffset);
    bool b_length = (i_Length == st_token.i_Length);

    return (b_type && b_offset && b_dOffset && b_length);
}

//
// Ctors
//
CT_KaiString::CT_KaiString()
{
    v_Null_();
}

CT_KaiString::CT_KaiString (const std::wstring& str_text,
                            const std::wstring& str_break,
                            const std::wstring& str_tab,
                            const std::wstring& str_escape,
                            const std::wstring& str_punctuation,
                            const std::wstring& str_endOfParagraph)
{
    v_Null_();

    assign (str_text);
    str_Break_ = str_break;
    str_Tab_ = str_tab;
    str_Escape_ = str_escape;
    str_Punctuation_ = str_punctuation;
    str_EndOfParagraph_ = str_endOfParagraph;

    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

CT_KaiString::CT_KaiString (LPCTSTR qcz_text,
                            LPCTSTR qcz_break,
                            LPCTSTR qcz_tab,
                            LPCTSTR qcz_escape,
                            LPCTSTR qcz_punctuation,
                            LPCTSTR qcz_endOfParagraph)
{
    v_Null_();

    assign (qcz_text);
    str_Break_ = qcz_break;
    str_Tab_ = qcz_tab;
    str_Escape_ = qcz_escape;
    str_Punctuation_ = qcz_punctuation;
    str_EndOfParagraph_ = qcz_endOfParagraph;

    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

CT_KaiString::CT_KaiString (const CT_KaiString& kstr)
{
    v_Null_();

    str_Break_ = kstr.str_Break_;
    str_Tab_ = kstr.str_Tab_;
    str_Escape_ = kstr.str_Escape_;
    str_Punctuation_ = kstr.str_Punctuation_;
    str_EndOfParagraph_ = kstr.str_EndOfParagraph_;

    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;

    vo_LastTokenizedContents_ = kstr.vo_LastTokenizedContents_;
    vo_Tokens_ = kstr.vo_Tokens_;

    assign (static_cast <std::wstring> (kstr));
}

//
// Dtor
//
/*
CT_KaiString::~CT_KaiString()
{
    v_Null_();
}
*/

void CT_KaiString::v_CopyAttributes (const CT_KaiString& kstr)
{
    str_Break_ = kstr.str_Break_;
    str_Tab_ = kstr.str_Tab_;
    str_Escape_ = kstr.str_Escape_;
    str_Punctuation_ = kstr.str_Punctuation_;
    str_EndOfParagraph_ = kstr.str_EndOfParagraph_;
    str_Separators_ = kstr.str_Separators_;
}

void CT_KaiString::v_ClearAttributes()
{
    str_Break_ = std::wstring(_T(""));
    str_Tab_.erase();
    str_Escape_.erase();
    str_Punctuation_.erase();
    str_EndOfParagraph_.erase();

    vo_LastTokenizedContents_.clear();
    vo_LastTokenizedContents_.push_back (std::wstring(_T("")));
    vo_LastTokenizedContents_.push_back (std::wstring(_T("")));
    vo_Tokens_.clear();
}

void CT_KaiString::v_Null_()
{
    this->erase();
    str_Break_ = std::wstring(_T(""));
    str_Tab_.erase();
    str_Escape_.erase();
    str_Punctuation_.erase();
    str_EndOfParagraph_.erase();

    vo_LastTokenizedContents_.clear();
    vo_LastTokenizedContents_.push_back (std::wstring(_T("")));
    vo_LastTokenizedContents_.push_back (std::wstring(_T("")));
    vo_Tokens_.clear();
}

//
// Assignment
//
void CT_KaiString::v_DeepCopy_ (const CT_KaiString& kstr)
{
    *this = kstr;
}

CT_KaiString& CT_KaiString::operator = (const CT_KaiString& kstr)
{
    v_Null_();

    assign (kstr);
    str_Break_ = kstr.str_Break_;
    str_Tab_ = kstr.str_Tab_;
    str_Escape_ = kstr.str_Escape_;
    str_Punctuation_ = kstr.str_Punctuation_;
    str_EndOfParagraph_ = kstr.str_EndOfParagraph_;
    str_Separators_ = kstr.str_Separators_;

    vo_LastTokenizedContents_ = kstr.vo_LastTokenizedContents_;
    vo_Tokens_ = kstr.vo_Tokens_;

    return *this;
}

CT_KaiString& CT_KaiString::operator = (const std::wstring& str)
{
    assign (str);
    return *this;
}

CT_KaiString& CT_KaiString::operator = (LPCTSTR pchr_)
{
    assign (pchr_);
    return *this;
}

//
// --- Private method(s) ---
//
void CT_KaiString::v_Synchronize_()
{
    if ((vo_LastTokenizedContents_[0] != static_cast <std::wstring> (*this)) ||
        (vo_LastTokenizedContents_[1] != str_Separators_))
    {
        vo_LastTokenizedContents_.clear();
        vo_LastTokenizedContents_.push_back (std::wstring(_T("")));
        vo_LastTokenizedContents_.push_back (std::wstring(_T("")));

        vo_Tokens_.clear();

        bool b_ = b_Tokenize_();

        vo_LastTokenizedContents_[0] = static_cast <std::wstring> (*this);
        vo_LastTokenizedContents_[1] = str_Separators_;
    }
    
}    //  v_Synchronize_

bool CT_KaiString::b_Tokenize_()
{
    if (length() == 0)
    {    
        INFO_TRACE (_T("Empty string"));
        return true;
    }

    std::wstring::size_type se_tokenStart = 0;
    std::wstring::size_type se_at = 0;
    et_TokenType eo_type = ec_TokenTypeFront;

    while (se_at != std::wstring::npos)
    {
        int i_length = 0;
        unsigned int ui_font = 0;
        int i_dOffset = -1;

        switch (eo_type)
        {
            case ec_TokenTypeFront:
            {
                se_at = 0;
                break;
            }
            case ec_TokenText:
            {
                se_at = find_first_of (str_Separators_, se_at);
                if (std::wstring::npos == se_at)
                {
                    break;
//                    continue;
                }
                _TCHAR chr_at = this->operator[] (se_at);
                while ((se_at != std::wstring::npos) &&
                       (str_Escape_.find (chr_at) != std::wstring::npos) &&
                       (se_at < length() - 1) &&
                       (this->operator[] (se_at) == this->operator[] (se_at+1)))

                {
                    se_at += 2;
                    se_at = find_first_of (str_Separators_, se_at);
                }
                break;
            }
            case ec_TokenDiacritics:
            case ec_TokenFormatting:
            case ec_TokenHiddenTag:
            {
                se_at = find_first_of (str_Escape_, se_tokenStart + 2) + 1;
                if (se_at == std::wstring::npos)
                {
                    ERROR_TRACE (_T("Unterminated escape sequence."));
                    return false;            
                }
                if ((ec_TokenFormatting == eo_type) || (ec_TokenHiddenTag == eo_type))
                {
                    std::wstring str_fd = substr (se_tokenStart + 2, 
                                                  se_at - se_tokenStart - 3);
                    if (ec_TokenFormatting == eo_type)
                    {
                        ui_font = ui_STRToUI (str_fd);
                    }
                    if (length() == se_at)
                    {
                        se_at = std::wstring::npos;
                    }
                }
                break;
            }
            case ec_TokenSpace:
            {
                se_at = find_first_not_of (str_Break_, se_at);
                break;
            }
            case ec_TokenTab:
            {
                se_at = find_first_not_of (str_Tab_, se_at);
                break;
            }
            case ec_TokenPunctuation:
            {
                se_at = find_first_not_of (str_Punctuation_, se_at);
                break;
            }
            case ec_TokenEndOfParagraph:
            {
                if (se_at != length() - 1)
                {
                    ERROR_TRACE (_T("Misplaced end of paragraph char."));
                    return false;            
                }
                se_at = std::wstring::npos;
                break;
            }
            case ec_TokenTypeBack:
            default:
            {
                ERROR_TRACE (_T("Unknown token type."));
                return false;
            }

        }    // switch ...

        if (eo_type != ec_TokenTypeFront)
        {
            int i_length = (se_at == std::wstring::npos) 
                            ? (int) (length() - se_tokenStart)
                            : (int) (se_at - se_tokenStart);
            ST_Token st_t (eo_type, (int)se_tokenStart, i_dOffset, ui_font, i_length);
            vo_Tokens_.push_back (st_t);
        }

        if (se_at == std::wstring::npos)
        {
            continue;
        }

        eo_type = eo_TokenTypeFromOffset_ ((int)se_at);
        se_tokenStart = se_at;

    }    // while (...)

    return true;

}    //  bool CT_KaiString::b_Tokenize_()

et_TokenType CT_KaiString::eo_TokenTypeFromOffset_ (int i_offset)
{
    et_TokenType eo_type = ec_TokenTypeFront;

    _TCHAR chr_at = at (i_offset);
    if (str_Separators_.find (chr_at) == std::wstring::npos)
    {
        eo_type = ec_TokenText;
    }
    else
    {
        if (str_Escape_.find (chr_at) != std::wstring::npos)
        {
            if (i_offset < (int)length() - 1)
            {
                if (at (i_offset + 1) != chr_at)
                {
                    if (at (i_offset + 1) == TCHAR ('D'))
                    {
                        eo_type = ec_TokenDiacritics;
                    }
                    else
                    {
                        if (at (i_offset + 1) == TCHAR ('F'))
                        {
                            eo_type = ec_TokenFormatting;
                        }
                        else
                        {
                            if (at (i_offset + 1) == TCHAR ('T'))
                            {
                                eo_type = ec_TokenHiddenTag;
                            }
                            else
                            {
                                ERROR_TRACE (_T("Unknown token type."));
                                return ec_TokenTypeFront;
                            }
                        }
                    }
                }
                else
                {
                    eo_type = ec_TokenText;
                }
            }    // if (se_at < length() - 1)
            else
            {
                ERROR_TRACE (_T("Illegal escape sequence."));
                return ec_TokenTypeFront;
            }
        }    // escape
        else 
        {
            if (str_Break_.find (chr_at) != std::wstring::npos)
            {
                eo_type = ec_TokenSpace;
            }
            else if (str_Tab_.find (chr_at) != std::wstring::npos)
            {
                eo_type = ec_TokenTab;
            }
            else if (str_Punctuation_.find (chr_at) != std::wstring::npos)
            {
                eo_type = ec_TokenPunctuation;
            }
            else if (str_EndOfParagraph_.find (chr_at) != std::wstring::npos)
            {
                eo_type = ec_TokenEndOfParagraph;
            }
        }
    }    // else (i.e. separators)

    return eo_type;

}    //  eo_TokenTypeFromOffset_ (...)

et_TokenType CT_KaiString::eo_TokenTypeFromChar_ (_TUCHAR chr_)
{
    et_TokenType eo_type = ec_TokenTypeFront;

    if (str_Separators_.find (chr_) == std::wstring::npos)
    {
        eo_type = ec_TokenText;
    }
    else
    {
        if (str_Escape_.find (chr_) != std::wstring::npos)
        {
            ERROR_TRACE (_T("Illegal character."));
        }    // escape
        else 
        {
            if (str_Break_.find (chr_) != std::wstring::npos)
            {
                eo_type = ec_TokenSpace;
            }
            else if (str_Tab_.find (chr_) != std::wstring::npos)
            {
                eo_type = ec_TokenTab;
            }
            else if (str_Punctuation_.find (chr_) != std::wstring::npos)
            {
                eo_type = ec_TokenPunctuation;
            }
            else if (str_EndOfParagraph_.find (chr_) != std::wstring::npos)
            {
                ERROR_TRACE (_T("Illegal character."));
            }
        }
    }    // else (i.e. separators)

    return eo_type;

}    //  eo_TokenTypeFromChar_ (...)

bool CT_KaiString::b_InsertChar_ (int i_offset, 
                                  _TUCHAR chr_,
                                  int& i_lastTokenBefore, // counting from start
                                  int& i_tokensAfter)     // counting from END
{
	std::vector<ST_Token>::iterator it_first = vo_Tokens_.begin();
    for (; it_first != vo_Tokens_.end(); ++it_first)
    {
        if ((*it_first).i_Offset + (*it_first).i_Length > i_offset)
        {
            break;
        }
    }

    if (vo_Tokens_.begin() == it_first)
    {
        ERROR_TRACE (_T("Bad insertion pos. or no formatting"));
        return false;
    }

    if (vo_Tokens_.end() == it_first)
    {
        ERROR_TRACE (_T("Insertion pos. not found"));
        return false;
    }

    // Insertion before font (except for paragraph initial)
    if ((it_first > vo_Tokens_.begin() + 1) && (i_offset == (*it_first).i_Offset))
    {
        std::vector<ST_Token>::iterator it_prev = it_first - 1;
        while (it_prev > vo_Tokens_.begin())
        {
            if (!(*it_prev).b_IsPrintable())
            {
                --it_prev;
            }
            else
            {
                break;
            }
        }

        if (!(*it_first).b_IsPrintable())
        {
            ATLASSERT(0);
            ERROR_TRACE (_T("Illegal token type."));
            return false;
        }

        it_first = it_prev;
        i_offset = (*it_first).i_Offset + (*it_first).i_Length;

    }

    int i_tokensBefore = (int)(it_first - vo_Tokens_.begin());
//    ATLASSERT (i_tokensBefore >= 1);
    i_lastTokenBefore = i_tokensBefore - 1;
    i_tokensAfter = (int)(vo_Tokens_.size() - i_tokensBefore - 1);
                    // counting from end

    insert (begin() + i_offset, chr_);

    et_TokenType eo_type = eo_TokenTypeFromChar_ (chr_);
    bool b_createNew = true;
    if (eo_type == (*it_first).eo_TokenType)
    {
        (*it_first).i_Length++;
        (*it_first).b_Measured = false;
        b_createNew = false;
    }
    else
    {
        if (i_offset == (*it_first).i_Offset)
        {
            ATLASSERT (vo_Tokens_.begin() != it_first);
			std::vector<ST_Token>::iterator it_prev = it_first - 1;
            if ((*it_prev).eo_TokenType == eo_type)
            {
                it_first = it_prev;
                (*it_first).i_Length++;
                (*it_first).b_Measured = false;
                --i_lastTokenBefore;
                b_createNew = false;
            }
        }
    }
    
    if (b_createNew)
    {
        ST_Token st_new (eo_TokenTypeFromChar_ (chr_), 
                         i_offset, 
                         0, 
                         (*it_first).ui_FontDescriptor, 
                         1);
        if (i_offset == (*it_first).i_Offset)
        {
            it_first = vo_Tokens_.insert (it_first, st_new);
//            --it_first;
        }
        else
        {
            int i_oldTokenLength = (*it_first).i_Length;
            (*it_first).i_Length = i_offset - (*it_first).i_Offset;
            (*it_first).b_Measured = false;
            ST_Token st_next ((*it_first).eo_TokenType,
                              i_offset + st_new.i_Length,
                              0,
                              st_new.ui_FontDescriptor,
                              i_oldTokenLength - (*it_first).i_Length);
            it_first = vo_Tokens_.insert (it_first + 1, st_new);
            it_first = vo_Tokens_.insert (it_first + 1, st_next);
        }
    }

	std::vector<ST_Token>::iterator it_t = it_first + 1;
    for (; it_t != vo_Tokens_.end(); ++it_t)
    {
        (*it_t).i_Offset++;
    }

    vo_LastTokenizedContents_[0] = static_cast <std::wstring> (*this);
    vo_LastTokenizedContents_[1] = str_Separators_;

    return true;

}    //  b_InsertChar_ (...)

bool CT_KaiString::b_DeleteChar_ (int i_offset, 
                                  bool& b_canOptimize,
                                  int& i_lastTokenBefore,   // counting from start
                                  int& i_tokensAfter)       // counting from END
{
    b_canOptimize = false;  // we don't know yet

	std::vector<ST_Token>::iterator it_first = vo_Tokens_.begin();
    for (; it_first != vo_Tokens_.end(); ++it_first)
    {
        if ((*it_first).i_Offset + (*it_first).i_Length > i_offset)
        {
            break;
        }
    }

    if (vo_Tokens_.begin() == it_first)
    {
        ERROR_TRACE (_T("Bad deletion pos. or no formatting"));
        return false;
    }

    if (vo_Tokens_.end() == it_first)
    {
        ERROR_TRACE (_T("Deletion pos. not found"));
        return false;
    }

    erase (begin() + i_offset);

    int i_tokensBefore = (int)(it_first - vo_Tokens_.begin());
    ATLASSERT (i_tokensBefore >= 1);
    i_tokensAfter = (int)(vo_Tokens_.size() - i_tokensBefore - 1);
                    // 0-based num counting from end

    if (1 == (*it_first).i_Length)
    {
        if (vo_Tokens_.begin() != it_first)
        {
            if (ec_TokenFormatting == (*(it_first - 1)).eo_TokenType)
            {
                if (vo_Tokens_.end() != (it_first + 1))
                {
                    if (ec_TokenFormatting == (*(it_first + 1)).eo_TokenType)
                    {
                        --i_tokensBefore;
                        return true;    // don't optimize
                    }
                }
            }
        }
    }

    b_canOptimize = true;

    if (1 < (*it_first).i_Length)
    {
        (*it_first).i_Length--;
        (*it_first).b_Measured = false;
    }
    else
    {
        if (it_first + 1 != vo_Tokens_.end())
        {
			std::vector<ST_Token>::iterator it_previous = it_first - 1;
			std::vector<ST_Token>::iterator it_next = it_first + 1;

            if ((it_next != vo_Tokens_.end()) && 
                ((*it_previous).eo_TokenType == (*it_next).eo_TokenType))
            {
                if (((*it_previous).eo_TokenType == ec_TokenText) ||
                    ((*it_previous).eo_TokenType == ec_TokenPunctuation) ||
                    ((*it_previous).eo_TokenType == ec_TokenSpace))
                {
                    (*it_previous).b_Measured = false;
                    (*it_previous).i_Length += (*it_next).i_Length;
                    vo_Tokens_.erase (it_first, it_next + 1);
                    --i_tokensAfter;
                    --i_tokensBefore;
                }
                else
                {
                    vo_Tokens_.erase (it_first);
                }
            }
            else
            {
                vo_Tokens_.erase (it_first);
            }
        }
    }

	std::vector<ST_Token>::iterator it_t = vo_Tokens_.begin() + 
                                      (int)(vo_Tokens_.size() - i_tokensAfter);
    for (; it_t != vo_Tokens_.end(); ++it_t)
    {
        (*it_t).i_Offset--;
    }

    i_lastTokenBefore = i_tokensBefore - 1;

    vo_LastTokenizedContents_[0] = static_cast <std::wstring> (*this);
    vo_LastTokenizedContents_[1] = str_Separators_;

    return true;

}    //  b_DeleteChar_ (...)

//
//  --- Public methods ---
//

//
// Serialization
//
void CT_KaiString::v_Load (CFile& co_file)
{
    v_Null_();

    int i_bytesRead = 0;
    int i_tokens = 0;

    V_LoadStlString (co_file, str_Break_);
    V_LoadStlString (co_file, str_Tab_);
    V_LoadStlString (co_file, str_Escape_);
    V_LoadStlString (co_file, str_Punctuation_);
    V_LoadStlString (co_file, str_EndOfParagraph_);
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;

    V_LoadStlString (co_file, static_cast <std::wstring&> (*this));

}    //  v_Load (...)

void CT_KaiString::v_Store (CFile& co_file)
{
    V_StoreStlString (co_file, str_Break_);
    V_StoreStlString (co_file, str_Tab_);
    V_StoreStlString (co_file, str_Escape_);
    V_StoreStlString (co_file, str_Punctuation_);
    V_StoreStlString (co_file, str_EndOfParagraph_);

    V_StoreStlString (co_file, static_cast <std::wstring>(*this));

}    //  v_Store (...)

void CT_KaiString::v_StoreAsPlainText (CFile& co_file)
{
    for (unsigned int ui_token = 0; ui_token < vo_Tokens_.size(); ++ui_token)
    {
        ST_Token& rst_token = vo_Tokens_[ui_token];
        if ((ec_TokenText == rst_token.eo_TokenType) ||
            (ec_TokenSpace == rst_token.eo_TokenType) ||
            (ec_TokenTab == rst_token.eo_TokenType) ||
            (ec_TokenPunctuation == rst_token.eo_TokenType))
        {
            LPCTSTR csz_text = this->c_str();
            co_file.Write (&csz_text[rst_token.i_Offset], rst_token.i_Length * sizeof (TCHAR));
        }
        if (ec_TokenEndOfParagraph == rst_token.eo_TokenType)
        {
            co_file.Write (_T("\x0D\x0A"), 4);
        }
    }
}    //  v_StoreAsText (...)

//
// Manipulation
//
void CT_KaiString::v_ToLower()
{
    if (empty())
    {
        return;
    }
    v_Synchronize_();
    for (unsigned int ui_token = 0; ui_token < vo_Tokens_.size(); ++ui_token)
    {
        ST_Token st_token = vo_Tokens_[ui_token];
        if (st_token.eo_TokenType != ec_TokenText)
        {
            continue;
        }
        if (st_token.i_Length < 1)
        {
            continue;
        }
        _TCHAR * qcz_data = new _TCHAR[length() + 1];
        memset ((void *)qcz_data, 0, (sizeof (_TCHAR)*(length() + 1)));
		_Copy_s (qcz_data, sizeof (_TCHAR)*(length() + 1), length());
        int i_firstAfter = st_token.i_Offset + st_token.i_Length;
        _TCHAR chr_save = qcz_data[i_firstAfter];
        qcz_data[i_firstAfter] = (_T('\0'));
        _tcslwr_s (qcz_data + st_token.i_Offset, st_token.i_Length);
        qcz_data[i_firstAfter] = chr_save;
        assign (qcz_data);
        delete[] qcz_data;
    }
}

void CT_KaiString::v_ToUpper()
{
    if (empty())
    {
        return;
    }
    v_Synchronize_();

    _TCHAR * sz_copy = new _TCHAR [length() + 1];
    sz_copy[length()] = _T('\0');
    _Copy_s (sz_copy, sizeof (_TCHAR)*(length() + 1), length());

    // Needed to avoid apparent MSFT bug
    char const * p_locale = setlocale (LC_CTYPE, ".ACP");
    //

    for (unsigned int ui_token = 0; ui_token < vo_Tokens_.size(); ++ui_token)
    {
        ST_Token st_token = vo_Tokens_[ui_token];
        if (st_token.eo_TokenType != ec_TokenText)
        {
            continue;
        }
        if (st_token.i_Length < 1)
        {
            continue;
        }
//        _TCHAR * qcz_data = new _TCHAR[length() + 1];
//        memset ((void *)qcz_data, 0, (sizeof (_TCHAR)*(length() + 1)));
//      _Copy_s (qcz_data, sizeof (_TCHAR)*(length() + 1), length());
        int i_firstAfter = st_token.i_Offset + st_token.i_Length;
        _TCHAR chr_save = sz_copy[i_firstAfter];
        sz_copy[i_firstAfter] = (_T('\0'));
        _tcsupr_s (&sz_copy[st_token.i_Offset], st_token.i_Length+1);
        sz_copy[i_firstAfter] = chr_save;
    }

    // Restore locale (MSFT bug)
    setlocale (LC_CTYPE, p_locale);

    assign (sz_copy);
    delete[] sz_copy;
}

//
// By default, we trim whitespace including tabs
//
void CT_KaiString::v_TrimLeft()
{
    if (empty())
    {
        return;
    }

    std::wstring str_charsToTrim = str_Break_ + str_Tab_;    
    std::wstring::size_type se_offset = find_first_not_of (str_charsToTrim);    
    erase (0, se_offset);    

}

void CT_KaiString::v_TrimLeft (const std::wstring& str_charsToTrim)
{
    if (empty())
    {
        return;
    }

    if (str_charsToTrim.empty())
    {
        INFO_TRACE (_T("No trim characters"))
        return;
    }
    else
    {
        INFO_TRACE (std::wstring (_T("Trim characters: ")) + str_charsToTrim);
    }

    std::wstring::size_type se_offset = find_first_not_of (str_charsToTrim);    
    erase (0, se_offset);    

}

void CT_KaiString::v_TrimRight()
{
    if (empty())
    {
        return;
    }

    std::wstring str_charsToTrim = str_Break_ + str_Tab_;
    if (str_charsToTrim.empty())
    {
        INFO_TRACE (_T("No trim characters"))
        return;
    }
    std::wstring::size_type se_offset = find_last_not_of (str_charsToTrim);
    erase (++se_offset);

}

void CT_KaiString::v_TrimRight (const std::wstring& str_charsToTrim)
{
    if (empty())
    {
        return;
    }
    
    if (str_charsToTrim.empty())
    {
        INFO_TRACE (_T("No trim characters"))
        return;
    }
    else
    {
        INFO_TRACE (std::wstring (_T("Trim characters: ")) + str_charsToTrim);
        return;
    }
    std::wstring::size_type se_offset = find_last_not_of (str_charsToTrim);
    erase (++se_offset);

}

int CT_KaiString::i_CompareNoCase (const std::wstring& str) const
{
    int result = _tcsicmp (c_str(), str.c_str());

    return result;
}

ST_Token CT_KaiString::st_GetToken (int i_at) 
{
    v_Synchronize_();
    return vo_Tokens_[i_at]; 
}

const ST_Token& CT_KaiString::rst_GetToken (int i_at) 
{
    v_Synchronize_();
    return vo_Tokens_[i_at]; 
}

std::wstring CT_KaiString::str_GetToken (int i_at)
{
    v_Synchronize_();
    const ST_Token& st_t = rst_GetToken (i_at);
    return substr (st_t.i_Offset, st_t.i_Length);
}

int CT_KaiString::i_GetNumOfTokens() 
{
    v_Synchronize_();
    return (int)vo_Tokens_.size(); 
}

int CT_KaiString::i_NTokens() 
{
    v_Synchronize_();
    return (int)vo_Tokens_.size(); 
}

int CT_KaiString::i_NFields (et_TokenType eo_type)
{
    v_Synchronize_();
    return i_GetNumOfFields (eo_type); 
}

int CT_KaiString::i_NFields (int i_offset, 
                             int i_length, 
                             et_TokenType eo_type)
{
    v_Synchronize_();
    return i_GetNumOfFields (i_offset, 
                             i_length, 
                             eo_type);
}

void CT_KaiString::v_SetBreakChars (const std::wstring& str_break) 
{
    str_Break_ = str_break;
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

std::wstring CT_KaiString::str_GetBreakChars()
{ 
    return str_Break_;
}

void CT_KaiString::v_SetTabChars (const std::wstring& str_tab) 
{
    str_Tab_ = str_tab;
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

std::wstring CT_KaiString::str_GetTabChars()
{
    return str_Tab_;
}

void CT_KaiString::v_SetEscChars (const std::wstring& str_escape) 
{ 
    str_Escape_ = str_escape;
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

std::wstring CT_KaiString::str_GetEscChars() 
{
    return str_Escape_;
}

void CT_KaiString::v_SetPunctChars (const std::wstring& str_punctuation) 
{
    str_Punctuation_ = str_punctuation;
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

std::wstring CT_KaiString::str_GetPunctChars()
{
    return str_Punctuation_;
}

void CT_KaiString::v_SetEndOfParagraphChars 
 (const std::wstring& str_endOfParagraph) 
{
    str_EndOfParagraph_ =  str_endOfParagraph;
    str_Separators_ = str_Break_ + 
                      str_Tab_ + 
                      str_Escape_ + 
                      str_Punctuation_ + 
                      str_EndOfParagraph_;
}

std::wstring CT_KaiString::str_GetEndOfParagraphChars ()
{
    return str_EndOfParagraph_; 
}

std::wstring CT_KaiString::str_GetField (int i_at, 
                                       et_TokenType eo_type)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    int i_token = 0;
	unsigned int ui_ = 0;
    for (; (ui_ < vo_Tokens_.size()); ++ui_)
    {
        if (vo_Tokens_[ui_].eo_TokenType == eo_type)
        {
            if (i_token < i_at)
            {
                i_token++;
            }
            else
            {
                break;
            }
        }
    }
    if (ui_ == vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[ui_];
    std::wstring str_token = substr (st_token.i_Offset, st_token.i_Length);

    return str_token;
}

ST_Token CT_KaiString::st_GetField (int i_at, et_TokenType eo_type)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    int i_token = 0;
	unsigned int ui_ = 0;
    for (; (ui_ < vo_Tokens_.size()); ++ui_)
    {
        if (vo_Tokens_[ui_].eo_TokenType == eo_type)
        {
            if (i_token < i_at)
            {
                i_token++;
            }
            else
            {
                break;
            }
        }
    }
    if (ui_ == vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }
    return vo_Tokens_[ui_];
}

ST_Token CT_KaiString::st_GetFieldFromOffset (int i_offset,
                                              et_TokenType eo_type)
{
    v_Synchronize_();

    int i_token = 0;
	unsigned int ui_ = 0;
    for (; (ui_ < vo_Tokens_.size()); ++ui_)
    {
        if ((vo_Tokens_[ui_].eo_TokenType == eo_type) &&
            (vo_Tokens_[ui_].i_Offset >= i_offset))
        {
            break;
        }
    }
    if (ui_ == vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }
    return vo_Tokens_[ui_];
}

ST_Token CT_KaiString::st_GetTokenFromOffset (int i_offset)
{
    int i_at = i_GetTokenNumFromOffset (i_offset);
    return vo_Tokens_[i_at];
}

int CT_KaiString::i_GetTokenNumFromOffset (int i_offset)
{
    v_Synchronize_();

    int i_token = 0;
	unsigned int ui_ = 0;
    for (; (ui_ < vo_Tokens_.size()); ++ui_)
    {
        ST_Token st_t = vo_Tokens_[ui_];
        if (st_t.i_Offset > i_offset)
        {
            break;
        }
    }

    if (ui_ == 0)
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    return static_cast<int>(ui_-1);

}   //  i_GetTokenNumFromOffset (...)

et_TokenType CT_KaiString::eo_GetTokenType (int i_at)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[i_at];

    return st_token.eo_TokenType;
}

et_TokenType CT_KaiString::eo_GetTokenType (int i_offset, int i_at)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

	unsigned int ui_ = 0;
    for (; ui_ < vo_Tokens_.size(); ++ui_)
    {
        if (vo_Tokens_[ui_].i_Offset >= i_offset)
        {
            break;
        }
    }

    if (i_at + ui_ >= vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[i_at+ui_];

    return st_token.eo_TokenType;

}    //  eo_GetTokenType (...)

bool CT_KaiString::b_GetNextToken (ST_Token& st_token)
{
    v_Synchronize_();

    //
    // Token --> iterator
    //
	std::vector<ST_Token>::iterator it_t = std::find (vo_Tokens_.begin(), 
                                                      vo_Tokens_.end(), 
                                                      st_token);
    if (it_t == vo_Tokens_.end())
    {
//        st_token.v_Null();
        throw CT_KaiException (CT_KaiException::ec_StringFieldOutOfRange,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T("b_GetNextToken (...)"),
                               _T("Token not in array"));
    }

    ++it_t;
    if (it_t == vo_Tokens_.end())
    {
        return false;
    }
    else
    {
        st_token = *it_t;
    }

    return true;

}    //  bool CT_KaiString::b_GetNextToken (ST_Token& st_token)

bool CT_KaiString::b_GetPrevToken (ST_Token& st_token)
{
    v_Synchronize_();

    //
    // Token --> iterator
    //
	std::vector<ST_Token>::iterator it_t = std::find (vo_Tokens_.begin(), 
											          vo_Tokens_.end(), 
                                                      st_token);
    if (it_t == vo_Tokens_.end())
    {
//        st_token.v_Null();
        throw CT_KaiException (CT_KaiException::ec_StringFieldOutOfRange,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T("b_GetPrevToken (...)"),
                               _T("Token not in array"));
    }

    if (it_t == vo_Tokens_.begin())
    {
        return false;
    }

    --it_t;
    st_token = *it_t;

    return true;

}    //  bool CT_KaiString::b_GetPrevToken (ST_Token& st_token)

int CT_KaiString::i_GetTokenNum (const ST_Token& st_token)
{
    v_Synchronize_();

    std::vector<ST_Token>::iterator it_t = std::find (vo_Tokens_.begin(), 
                                                      vo_Tokens_.end(), 
                                                      st_token);
    if (it_t == vo_Tokens_.end())
    {
        throw CT_KaiException (CT_KaiException::ec_StringFieldOutOfRange,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T("i_GetTokenNum (...)"),
                               _T("Token not in array"));
    }

    return static_cast<int> (it_t - vo_Tokens_.begin());

}    // int CT_KaiString::i_GetTokenNum (...)

int CT_KaiString::i_GetNumOfFields (et_TokenType eo_type)
{
    v_Synchronize_();

    int i_tokens = 0;
    for (unsigned int ui_ = 0; ui_ < vo_Tokens_.size(); ++ ui_)
    {
        if (vo_Tokens_[ui_].eo_TokenType == eo_type)
        {
            ++i_tokens;
        }
    }
    
    return i_tokens;
}

int CT_KaiString::i_GetNumOfFields (int i_offset, 
                                    int i_length,
                                    et_TokenType eo_type)
{
    v_Synchronize_();

	unsigned int ui_ = 0;
    for (; ui_ < vo_Tokens_.size(); ++ui_)
    {
        if (vo_Tokens_[ui_].i_Offset >= i_offset)
        {
            break;
        }
    }
    int i_tokens = 0;
	unsigned int uj_ = ui_;
    for (; (uj_ < vo_Tokens_.size()) && 
         (vo_Tokens_[uj_].i_Offset < i_offset + i_length); 
         ++ uj_)
    {
        if (vo_Tokens_[uj_].eo_TokenType == eo_type)
        {
            ++i_tokens;
        }
    }
    
    return i_tokens;
}

int CT_KaiString::i_GetFieldLength (int i_at, et_TokenType eo_type)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    int i_token = 0;
	unsigned int ui_ = 0;
    for (; (ui_ < vo_Tokens_.size()); ++ui_)
    {
        if (vo_Tokens_[ui_].eo_TokenType == eo_type)
        {
            if (i_token < i_at)
            {
                i_token++;
            }
            else
            {
                break;
            }
        }
    }
    if (ui_ == vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[i_at];
    return st_token.i_Length;
}

int CT_KaiString::i_GetFieldLength (int i_offset, 
                                    int i_at, 
                                    et_TokenType eo_type)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

	unsigned int ui_ = 0;
    for (; ui_ < vo_Tokens_.size(); ++ui_)
    {
        if (vo_Tokens_[ui_].i_Offset >= i_offset)
        {
            break;
        }
    }
    if (ui_ >= vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    int i_token = 0;
	unsigned int uj_ = ui_;
    for (; (uj_ < vo_Tokens_.size()); ++uj_)
    {
        if (vo_Tokens_[uj_].eo_TokenType == eo_type)
        {
            if (i_token < i_at)
            {
                i_token++;
            }
            else
            {
                break;
            }
        }
    }
    if (uj_ == vo_Tokens_.size())
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[i_at];

    return st_token.i_Length;
}

int CT_KaiString::i_GetVisibleLength()
{
    v_Synchronize_();

    int i_vlength = 0;
    std::vector<ST_Token>::iterator it_t = vo_Tokens_.begin();
    for (; it_t != vo_Tokens_.end(); ++it_t)    
    {
        if ((*it_t).b_IsLinearText())
        {
            i_vlength += (*it_t).i_Length;
        }
    }
    return i_vlength;
}

bool CT_KaiString::b_DeleteToken (int i_at)
{
    v_Synchronize_();

    if (i_at >= static_cast<int>(vo_Tokens_.size()))
    {
        throw CT_KaiException (CT_KaiException::ec_IndexOutOfBounds,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T(__FUNCTION__),
                               _T("vo_Tokens_ member index out of range"));
    }

    ST_Token st_token = vo_Tokens_[i_at];

    std::wstring str_new (substr (0, st_token.i_Offset));
    str_new += substr (st_token.i_Offset + st_token.i_Length);
    assign (str_new);

    v_Synchronize_();

    return true;
}

bool CT_KaiString::b_ExtractWords()
{
    CT_Word co_word (this);
    std::vector<ST_Token>::iterator it_t = vo_Tokens_.begin();
    for (; it_t != vo_Tokens_.end(); ++it_t)
    {
        if ((ec_TokenText == (*it_t).eo_TokenType) ||
            (ec_TokenDiacritics == (*it_t).eo_TokenType))
        {
            co_word.vec_Tokens_.push_back ((int)(it_t - vo_Tokens_.begin()));
        }
        
        if ((ec_TokenSpace == (*it_t).eo_TokenType) ||
            (ec_TokenTab == (*it_t).eo_TokenType))
        {
            if (co_word.vec_Tokens_.size() > 0)
            {
                vo_Words_.push_back (co_word);
                co_word.vec_Tokens_.clear();
            }
        }
        if (ec_TokenEndOfParagraph == (*it_t).eo_TokenType)
        {
            if (co_word.vec_Tokens_.size() > 0)
            {
                vo_Words_.push_back (co_word);
            }
        }
    }

    return true;

}   // GetWords()

bool CT_KaiString::b_SplitNoHyphen (ST_Token& st_token, int i_at)
{
    std::vector<ST_Token>::iterator it_left = std::find (vo_Tokens_.begin(), 
                                                         vo_Tokens_.end(), 
                                                         st_token);
    if (it_left == vo_Tokens_.end())
    {
//        st_token.v_Null();
        throw CT_KaiException (CT_KaiException::ec_StringFieldOutOfRange,
                               _T("KaiString.cpp"),
                               _T("CT_KaiString"),
                               _T("b_SplitNoHyphen (...)"),
                               _T("Token not in array"));
    }

    ST_Token st_next (*it_left);
    st_next.i_Offset = i_at;
    st_next.i_Length = it_left->i_Offset + it_left->i_Length - i_at;
    st_next.b_Measured = false;

//    it_left->eo_SplitType = ec_SplitNoHyphen;
    it_left->i_Length = i_at - it_left->i_Offset;
    it_left->b_Measured = false;

    vo_Tokens_.insert (it_left + 1, st_next);

    return true;

}    // b_SplitNoHyphen (...)

bool CT_KaiString::b_Hyphenate (int i_at)
{
    //
    // *** TBD ***
    //

    return true;
}

//
// Diagnostics
//
void CT_KaiString::v_Dump()
{
    DUMP_TRACE (_T("-------- CT_KaiString -----------------------"));

    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("String contents = ")) +
                 static_cast <std::wstring> (*this));
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("str_Break_ = ")) 
                 + str_Break_);
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("str_Tab_ = ")) + 
                 str_Tab_);
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("str_Escape_ = ")) + 
                 str_Escape_);
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("str_Punctuation_ = "))
                 + str_Punctuation_);
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring 
                        (_T("str_EndOfParagraph_ = ")) + str_EndOfParagraph_);
    DUMP_TRACE (std::wstring (_T("\t")) + 
                 std::wstring (_T("vo_LastTokenizedContents_[1] = ")) +
                 vo_LastTokenizedContents_[1]);
    DUMP_TRACE (std::wstring (_T("\t")) + std::wstring (_T("vo_Tokens_ = ")));
    for (unsigned int ui = 0; ui < vo_Tokens_.size(); ++ui)
    {
        ST_Token st_token = vo_Tokens_[ui];
        std::wstring str_type;
        switch (st_token.eo_TokenType)
        {
            case ec_TokenTypeFront:
            {
                str_type = _T("ec_TokenTypeFront");
                break;
            }
            case ec_TokenText:
            {
                str_type = _T("ec_TokenText");
                break;
            }
            case ec_TokenSpace:
            {
                str_type = _T("ec_TokenSpace");
                break;
            }
            case ec_TokenTab:
            {
                str_type = _T("ec_TokenTab");
                break;
            }
            case ec_TokenDiacritics:
            {
                str_type = _T("ec_TokenDiacritics");
                break;
            }
            case ec_TokenPunctuation:
            {
                str_type = _T("ec_TokenPunctuation");
                break;
            }
            case ec_TokenEndOfParagraph:
            {
                str_type = _T("ec_TokenEndOfParagraph");
                break;
            }
            case ec_TokenFormatting:
            {
                str_type = _T("ec_TokenFormatting");
                break;
            }
            case ec_TokenHiddenTag:
            {
                str_type = _T("ec_TokenHiddenTag");
                break;
            }
            case ec_TokenTypeBack:
            {
                str_type = _T("ec_TokenTypeBack");
                break;
            }
            default:
            {
                str_type = _T("Unknown; numeric value = ");
                str_type += STR_UIToStr 
                 (static_cast <unsigned int>(st_token.eo_TokenType));
            }
        }    // switch (...)

        DUMP_TRACE (std::wstring (_T("\t\t")) + _T("Token type:\t") + str_type);
        DUMP_TRACE (std::wstring (_T("\t\t")) + _T("Offset:\t") + 
                    STR_IToStr (st_token.i_Offset));
        DUMP_TRACE (std::wstring (_T("\t\t")) + _T("Length:\t") + 
                    STR_IToStr (st_token.i_Length));

    } // for (int i = 0; i < vo_Tokens_.size(); ++i)

    DUMP_TRACE (std::wstring (_T("\t")) + 
                 std::wstring (_T("vo_LastTokenizedContents_[0] = ")) +
                 vo_LastTokenizedContents_[0]);

    DUMP_TRACE (_T("-------- CT_KaiString: end of dump ----------"));

}    //  v_Dump()
