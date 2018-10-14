#include "SDL.h"
#include "SDL_syswm.h"

#include <DirectXPackedVector.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3d10.h>
#include <D3DCompiler.h>
#include <comdef.h>

#include "maths/maths.h"

IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
ID3D11Device *dev;                     // the pointer to our Direct3D device interface
ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
ID3D11RenderTargetView *backbuffer;    // global declaration
ID3D11Buffer* wvpBuffer;			   // buffer for wvp

float width = 640.0f;
float height = 480.0f;

struct Vertex
{
	Vertex(vec3 pos, vec3 col) : pos(pos), col(col) {}

	vec3 pos{ vec3(0.0f, 0.0f, 0.0f) };
	vec3 col{ vec3(0.0f, 0.0f, 0.0f) };
};

struct cbPerObject
{
	mat4 wvp;
};
cbPerObject perObject;

void InitScene()
{
	HRESULT hr;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;




	// Runtime Compile shaders
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
	}
	hr = D3DCompileFromFile(L"shaders/Shader.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
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

	// define the model to draw
	Vertex triangle[] = {
		Vertex(vec3(0.0f, 0.5f, 0.5f), color(1.0f, 0.0f, 0.0f)),
		Vertex(vec3(0.5f, -0.5f, 0.5f), color(0.0f, 1.0f, 0.0f)),
		Vertex(vec3(-0.5f, -0.5f, 0.5f), color(0.0f, 0.0f, 1.0f))
	};
	int nVerts = 3;

	DWORD indices[] = {
		0, 1, 2, 0
	};
	int nIndices = 4;




	// Create vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * nVerts;
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




	// Create an index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * nIndices;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	// fill the index buffer with actual data
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = indices;
	ID3D11Buffer* indexBuffer;
	dev->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);

	devcon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);



	// Create a constant buffer (uniform) for the WVP
	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbPerObject);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	dev->CreateBuffer(&wvpBufferDesc, nullptr, &wvpBuffer);



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
	viewport.Width = width;
	viewport.Height = height;

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
		int(width),
		int(height),
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

	float rotation = 0.0f;

	SDL_Event event;
	bool bContinue = true;
	while (bContinue)
	{
		// clear the back buffer to a deep blue
		float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
		devcon->ClearRenderTargetView(backbuffer, color);

		rotation += 0.5f;
		mat4 rotmat = MakeRotate(vec3(0.0f, 0.0f, rotation));

		mat4 world = rotmat; // transform into world space
		mat4 view = MakeTranslate(vec3(0.0f, 0.0f, 0.0f)); // transform into camera space

		float aR = width / height;
		mat4 projection = MakeOrthographic(-1.0f * aR, 1.0f * aR, -1.0f, 1.0f, 0.1f, 10.0f); // transform into screen space

		mat4 wvp = projection * view * world;

		perObject.wvp = wvp;
		devcon->UpdateSubresource(wvpBuffer, 0, nullptr, &perObject, 0, 0);
		devcon->VSSetConstantBuffers(0, 1, &wvpBuffer);

		// do 3D rendering on the back buffer here
		devcon->DrawIndexed(3, 0, 0);

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