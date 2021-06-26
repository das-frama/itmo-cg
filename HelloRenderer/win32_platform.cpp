// Don't worry it's a unity build!
#include "pch.h"

#include "utils.cpp"
#include "math.cpp"

#include "platform_common.cpp"
#include "game.cpp"

// Globals.
f32 frequency_counter;
int client_width = 1280;
int client_height = 720;

Microsoft::WRL::ComPtr<ID3D11Device> device;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swap_chain = nullptr;
IDXGISwapChain1* swap_chain_1 = nullptr;
ID3D11RenderTargetView* render_view = nullptr;
ID3D11DepthStencilView* depth_view = nullptr;
ID3DUserDefinedAnnotation* debug_annotation = nullptr;
ID3D11Debug* debug = nullptr;
ID3D11RasterizerState* rast_state = nullptr;
ID3D11Texture2D* back_buffer = nullptr;
ID3D11Texture2D* depth_buffer = nullptr;

inline f32
os_seconds_elapsed(u64 last_counter) {
	LARGE_INTEGER current_counter;
	QueryPerformanceCounter(&current_counter);
	return (f32)(current_counter.QuadPart - last_counter) / frequency_counter;
}

LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM w_param, LPARAM l_param) {
	//LRESULT result = 0;
	switch (msg) {
	case WM_CLOSE:
	case WM_QUIT:
		running = false;
		break;
	}

	return DefWindowProc(window, msg, w_param, l_param);;
}

HWND win32_create_window(HINSTANCE h_instance, LPCWSTR name) {
	// Register window class.
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = window_callback;
	wc.lpszClassName = L"Game Window Class";

	RegisterClassEx(&wc);

	// Create window instance.
	return CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, name,
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, client_width, client_height,
		nullptr, nullptr, h_instance, nullptr
	);
}

internal void
create_back_buffer() {
	HRESULT res = swap_chain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&back_buffer);
	res = device->CreateRenderTargetView(back_buffer, nullptr, &render_view);

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.ArraySize = 1;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.Width = client_width;
	depthTexDesc.Height = client_height;
	depthTexDesc.SampleDesc = { 1, 0 };

	res = device->CreateTexture2D(&depthTexDesc, nullptr, &depth_buffer);
	res = device->CreateDepthStencilView(depth_buffer, nullptr, &depth_view);
}

internal void
prepare_d3d11(HWND win32_window) {
	DXGI_SWAP_CHAIN_DESC swap_desc = {};
	swap_desc.BufferCount = 2;
	swap_desc.BufferDesc.Width = client_width;
	swap_desc.BufferDesc.Height = client_height;
	swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_desc.OutputWindow = win32_window;
	swap_desc.Windowed = true;
	swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swap_desc.SampleDesc.Count = 1;
	swap_desc.SampleDesc.Quality = 0;

	D3D_FEATURE_LEVEL feature_level[] = { D3D_FEATURE_LEVEL_11_1 };
	HRESULT res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		feature_level,
		1,
		D3D11_SDK_VERSION,
		&swap_desc,
		&swap_chain,
		&device,
		nullptr,
		&context
	);

	create_back_buffer();

	swap_chain->QueryInterface<IDXGISwapChain1>(&swap_chain_1);
	context->QueryInterface(IID_ID3DUserDefinedAnnotation, (void**)&debug_annotation);
	device->QueryInterface(IID_ID3D11Debug, (void**)&debug);
}

internal void
prepare_frame() {
	context->ClearState();

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(client_width);
	viewport.Height = static_cast<float>(client_height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	context->OMSetRenderTargets(1, &render_view, depth_view);

	context->RSSetViewports(1, &viewport);
	context->RSSetState(rast_state);

	context->ClearRenderTargetView(render_view, DirectX::SimpleMath::Color(0, 0, 0, 1));
	context->ClearDepthStencilView(depth_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


internal void
end_frame() {
	swap_chain_1->Present(1, 0);
}

int CALLBACK WinMain(
	HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) {

	// Set up window.
	auto win32_window = win32_create_window(hInstance, L"Frama Sailor Go!");
	{
		ShowWindow(win32_window, SW_SHOW);
		SetForegroundWindow(win32_window);
		SetFocus(win32_window);
		ShowCursor(true);
	}

	// Prepare d3d11.
	{
		prepare_d3d11(win32_window);
	}

	// Set up game vars.
	Input input = { 0 };
	f32 delta_time = 0.016666f;
	LARGE_INTEGER frame_begin_time;
	QueryPerformanceCounter(&frame_begin_time);
	f32 performance_frequency;
	{
		LARGE_INTEGER perf;
		QueryPerformanceFrequency(&perf);
		performance_frequency = (f32)perf.QuadPart;
	}


	// Game Loop.
	while (running) {
		prepare_frame();

		// Reset input.
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].changed = false;
		}

		// Message.
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG)); // Initialize the message structure.

		while (PeekMessage(&msg, win32_window, 0, 0, PM_REMOVE)) {
			switch (msg.message) {
			case WM_KEYUP:
			case WM_KEYDOWN: {
				u32 vk_code = (u32)msg.wParam;
				bool is_down = ((msg.lParam & (1 << 31)) == 0);

#define process_button(b, vk)\
case vk: {\
input.buttons[b].changed = is_down != input.buttons[b].is_down;\
input.buttons[b].is_down = is_down;\
} break;

				switch (vk_code) {
					process_button(BUTTON_UP, VK_UP);
					process_button(BUTTON_DOWN, VK_DOWN);
					process_button(BUTTON_W, 'W');
					process_button(BUTTON_S, 'S');
					process_button(BUTTON_LEFT, VK_LEFT);
					process_button(BUTTON_RIGHT, VK_RIGHT);
					process_button(BUTTON_ENTER, VK_RETURN);
				}
			} break;

			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		// Simulate.
		simulate_game(&input, delta_time);

		// Render.


		// Frame time.
		LARGE_INTEGER frame_end_time;
		QueryPerformanceCounter(&frame_end_time);
		delta_time = (float)(frame_end_time.QuadPart - frame_begin_time.QuadPart) / performance_frequency;
		frame_begin_time = frame_end_time;

		// End Frame.
		end_frame();
	}

	return 0;
}