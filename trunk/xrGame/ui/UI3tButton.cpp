// File:        UI3tButton.cpp
// Description: Button with 3 texutres (for <enabled>, <disabled> and <touched> states)
// Created:     07.12.2004
// Author:      Serhiy 0. Vynnychenk0
// Mail:        narrator@gsc-game.kiev.ua
//
// copyright 2004 GSC Game World
//

#include "StdAfx.h"
#include "UI3tButton.h"
#include "UIXmlInit.h"

CUI3tButton::CUI3tButton()
{
	m_bTextureEnable						= false;
	m_bUseTextColor[D]						= true;
	m_bUseTextColor[H]						= false;
	m_bUseTextColor[T]						= false;

	m_dwTextColor[E] 						= 0xFFFFFFFF;
	m_dwTextColor[D] 						= 0xFFAAAAAA;
	m_dwTextColor[H] 						= 0xFFFFFFFF;
	m_dwTextColor[T] 						= 0xFFFFFFFF;

	AttachChild								(&m_background);
	AttachChild								(&m_hint);

	m_bEnableTextHighlighting				= false;
	m_bCheckMode							= false;
	m_bWasAppliedBaseTexScaleUsing			= false;
	SetPushOffset							(Fvector2().set(0.0f,0.0f));

	m_BtnStatic								= nullptr;
	m_BtnStaticParams.m_bNeedClrChanging	= false;
	u32 def_clr								= color_rgba(255, 255, 255, 255);
	m_BtnStaticParams.m_ClrStateE			= def_clr;
	m_BtnStaticParams.m_ClrStateD			= def_clr;
	m_BtnStaticParams.m_ClrStateT			= def_clr;
	m_BtnStaticParams.m_ClrStateH			= def_clr;
}

CUI3tButton::~CUI3tButton()
{
	if (m_BtnStatic)
	{
		DetachChild(m_BtnStatic);
		xr_delete(m_BtnStatic);
	}
}

void CUI3tButton::AddStatic()
{
	if (!m_BtnStatic)
	{
		m_BtnStatic = xr_new<CUIStatic>();
		m_BtnStatic->Init(nullptr, -(GetWidth() / 2.f), 0.f, 80.f, 10.f);
		m_BtnStatic->SetTextComplexMode(true);
		AttachChild(m_BtnStatic);
	}
}

void CUI3tButton::OnClick()
{
	CUIButton::OnClick	();
	PlaySoundT			();
}

bool CUI3tButton::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	switch (mouse_action)
	{
		case WINDOW_LBUTTON_DOWN:
		{
			PlaySoundT();
		}
		case WINDOW_LBUTTON_UP:
		{
			if (!CursorOverWindow() && IsEnabled())
			{
				if (m_eButtonState == BUTTON_PUSHED)
					m_eButtonState = BUTTON_NORMAL;
			}
		}
	}
	if (m_bCheckMode)
		return CUIWindow::OnMouseAction(x,y,mouse_action);
	else
		return CUIButton::OnMouseAction(x,y,mouse_action);
}

bool CUI3tButton::OnMouseDown(int mouse_btn)
{
	if (m_bCheckMode)
	{
		if (mouse_btn==MOUSE_1)
		{
			if (m_eButtonState == BUTTON_NORMAL)
				m_eButtonState = BUTTON_PUSHED;
			else
				m_eButtonState = BUTTON_NORMAL;
		}
		GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);
		return true;
	}
	else
		return CUIButton::OnMouseDown(mouse_btn);
}

void CUI3tButton::OnFocusLost()
{
	CUIButton::OnFocusLost();
//.	if(BUTTON_PUSHED == m_eButtonState)
//.		m_eButtonState = BUTTON_NORMAL;
}

void CUI3tButton::OnFocusReceive()
{
	CUIButton::OnFocusReceive	();
	PlaySoundH					();
}

void CUI3tButton::InitSoundH(LPCSTR sound_file)
{
	::Sound->create		(m_sound_h, sound_file,st_Effect,sg_SourceType);
}

void CUI3tButton::InitSoundT(LPCSTR sound_file)
{
	::Sound->create		(m_sound_t, sound_file,st_Effect,sg_SourceType); 
}

void CUI3tButton::PlaySoundT()
{
	if (m_sound_t._handle())
		m_sound_t.play(NULL, sm_2D);
}

void CUI3tButton::PlaySoundH()
{
	if (m_sound_h._handle())
		m_sound_h.play(NULL, sm_2D);
}

void CUI3tButton::Init(float x, float y, float width, float height)
{
	m_background.Init			(0, 0, width, height);
	CUIButton::Init				(x, y, width, height);
}

void CUI3tButton::SetWidth(float width)
{
	CUIButton::SetWidth			(width);
	m_background.SetWidth		(width);
}

void CUI3tButton::SetHeight(float height)
{
	CUIButton::SetHeight		(height);
	m_background.SetHeight		(height);
}

void CUI3tButton::InitTexture(LPCSTR tex_name)
{
	string_path 		tex_enabled;
	string_path 		tex_disabled;
	string_path 		tex_touched;
	string_path 		tex_highlighted;

	// enabled state texture
	strcpy				(tex_enabled,    tex_name);
	strcat				(tex_enabled,   "_e");

	// pressed state texture
	strcpy				(tex_disabled,   tex_name);
	strcat				(tex_disabled,   "_d");

	// touched state texture
	strcpy				(tex_touched, tex_name);
	strcat				(tex_touched, "_t");

	// touched state texture
	strcpy				(tex_highlighted, tex_name);
	strcat				(tex_highlighted, "_h");

	this->InitTexture	(tex_enabled, tex_disabled, tex_touched, tex_highlighted);		
}

void CUI3tButton::InitTexture(LPCSTR tex_enabled, 
							  LPCSTR tex_disabled, 
							  LPCSTR tex_touched, 
							  LPCSTR tex_highlighted)
{
	m_background.InitEnabledState		(tex_enabled);
	m_background.InitDisabledState		(tex_disabled);
	m_background.InitTouchedState		(tex_touched);
	m_background.InitHighlightedState	(tex_highlighted);
	m_background.SetMirrorMode			(eMirrorMode);
	this->m_bTextureEnable = true;
}

void CUI3tButton::SetTextColor(u32 color)
{
	m_dwTextColor[E] = color;
}

void CUI3tButton::SetTextColorD(u32 color)
{
	SetTextColor(color, CUIStatic::D);
}

void CUI3tButton::SetTextColorH(u32 color)
{
	SetTextColor(color, CUIStatic::H);
}

void CUI3tButton::SetTextColorT(u32 color)
{
	SetTextColor(color, CUIStatic::T);
}

void CUI3tButton::SetBaseTextColor(u32 color)
{
	m_dwTextColor[E] = color;
	m_dwBaseTextColor[E] = color;
}

void CUI3tButton::SetBaseTextColorD(u32 color)
{
	SetBaseTextColorS(color, CUIStatic::D);
}

void CUI3tButton::SetBaseTextColorH(u32 color)
{
	SetBaseTextColorS(color, CUIStatic::H);
}

void CUI3tButton::SetBaseTextColorT(u32 color)
{
	SetBaseTextColorS(color, CUIStatic::T);
}

void CUI3tButton::SetTextureOffset(float x, float y)
{
	this->m_background.SetTextureOffset(x, y);
}

void CUI3tButton::SetBaseTextureOffset(float x, float y)
{
	this->m_background.SetBaseTextureOffset(x, y);
}

void CUI3tButton::SetTextureOffset(Fvector2 offset)
{
	this->m_background.SetTextureOffset(offset);
}

void CUI3tButton::SetBaseTextureOffset(Fvector2 offset)
{
	this->m_background.SetBaseTextureOffset(offset);
}

void CUI3tButton::DrawTexture()
{
	if (m_bTextureEnable)
	{
		m_background.SetStretchTexture(GetStretchTexture());
		m_background.Draw();
	}
}

void CUI3tButton::Update()
{
	CUIButton::Update();

	if (m_bTextureEnable)
	{
		if (&m_background)
		{
			if (m_bBaseTexScaleUsing && !m_bWasAppliedBaseTexScaleUsing)
			{
				m_background.SetScaleTexUsing(m_bBaseTexScaleUsing);
				m_background.SetScaleTex(m_fTexScale);
				m_bWasAppliedBaseTexScaleUsing = true;
			}
		}
		if (!m_bIsEnabled)
		{
			if (m_BtnStatic && m_BtnStaticParams.m_bNeedClrChanging)
				m_BtnStatic->SetTextColor(m_BtnStaticParams.m_ClrStateD);
			m_background.SetState(S_Disabled);
		}
		else if (CUIButton::BUTTON_PUSHED == m_eButtonState)
		{
			if (m_BtnStatic && m_BtnStaticParams.m_bNeedClrChanging)
				m_BtnStatic->SetTextColor(m_BtnStaticParams.m_ClrStateT);
			m_background.SetState(S_Touched);
		}
		else if (m_bCursorOverWindow)
		{
			if (m_BtnStatic && m_BtnStaticParams.m_bNeedClrChanging)
				m_BtnStatic->SetTextColor(m_BtnStaticParams.m_ClrStateH);
			m_background.SetState(S_Highlighted);
		}
		else
		{
			if (m_BtnStatic && m_BtnStaticParams.m_bNeedClrChanging)
				m_BtnStatic->SetTextColor(m_BtnStaticParams.m_ClrStateE);
			m_background.SetState(S_Enabled);
		}
	}

	u32 textColor;
	u32 hintColor;

	if (!m_bIsEnabled)
	{
		textColor = m_bUseTextColor[D] ? m_dwTextColor[D] : m_dwTextColor[E];
		hintColor = m_hint.m_bUseTextColor[D] ? m_hint.m_dwTextColor[D] : m_hint.m_dwTextColor[E];
	}
	else if (CUIButton::BUTTON_PUSHED == m_eButtonState)
	{
		textColor = m_bUseTextColor[T] ? m_dwTextColor[T] : m_dwTextColor[E];
		hintColor = m_hint.m_bUseTextColor[T] ? m_hint.m_dwTextColor[T] : m_hint.m_dwTextColor[E];
	}
	else if (m_bCursorOverWindow)
	{
		textColor = m_bUseTextColor[H] ? m_dwTextColor[H] : m_dwTextColor[E];
		hintColor = m_hint.m_bUseTextColor[H] ? m_hint.m_dwTextColor[H] : m_hint.m_dwTextColor[E];
	}
	else
	{
		textColor = m_dwTextColor[E];
		hintColor = m_hint.m_dwTextColor[E];
	}

	CUIStatic::SetTextColor		(textColor);
	m_hint.SetTextColor			(hintColor);
}

void CUI3tButton::SetBtnStaticClrE(u32 clr)
{
	m_BtnStaticParams.m_ClrStateE = clr;
}

void CUI3tButton::SetBtnStaticClrD(u32 clr)
{
	m_BtnStaticParams.m_ClrStateD = clr;
}

void CUI3tButton::SetBtnStaticClrT(u32 clr)
{
	m_BtnStaticParams.m_ClrStateT = clr;
}

void CUI3tButton::SetBtnStaticClrH(u32 clr)
{
	m_BtnStaticParams.m_ClrStateH = clr;
}
