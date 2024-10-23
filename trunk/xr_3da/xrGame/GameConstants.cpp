#include "StdAfx.h"
#include "GameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"

bool	m_bDisableStopping					= true;
bool	m_bDisableStoppingBolt				= true;
bool	m_bDisableStoppingGrenade			= true;
bool	m_bMergeAmmoLineWithFiremode		= true;
bool	m_bShowContactBio					= false;

namespace GameConstants
{
	void LoadConstants()
	{
		if (!pConstantsSettings)
		{
			Msg("# GameConstants file does not exists");
			return;
		}
		m_bDisableStopping					= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_empty", true);
		m_bDisableStoppingBolt				= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_bolt", true);
		m_bDisableStoppingGrenade			= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "disable_stopping_grenade", true);
		m_bMergeAmmoLineWithFiremode		= READ_IF_EXISTS(pConstantsSettings, r_bool, "gameplay", "merge_ammo_line_with_firemode", true);
		m_bShowContactBio					= READ_IF_EXISTS(pConstantsSettings, r_bool, "ui_settings", "show_contact_bio", false);

		Msg("# GameConstants are loaded");
	}

	bool GetDisableStopping()
	{
		return m_bDisableStopping;
	}

	bool GetDisableStoppingBolt()
	{
		return m_bDisableStoppingBolt;
	}

	bool GetDisableStoppingGrenade()
	{
		return m_bDisableStoppingGrenade;
	}

	bool GetMergedAmmoLineWithFiremodes()
	{
		return m_bDisableStoppingGrenade;
	}

	bool GetContactsBioShowing()
	{
		return m_bShowContactBio;
	}
}
