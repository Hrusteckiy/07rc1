// File:		UI_IB_Static.h
// Description:	Inheritance of UIInteractiveBackground template class with some
//				CUIStatic features
// Created:		09.02.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua

// Copyright 2005 GSC Game World

#include "StdAfx.h"
#include "UIIBStatic.h"

void CUIIBStatic::SetBaseTextureOffset(float x, float y)
{
	if (m_stateCurrent)
		m_stateCurrent->SetBaseTextureOffset(x,y);

	if (m_stateEnabled)
		m_stateEnabled->SetBaseTextureOffset(x,y);

	if (m_stateDisabled)
		m_stateDisabled->SetBaseTextureOffset(x,y);

	if (m_stateHighlighted)
		m_stateHighlighted->SetBaseTextureOffset(x,y);

	if (m_stateTouched)
		m_stateTouched->SetBaseTextureOffset(x,y);
}

void CUIIBStatic::SetTextureOffset(float x, float y)
{
	if (m_stateCurrent)
		m_stateCurrent->SetTextureOffset(x,y);

	if (m_stateEnabled)
		m_stateEnabled->SetTextureOffset(x,y);

	if (m_stateDisabled)
		m_stateDisabled->SetTextureOffset(x,y);

	if (m_stateHighlighted)
		m_stateHighlighted->SetTextureOffset(x,y);

	if (m_stateTouched)
		m_stateTouched->SetTextureOffset(x,y);
}

void CUIIBStatic::SetBaseTextureOffset(Fvector2 offset)
{
	if (m_stateCurrent)
		m_stateCurrent->SetBaseTextureOffset(offset);

	if (m_stateEnabled)
		m_stateEnabled->SetBaseTextureOffset(offset);

	if (m_stateDisabled)
		m_stateDisabled->SetBaseTextureOffset(offset);

	if (m_stateHighlighted)
		m_stateHighlighted->SetBaseTextureOffset(offset);

	if (m_stateTouched)
		m_stateTouched->SetBaseTextureOffset(offset);
}

void CUIIBStatic::SetTextureOffset(Fvector2 offset)
{
	if (m_stateCurrent)
		m_stateCurrent->SetTextureOffset(offset);

	if (m_stateEnabled)
		m_stateEnabled->SetTextureOffset(offset);

	if (m_stateDisabled)
		m_stateDisabled->SetTextureOffset(offset);

	if (m_stateHighlighted)
		m_stateHighlighted->SetTextureOffset(offset);

	if (m_stateTouched)
		m_stateTouched->SetTextureOffset(offset);
}

void CUIIBStatic::SetStretchTexture(bool stretch_texture)
{
	if (m_stateCurrent)
		m_stateCurrent->SetStretchTexture(stretch_texture);

	if (m_stateEnabled)
		m_stateEnabled->SetStretchTexture(stretch_texture);

	if (m_stateDisabled)
		m_stateDisabled->SetStretchTexture(stretch_texture);

	if (m_stateHighlighted)
		m_stateHighlighted->SetStretchTexture(stretch_texture);

	if (m_stateTouched)
		m_stateTouched->SetStretchTexture(stretch_texture);
}
