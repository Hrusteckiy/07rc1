#include "stdafx.h"
#include "UICursor.h"

#include "../xr_3da/CustomHUD.h"
#include "../xr_3da/xr_input.h"

#include "UI.h"
#include "HUDManager.h"
#include "ui/UIStatic.h"

CUICursor::CUICursor() : m_static(NULL), m_b_use_win_cursor(false)
{
	bVisible							= false;
	vPrevPos.set						(0.0f, 0.0f);
	vPos.set							(0.f, 0.f);
	InitInternal						();
	Device.seqRender.Add				(this,-3/*2*/);
	Device.seqResolutionChanged.Add		(this);
}
//--------------------------------------------------------------------
CUICursor::~CUICursor()
{
	xr_delete							(m_static);
	Device.seqRender.Remove				(this);
	Device.seqResolutionChanged.Remove	(this);
}

void CUICursor::OnScreenResolutionChanged()
{
	xr_delete		(m_static);
	InitInternal	();
}

void CUICursor::InitInternal()
{
	m_static					= xr_new<CUIStatic>();
	m_static->InitTextureEx		("ui\\ui_ani_cursor", "hud\\cursor");
	Frect						rect;
	rect.set					(0.0f, 0.0f, 45.0f, 45.0f);
	m_static->SetOriginalRect	(rect);
	Fvector2					sz;
	sz.set						(rect.rb);
	sz.x						*= UI()->get_current_kx();

	m_static->SetWndSize		(sz);
	m_static->SetStretchTexture	(true);

	u32 screen_size_x	= GetSystemMetrics(SM_CXSCREEN);
	u32 screen_size_y	= GetSystemMetrics(SM_CYSCREEN);
	m_b_use_win_cursor	= (screen_size_y >= Device.dwHeight && screen_size_x >= Device.dwWidth) || pInput->get_exclusive_mode();
}

//--------------------------------------------------------------------
u32 last_render_frame = 0;
void CUICursor::OnRender()
{
	if (!IsVisible())
		return;
#ifdef DEBUG
	VERIFY(last_render_frame != Device.dwFrame);
	last_render_frame = Device.dwFrame;

	if (bDebug)
	{
		CGameFont* F	= UI()->Font()->pFontDI;
		F->SetAligment	(CGameFont::alCenter);
		F->SetHeightI	(0.02f);
		F->OutSetI		(0.f, -0.9f);
		F->SetColor		(0xffffffff);
		Fvector2		pt = GetCursorPosition();
		F->OutNext		("%f-%f", pt.x, pt.y);
	}
#endif

	m_static->SetWndPos	(vPos);
	m_static->Update	();
	m_static->Draw		();
}

Fvector2 CUICursor::GetCursorPosition()
{
	return  vPos;
}

Fvector2 CUICursor::GetCursorPositionDelta()
{
	Fvector2 res_delta;

	res_delta.x = vPos.x - vPrevPos.x;
	res_delta.y = vPos.y - vPrevPos.y;
	return res_delta;
}

void CUICursor::UpdateCursorPosition(int _dx, int _dy)
{
	Fvector2	p;
	vPrevPos = vPos;
	if (m_b_use_win_cursor)
	{
		POINT pt;
		if (!GetCursorPos(&pt))
			return;

		// переводим в координаты клиента нашего окна
		HWND hWnd = Device.m_hWnd; // или как у тебя называется
		POINT cpt = pt;

		if (!ScreenToClient(hWnd, &cpt))
		{
			// редкий фолбэк, если вдруг не удалось
			RECT wr; GetWindowRect(hWnd, &wr);
			cpt.x = pt.x - wr.left;
			cpt.y = pt.y - wr.top;
		}

		// подрежем в пределах клиента (на borderless == размеру backbuffer'а)
		if (cpt.x < 0) cpt.x = 0;
		if (cpt.y < 0) cpt.y = 0;
		if (cpt.x > (LONG)Device.dwWidth - 1) cpt.x = (LONG)Device.dwWidth - 1;
		if (cpt.y > (LONG)Device.dwHeight - 1) cpt.y = (LONG)Device.dwHeight - 1;

		// теперь нормализация под UI-базу
		vPrevPos = vPos;
		vPos.x = (float)cpt.x * (UI_BASE_WIDTH / (float)Device.dwWidth);
		vPos.y = (float)cpt.y * (UI_BASE_HEIGHT / (float)Device.dwHeight);
	}
	else
	{
		float sens	= 1.0f;
		vPos.x		+= _dx * sens;
		vPos.y		+= _dy * sens;
	}
	clamp			(vPos.x, 0.f, UI_BASE_WIDTH);
	clamp			(vPos.y, 0.f, UI_BASE_HEIGHT);
}

void CUICursor::SetUICursorPosition(Fvector2 pos)
{
	clamp(pos.x, 0.f, UI_BASE_WIDTH);
	clamp(pos.y, 0.f, UI_BASE_HEIGHT);

	HWND hWnd = Device.m_hWnd;

	// 1) Текущая клиентка
	RECT rc{};
	GetClientRect(hWnd, &rc);
	const float cw = float(rc.right - rc.left);
	const float ch = float(rc.bottom - rc.top);

	// 2) UI -> клиентские пиксели
	POINT cpt;
	cpt.x = LONG(floorf(pos.x * (cw / UI_BASE_WIDTH) + 0.5f));
	cpt.y = LONG(floorf(pos.y * (ch / UI_BASE_HEIGHT) + 0.5f));

	// 4) Клиент -> экран и SetCursorPos
	ClientToScreen(hWnd, &cpt);
	SetCursorPos(cpt.x, cpt.y);

	// 5) Жёсткая синхронизация внутренних координат (не ждать физдвижения)
	if (m_b_use_win_cursor)
	{
		POINT pt{};
		GetCursorPos(&pt);            // уже центр по экрану
		ScreenToClient(hWnd, &pt);    // обратно в клиент
		const float sx = UI_BASE_WIDTH / cw;
		const float sy = UI_BASE_HEIGHT / ch;
		vPrevPos.set(pos);
		vPos.x = pt.x * sx;
		vPos.y = pt.y * sy;
	}
	else
	{
		vPrevPos.set(pos);
		vPos.set(pos);
	}
}
