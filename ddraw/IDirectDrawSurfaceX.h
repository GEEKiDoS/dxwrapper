#pragma once

class m_IDirectDrawSurfaceX
{
private:
	IDirectDrawSurface7 *ProxyInterface;
	m_IDirectDrawSurface7 *WrapperInterface;
	DWORD DirectXVersion;
	DWORD ProxyDirectXVersion;
	IID WrapperID;
	ULONG RefCount = 1;

	// Convert to d3d9
	m_IDirectDrawX *ddrawParent;
	DDSURFACEDESC2 surfaceDesc;
	LONG surfaceWidth;
	LONG surfaceHeight;
	DDCOLORKEY colorKeys[4];		// Color keys(DDCKEY_DESTBLT, DDCKEY_DESTOVERLAY, DDCKEY_SRCBLT, DDCKEY_SRCOVERLAY)
	LONG overlayX = 0;
	LONG overlayY = 0;
	m_IDirectDrawPalette *attachedPalette = nullptr;	// Associated palette

public:
	m_IDirectDrawSurfaceX(IDirectDrawSurface7 *pOriginal, DWORD Version, m_IDirectDrawSurface7 *Interface) : ProxyInterface(pOriginal), DirectXVersion(Version), WrapperInterface(Interface)
	{
		InitWrapper();
	}
	m_IDirectDrawSurfaceX(m_IDirectDrawX *Interface, DWORD Version, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD displayModeWidth, DWORD displayModeHeight) : 
		ddrawParent(Interface), DirectXVersion(Version), surfaceWidth(displayModeWidth), surfaceHeight(displayModeHeight)
	{
		ProxyInterface = nullptr;
		WrapperInterface = nullptr;

		InitWrapper();

		// Init color keys
		for (int i = 0; i < 4; i++)
		{
			memset(&colorKeys[i], 0, sizeof(DDCOLORKEY));
		}

		//set width and height
		/*if(lpDDSurfaceDesc->dwFlags & DDSD_WIDTH)
		{
		surfaceWidth = lpDDSurfaceDesc->dwWidth;
		}
		else
		{
		surfaceWidth = displayModeWidth;
		}
		if(lpDDSurfaceDesc->dwFlags & DDSD_HEIGHT)
		{
		surfaceHeight = lpDDSurfaceDesc->dwHeight;
		}
		else
		{
		surfaceHeight = displayModeHeight;
		}

		//allocate virtual video memory raw
		double indexSize = 1;
		if(lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1)
		{
		indexSize = 0.125;
		}
		else if(lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2)
		{
		indexSize = 0.25;
		}
		else if(lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
		{
		indexSize = 0.5;
		}
		else if(lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8 | lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXEDTO8)
		{
		indexSize = 1;
		}*/

		//Overallocate the memory to prevent access outside of memory range by the exe
		//if(!ReInitialize(displayWidth, displayHeight)) return DDERR_OUTOFMEMORY;
		//maximum supported resolution is 1920x1440
		rawVideoMem = new BYTE[1920 * 1440];

		// Clear raw memory
		ZeroMemory(rawVideoMem, 1920 * 1440 * sizeof(BYTE));

		// Allocate virtual video memory RGB
		rgbVideoMem = new UINT32[surfaceWidth * surfaceHeight];

		// Clear rgb memory
		ZeroMemory(rgbVideoMem, surfaceWidth * surfaceHeight * sizeof(UINT32));

		// Copy surface description
		memcpy(&surfaceDesc, lpDDSurfaceDesc, sizeof(DDSURFACEDESC2));
	}
	~m_IDirectDrawSurfaceX()
	{
		// Free memory for internal structures
		if (rawVideoMem != nullptr)
		{
			delete rawVideoMem;
			rawVideoMem = nullptr;
		}
		if (rgbVideoMem != nullptr)
		{
			delete rawVideoMem;
			rgbVideoMem = nullptr;
		}
	}

	void InitWrapper()
	{
		WrapperID = (DirectXVersion == 1) ? IID_IDirectDrawSurface :
			(DirectXVersion == 2) ? IID_IDirectDrawSurface2 :
			(DirectXVersion == 3) ? IID_IDirectDrawSurface3 :
			(DirectXVersion == 4) ? IID_IDirectDrawSurface4 :
			(DirectXVersion == 7) ? IID_IDirectDrawSurface7 : IID_IDirectDrawSurface7;

		if (Config.Dd7to9)
		{
			ProxyDirectXVersion = 9;
		}
		else
		{
			REFIID ProxyID = ConvertREFIID(WrapperID);
			ProxyDirectXVersion = (ProxyID == IID_IDirectDrawSurface) ? 1 :
				(ProxyID == IID_IDirectDrawSurface2) ? 2 :
				(ProxyID == IID_IDirectDrawSurface3) ? 3 :
				(ProxyID == IID_IDirectDrawSurface4) ? 4 :
				(ProxyID == IID_IDirectDrawSurface7) ? 7 : 7;
		}

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert DirectDrawSurface v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
	}

	UINT32 *rgbVideoMem = nullptr;		// RGB video memory
	BYTE *rawVideoMem = nullptr;		// Virtual video memory

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return WrapperID; }
	IDirectDrawSurface7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirectDrawSurface7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef) (THIS);
	STDMETHOD_(ULONG, Release) (THIS);
	/*** IDirectDrawSurface methods ***/
	STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE7);
	STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT);
	STDMETHOD(Blt)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDBLTFX);
	STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH, DWORD, DWORD);
	STDMETHOD(BltFast)(THIS_ DWORD, DWORD, LPDIRECTDRAWSURFACE7, LPRECT, DWORD);
	STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD, LPVOID, LPDDENUMSURFACESCALLBACK7);
	STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE7, DWORD);
	STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS2, LPDIRECTDRAWSURFACE7 FAR *);
	STDMETHOD(GetBltStatus)(THIS_ DWORD);
	STDMETHOD(GetCaps)(THIS_ LPDDSCAPS2);
	STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER FAR*);
	STDMETHOD(GetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(GetDC)(THIS_ HDC FAR *);
	STDMETHOD(GetFlipStatus)(THIS_ DWORD);
	STDMETHOD(GetOverlayPosition)(THIS_ LPLONG, LPLONG);
	STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE FAR*);
	STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT);
	STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC2);
	STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW, LPDDSURFACEDESC2);
	STDMETHOD(IsLost)(THIS);
	STDMETHOD(Lock)(THIS_ LPRECT, LPDDSURFACEDESC2, DWORD, HANDLE);
	STDMETHOD(ReleaseDC)(THIS_ HDC);
	STDMETHOD(Restore)(THIS);
	STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER);
	STDMETHOD(SetColorKey)(THIS_ DWORD, LPDDCOLORKEY);
	STDMETHOD(SetOverlayPosition)(THIS_ LONG, LONG);
	STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE);
	STDMETHOD(Unlock)(THIS_ LPRECT);
	STDMETHOD(UpdateOverlay)(THIS_ LPRECT, LPDIRECTDRAWSURFACE7, LPRECT, DWORD, LPDDOVERLAYFX);
	STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD);
	STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD, LPDIRECTDRAWSURFACE7);
	/*** Added in the v2 interface ***/
	STDMETHOD(GetDDInterface)(THIS_ LPVOID FAR *);
	STDMETHOD(PageLock)(THIS_ DWORD);
	STDMETHOD(PageUnlock)(THIS_ DWORD);
	/*** Added in the v3 interface ***/
	STDMETHOD(SetSurfaceDesc)(THIS_ LPDDSURFACEDESC2, DWORD);
	/*** Added in the v4 interface ***/
	STDMETHOD(SetPrivateData)(THIS_ REFGUID, LPVOID, DWORD, DWORD);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID, LPVOID, LPDWORD);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID);
	STDMETHOD(GetUniquenessValue)(THIS_ LPDWORD);
	STDMETHOD(ChangeUniquenessValue)(THIS);
	/*** Moved Texture7 methods here ***/
	STDMETHOD(SetPriority)(THIS_ DWORD);
	STDMETHOD(GetPriority)(THIS_ LPDWORD);
	STDMETHOD(SetLOD)(THIS_ DWORD);
	STDMETHOD(GetLOD)(THIS_ LPDWORD);
};
