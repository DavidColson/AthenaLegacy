
#include "renderproxy.h"

#include <D3DCompiler.h>
#include <d3d11.h>
#include <d3d10.h>
#include <stdio.h>

#include "Maths/Matrix.h"
#include "Maths/Vec4.h"
#include "Maths/Vec3.h"
#include "Renderer/Renderer.h"

struct cbPerObject
{
	Matrixf wvp;
	float lineThickness;
	float pad1{ 0.0f };
	float pad2{ 0.0f };
	float pad3{ 0.0f };
};
cbPerObject perObject;

RenderProxy::RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices)
{
	// #TODO: There should be no need for render proxies to have access to the GfxDevice context
	Context* pCtx = GfxDevice::GetContext();

	// Create vertex buffer
	// ********************

	vertBuffer.Create(vertices.size(), sizeof(Vertex), vertices.data());

	// Create an index buffer
	// **********************

	numberOfIndices = (UINT)indices.size();
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * UINT(indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	// fill the index buffer with actual data
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = indices.data();
	pCtx->pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &pIndexBuffer);



	// Create a constant buffer (uniform) for the WVP
	// **********************************************

	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbPerObject);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	pCtx->pDevice->CreateBuffer(&wvpBufferDesc, nullptr, &pWVPBuffer);
	
}

void RenderProxy::Draw()
{
	// #TODO: There should be no need for render proxies to have access to the GfxDevice context
	Context* pCtx = GfxDevice::GetContext();
	
	// Set vertex buffer as active
	vertBuffer.Bind();

	pCtx->pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Matrixf posMat = Matrixf::Translate(pos);
	Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, rot));
	Matrixf scaMat = Matrixf::Scale(sca);
	Matrixf pivotAdjust = Matrixf::Translate(Vec3f(-0.5f, -0.5f, 0.0f));

	Matrixf world = posMat * rotMat * scaMat * pivotAdjust; // transform into world space
	Matrixf view = Matrixf::Translate(Vec3f(0.0f, 0.0f, 0.0f)); // transform into camera space

	Matrixf projection = Matrixf::Orthographic(0.f, pCtx->windowWidth, 0.0f, pCtx->windowHeight, -1.0f, 10.0f); // transform into screen space
	
	Matrixf wvp = projection * view * world;

	perObject.wvp = wvp;
	perObject.lineThickness = lineThickness;
	pCtx->pDeviceContext->UpdateSubresource(pWVPBuffer, 0, nullptr, &perObject, 0, 0);
 	pCtx->pDeviceContext->VSSetConstantBuffers(0, 1, &(pWVPBuffer));
	pCtx->pDeviceContext->GSSetConstantBuffers(0, 1, &(pWVPBuffer));

	// do 3D rendering on the back buffer here
	pCtx->pDeviceContext->DrawIndexed(numberOfIndices, 0, 0);
}
