/*
** Copyright (C) 2015 Austin Borger <aaborger@gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
** API documentation is available here:
**		https://github.com/AustinBorger/DXWindow
*/

#include "DXWindow.h"
#include "QueryInterface.h"
#include <atlbase.h>
#include <string>

#ifdef _DEBUG

#include <iostream>

#endif

#include "VertexShader.h"
#include "PixelShader.h"

#pragma comment(lib, "DXWindow.lib")

#define FILENAME L"Main.cpp"
#define HANDLE_HR(Line) if (FAILED(hr)) { OnObjectFailure(FILENAME, Line, hr); }
#define HANDLE_ERR(Line) if (FAILED(HRESULT_FROM_WIN32(GetLastError()))) { OnObjectFailure(FILENAME, Line, HRESULT_FROM_WIN32(GetLastError())); }

#ifdef _DXWINDOW_TEST_11

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#elif defined(_DXWINDOW_TEST_12)

#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")

#endif

void HandleHR(LPCWSTR File, UINT Line, HRESULT hr) {
	if (FAILED(hr)) {
		_com_error e(hr);
		std::wstring Message;

		Message.append(File);
		Message.append(L" @ Line ");
		Message.append(std::to_wstring(Line));
		Message.append(L":\n\n");
		Message.append(e.ErrorMessage());

		MessageBoxW(NULL, Message.c_str(), L"Error", MB_ICONERROR | MB_OK);

		exit(hr);
	}
}

#ifdef _DXWINDOW_TEST_11

// MAIN FUNCTION SIGNATURE DEPENDENT ON DEBUG OR RELEASE
#ifdef _DEBUG

int main() {

#else

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {

#endif

	class X : public CDXWindowCallback {
	public:
		X() : run(true) {
			DXWINDOW_DESC Desc;

			Desc.AllowToggle = TRUE;
			Desc.Cursor = NULL;
			Desc.FullscreenState = DXWINDOW_FULLSCREEN_STATE_FULLSCREEN;
			Desc.Height = 720;
			Desc.IconLarge = NULL;
			Desc.IconSmall = NULL;
			Desc.InitFullscreen = FALSE;
			Desc.Instance = GetModuleHandleW(NULL);
			Desc.NumBuffers = 2;
			Desc.Title = L"DXWindowTest";
			Desc.Width = 1280;
			Desc.WindowState = DXWINDOW_WINDOW_STATE_WINDOWED;

			HRESULT hr = D3D11CreateDevice (
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				NULL,
				NULL,
				0,
				D3D11_SDK_VERSION,
				&Device,
				NULL,
				&DeviceContext
			); HANDLE_HR(__LINE__);

			hr = Device->CreateVertexShader (
				VertexShader,
				sizeof(VertexShader),
				nullptr,
				&VS
			); HANDLE_HR(__LINE__);

			hr = Device->CreatePixelShader (
				PixelShader,
				sizeof(PixelShader),
				nullptr,
				&PS
			); HANDLE_HR(__LINE__);

			struct Info {
				float time;
				float pad[7];
			} i;

			D3D11_BUFFER_DESC cBufferDesc = { 0 };
			cBufferDesc.ByteWidth = sizeof(Info);
			cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cBufferDesc.MiscFlags = NULL;
			cBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA sub;
			sub.pSysMem = &i;
			sub.SysMemPitch = 0;
			sub.SysMemSlicePitch = 0;

			hr = Device->CreateBuffer (
				&cBufferDesc,
				&sub,
				&TimeBuffer
			); HANDLE_HR(__LINE__);

			hr = DXWindowCreateWindow (
				&Desc,
				Device,
				this,
				&Window
			); HANDLE_HR(__LINE__);
		}

		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
			QUERY_INTERFACE_CAST(IDXWindowCallback);
			QUERY_INTERFACE_CAST(IUnknown);
			QUERY_INTERFACE_FAIL();
		}

		ULONG STDMETHODCALLTYPE AddRef() {
			return 1;
		}

		ULONG STDMETHODCALLTYPE Release() {
			return 1;
		}

		VOID STDMETHODCALLTYPE OnObjectFailure(LPCWSTR File, UINT Line, HRESULT hr) final {
			HandleHR(File, Line, hr);
		}

		VOID STDMETHODCALLTYPE OnWindowClose(IDXWindow* Window) final {
			run = false;
		}

		VOID STDMETHODCALLTYPE OnKeyDown(IDXWindow* Window, WPARAM wParam, LPARAM lParam) final {
			if (wParam == VK_F1) {
				Window->SetState(DXWINDOW_STATE_WINDOWED);
			} else if (wParam == VK_F2) {
				Window->SetState(DXWINDOW_STATE_BORDERLESS);
			} else if (wParam == VK_F3) {
				Window->SetState(DXWINDOW_STATE_FULLSCREEN_WINDOW);
			} else if (wParam == VK_F4) {
				Window->SetState(DXWINDOW_STATE_FULLSCREEN);
			} else if (wParam == VK_F5) {
				Window->SetWindowResolution(640, 640);
			} else if (wParam == VK_F6) {
				Window->SetWindowResolution(1280, 720);
			}
		}

		VOID STDMETHODCALLTYPE OnBackBufferRelease(IDXWindow* Window) final {
			// CONSOLE OUTPUT

#ifdef _DEBUG

			std::wcout << "OnBackBufferRelease()" << std::endl;

#endif

			BackBufTex.Release();
			BackBufRTV.Release();
		}

		VOID STDMETHODCALLTYPE OnBackBufferCreate(IDXWindow* Window) final {
			HRESULT hr = S_OK;

			Window->GetBuffer(0, IID_PPV_ARGS(&BackBufTex));

			D3D11_TEXTURE2D_DESC Desc;
			BackBufTex->GetDesc(&Desc);

			Viewport.MaxDepth = 1.0f;
			Viewport.MinDepth = 0.0f;
			Viewport.TopLeftX = 0.0f;
			Viewport.TopLeftY = 0.0f;
			Viewport.Width = (FLOAT)(Desc.Width);
			Viewport.Height = (FLOAT)(Desc.Height);

			// CONSOLE OUTPUT

#ifdef _DEBUG

			std::wcout << "OnBackBufferCreate():" << std::endl;
			std::wcout << "\t" << "Back Buffer Width: " << Viewport.Width << std::endl;
			std::wcout << "\t" << "Back Buffer Height: " << Viewport.Height << std::endl;

#endif

			hr = Device->CreateRenderTargetView (
				BackBufTex,
				nullptr,
				&BackBufRTV
			); HANDLE_HR(__LINE__);
		}

		VOID Run() {
			LARGE_INTEGER liFrequency;
			LARGE_INTEGER liCounter;
			double CounterSeconds;
			QueryPerformanceFrequency(&liFrequency);

			struct Info {
				float time;
				float pad[7];
			} i;

			while (run) {
				Window->PumpMessages();

				FLOAT color[] = { 1.0, 1.0, 1.0, 1.0 };

				DeviceContext->ClearRenderTargetView(BackBufRTV, color);

				QueryPerformanceCounter(&liCounter);
				CounterSeconds = double(liCounter.QuadPart) / double(liFrequency.QuadPart);
				i.time = float(CounterSeconds);
				D3D11_MAPPED_SUBRESOURCE sub;
				ZeroMemory(&sub, sizeof(sub));
				DeviceContext->Map(TimeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
				memcpy(sub.pData, &i, sizeof(Info));
				DeviceContext->Unmap(TimeBuffer, 0);

				ID3D11RenderTargetView* rtv = BackBufRTV;
				ID3D11Buffer* timeBuffer = TimeBuffer;

				DeviceContext->RSSetViewports(1, &Viewport);
				DeviceContext->VSSetShader(VS, nullptr, 0);
				DeviceContext->PSSetShader(PS, nullptr, 0);
				DeviceContext->PSSetConstantBuffers(0, 1, &timeBuffer);
				DeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
				DeviceContext->IASetInputLayout(nullptr);
				DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				DeviceContext->Draw(3, 0);

				Window->Present(1, 0);
			}
		}

		bool run;
		CComPtr<ID3D11Texture2D> BackBufTex;
		CComPtr<ID3D11RenderTargetView> BackBufRTV;
		CComPtr<ID3D11Device> Device;
		CComPtr<ID3D11DeviceContext> DeviceContext;
		CComPtr<ID3D11VertexShader> VS;
		CComPtr<ID3D11PixelShader> PS;
		CComPtr<ID3D11Buffer> TimeBuffer;
		D3D11_VIEWPORT Viewport;

		CComPtr<IDXWindow> Window;
	} x;

	x.Run();

	return 0;
}

#elif defined(_DXWINDOW_TEST_12)

int main() {
	class X : public CDXWindowCallback {
	public:
		X() : run(true) {
			HRESULT hr = S_OK;

#ifdef _DEBUG

			CComPtr<ID3D12Debug> Debug;

			hr = D3D12GetDebugInterface(
				IID_PPV_ARGS(&Debug)
			); HANDLE_HR(__LINE__);

			Debug->EnableDebugLayer();

#endif

			hr = D3D12CreateDevice (
				nullptr,
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&Device)
			); HANDLE_HR(__LINE__);

#ifdef _DEBUG

			hr = Device->QueryInterface (
				IID_PPV_ARGS(&InfoQueue)
			); HANDLE_HR(__LINE__);

#endif

			D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
			CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			CommandQueueDesc.NodeMask = NULL;
			CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			CommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

			hr = Device->CreateCommandQueue (
				&CommandQueueDesc,
				IID_PPV_ARGS(&CommandQueue)
			); HANDLE_HR(__LINE__);

			hr = Device->CreateCommandAllocator (
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(&CommandAllocator)
			); HANDLE_HR(__LINE__);

			hr = Device->CreateCommandList (
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				CommandAllocator,
				nullptr,
				IID_PPV_ARGS(&CommandList)
			); HANDLE_HR(__LINE__);

			hr = CommandList->Close(); HANDLE_HR(__LINE__);

			hr = Device->CreateFence (
				0,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&Fence)
			); HANDLE_HR(__LINE__);

			SetLastError(0);

			FenceEvent = CreateEventExW (
				NULL,
				FALSE,
				FALSE,
				EVENT_ALL_ACCESS
			); HANDLE_ERR(__LINE__);

			FenceValue = 1;

			DXWINDOW_DESC Desc;

			Desc.AllowToggle = TRUE;
			Desc.Cursor = NULL;
			Desc.FullscreenState = DXWINDOW_FULLSCREEN_STATE_FULLSCREEN;
			Desc.Height = 720;
			Desc.IconLarge = NULL;
			Desc.IconSmall = NULL;
			Desc.InitFullscreen = FALSE;
			Desc.Instance = GetModuleHandleW(NULL);
			Desc.NumBuffers = 2;
			Desc.Title = L"DXWindowTest";
			Desc.Width = 1280;
			Desc.WindowState = DXWINDOW_WINDOW_STATE_WINDOWED;

			hr = DXWindowCreateWindow (
				&Desc,
				CommandQueue,
				this,
				&Window
			); HANDLE_HR(__LINE__);
		}

		STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
			QUERY_INTERFACE_CAST(IDXWindowCallback);
			QUERY_INTERFACE_CAST(IUnknown);
			QUERY_INTERFACE_FAIL();
		}

		ULONG STDMETHODCALLTYPE AddRef() {
			return 1;
		}

		ULONG STDMETHODCALLTYPE Release() {
			return 1;
		}

		VOID STDMETHODCALLTYPE OnObjectFailure(LPCWSTR File, UINT Line, HRESULT hr) final {

#ifdef _DEBUG

			UINT64 nMessages = InfoQueue->GetNumStoredMessages();
			SIZE_T Size;

			std::wcout << "OnObjectFailure():" << std::endl;
			std::wcout << "\t" << "nMessages: " << nMessages << std::endl;

			for (UINT64 i = 0; i < nMessages; i++) {
				InfoQueue->GetMessageW(i, nullptr, &Size);
				D3D12_MESSAGE* pMessage = (D3D12_MESSAGE*)malloc(Size);
				InfoQueue->GetMessageW(i, pMessage, &Size);
				std::wcout << pMessage->pDescription << std::endl;
				free(pMessage);
			}

#endif

			HandleHR(File, Line, hr);
		}

		VOID STDMETHODCALLTYPE OnWindowClose(IDXWindow* Window) final {
			run = false;
		}

		VOID STDMETHODCALLTYPE OnKeyDown(IDXWindow* Window, WPARAM wParam, LPARAM lParam) final {
			if (wParam == VK_F1) {
				Window->SetState(DXWINDOW_STATE_WINDOWED);
			} else if (wParam == VK_F2) {
				Window->SetState(DXWINDOW_STATE_BORDERLESS);
			} else if (wParam == VK_F3) {
				Window->SetState(DXWINDOW_STATE_FULLSCREEN_WINDOW);
			} else if (wParam == VK_F4) {
				Window->SetState(DXWINDOW_STATE_FULLSCREEN);
			} else if (wParam == VK_F5) {
				Window->SetWindowResolution(640, 640);
			} else if (wParam == VK_F6) {
				Window->SetWindowResolution(1280, 720);
			}
		}

		VOID STDMETHODCALLTYPE OnBackBufferRelease(IDXWindow* Window) final {
			// CONSOLE OUTPUT

#ifdef _DEBUG

			std::wcout << "OnBackBufferRelease()" << std::endl;

#endif

			RenderTargetResource[0].Release();
			RenderTargetResource[1].Release();
			RenderTargetHeap.Release();
			RenderTargetViewHandle.ptr = 0;
			RenderTargetBytes = 0;
		}

		VOID STDMETHODCALLTYPE OnBackBufferCreate(IDXWindow* Window) final {
			HRESULT hr = S_OK;

			D3D12_DESCRIPTOR_HEAP_DESC RenderTargetHeapDesc;
			RenderTargetHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			RenderTargetHeapDesc.NodeMask = NULL;
			RenderTargetHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			RenderTargetHeapDesc.NumDescriptors = 2;

			hr = Device->CreateDescriptorHeap (
				&RenderTargetHeapDesc,
				IID_PPV_ARGS(&RenderTargetHeap)
			); HANDLE_HR(__LINE__);

			RenderTargetViewHandle = RenderTargetHeap->GetCPUDescriptorHandleForHeapStart();

			RenderTargetBytes = Device->GetDescriptorHandleIncrementSize (
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV
			);

			Window->GetBuffer(0, IID_PPV_ARGS(&RenderTargetResource[0]));

			Device->CreateRenderTargetView (
				RenderTargetResource[0],
				nullptr,
				RenderTargetViewHandle
			);

			RenderTargetViewHandle.ptr += RenderTargetBytes;

			Window->GetBuffer(1, IID_PPV_ARGS(&RenderTargetResource[1]));

			Device->CreateRenderTargetView (
				RenderTargetResource[1],
				nullptr,
				RenderTargetViewHandle
			);

			D3D12_RESOURCE_DESC ResourceDesc = RenderTargetResource[0]->GetDesc();

			Viewport.MaxDepth = 1.0f;
			Viewport.MinDepth = 0.0f;
			Viewport.TopLeftX = 0.0f;
			Viewport.TopLeftY = 0.0f;
			Viewport.Width = (FLOAT)(ResourceDesc.Width);
			Viewport.Height = (FLOAT)(ResourceDesc.Height);

			ScissorRect.left = 0;
			ScissorRect.top = 0;
			ScissorRect.right = (LONG)(ResourceDesc.Width);
			ScissorRect.bottom = (LONG)(ResourceDesc.Height);

			BufferIndex = 0;

			// CONSOLE OUTPUT

#ifdef _DEBUG

			std::wcout << "OnBackBufferCreate():" << std::endl;
			std::wcout << "\t" << "Back Buffer Width: " << Viewport.Width << std::endl;
			std::wcout << "\t" << "Back Buffer Height: " << Viewport.Height << std::endl;

#endif
		}

		VOID Run() {
			HRESULT hr = S_OK;

			D3D12_RESOURCE_BARRIER Barrier;
			D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewPtr;
			ID3D12CommandList* ppCommandLists[1];
			unsigned long long FenceToWaitFor = 0;

			LARGE_INTEGER liFrequency;
			LARGE_INTEGER liCounter;
			double CounterSeconds;
			QueryPerformanceFrequency(&liFrequency);

			while (run) {
				Window->PumpMessages();

				hr = CommandAllocator->Reset(); HANDLE_HR(__LINE__);

				hr = CommandList->Reset (
					CommandAllocator,
					nullptr
				); HANDLE_HR(__LINE__);

				CommandList->RSSetViewports(1, &Viewport);
				CommandList->RSSetScissorRects(1, &ScissorRect);

				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.Transition.pResource = RenderTargetResource[BufferIndex];
				Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
				Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
				Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

				CommandList->ResourceBarrier(1, &Barrier);

				RenderTargetViewPtr.ptr = RenderTargetViewHandle.ptr + BufferIndex * RenderTargetBytes;

				CommandList->OMSetRenderTargets(1, &RenderTargetViewPtr, FALSE, nullptr);

				FLOAT color[] = { 1.0, 1.0, 1.0, 1.0 };

				CommandList->ClearRenderTargetView(RenderTargetViewHandle, color, 0, nullptr);

				Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				Barrier.Transition.pResource = RenderTargetResource[BufferIndex];
				Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

				CommandList->ResourceBarrier(1, &Barrier);

				hr = CommandList->Close(); HANDLE_HR(__LINE__);

				ppCommandLists[0] = CommandList;

				CommandQueue->ExecuteCommandLists(1, ppCommandLists);

				Window->Present(1, 0);

				FenceToWaitFor = FenceValue;

				hr = CommandQueue->Signal (
					Fence,
					FenceToWaitFor
				); HANDLE_HR(__LINE__);

				FenceValue++;

				if (Fence->GetCompletedValue() < FenceToWaitFor) {
					hr = Fence->SetEventOnCompletion (
						FenceToWaitFor,
						FenceEvent
					); HANDLE_HR(__LINE__);

					WaitForSingleObject(FenceEvent, INFINITE);
				}

				BufferIndex = 1 - BufferIndex;
			}
		}

	private:
		bool run;

		CComPtr<IDXWindow> Window;

		CComPtr<ID3D12Device> Device;
		CComPtr<ID3D12CommandQueue> CommandQueue;
		CComPtr<ID3D12CommandAllocator> CommandAllocator;
		CComPtr<ID3D12GraphicsCommandList> CommandList;
		CComPtr<ID3D12DescriptorHeap> RenderTargetHeap;
		CComPtr<ID3D12Resource> RenderTargetResource[2];
		CComPtr<ID3D12Fence> Fence;
		CComPtr<ID3D12PipelineState> PipelineState;

		D3D12_VIEWPORT Viewport;
		D3D12_RECT ScissorRect;
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetViewHandle;
		UINT RenderTargetBytes;
		HANDLE FenceEvent;
		UINT BufferIndex;
		unsigned long long FenceValue;

#ifdef _DEBUG

		CComPtr<ID3D12InfoQueue> InfoQueue;

#endif
	} x;

	x.Run();

	return 0;
}

#endif