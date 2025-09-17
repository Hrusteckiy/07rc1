#include "stdafx.h"

#include "UIMessageBox.h"
#include "UIMessageBoxEx.h"
#include "../UIDialogHolder.h"
#include <dinput.h>

CUIMessageBoxEx::CUIMessageBoxEx(
){
	m_pMessageBox = xr_new<CUIMessageBox>();
	m_pMessageBox->SetWindowName("msg_box");
//	m_pMessageBox->SetAutoDelete(true);
	AttachChild(m_pMessageBox);
}

CUIMessageBoxEx::~CUIMessageBoxEx()
{
	xr_delete(m_pMessageBox);
}

void CUIMessageBoxEx::Init(LPCSTR xml_template)
{
	CUIDialogWnd::Init(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);
	m_pMessageBox->Init(xml_template);
}

void CUIMessageBoxEx::SetText(LPCSTR text)
{
	m_pMessageBox->SetText(text);

}

LPCSTR CUIMessageBoxEx::GetText()
{
	return m_pMessageBox->GetText();
}

void CUIMessageBoxEx::SendMessage(CUIWindow* pWnd, s16 msg, void* pData /* = nullptr */)
{
	CUIWndCallback::OnEvent(pWnd, msg, pData);
	if (pWnd == m_pMessageBox)
	{
		switch (msg)
		{
			case MESSAGE_BOX_OK_CLICKED:
			case MESSAGE_BOX_YES_CLICKED:
			case MESSAGE_BOX_NO_CLICKED:
			case MESSAGE_BOX_CANCEL_CLICKED:
			case MESSAGE_BOX_QUIT_WIN_CLICKED:
			case MESSAGE_BOX_QUIT_GAME_CLICKED:
				GetHolder()->StartStopMenu(this, true);
			default:
				break;
		}

		if (GetMessageTarget())
			GetMessageTarget()->SendMessage(this,msg,pData);
	}
	
}

LPCSTR CUIMessageBoxEx::GetHost()
{
	return m_pMessageBox->GetHost();
}

LPCSTR CUIMessageBoxEx::GetPassword()
{
	return m_pMessageBox->GetPassword();
}

bool CUIMessageBoxEx::IR_OnKeyboardPress(int dik)
{
	if ( dik == DIK_NUMPADENTER || dik == DIK_RETURN || dik == DIK_SPACE)
	{
		m_pMessageBox->OnYesOk();
		return true;
	}
	else if ( dik == DIK_ESCAPE)
	{
		GetHolder()->StartStopMenu(this, true);
		return true;
	}
	return CUIDialogWnd::IR_OnKeyboardPress(dik);
}
