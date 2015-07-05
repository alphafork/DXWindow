#pragma once

#include <Windows.h>
#include <comdef.h>
#include <atlbase.h>
#include <dxgi.h>

#include "DXWindow.h"
#include "Output.h"
#include "OutputEnum.h"

class CDXWindow;

class SwapChainController {
public:
	SwapChainController(CDXWindow& Window, OutputEnum& Enum);

	HRESULT Initialize(CComPtr<IUnknown> DeviceUnk, CComPtr<IDXWindowCallback> Callback, HWND Handle);

	HRESULT ToggleFullscreen();

	HRESULT ResizeBuffers();

	HRESULT GetDesktopArea(RECT& DesktopArea);

	HRESULT Present(UINT SyncInterval, UINT Flags);

	HRESULT GetBackBuffer(REFIID rIID, void** ppvBackBuffer);

private:
	CDXWindow& m_Window;
	CComPtr<IDXWindowCallback> m_Callback;
	CComPtr<IDXGISwapChain> m_SwapChain;
	HWND m_Handle;
	OutputEnum& m_OutputEnum;

	BOOL IsFullscreen() {
		BOOL Fullscreen;

		m_SwapChain->GetFullscreenState (
			&Fullscreen,
			nullptr
		);

		return Fullscreen;
	}

	HRESULT CreateSwapChain(CComPtr<IUnknown> DeviceUnk);
};