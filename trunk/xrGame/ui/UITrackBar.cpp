#include "StdAfx.h"

#include "UITrackBar.h"
//.#include "UITrackButton.h"
#include "UIFrameLineWnd.h"
#include "UI3tButton.h"
#include "UITextureMaster.h"
#include "../xr_3da/xr_input.h"
#include "../GameConstants.h"
#include "../string_table.h"
#include <sstream> // for std::ostringstream
#include <iomanip> // for std::setprecision

#define DEF_CONTROL_HEIGHT		21
#define FRAME_LINE_TEXTURE		"ui_slider_e"
#define FRAME_LINE_TEXTURE_D	"ui_slider_d"
#define SLIDER_TEXTURE			"ui_slider_button"

CUITrackBar::CUITrackBar() : m_f_min(0), m_f_max(1.f), m_f_val(0.f), m_f_back_up(0.f), m_i_back_up(0), m_f_step(0.01f), m_b_invert(false), m_mode(eTrackBarModeFloat), m_tokens(nullptr), m_i_num_of_signs(1)
{
	m_b_mouse_capturer				= false;
	m_pFrameLine					= xr_new<CUIFrameLineWnd>();
	AttachChild						(m_pFrameLine);
	m_pFrameLine->SetAutoDelete		(true);
	m_pFrameLine_d					= xr_new<CUIFrameLineWnd>();
	m_pFrameLine_d->SetVisible		(false);
	AttachChild						(m_pFrameLine_d);
	m_pFrameLine_d->SetAutoDelete	(true);
	m_pSlider						= xr_new<CUI3tButton>();
	AttachChild						(m_pSlider);
	m_pSlider->SetAutoDelete		(true);
//.	m_pSlider->SetOwner				(this);
}

bool CUITrackBar::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	CUIWindow::OnMouseAction(x, y, mouse_action);
	
	switch (mouse_action)
	{
		case WINDOW_MOUSE_MOVE:
		{
			if (m_bCursorOverWindow && m_b_mouse_capturer)
			{
				if (pInput->iGetAsyncBtnState(0))
					UpdatePosRelativeToMouse();

				GetMessageTarget()->SendMessage(this, WINDOW_MOUSE_MOVE, NULL);
			}
		}break;
		case WINDOW_LBUTTON_DOWN:
		{
			m_b_mouse_capturer = m_bCursorOverWindow;
			if (m_b_mouse_capturer)
				UpdatePosRelativeToMouse();
		}break;

		case WINDOW_LBUTTON_UP:
		{
			m_b_mouse_capturer = false;
		}break;
		case WINDOW_MOUSE_WHEEL_UP:
		{
			if (IsFltMode())
			{
				m_f_val -= GetInvert() ? -m_f_step : m_f_step;
				clamp(m_f_val, m_f_min, m_f_max);
			}
			else
			{
				if (IsIntMode())
					m_i_val -= GetInvert() ? -m_i_step : m_i_step;
				else
					m_i_val -= GetInvert() ? -1 : 1;
				clamp(m_i_val, m_i_min, m_i_max);
			}

			UpdatePos();

			GetMessageTarget()->SendMessage(this, TRACK_MOVE, NULL);
		}break;
		case WINDOW_MOUSE_WHEEL_DOWN:
		{
			if (IsFltMode())
			{
				m_f_val += GetInvert() ? -m_f_step : m_f_step;
				clamp(m_f_val, m_f_min, m_f_max);
			}
			else
			{
				if (IsIntMode())
					m_i_val += GetInvert() ? -m_i_step : m_i_step;
				else
					m_i_val += GetInvert() ? -1 : 1;
				clamp(m_i_val, m_i_min, m_i_max);
			}

			UpdatePos();

			GetMessageTarget()->SendMessage(this, TRACK_MOVE, NULL);
		}break;
	}
	if (m_bCursorOverWindow && m_b_mouse_capturer)
	{
		if (pInput->iGetAsyncBtnState(0))
			UpdatePosRelativeToMouse();
	}
	return true;
}

void CUITrackBar::InitTrackBar(Fvector2 pos, Fvector2 size)
{
	string128					buf;
	float						item_height;
	float						item_width;
	SetHeight					(DEF_CONTROL_HEIGHT);

	item_height					= CUITextureMaster::GetTextureHeight(strconcat(sizeof(buf), buf, FRAME_LINE_TEXTURE, "_b"));
	m_pFrameLine->Init			(0, (size.y - item_height) / 2, size.x, item_height);
	m_pFrameLine->InitTexture	(FRAME_LINE_TEXTURE);
	m_pFrameLine_d->Init		(0, (size.y - item_height) / 2, size.x, item_height);
	m_pFrameLine_d->InitTexture	(FRAME_LINE_TEXTURE_D);

	strconcat					(sizeof(buf), buf, SLIDER_TEXTURE, "_e");
	item_width					= CUITextureMaster::GetTextureWidth(buf);
	item_height					= CUITextureMaster::GetTextureHeight(buf);
	m_pSlider->Init				(0, (size.y - item_height) / 2, item_width, item_height);
	m_pSlider->InitTexture		(SLIDER_TEXTURE);
	
	if (GameConstants::GetTrackBarValuesShowing() || IsTokenMode())
	{
		m_pSlider->AddStatic				();
		m_pSlider->SetStaticColorChanging	(true);
		CUIStatic* pStatic					= m_pSlider->GetBtnStatic();
		pStatic->SetTextComplexMode			(false);
		pStatic->SetWndSize					(Fvector2().set(item_width, item_height));
		pStatic->SetWndPos					(0.f, IsTokenMode() ? item_height : 0.f);
		pStatic->SetTextAlignment			(ETextAlignment::alCenter);
		pStatic->SetVTextAlignment			(EVTextAlignment::valCenter);
	}
}

void CUITrackBar::SetCurrentValue()
{
	if (IsTokenMode())
	{
		LPCSTR val = GetOptStringValue();
		for (xr_token* tok = GetOptToken(); tok->name; ++tok)
		{
			if (stricmp(tok->name, val) == 0)
			{
				m_i_val = tok->id + 1;
				break;
			}
		}
	}
	else
	{
		if (IsFltMode())
		{
			float minn = 0.f;
			float maxx = 0.f;

			GetOptFloatValue(m_f_val, minn, maxx);

			m_f_min = minn;
			m_f_max = maxx;

			// clamp current value to bounds
			clamp(m_f_val, m_f_min, m_f_max);
		}
		else // for bool and int mode it will be the same
		{
			int minn = 0;
			int maxx = 0;

			GetOptIntegerValue(m_i_val, minn, maxx);

			m_i_min = minn;
			m_i_max = maxx;

			// clamp current value to bounds
			clamp(m_i_val, m_i_min, m_i_max);
		}
	}

	UpdatePos			();
}

void CUITrackBar::Draw()
{
	CUIWindow::Draw();
}

// Форматирование текущего значения
static std::string FormatFloatWithStep(float value, int num_of_signs)
{
	// Вычисляем множитель на основе количества знаков после запятой
	float multiplier = std::pow(10.0f, num_of_signs);

	// Округляем значение с учетом заданной точности
	float rounded_value = std::round(value * multiplier) / multiplier;

	// Проверяем, является ли округлённое значение целым
	if (std::fabs(std::floor(rounded_value) - rounded_value) < 0.00001f)
	{
		return std::to_string(static_cast<int>(rounded_value)); // Преобразуем в строку как целое число
	}

	// Если дробная часть есть, форматируем с указанной точностью
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(num_of_signs) << rounded_value;
	return oss.str();
}

void CUITrackBar::Update()
{
	CUIWindow::Update();

	if (m_b_mouse_capturer)
	{
		if (!pInput->iGetAsyncBtnState(0))
			m_b_mouse_capturer = false;
	}
	if (m_pSlider)
	{
		if (!m_b_mouse_capturer)
		{
			if (!m_pSlider->CursorOverWindow() || !CursorOverWindow())
				m_pSlider->SetButtonMode(CUIButton::BUTTON_NORMAL);
		}
		CUIStatic* pUIStatic = m_pSlider->GetBtnStatic();
		if (pUIStatic)
		{
			std::string out_str = "";
			switch (m_mode)
			{
				case eTrackBarModeInt:
				{
					out_str = std::to_string(m_i_val);
				}break;
				case eTrackBarModeFloat:
				{
					out_str = FormatFloatWithStep(m_f_val, m_i_num_of_signs);
				}break;
				case eTrackBarModeToken:
				{
					out_str = *CStringTable().translate(GetOptStringValue());
				}break;
				case eTrackBarModeBool:
				{
					out_str = m_i_val == m_i_min ? *CStringTable().translate("st_track_opt_off") : *CStringTable().translate("st_track_opt_on");
				}break;
			}
			pUIStatic->SetText(out_str.c_str());
		}
	}

}

void CUITrackBar::SaveValue()
{
	CUIOptionsItem::SaveValue();
	if (IsTokenMode())
	{
		if (strcmp("not_an_option", GetEntry()))
		{
			xr_token* tok = GetOptToken();
			LPCSTR cur_val = get_token_name(tok, m_i_val - 1);
			SaveOptStringValue(cur_val);
		}
	}
	else
	{
		if (IsFltMode())
			SaveOptFloatValue(m_f_val);
		else
			SaveOptIntegerValue(m_i_val);
	}
}

bool CUITrackBar::IsChanged()
{
	if (IsFltMode())
	{
		return !fsimilar(m_f_back_up, m_f_val);
	}
	else
	{
		return (m_i_back_up != m_i_val);
	}
}

void CUITrackBar::SetStep(float step)
{
	if (IsFltMode())
		m_f_step	= step;
	else
		m_i_step	= iFloor(step);
}

void CUITrackBar::SaveBackUpValue()
{
	if (IsFltMode())
		m_f_back_up		= m_f_val;
	else
		m_i_back_up		= m_i_val;
}

void CUITrackBar::Undo()
{
	if (IsFltMode())
		m_f_val			= m_f_back_up;
	else
		m_i_val			= m_i_back_up;

	SaveValue			();
	SetCurrentValue		();
}

void CUITrackBar::Enable(bool status)
{
	m_bIsEnabled				= status;
	m_pFrameLine->SetVisible	(status);
	m_pSlider->Enable			(status);
	m_pFrameLine_d->SetVisible	(!status);
}

void CUITrackBar::UpdatePosRelativeToMouse()
{
	float btn_width = m_pSlider->GetWidth();
	float window_width = GetWidth();
	float fpos = m_cursor_pos.x;

	if (GetInvert())
	{
		fpos = window_width - fpos;
	}

	if (fpos < btn_width / 2)
	{
		fpos = btn_width / 2;
	}
	else if (fpos > window_width - btn_width / 2)
	{
		fpos = window_width - btn_width / 2;
	}

	float __fval = (IsFltMode()) ? m_f_val : (float)m_i_val;
	float __fmax = (IsFltMode()) ? m_f_max : (float)m_i_max;
	float __fmin = (IsFltMode()) ? m_f_min : (float)m_i_min;
	float __fstep = (IsFltMode()) ? m_f_step : (IsIntMode()) ? (float)m_i_step : 1.f;

	__fval = (__fmax - __fmin) * (fpos - btn_width / 2) / (window_width - btn_width) + __fmin;

	float _d = (__fval - __fmin);
	float _v = _d / __fstep;
	int _vi = iFloor(_v);
	float _vf = __fstep * _vi;

	if (_d - _vf > __fstep / 2.0f)
	{
		_vf += __fstep;
	}

	__fval = __fmin + _vf;

	clamp(__fval, __fmin, __fmax);

	if (IsTokenMode())
	{
		int new_index = (int)__fval;

		if (new_index != m_i_val)
		{
			m_i_val = new_index;
			xr_token* tok = GetOptToken();
			LPCSTR cur_val = get_token_name(tok, m_i_val - 1);
			SaveOptStringValue(cur_val);
		}
	}
	else
	{
		// Режим чисел и флагов
		if (IsFltMode())
		{
			if (!fsimilar(m_f_val, __fval))
			{
				m_f_val = __fval;
			}
		}
		else
		{
			int new_val = iFloor(__fval);
			if (m_i_val != new_val)
			{
				m_i_val = new_val;
			}
		}
	}

	GetMessageTarget()->SendMessage(this, BUTTON_CLICKED, NULL);

	UpdatePos();
}

void CUITrackBar::SetTokenValues(xr_token* tokens)
{
	m_tokens = tokens;

	int count = 0;
	if (m_tokens)
	{
		for (xr_token* tok = m_tokens; tok->name; ++tok)
		{
			++count;
}
	}

	if (count > 0)
	{
		m_i_min = 1;
		m_i_max = count;
	}

	LPCSTR current_value = GetOptStringValue();
	m_i_val = m_i_min;

	for (int i = 0; i < count; ++i)
	{
		if (stricmp(m_tokens[i].name, current_value) == 0)
		{
			m_i_val = i + 1;
			break;
		}
	}

	clamp(m_i_val, m_i_min, m_i_max);
}

void CUITrackBar::UpdatePos()
{
#ifdef DEBUG
	
	if(IsFltMode())
		R_ASSERT2(m_f_val >= m_f_min && m_f_val <= m_f_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );
	else
		R_ASSERT2(m_i_val >= m_i_min && m_i_val <= m_i_max, "CUITrackBar::UpdatePos() - m_val >= m_min && m_val <= m_max" );

#endif

	float btn_width				= m_pSlider->GetWidth();
	float window_width			= GetWidth();
	float free_space			= window_width - btn_width;
	Fvector2 pos				= m_pSlider->GetWndPos();
	
	float __fval	= (IsFltMode()) ? m_f_val : (float)m_i_val;
	float __fmax	= (IsFltMode()) ? m_f_max : (float)m_i_max;
	float __fmin	= (IsFltMode()) ? m_f_min : (float)m_i_min;


	pos.x						= (__fval - __fmin) * free_space / (__fmax - __fmin);
	if (GetInvert())
		pos.x					= free_space - pos.x;

	m_pSlider->SetWndPos		(pos);
	SaveValue					();
}

void CUITrackBar::OnMessage(const char* message)
{
	if (0 == xr_strcmp(message,"set_default_value"))
	{
		if (IsFltMode())
			m_f_val = m_f_min + (m_f_max - m_f_min) / 2.0f;
		else
			m_i_val = m_i_min + iFloor((m_i_max - m_i_min) / 2.0f);

		UpdatePos();
	}
}

bool CUITrackBar::GetCheck() const
{
	VERIFY(!IsFltMode());
	return !!m_i_val;
}

void CUITrackBar::SetCheck(bool b)
{
	VERIFY(!IsFltMode());
	m_i_val = (b) ? m_i_max : m_i_min;
}
