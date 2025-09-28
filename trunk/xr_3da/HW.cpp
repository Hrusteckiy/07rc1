// HW.cpp: implementation of the CHW class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#pragma warning(disable:4995)
#include <d3dx9.h>
#include <wingdi.h> 
#include <sdkddkver.h>
#pragma warning(default:4995)
#include "HW.h"
#include "XR_IOConsole.h"

ENGINE_API xr_token* vid_disp_token = nullptr;
ENGINE_API int       ps_rs_display = -1; // -1 = авто (как сейчас), 0..N-1 = конкретный монитор

static LPCSTR xr_strdupA(const char* s) { size_t n = xr_strlen(s) + 1; char* p = (char*)xr_malloc(n); memcpy(p, s, n); return p; }

#ifndef _EDITOR
	void	fill_vid_mode_list			(CHW* _hw);
	void	free_vid_mode_list			();
#else
	void	fill_vid_mode_list			(CHW* _hw)	{};
	void	free_vid_mode_list			()			{};
#endif

	void	free_vid_mode_list			();

ENGINE_API CHW			HW;

#ifdef DEBUG
IDirect3DStateBlock9*	dwDebugSB = 0;
#endif

void CHW::Reset		(HWND hwnd)
{
#ifdef DEBUG
	_RELEASE			(dwDebugSB);
#endif
	_RELEASE			(pBaseZB);
	_RELEASE			(pBaseRT);

#ifndef _EDITOR
#ifndef DEDICATED_SERVER
	BOOL	bWindowed		= !psDeviceFlags.is	(rsFullscreen);
#else
	BOOL	bWindowed		= TRUE;
#endif

	selectResolution		(DevPP.BackBufferWidth, DevPP.BackBufferHeight, bWindowed);
	// Windoze
	DevPP.SwapEffect			= bWindowed?D3DSWAPEFFECT_COPY:D3DSWAPEFFECT_DISCARD;
	DevPP.Windowed				= bWindowed;
	DevPP.PresentationInterval	= selectPresentInterval();

	if (!bWindowed)
		DevPP.FullScreen_RefreshRateInHz = selectRefresh(DevPP.BackBufferWidth, DevPP.BackBufferHeight, Caps.fTarget);
	else
		DevPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
#endif

	while	(TRUE)	{
		HRESULT _hr							= HW.pDevice->Reset	(&DevPP);
		if (SUCCEEDED(_hr))					break;
		Msg		("! ERROR: [%dx%d]: %s",DevPP.BackBufferWidth,DevPP.BackBufferHeight,Debug.error2string(_hr));
		Sleep	(100);
	}
	R_CHK				(pDevice->GetRenderTarget			(0,&pBaseRT));
	R_CHK				(pDevice->GetDepthStencilSurface	(&pBaseZB));
#ifdef DEBUG
	R_CHK				(pDevice->CreateStateBlock			(D3DSBT_ALL,&dwDebugSB));
#endif
#ifndef _EDITOR
	updateWindowProps	(hwnd);
#endif
}

xr_token*				vid_mode_token = NULL;

void CHW::CreateD3D	()
{
	LPCSTR		_name			= "d3d9.dll";

	hD3D9            			= LoadLibrary(_name);
	R_ASSERT2	           	 	(hD3D9,"Can't find 'd3d9.dll'\nPlease install latest version of DirectX before running this program");
	typedef IDirect3D9 * WINAPI _Direct3DCreate9(UINT SDKVersion);
	_Direct3DCreate9* createD3D	= (_Direct3DCreate9*)GetProcAddress(hD3D9,"Direct3DCreate9");	R_ASSERT(createD3D);
	this->pD3D 					= createD3D( D3D_SDK_VERSION );
	R_ASSERT2					(this->pD3D,"Please install DirectX 9.0c");
}

void CHW::DestroyD3D()
{
	_RELEASE					(this->pD3D);
	FreeLibrary					(hD3D9);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
D3DFORMAT CHW::selectDepthStencil	(D3DFORMAT fTarget)
{
	// R2 hack
#pragma todo("R2 need to specify depth format")
	if (psDeviceFlags.test(rsR2))	return D3DFMT_D24S8;

	// R1 usual
	static	D3DFORMAT	fDS_Try1[6] =
	{D3DFMT_D24S8,D3DFMT_D24X4S4,D3DFMT_D32,D3DFMT_D24X8,D3DFMT_D16,D3DFMT_D15S1};

	D3DFORMAT*	fDS_Try			= fDS_Try1;
	int			fDS_Cnt			= 6;

	for (int it = 0; it<fDS_Cnt; it++){
		if (SUCCEEDED(pD3D->CheckDeviceFormat(
			DevAdapter,DevT,fTarget,
			D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,fDS_Try[it])))
		{
			if( SUCCEEDED( pD3D->CheckDepthStencilMatch(
				DevAdapter,DevT,
				fTarget, fTarget, fDS_Try[it]) ) )
			{
				return fDS_Try[it];
			}
		}
	}
	return D3DFMT_UNKNOWN;
}

void	CHW::DestroyDevice	()
{
	_SHOW_REF				("refCount:pBaseZB",pBaseZB);
	_RELEASE				(pBaseZB);

	_SHOW_REF				("refCount:pBaseRT",pBaseRT);
	_RELEASE				(pBaseRT);
#ifdef DEBUG
	_SHOW_REF				("refCount:dwDebugSB",dwDebugSB);
	_RELEASE				(dwDebugSB);
#endif
#ifdef _EDITOR
	_RELEASE				(HW.pDevice);
#else
	_SHOW_REF				("DeviceREF:",HW.pDevice);
	_RELEASE				(HW.pDevice);
#endif    
	DestroyD3D				();
	
#ifndef _EDITOR
	free_vid_mode_list		();
#endif
	free_mon_token_list		();
}
void	CHW::selectResolution	(u32 &dwWidth, u32 &dwHeight, BOOL bWindowed)
{
	fill_vid_mode_list			(this);
#ifdef DEDICATED_SERVER
	dwWidth		= 640;
	dwHeight	= 480;
#else
	if(bWindowed)
	{
		dwWidth		= psCurrentVidMode[0];
		dwHeight	= psCurrentVidMode[1];
	}else //check
	{
#ifndef _EDITOR
		string64					buff;
		sprintf_s					(buff,sizeof(buff),"%dx%d",psCurrentVidMode[0],psCurrentVidMode[1]);
		
		if(_ParseItem(buff,vid_mode_token)==u32(-1)) //not found
		{ //select safe
			sprintf_s				(buff,sizeof(buff),"vid_mode %s",vid_mode_token[0].name);
			Console->Execute		(buff);
		}

		dwWidth						= psCurrentVidMode[0];
		dwHeight					= psCurrentVidMode[1];
#endif
	}
#endif

}

void CHW::CreateDevice(HWND m_hWnd)
{
	CreateD3D					();
	fill_mon_token_list			();

#ifndef DEDICATED_SERVER
	const BOOL bWindowed = !psDeviceFlags.is(rsFullscreen);
#else
	const BOOL bWindowed = TRUE;
#endif

	// базовый выбор устройства
	DevAdapter = (UserAdapter != UINT_MAX) ? UserAdapter : D3DADAPTER_DEFAULT;
	DevT = Caps.bForceGPU_REF ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL;

	// NVPerfHUD override (оставляем как было)
	for (UINT Adapter = 0; pD3D && Adapter < pD3D->GetAdapterCount(); Adapter++)
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		if (SUCCEEDED(pD3D->GetAdapterIdentifier(Adapter, 0, &Identifier)) && xr_strcmp(Identifier.Description, "NVIDIA NVPerfHUD") == 0)
		{
			DevAdapter = Adapter;
			DevT = D3DDEVTYPE_REF;
			break;
		}
	}

	// Display the name of video board
	D3DADAPTER_IDENTIFIER9	adapterID;
	R_CHK	(pD3D->GetAdapterIdentifier(DevAdapter,0,&adapterID));
	Msg("* GPU [vendor:%X]-[device:%X]: %s", adapterID.VendorId, adapterID.DeviceId, adapterID.Description);

	u16	drv_Product		= HIWORD(adapterID.DriverVersion.HighPart);
	u16	drv_Version		= LOWORD(adapterID.DriverVersion.HighPart);
	u16	drv_SubVersion	= HIWORD(adapterID.DriverVersion.LowPart);
	u16	drv_Build		= LOWORD(adapterID.DriverVersion.LowPart);
	Msg("* GPU driver: %d.%d.%d.%d", u32(drv_Product), u32(drv_Version), u32(drv_SubVersion), u32(drv_Build));

	Caps.id_vendor	= adapterID.VendorId;
	Caps.id_device	= adapterID.DeviceId;

	// Retreive windowed mode
	D3DDISPLAYMODE mWindowed;
	R_CHK(pD3D->GetAdapterDisplayMode(DevAdapter, &mWindowed));

	// Select back-buffer & depth-stencil format
	D3DFORMAT&	fTarget	= Caps.fTarget;
	D3DFORMAT&	fDepth	= Caps.fDepth;
	if (bWindowed)
	{
		fTarget = mWindowed.Format;
		R_CHK(pD3D->CheckDeviceType	(DevAdapter,DevT,fTarget,fTarget,TRUE));
		fDepth  = selectDepthStencil(fTarget);
	}
	else
	{
		switch (psCurrentBPP)
		{
		case 32:
			fTarget = D3DFMT_X8R8G8B8;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_A8R8G8B8;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_R8G8B8;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_UNKNOWN;
			break;
		case 16:
		default:
			fTarget = D3DFMT_R5G6B5;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_X1R5G5B5;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_X4R4G4B4;
			if (SUCCEEDED(pD3D->CheckDeviceType(DevAdapter, DevT, fTarget, fTarget, FALSE)))
				break;
			fTarget = D3DFMT_UNKNOWN;
			break;
		}
		fDepth  = selectDepthStencil(fTarget);
	}

	if ((D3DFMT_UNKNOWN == fTarget) || (D3DFMT_UNKNOWN == fTarget))
	{
		Msg					("Failed to initialize graphics hardware.\nPlease try to restart the game.");
		FlushLog			();
		MessageBox			(NULL, "Failed to initialize graphics hardware.\nPlease try to restart the game.", "Error!", MB_OK | MB_ICONERROR);
		TerminateProcess	(GetCurrentProcess(), 0);
	}


	// Set up the presentation parameters
	D3DPRESENT_PARAMETERS&	P	= DevPP;
	ZeroMemory					(&P, sizeof(P));

#ifndef _EDITOR
	selectResolution	(P.BackBufferWidth, P.BackBufferHeight, bWindowed);
#endif
// Back buffer
//.	P.BackBufferWidth		= dwWidth;
//. P.BackBufferHeight		= dwHeight;
	P.BackBufferFormat		= fTarget;
	P.BackBufferCount		= 1;

	// Multisample
	P.MultiSampleType		= D3DMULTISAMPLE_NONE;
	P.MultiSampleQuality	= 0;

	// Windoze
	P.SwapEffect			= bWindowed ? D3DSWAPEFFECT_COPY : D3DSWAPEFFECT_DISCARD;
	P.hDeviceWindow			= m_hWnd;
	P.Windowed				= bWindowed;

	// Depth/stencil
	P.EnableAutoDepthStencil= TRUE;
	P.AutoDepthStencilFormat= fDepth;
	P.Flags					= 0;	//. D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	// Refresh rate
	P.PresentationInterval	= selectPresentInterval();
	if (!bWindowed)
		P.FullScreen_RefreshRateInHz = selectRefresh(P.BackBufferWidth, P.BackBufferHeight, fTarget);
	else
		P.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	// Create the device
	u32 GPU		= selectGPU();	
	HRESULT R	= HW.pD3D->CreateDevice(DevAdapter,
										DevT,
										m_hWnd,
										GPU | D3DCREATE_MULTITHREADED,	//. ? locks at present
										&P,
										&pDevice );
	
	if (FAILED(R))
	{
		R	= HW.pD3D->CreateDevice(	DevAdapter,
										DevT,
										m_hWnd,
										GPU | D3DCREATE_MULTITHREADED,	//. ? locks at present
										&P,
										&pDevice );
	}
	if (D3DERR_DEVICELOST == R)
	{
		// Fatal error! Cannot create rendering device AT STARTUP !!!
		Msg					("Failed to initialize graphics hardware.\nPlease try to restart the game.");
		FlushLog			();
		MessageBox			(NULL, "Failed to initialize graphics hardware.\nPlease try to restart the game.", "Error!", MB_OK | MB_ICONERROR);
		TerminateProcess	(GetCurrentProcess(), 0);
	};
	R_CHK		(R);

	_SHOW_REF	("* CREATE: DeviceREF:",HW.pDevice);
	switch (GPU)
	{
	case D3DCREATE_SOFTWARE_VERTEXPROCESSING:
		Log	("* Vertex Processor: SOFTWARE");
		break;
	case D3DCREATE_MIXED_VERTEXPROCESSING:
		Log	("* Vertex Processor: MIXED");
		break;
	case D3DCREATE_HARDWARE_VERTEXPROCESSING:
		Log	("* Vertex Processor: HARDWARE");
		break;
	case D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE:
		Log	("* Vertex Processor: PURE HARDWARE");
		break;
	}

	// Capture misc data
#ifdef DEBUG
	R_CHK	(pDevice->CreateStateBlock			(D3DSBT_ALL,&dwDebugSB));
#endif
	R_CHK	(pDevice->GetRenderTarget			(0,&pBaseRT));
	R_CHK	(pDevice->GetDepthStencilSurface	(&pBaseZB));
	u32	memory									= pDevice->GetAvailableTextureMem	();
	Msg		("*     Texture memory: %d M",		memory / (1024 * 1024));
	Msg		("*          DDI-level: %2.1f",		float(D3DXGetDriverLevel(pDevice)) / 100.f);
#ifndef _EDITOR
	updateWindowProps							(m_hWnd);
	fill_vid_mode_list							(this);
#endif
}

u32	CHW::selectPresentInterval()
{
	D3DCAPS9	caps;
	pD3D->GetDeviceCaps(DevAdapter, DevT, &caps);

	if (!psDeviceFlags.test(rsVSync)) 
	{
		if (caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE)
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		if (caps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE)
			return D3DPRESENT_INTERVAL_ONE;
	}
	return D3DPRESENT_INTERVAL_DEFAULT;
}

u32 CHW::selectGPU()
{
	if (Caps.bForceGPU_SW)
		return D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	D3DCAPS9	caps;
	pD3D->GetDeviceCaps(DevAdapter, DevT, &caps);

	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		if (Caps.bForceGPU_NonPure)
			return D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
		{
			if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
				return D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE;
			else
				return D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		// return D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else
		return D3DCREATE_SOFTWARE_VERTEXPROCESSING;
}

u32 CHW::selectRefresh(u32 dwWidth, u32 dwHeight, D3DFORMAT fmt)
{
	if (psDeviceFlags.is(rsRefresh60hz))
		return D3DPRESENT_RATE_DEFAULT;
	else
	{
		u32 selected	= D3DPRESENT_RATE_DEFAULT;
		u32 count		= pD3D->GetAdapterModeCount(DevAdapter,fmt);
		for (u32 I = 0; I < count; I++)
		{
			D3DDISPLAYMODE	Mode;
			pD3D->EnumAdapterModes(DevAdapter,fmt,I,&Mode);
			if (Mode.Width == dwWidth && Mode.Height == dwHeight)
			{
				if (Mode.RefreshRate>selected)
					selected = Mode.RefreshRate;
			}
		}
		return selected;
	}
}

BOOL	CHW::support	(D3DFORMAT fmt, DWORD type, DWORD usage)
{
	HRESULT hr		= pD3D->CheckDeviceFormat(DevAdapter,DevT,Caps.fTarget,usage,(D3DRESOURCETYPE)type,fmt);
	if (FAILED(hr))	return FALSE;
	else			return TRUE;
}


static void ClampToMonitorRect(int& x, int& y, int& w, int& h, const RECT& mon)
{
	if (w > (mon.right - mon.left))  w = (mon.right - mon.left);
	if (h > (mon.bottom - mon.top))  h = (mon.bottom - mon.top);

	if (x < mon.left) x = mon.left;
	if (y < mon.top)  y = mon.top;

	if (x + w > mon.right)  x = mon.right - w;
	if (y + h > mon.bottom) y = mon.bottom - h;
}

// --- «пришить» WS_POPUP к границам монитора с учётом DPI-виртуализации ---
static void SnapBorderlessToMonitor(HWND hWnd, const RECT& mon,
	int x, int y, int w, int h, UINT swpFlags)
{
	// 1-й проход
	SetWindowPos(hWnd, nullptr, x, y, w, h,
		SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW | swpFlags);

	// Проверяем, где окно оказалось реально
	RECT wr; GetWindowRect(hWnd, &wr);

	int rw = wr.right - wr.left;
	int rh = wr.bottom - wr.top;

	// Если есть расхождение — второй корректирующий проход
	if (wr.left != x || wr.top != y || rw != w || rh != h)
	{
		int dx = x - wr.left;
		int dy = y - wr.top;
		int dw = w - rw;
		int dh = h - rh;

		SetWindowPos(hWnd, nullptr, wr.left + dx, wr.top + dy, rw + dw, rh + dh,
			SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW | swpFlags);
	}
}

void CHW::updateWindowProps(HWND hWnd)
{
#ifndef DEDICATED_SERVER
	const BOOL bWindowed = !psDeviceFlags.is(rsFullscreen);
#else
	const BOOL bWindowed = TRUE;
#endif

	if (!bWindowed)
	{
		// фуллскрин
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLong(hWnd, GWL_EXSTYLE, 0);
		SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
#ifndef DEDICATED_SERVER
		ShowCursor(FALSE); SetForegroundWindow(hWnd);
#endif
		return;
	}

	// ---------- целевой монитор (по rs_monitor/-monitor -> по адаптеру -> ближайший) ----------
	HMONITOR target = NULL;

	if (UserMonitor == UINT_MAX && ps_rs_display >= 0)
		UserMonitor = (UINT)ps_rs_display;

	if (UserMonitor >= 0)
	{
		struct { UINT want, cur; HMONITOR res; } ctx{ (UINT)UserMonitor, 0, NULL };
		auto cb = [](HMONITOR hm, HDC, LPRECT, LPARAM lp)->BOOL {
			auto& c = *reinterpret_cast<decltype(ctx)*>(lp);
			if (c.cur == c.want)
			{
				c.res = hm;
				return FALSE;
			}
			++c.cur; return TRUE;
		};
		EnumDisplayMonitors(nullptr, nullptr, (MONITORENUMPROC)cb, (LPARAM)&ctx);
		target = ctx.res;
	}
	if (!target && UserAdapter != UINT_MAX && pD3D)
		target = pD3D->GetAdapterMonitor(UserAdapter);
	if (!target)
		target = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO mi{ sizeof(mi) };
	GetMonitorInfoW(target, &mi);

	// ---------- стиль окна (бордерлесс) ----------
	DWORD style = WS_POPUP | WS_VISIBLE;
	DWORD exstyle = 0;
	SetWindowLong(hWnd, GWL_STYLE, style);
	SetWindowLong(hWnd, GWL_EXSTYLE, exstyle);
	SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
		SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// ---------- размеры и позиция ----------
	bool wantCenter = !strstr(Core.Params, "-center_screen");

	const int monW = mi.rcMonitor.right - mi.rcMonitor.left;
	const int monH = mi.rcMonitor.bottom - mi.rcMonitor.top;

	// авто: full-borderless только если backbuffer больше монитора
	const bool makeFull =
		(int)DevPP.BackBufferWidth > monW ||
		(int)DevPP.BackBufferHeight > monH;

	int outW = (int)DevPP.BackBufferWidth;
	int outH = (int)DevPP.BackBufferHeight;
	int x, y;

	if (makeFull)
	{
		// всегда во весь выбранный монитор
		x = mi.rcMonitor.left;
		y = mi.rcMonitor.top;
		outW = monW;
		outH = monH;
	}
	else if (wantCenter)
	{
		// центрируем ТОЛЬКО если есть -center_screen (в рабочей области выбранного монитора)
		const RECT& wrk = mi.rcWork;
		x = wrk.left + ((wrk.right - wrk.left) - outW) / 2;
		y = wrk.top + ((wrk.bottom - wrk.top) - outH) / 2;
	}
	else
	{
		// без center_screen — левый верх рабочей области ВЫБРАННОГО монитора
		x = mi.rcWork.left;
		y = mi.rcWork.top;
	}

	ClampToMonitorRect(x, y, outW, outH, mi.rcMonitor);
	SetWindowPos(hWnd, nullptr, x, y, outW, outH,
		SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

#ifndef DEDICATED_SERVER
	ShowCursor(FALSE);
	SetForegroundWindow(hWnd);
#endif
}

struct _uniq_mode
{
	_uniq_mode(LPCSTR v):_val(v){}
	LPCSTR _val;
	bool operator() (LPCSTR _other) {return !stricmp(_val,_other);}
};

#ifndef _EDITOR
void free_vid_mode_list()
{
	for( int i=0; vid_mode_token[i].name; i++ )
	{
		xr_free					(vid_mode_token[i].name);
	}
	xr_free						(vid_mode_token);
	vid_mode_token				= NULL;
}

void	fill_vid_mode_list			(CHW* _hw)
{
	if(vid_mode_token != NULL)		return;
	xr_vector<LPCSTR>	_tmp;
	u32 cnt = _hw->pD3D->GetAdapterModeCount	(_hw->DevAdapter, _hw->Caps.fTarget);

	u32 i;
	for(i=0; i<cnt;++i)
	{
		D3DDISPLAYMODE	Mode;
		string32		str;

		_hw->pD3D->EnumAdapterModes(_hw->DevAdapter, _hw->Caps.fTarget, i, &Mode);
		if(Mode.Width < 800)		continue;

		sprintf_s						(str,sizeof(str),"%dx%d", Mode.Width, Mode.Height);
	
		if(_tmp.end() != std::find_if(_tmp.begin(), _tmp.end(), _uniq_mode(str)))
			continue;

		_tmp.push_back				(NULL);
		_tmp.back()					= xr_strdup(str);
	}

	u32 _cnt						= _tmp.size()+1;

	vid_mode_token					= xr_alloc<xr_token>(_cnt);

	vid_mode_token[_cnt-1].id			= -1;
	vid_mode_token[_cnt-1].name		= NULL;

#ifdef DEBUG
	Msg("Available video modes[%d]:",_tmp.size());
#endif // DEBUG
	for(i=0; i<_tmp.size();++i)
	{
		vid_mode_token[i].id		= i;
		vid_mode_token[i].name		= _tmp[i];
#ifdef DEBUG
		Msg							("[%s]",_tmp[i]);
#endif // DEBUG
	}
}
#endif

static std::wstring GetMonitorFriendlyByGdiName(const wchar_t* gdiName)
{
	if (!gdiName || !*gdiName)
		return L"";

	UINT32 pathCount = 0, modeCount = 0;
	if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE, &pathCount, &modeCount) != ERROR_SUCCESS)
		return L"";

	std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
	std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
	if (QueryDisplayConfig(QDC_ONLY_ACTIVE, &pathCount, paths.data(),
		&modeCount, modes.data(), nullptr) != ERROR_SUCCESS)
		return L"";

	paths.resize(pathCount);

	for (const auto& p : paths)
	{
		DISPLAYCONFIG_SOURCE_DEVICE_NAME src = {};
		src.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
		src.header.size = sizeof(src);
		src.header.adapterId = p.sourceInfo.adapterId;
		src.header.id = p.sourceInfo.id;

		if (DisplayConfigGetDeviceInfo(&src.header) != ERROR_SUCCESS)
			continue;

		if (lstrcmpiW(src.viewGdiDeviceName, gdiName) != 0)
			continue;

		DISPLAYCONFIG_TARGET_DEVICE_NAME tgt = {};
		tgt.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
		tgt.header.size = sizeof(tgt);
		tgt.header.adapterId = p.targetInfo.adapterId;
		tgt.header.id = p.targetInfo.id;

		if (DisplayConfigGetDeviceInfo(&tgt.header) == ERROR_SUCCESS)
		{
			// flags — это структура; у неё есть поле friendlyNameFromEdid (бит 0)
			if (tgt.monitorFriendlyDeviceName[0])
				return tgt.monitorFriendlyDeviceName;
			// на всякий случай вернём путь
			if (tgt.monitorDevicePath[0])
				return tgt.monitorDevicePath;
		}
	}
	return L"";
}

static std::wstring GetMonitorFriendlyByHMONITOR(HMONITOR hm)
{
	if (!hm) return L"";

	MONITORINFOEXW mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(hm, reinterpret_cast<MONITORINFO*>(&mi)))
		return L"";

	// основной путь
	std::wstring s = GetMonitorFriendlyByGdiName(mi.szDevice);
	if (!s.empty()) return s;

	// фолбэк через EnumDisplayDevices
	DISPLAY_DEVICEW dd; ZeroMemory(&dd, sizeof(dd)); dd.cb = sizeof(dd);
	if (EnumDisplayDevicesW(mi.szDevice, 0, &dd, 0) && dd.DeviceString[0])
		return dd.DeviceString;

	// крайний фолбэк
	return mi.szDevice;
}

// ---- локальный контекст перечисления ----
struct MonEnumCtx {
	xr_vector<LPCSTR>* names;
	int primary; // индекс primary (0..N-1), -1 если не определён
};

void CHW::free_mon_token_list()
{
	if (!vid_disp_token)
		return;
	for (int i = 0; vid_disp_token[i].name; ++i)
		xr_free(vid_disp_token[i].name);
	xr_free(vid_disp_token);
	vid_disp_token = NULL;
}

static void rtrim_w(wchar_t* s)
{
	size_t n = wcslen(s);
	while (n && (s[n - 1] == L' ' || s[n - 1] == L'\t' || s[n - 1] == L'\r' || s[n - 1] == L'\n'))
		s[--n] = L'\0';
}

static void rtrim_a(char* s)
{
	size_t n = xr_strlen(s);
	while (n && ((unsigned char)s[n - 1] <= ' ')) // пробелы и управляющие
		s[--n] = '\0';
}

static BOOL CALLBACK BuildMonTokenProc(HMONITOR hm, HDC, LPRECT, LPARAM lp)
{
	MonEnumCtx* ctx = reinterpret_cast<MonEnumCtx*>(lp);
	xr_vector<LPCSTR>& out = *ctx->names;

	MONITORINFOEXW mi; ZeroMemory(&mi, sizeof(mi)); mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(hm, reinterpret_cast<MONITORINFO*>(&mi))) {
		char fb[32]; xr_sprintf(fb, "MON_%d", int(out.size() + 1));
		out.push_back(xr_strdup(fb));
		return TRUE;
	}

	const int idx = (int)out.size();
	if (mi.dwFlags & MONITORINFOF_PRIMARY)
	{
		ctx->primary = idx;
	}

	// дружелюбное имя; может приходить с пробелами справа из EDID
	std::wstring friendly = GetMonitorFriendlyByHMONITOR(hm);

	wchar_t nameW[256];
	if (!friendly.empty()) {
		wcsncpy(nameW, friendly.c_str(), _countof(nameW) - 1);
		nameW[_countof(nameW) - 1] = L'\0';
		rtrim_w(nameW);                       // ВАЖНО: режем хвост
	}
	else {
		wcsncpy(nameW, mi.szDevice, _countof(nameW) - 1);
		nameW[_countof(nameW) - 1] = L'\0';
	}

	// финальная подпись токена
	wchar_t wtitle[256];
	swprintf(wtitle, L"%s%s",
		nameW,
		(mi.dwFlags & MONITORINFOF_PRIMARY) ? L" (primary)" : L"");

	char title[256] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, wtitle, -1, title, sizeof(title), NULL, NULL);
	rtrim_a(title);                            // защита от хвостов после конвертации

	out.push_back(xr_strdup(title));
	return TRUE;
}

// ---- сборка токена мониторов + установка дефолта ----
void CHW::fill_mon_token_list()
{
	if (vid_disp_token) return;

	xr_vector<LPCSTR> names; names.reserve(8);
	MonEnumCtx ctx; ctx.names = &names; ctx.primary = -1;

	EnumDisplayMonitors(NULL, NULL, BuildMonTokenProc, reinterpret_cast<LPARAM>(&ctx));

	const u32 cnt = names.size() + 1;
	vid_disp_token = xr_alloc<xr_token>(cnt);
	vid_disp_token[cnt - 1].id = -1;
	vid_disp_token[cnt - 1].name = NULL;

#ifdef DEBUG
	Msg("Available monitors[%d]:", names.size());
#endif
	for (u32 i = 0; i < names.size(); ++i)
	{
		vid_disp_token[i].id = (int)i;
		vid_disp_token[i].name = names[i];
		if (ctx.primary != -1)
			PrimaryMonitorID = (int)i;
#ifdef DEBUG
		Msg("[%s]", names[i]);
#endif
	}
}

