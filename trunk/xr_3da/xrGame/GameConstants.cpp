#include "StdAfx.h"
#include "GameConstants.h"
#include "GamePersistent.h"
#include "game_cl_single.h"
#include "Actor.h"
#include "Inventory.h"

namespace GameConstants
{
	void LoadConstants()
	{
		if (!pConstantsSettings)
		{
			Msg("# GameConstants file does not exists");
			return;
		}

		Msg("# GameConstants are loaded");
	}
}
