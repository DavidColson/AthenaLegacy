#include "SDL.h"
#include "SDL_syswm.h"

#include <DirectXPackedVector.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3d10.h>
#include <D3DCompiler.h>
#include <comdef.h>

IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // global declaration

struct Point
{
	Point(float x, float y, float z) : x(x), y(y), z(z) {}

	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
};

struct Vertex
{
	Vertex(Point pos, Point col) : pos(pos), col(col) {}

	Point pos{ Point(0.0f, 0.0f, 0.0f) };
	Point col{ Point(0.0f, 0.0f, 0.0f) };
};

void InitScene()
{
	HRESULT hr;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	// Runtime Compile shaders
	hr = D3DCompileFromFile(L"Shader.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(L"Shader.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

	// Create shader objects
	hr = dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);
	hr = dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

	// Set Shaders to active
	devcon->VSSetShader(vertexShader, 0, 0);
	devcon->PSSetShader(pixelShader, 0, 0);

	// Create vertex buffer
	Vertex triangle[] = {
		Vertex(Point(0.0f, 0.5f, 0.5f), Point(1.0f, 0.0f, 0.0f)),
		Vertex(Point(0.5f, -0.5f, 0.5f), Point(0.0f, 1.0f, 0.0f)),
		Vertex(Point(-0.5f, -0.5f, 0.5f), Point(0.0f, 0.0f, 1.0f))
	};

	// Describe the buffer 
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// fill the buffer with actual data
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = triangle;

	ID3D11Buffer* vertBuffer;
	hr = dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertBuffer);

	// Set vertex buffer as active
	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	// Create an input layout
	ID3D11InputLayout* vertLayout;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = dev->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &vertLayout);

	// Set the input layout as active
	devcon->IASetInputLayout(vertLayout);

	// Set primitive Topology
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Create viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = 640;
	viewport.Height = 480;

	// Set Viewport as active
	devcon->RSSetViewports(1, &viewport);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		"DirectX",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		0
	);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hwnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	InitScene();

	SDL_Event event;
	bool bContinue = true;
	while (bContinue)
	{
		// clear the back buffer to a deep blue
		float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
		devcon->ClearRenderTargetView(backbuffer, color);

		// do 3D rendering on the back buffer here
		devcon->Draw(3, 0);

		// switch the back buffer and the front buffer
		swapchain->Present(0, 0);

		if (SDL_PollEvent(&event)) {
			/* an event was found */
			switch (event.type) {
			case SDL_QUIT:
				bContinue = false;
				break;
			}
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}