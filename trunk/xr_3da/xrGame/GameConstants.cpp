#include "StdAfx.h"
#include "GameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"

bool	m_DisableStopping				= true;
bool	m_DisableStoppingBolt			= true;
bool	m_DisableStoppingGrenade		= true;

namespace GameConstants
{
	void LoadConstants()
	{
		if (!pConstantsSettings)
		{
			Msg("# GameConstants file does not exists");
			return;
		}
		m_DisableStopping				= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_empty", true);
		m_DisableStoppingBolt			= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_bolt", true);
		m_DisableStoppingGrenade		= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_grenade", true);

		Msg("# GameConstants are loaded");
	}

	bool GetDisableStopping()
	{
		return m_DisableStopping;
	}

	bool GetDisableStoppingBolt()
	{
		return m_DisableStoppingBolt;
	}

	bool GetDisableStoppingGrenade()
	{
		return m_DisableStoppingGrenade;
	}
}
