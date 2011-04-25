//==============================================================================
//
//    Copyright (c) Konstantin Bogatyrev, 2011
//    
//    Facility: Kai 2.1
//
//    Module description: custom string class.
//
//==============================================================================

#pragma once

#include "StdAfx.h" 

namespace KaiHelpers
{

//
// Helper objects
//
enum et_TokenType
{
    ec_TokenTypeFront    = 10001,
    ec_TokenText,                   // a word as defined in RIS
    ec_TokenSpace,                  // a sequence of break chars
    ec_TokenTab,                    // a sequence of tabs
    ec_TokenDiacritics,             // a sequence of superscripts
    ec_TokenPunctuation,            // a sequence of punct. marks i.e. ; or ?!...
    ec_TokenEndOfParagraph,         // B6 hex
    ec_TokenFormatting,             // formatting
    ec_TokenHiddenTag,
    ec_TokenTypeBack    = ec_TokenHiddenTag + 1
};

/*
enum et_SplitType
{
    ec_SplitTypeFront    = 10101,
    ec_NoSplit,
    ec_SplitNoHyphen,               // join with next token
    ec_Hyphenated,                  // left side of hyphenated word
    ec_SplitTypeBack    = ec_Hyphenated + 1
};
*/

struct AFX_EXT_CLASS ST_Token
{
    et_TokenType eo_TokenType;
    int i_Offset;
    int i_DiacrOffset;                  // do we really need it??
    unsigned int ui_FontDescriptor;     // only if ec_TokenFormatting
    int i_Length;
    bool b_Measured;
    int i_ScreenLength;
    int i_ScreenHeight;
    int i_ExtraSpace;       // space tokens only
    int i_PrintLength;
    int i_ExtraPrintSpace;

    ST_Token();
    ST_Token (et_TokenType eo_type, 
              int i_offset, 
              int i_diacrOffset, 
              unsigned int ui_font, 
              int i_length);
    ST_Token (const ST_Token&);
    ST_Token& operator = (const ST_Token&);
    bool operator < (const ST_Token&) const;
    bool operator == (const ST_Token&) const;

    int i_GetLinearTextLength() const
    {
        if ((ec_TokenTypeFront == eo_TokenType) ||
            (ec_TokenDiacritics == eo_TokenType) ||
            (ec_TokenFormatting == eo_TokenType) ||
            (ec_TokenHiddenTag == eo_TokenType) ||
            (ec_TokenTypeBack == eo_TokenType))
        {
            return 0;
        }
        else
        {
            return i_Length;
        }
    }

    bool b_IsLinearText() const
    {
        if ((ec_TokenTypeFront == eo_TokenType) ||
            (ec_TokenDiacritics == eo_TokenType) ||
            (ec_TokenFormatting == eo_TokenType) ||
            (ec_TokenHiddenTag == eo_TokenType) ||
            (ec_TokenTypeBack == eo_TokenType))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool b_IsPrintable() const
    {
        if ((ec_TokenTypeFront == eo_TokenType) ||
            (ec_TokenFormatting == eo_TokenType) ||
            (ec_TokenHiddenTag == eo_TokenType) ||
            (ec_TokenTypeBack == eo_TokenType))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

class AFX_EXT_CLASS CT_KaiString : public std::wstring
{
friend class CT_Word;

protected:
    //
    // Non-public data members:
    //
    std::wstring str_Break_;
    std::wstring str_Tab_;
    std::wstring str_Escape_;
    std::wstring str_Punctuation_;
    std::wstring str_EndOfParagraph_;
    std::wstring str_Separators_;

//    EXP_IMP_TEMPLATE template class KAI_API std::vector<ST_Token>;
    template class std::vector<ST_Token>;
    std::vector<ST_Token> vo_Tokens_;

//    EXP_IMP_TEMPLATE template class KAI_API std::vector<std::wstring>;
    template class std::vector<std::wstring>;
    std::vector<std::wstring> vo_LastTokenizedContents_;
        // ^-- a snapshot of string as of last tokenization; 1st
        // element is the STL string, 2nd element is a concatenation
        // of separators in the order of declaration
        //

    std::vector<CT_Word> vo_Words_;
    
//
// C-tors and d-tors
//
public:
    CT_KaiString();
    CT_KaiString (const std::wstring& str_text,
                  const std::wstring& str_break = _T(""),
                  const std::wstring& str_tab = _T(""),
                  const std::wstring& str_escape = _T(""),
                  const std::wstring& str_punctuation = _T(""),
                  const std::wstring& str_endOfParagraph = _T(""));
    CT_KaiString (LPCTSTR qcz_text, 
                  LPCTSTR qcz_break = _T(""),
                  LPCTSTR qcz_tab = _T(""),
                  LPCTSTR qcz_escape = _T(""),
                  LPCTSTR qcz_punctuation = _T(""),
                  LPCTSTR qcz_endOfParagraph = _T(""));
    CT_KaiString (const CT_KaiString& kstr);
    virtual ~CT_KaiString() {};

    void v_CopyAttributes (const CT_KaiString&);
    void v_ClearAttributes();

//
// Non-public methods
//
protected:
    void v_Null_();
    void v_DeepCopy_ (const CT_KaiString&);
    void v_Synchronize_();
    bool b_Tokenize_();
    et_TokenType eo_TokenTypeFromOffset_ (int i_offset);
    et_TokenType eo_TokenTypeFromChar_ (_TUCHAR chr_);

    //
    // Optimized single char operations
    //
    bool b_InsertChar_ (int i_offset, 
                        _TUCHAR chr_, 
                        int& i_lastTokenBefore,     // counting from start
                        int& i_tokensAfter);        // counting from END

    // This method allows optimized paragraph rendering if
    //  1. target character is not preceded by diacritic and
    //  2. deletion will not result in a hanging font descriptor
    //  otherwise b_canOptimize will be set to false
    bool b_DeleteChar_ (int i_offset, 
                        bool& b_canOptimize,
                        int& i_lastTokenBefore,   // counting from start
                        int& i_tokensAfter);      // counting from END

//
// Public operators
//
public:
    CT_KaiString& operator = (const CT_KaiString& str);
    CT_KaiString& operator = (const std::wstring& str);
    CT_KaiString& operator = (LPCTSTR pchr_);

//
// Public methods
//
public:
    void v_Load (CFile&);
    void v_Store (CFile&);
    void v_StoreAsPlainText (CFile& co_file);
    void v_ToLower();
    void v_ToUpper();
//    void v_ToTitle();
    void v_TrimLeft();
    void v_TrimLeft (const std::wstring& str_charsToTrim);
    void v_TrimRight();
    void v_TrimRight (const std::wstring& str_charsToTrim);
    int i_CompareNoCase (const std::wstring& str) const;
    std::wstring str_GetField (int i_at, 
                             et_TokenType eo_type = ec_TokenText);
    ST_Token st_GetField (int i_at,
                          et_TokenType eo_type = ec_TokenText);
    ST_Token st_GetFieldFromOffset (int i_offset,
                                    et_TokenType eo_type = ec_TokenText);
    ST_Token st_GetTokenFromOffset (int i_offset);
    int i_GetTokenNumFromOffset (int i_offset);
    et_TokenType eo_GetTokenType (int i_at);
    et_TokenType eo_GetTokenType (int i_offset, int i_at);
    ST_Token st_GetToken (int i_at);
    const ST_Token& rst_GetToken (int i_at);
    std::wstring str_GetToken (int i_at);
    bool b_GetNextToken (ST_Token&);
    bool b_GetPrevToken (ST_Token&);
    int i_GetTokenNum (const ST_Token&);
    int i_GetNumOfFields (et_TokenType eo_type = ec_TokenText);
    int i_GetNumOfFields (int i_offset, 
                          int i_length,
                          et_TokenType eo_type = ec_TokenText);
    int i_NFields (et_TokenType eo_type = ec_TokenText);
    int i_NFields (int i_offset, 
                   int i_length,
                   et_TokenType eo_type = ec_TokenText);
    int i_GetNumOfTokens();
    int i_NTokens();
    int i_GetVisibleLength();
    int i_GetFieldLength (int i_at,
                          et_TokenType eo_type = ec_TokenText);
    int i_GetFieldLength (int i_offset, 
                          int i_at,
                          et_TokenType eo_type = ec_TokenText);

    void v_SetBreakChars (const std::wstring& str_break);
    std::wstring str_GetBreakChars();
    void v_SetTabChars (const std::wstring& str_tab);
    std::wstring str_GetTabChars();
    void v_SetEscChars (const std::wstring& str_escape);
    std::wstring str_GetEscChars();
    void v_SetPunctChars (const std::wstring& str_punctuation);
    std::wstring str_GetPunctChars();
    void v_SetEndOfParagraphChars (const std::wstring& str_endOfParagraph);
    std::wstring str_GetEndOfParagraphChars();

    bool b_DeleteToken (int i_at);

    bool b_ExtractWords();

    bool b_SplitNoHyphen (ST_Token& st_token, int i_at);
    bool b_Hyphenate (int i_at);

    void v_Dump();
};

}   //  namespace KaiLibMFC
