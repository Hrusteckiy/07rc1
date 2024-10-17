#pragma once
#include "UIWindow.h"
#include "../inventory_space.h"

#include "UIStatic.h"
#include "UIDragDropListEx.h"
#include "UIMultiTextStatic.h"
#include "UI3tButton.h"
#include "UICharacterInfo.h"
#include "UIItemInfo.h"

class CInventoryOwner;
class CEatableItem;
class CTrade;
struct SDrawStaticStruct;

class CUIDragDropListEx;
class CUICellItem;
class CUIMultiTextStatic;
class CUI3tButton;
class CUIItemInfo;
class CUICharacterInfo;
class CUIXml;

class CUITradeWnd: public CUIWindow
{
private:
	typedef CUIWindow inherited;
public:
						CUITradeWnd					();
	virtual				~CUITradeWnd				();

	virtual void		Init						();

	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	void				InitTrade					(CInventoryOwner* pOur, CInventoryOwner* pOthers);
	
	virtual void 		Draw						();
	virtual void 		Update						();
	virtual void 		Show						();
	virtual void 		Hide						();

	void 				DisableAll					();
	void 				EnableAll					();
	virtual bool		OnKeyboard					(int dik, EUIMessages keyboard_action);

	void 				SwitchToTalk				();
	void 				StartTrade					();
	void 				StopTrade					();
protected:
	bool				bStarted;
	bool 				ToOurTrade					();
	bool 				ToOthersTrade				();
	bool 				ToOurBag					();
	bool 				ToOthersBag					();
	void 				SendEvent_ItemDrop			(PIItem pItem);
	
	u32					CalcItemsPrice				(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying);
	float				CalcItemsWeight				(CUIDragDropListEx* pList);

	void				TransferItems				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);

	void				PerformTrade				();
	void				UpdatePrices				();
	void				ColorizeItem				(CUICellItem* itm, bool b);

	enum EListType{eNone,e1st,e2nd,eBoth};
	void				UpdateLists					(EListType);

	void				FillList					(TIItemContainer& cont, CUIDragDropListEx& list, bool do_colorize);

	bool				m_bDealControlsVisible;

	bool				CanMoveToOther				(PIItem pItem);

	//указатели игрока и того с кем торгуем
	CInventory*			m_pInv;
	CInventory*			m_pOthersInv;
	CInventoryOwner*	m_pInvOwner;
	CInventoryOwner*	m_pOthersInvOwner;
	CTrade*				m_pTrade;
	CTrade*				m_pOthersTrade;

	u32					m_iOurTradePrice;
	u32					m_iOthersTradePrice;


	CUICellItem*		m_pCurrentCellItem;
	TIItemContainer		ruck_list;
	CUIStatic			UIStaticTop;
	CUIStatic			UIStaticBottom;

	CUIStatic			UIOurBagWnd;
	CUIStatic			UIOurMoneyStatic;
	CUIStatic			UIOthersBagWnd;
	CUIStatic			UIOtherMoneyStatic;
	CUIDragDropListEx	UIOurBagList;
	CUIDragDropListEx	UIOthersBagList;

	CUIStatic			UIOurTradeWnd;
	CUIStatic			UIOthersTradeWnd;
	CUIMultiTextStatic	UIOurPriceCaption;
	CUIMultiTextStatic	UIOthersPriceCaption;
	CUIDragDropListEx	UIOurTradeList;
	CUIDragDropListEx	UIOthersTradeList;

	//кнопки
	CUI3tButton			UIPerformTradeButton;
	CUI3tButton			UIToTalkButton;

	//информация о персонажах
	CUIStatic			UIOurIcon;
	CUIStatic			UIOthersIcon;
	CUICharacterInfo	UICharacterInfoLeft;
	CUICharacterInfo	UICharacterInfoRight;

	//информация о перетаскиваемом предмете
	CUIStatic			UIDescWnd;
	CUIItemInfo			UIItemInfo;

	SDrawStaticStruct*	UIDealMsg;

	void				SetCurrentItem				(CUICellItem* itm);
	CUICellItem*		CurrentItem					();
	PIItem				CurrentIItem				();

	bool	xr_stdcall	OnItemDrop					(CUICellItem* itm);
	bool	xr_stdcall	OnItemStartDrag				(CUICellItem* itm);
	bool	xr_stdcall	OnItemDbClick				(CUICellItem* itm);
	bool	xr_stdcall	OnItemSelected				(CUICellItem* itm);
	bool	xr_stdcall	OnItemRButtonClick			(CUICellItem* itm);

	void				BindDragDropListEnents		(CUIDragDropListEx* lst);

};