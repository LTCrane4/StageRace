// mEdit.cpp : implementation file
//

#include "stdafx.h"
#include "mEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CmEdit

void AFXAPI DDX_CmEdit(CDataExchange* pDX, int nIDC, CmEdit& rControl, CString& data)
{
    DDX_Control(pDX, nIDC, (CWnd&)rControl);
    if (!pDX->m_bSaveAndValidate)
    {
        rControl.SetData(data);
    }
    else
    {
        data = rControl.GetData();
    }
}

IMPLEMENT_DYNCREATE(CmEdit, CEdit)

CmEdit::CmEdit()
  : m_bInit(false),
    m_strPrompt("_"),
    m_iNumChars(0),
    m_iCurrentChar(0),
    m_bShiftDown(false)
{
    for (int i = 0; i < MAX_CHARS; i++)
        m_Char[i] = NULL;

    //SubclassWindow(pParent->m_hWnd);
    //pParent->SubclassWindow(this->m_hWnd);
}

CmEdit::~CmEdit()
{
    for (int i = 0; i < MAX_CHARS; i++)
        if (m_Char[i] != NULL)
        {
            delete m_Char[i];
            m_Char[i] = NULL;
        }
}

BEGIN_MESSAGE_MAP(CmEdit, CEdit)
	//{{AFX_MSG_MAP(CmEdit)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_PASTE,Paste)
	ON_MESSAGE(WM_CUT,Cut)
	ON_MESSAGE(WM_CLEAR,Delete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CmEdit message handlers

int CmEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    return 0;
}

BOOL CmEdit::PreCreateWindow(CREATESTRUCT& cs) 
{
    cs.style |= WS_TABSTOP | WS_VISIBLE | WS_DISABLED | ES_AUTOHSCROLL;
    cs.dwExStyle |= WS_EX_OVERLAPPEDWINDOW;
    
	return CEdit::PreCreateWindow(cs);
}

void CmEdit::SetData(const CString& data)
{
    CString strWindowText;
    CString strData;
    int     iNextDataChar = 0;
    
    for (int i = 1; i <= m_iNumChars; i++)
    {
        if (m_Char[i - 1]->m_bStaticChar)
        {
            strWindowText += m_Char[i - 1]->m_strValids;
        }
        else
        {
            if (iNextDataChar < data.GetLength())
            {
                strData = data[iNextDataChar++];
                if (m_Char[i - 1]->m_strValids.Find(strData[0]) < 0)
                    strData = m_strPrompt;
            }
            else
            {
                strData = m_strPrompt;
            }
            
            strWindowText += strData[0];
        }
    }
    SetWindowText(strWindowText);
	m_iCurrentChar = 0;
}

CString CmEdit::GetData()
{
    CString strWindowText;
    GetWindowText(strWindowText);
    CString strData;
    
    for (int i = 1; i <= m_iNumChars; i++)
    {
        if (!m_Char[i - 1]->m_bStaticChar)
        {
            if (strWindowText.GetLength() >= i)
            {
                if (strWindowText[i - 1] != m_strPrompt)
                    strData += strWindowText[i - 1];
            }
            else
                strData += m_strPrompt;
        }
    }

    return strData;
}

LRESULT CmEdit::Delete(WPARAM, LPARAM)
{
	this->Delete();
	return NULL;
}

void CmEdit::Delete()
{
	int x, y, z;
	GetSel(x,y);
	CString szReplaceText = "";
	for (z = x; z < y; z++)
	{
		if (m_Char[z]->m_bStaticChar)
		{
			szReplaceText += m_Char[z]->m_strValids;
		}
		else
		{
			szReplaceText += m_strPrompt;
		}
	}
	ReplaceSel(szReplaceText);
	m_iCurrentChar = FindNextChar(x, false, true);
	SetSel(m_iCurrentChar, m_iCurrentChar);
}

void CmEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    switch (nChar)
    {
		case VK_TAB:
			break;
		case VK_BACK:
			if(!(GetStyle()&ES_READONLY))
			{
				int x, y;
				GetSel(x,y);
				if (x != y) Delete();
				else if (m_iCurrentChar > 0)
				{
					int iNewNext = FindNextChar(m_iCurrentChar - 1, false, false);
					if (iNewNext >= 0)
					{
						SetSel(iNewNext, iNewNext + 1);
						ReplaceSel(m_strPrompt);
						m_iCurrentChar = iNewNext;
						SetSel(m_iCurrentChar, m_iCurrentChar);
					}
				}
			}
			break;
		case 3: //Copy
			CEdit::OnChar(nChar, nRepCnt, nFlags);
			break;
		case 24: // Cut
			SendMessage( WM_CHAR, 0x03, 0x2e0001 );
			if(!(GetStyle()&ES_READONLY))
				Delete();
			break;
		case 22: //Paste
			if(!(GetStyle()&ES_READONLY))
				Paste();
			break;
		default:    
			if(!(GetStyle()&ES_READONLY))
			{
				int x, y;
				GetSel(x,y);
				if (x != y) Delete();

				if (m_iCurrentChar < m_iNumChars)
				{
					if ( m_Char[m_iCurrentChar]->m_strValids.Find((TCHAR)nChar) >= 0 )
					{
						int iNewNext = FindNextChar(m_iCurrentChar + 1, false, true);
						SetSel(m_iCurrentChar, m_iCurrentChar + 1);
						ReplaceSel(CString((TCHAR)nChar));
						if (iNewNext >= 0)
						{
							m_iCurrentChar = iNewNext;
						}
						else
						{
							m_iCurrentChar++;
						}
						SetSel(m_iCurrentChar, m_iCurrentChar);
					}
					else
					{
						MessageBeep(MB_ICONEXCLAMATION);
					}
				}
			}
			break;
    }
	//CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CmEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    switch (nChar)
    {
		case VK_SHIFT:
			m_bShiftDown = false;
			break;
    
		default:
			break;
    }
    
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CmEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    int iNext = 0;
    CWnd* pNextItem = NULL;
    
    switch (nChar)
    {
		// Tab key
		case VK_TAB:
			pNextItem = (((CWnd*)GetParent())->GetNextDlgTabItem(this, (m_bShiftDown) ? TRUE : FALSE));
			if (pNextItem->IsKindOf(RUNTIME_CLASS(CmEdit)))
				((CmEdit*)pNextItem)->m_bShiftDown = m_bShiftDown;
			(((CWnd*)GetParent())->GetNextDlgTabItem(this, (m_bShiftDown) ? TRUE : FALSE))->SetFocus();
			break;

		// Shift key
		case VK_SHIFT:
			m_bShiftDown = true;
			break;

		// Left arrow key
		case VK_LEFT:
			if( m_iCurrentChar )
			{
				iNext = FindNextChar(m_iCurrentChar - 1, false, false);
				if (iNext < 0) iNext = 0;
				for( int i=(m_iCurrentChar-1-iNext); i; i-- )
					CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
				m_iCurrentChar = iNext;
			}
			break;

		// Right arrow key
		case VK_RIGHT:
			if( m_iCurrentChar<m_iNumChars )
			{
				iNext = FindNextChar(m_iCurrentChar + 1, false, true);
				if (iNext < 0 ) iNext = m_iNumChars;
				for( int i=(iNext-m_iCurrentChar-1); i; i-- )
					CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
				m_iCurrentChar = iNext;
			}
			break;

		// Home key
		case VK_HOME:
			m_iCurrentChar = 0;
			break;

		// End key
		case VK_END:
			m_iCurrentChar = m_iNumChars;
			break;

		// Delete key
		case VK_DELETE:
			if(!(GetStyle()&ES_READONLY))
			{
				int x, y;
				GetSel(x,y);
				if (x != y)
					Delete();
				else if (m_iCurrentChar
							<= FindNextChar(m_iNumChars - 1, false, false))
				{
					SetSel(m_iCurrentChar, m_iCurrentChar + 1);
					ReplaceSel(m_strPrompt);
					iNext = FindNextChar(m_iCurrentChar + 1, false, true);
					if (iNext >= 0)
						m_iCurrentChar = iNext;
					else
						m_iCurrentChar++;
					SetSel(m_iCurrentChar, m_iCurrentChar);
				}
			}
			return;

		default:
			break;

    }

    CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

bool CmEdit::Init(CString strMask, CString strInitialData)
{
    m_bInit = false;
    
    if (!Parse(strMask))
    {
        SetWindowText(TEXT("Init failed."));
        EnableWindow(FALSE);
        return false;
    }

    if (!SetValidChars(strMask))
    {
        SetWindowText(TEXT("SetValidChars failed."));
        EnableWindow(FALSE);
        return false;                          
    }
    
    SetData(strInitialData);
    m_iCurrentChar = FindNextChar(0, false, true);
    this->SetFocus();
    SetSel(m_iCurrentChar, m_iCurrentChar);

    EnableWindow(TRUE);  // this line is important!
    m_bInit = true;

    return true;
}

int CmEdit::FindNextChar(int iStartPos, bool bStatic, bool bForward)
{
    int iIndex = 0;
    int iReturnVal = -1;
    
    if (bForward)
    {
        for (int iIndex = iStartPos; iIndex < m_iNumChars; iIndex++)
        {
            if (m_Char[iIndex]->m_bStaticChar == bStatic)
            {
                iReturnVal = iIndex;
                break;
            }
        }
    }
    else
    {
        for (int iIndex = iStartPos; iIndex >= 0; iIndex--)
        {
            if (m_Char[iIndex]->m_bStaticChar == bStatic)
            {
                iReturnVal = iIndex;
                break;
            }
        }
    }

    return iReturnVal;
}

bool CmEdit::SetValidChars(const CString& strMask)
{
    CString strChars;
    CString strRangeLow; 
    bool    bInside       = false;
    bool    bInsideEscape = false;
    bool    bInsideRange  = false;
    int     iNextChar     = 0;
    int     i;
    
    // Clear valid chars
    for (i = 0; i < MAX_CHARS; i++)
        if (m_Char[i] != NULL)
        {
            delete m_Char[i];
            m_Char[i] = NULL;
        }

    // Walk through mask string
    for (i = 0; i < strMask.GetLength(); i++)
    {
        switch (strMask[i])
        {
        case '[':   bInside = true;
                    break;            //done

        case ']':   bInside = false;
                    m_Char[iNextChar++] = new CmEditChar(strChars, false);
                    strChars = "";
                    break;

        case '-':   if (bInside)
                    {
                        if (bInsideEscape)
                        {
                            strChars += "-";
                            bInsideEscape = false;  //done
                        }
                        else
                        {
                            if (strChars.GetLength() == 0)
                                return false;
                            bInsideRange = true;
                            strRangeLow = strChars.Right(1);
                            strChars = strChars.Left(strChars.GetLength() - 1); //done
                        }
                    }
                    else
                    {
                        m_Char[iNextChar++] = new CmEditChar("-", true); //done
                    }
                    break;

        case '\\':  if (bInside)
                    {
                        if (!bInsideEscape)
                        {
                            bInsideEscape = true;  //done
                        }
                        else
                        {
                            if (bInsideRange)
                            {
                                strChars += GetRange(strRangeLow, "\\");
                                strRangeLow = "";
                                bInsideRange = false;  //done
                            }
                            else
                            {
                                strChars += "\\";
                                bInsideEscape = false;  //done
                            }
                        }
                    }
                    else
                    {
                        m_Char[iNextChar++] = new CmEditChar("\\", true); //done
                    }
                    break;

        default:    if (bInside)
                    {
                        if (bInsideEscape)
                        {
                            bInsideEscape = false;        //done
                        }

                        if (bInsideRange)
                        {
                            strChars += GetRange(strRangeLow, strMask[i]);
                            strRangeLow = "";
                            bInsideRange = false;   //done
                        }
                        else
                        {
                            strChars += strMask[i]; //done
                        }
                    }
                    else
                    {
                        m_Char[iNextChar++] = new CmEditChar(strMask[i], true); //done
                    }
                    break;
        }
    }

    m_iNumChars = iNextChar;
    SetLimitText(m_iNumChars);
    
    return true;
}

CString CmEdit::GetRange(CString szLow, CString szHigh)
{
    CString szReturn = "";

    if (szLow.GetLength() == 1 && szHigh.GetLength() == 1)
    {
        TCHAR first = szLow[0];
        TCHAR last  = szHigh[0];

        for (int i = first; i <= last; i++)
            szReturn += CString((TCHAR)i);
    }

    return szReturn;
}

bool CmEdit::Parse(const CString& strMask)
{
    bool bValid = true;
    int iBracketState = 0;
    //int iDashState = 0;
    
    for (int i = 1; i <= strMask.GetLength(); i++)
    {
        switch (strMask[i - 1])
        {
        case '[':   iBracketState++; break;

        case ']':   iBracketState--; break;

        //case '-':   if (iBracketState > 0)
        //                iDashState = 1;
        //            break;

        default:    //if (iBracketState > 0) && iDashState == 1)
                    //    iDashState = 0;
                    break;
        }

        // Make sure the [ ] are in sync
        if (iBracketState < 0 || iBracketState > 1)
            bValid = false;

        // Make sure there is a character after a '-' inside the [ ]
        //if (iBracketState == 0 && iDashState == 1)
        //    bValid = false;
    }

    return (iBracketState == 0 /*&& iDashState == 0*/ && bValid) ? true : false;
}

bool CmEdit::SetPromptChar(CString strPromptChar)
{
    if (strPromptChar.GetLength() != 1)
        return false;

    m_strPrompt = strPromptChar;
    return true;
}


void CmEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);

	// Get the actual cursor position for future reference
	int dummy;
	GetSel(dummy, m_iCurrentChar);
}

void CmEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CEdit::OnLButtonUp(nFlags, point);

	// Get the actual cursor position for future reference
	int dummy;
	GetSel(dummy, m_iCurrentChar);
}

void CmEdit::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Select the entire text (so, not only one portion of characters)
    SetSel(0, -1);
	m_iCurrentChar = m_iNumChars;
}

LRESULT CmEdit::Paste(WPARAM, LPARAM)
{
	this->Paste();
	return NULL;
}

void CmEdit::Paste()
{
	if(!(GetStyle()&ES_READONLY))
	{
		// Open the clipboard
		::OpenClipboard(NULL);
		// Get the clipboard data handle
		HANDLE hData = ::GetClipboardData(CF_TEXT);
		// Get the clipboard data
		LPVOID lpData = ::GlobalLock(hData);

		try
		{
			CString strWindowText;
			GetWindowText(strWindowText);

			int x, y;
			GetSel(x,y);
			if( x==y ) y = m_iNumChars-1;
			// Set up a char string for the clipboard data
			char* pStr = (char*)lpData;

			// Read out the string until null terminator
			while((*pStr!='\0') && (x<=y))
			{
				if (m_Char[x]->m_bStaticChar)
					*pStr;
				else
				{
					if (m_Char[x]->m_strValids.Find(*pStr) < 0)
						strWindowText.SetAt(x, m_strPrompt[0]);
					else
						strWindowText.SetAt(x, (char)*pStr );
				}
				pStr++;
				x++;
			}
			SetWindowText(strWindowText);
			m_iCurrentChar = 0;
		}
		catch(...)
		{
		}

		// Close the clipboard
		::CloseClipboard();
	}
}

LRESULT CmEdit::Cut(WPARAM, LPARAM)
{
	this->Cut();
	return NULL;
}

void CmEdit::Cut()
{
	SendMessage( WM_CHAR, 0x03, 0x2e0001 );
	if(!(GetStyle()&ES_READONLY))
		Delete();
}
