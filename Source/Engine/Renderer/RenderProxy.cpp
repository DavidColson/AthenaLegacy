
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
	Matrixf m_wvp;
	float m_lineThickness;
	float pad1{ 0.0f };
	float pad2{ 0.0f };
	float pad3{ 0.0f };
};
cbPerObject perObject;

RenderProxy::RenderProxy(std::vector<Vertex> vertices, std::vector<int> indices)
	: m_vertices(vertices), m_indices(indices)
{
	// Create vertex buffer
	// ********************

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * UINT(m_vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// fill the buffer with actual data
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = m_vertices.data();

	Graphics::GetContext()->m_pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_pVertBuffer);

	// Create an index buffer
	// **********************

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * UINT(m_indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	// fill the index buffer with actual data
	D3D11_SUBRESOURCE_DATA indexBufferData;
	ZeroMemory(&indexBufferData, sizeof(indexBufferData));
	indexBufferData.pSysMem = m_indices.data();
	Graphics::GetContext()->m_pDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_pIndexBuffer);



	// Create a constant buffer (uniform) for the WVP
	// **********************************************

	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbPerObject);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	Graphics::GetContext()->m_pDevice->CreateBuffer(&wvpBufferDesc, nullptr, &m_pWVPBuffer);
	
}

void RenderProxy::Draw()
{
	// Set vertex buffer as active
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::GetContext()->m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertBuffer, &stride, &offset);

	Graphics::GetContext()->m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Matrixf posMat = Matrixf::Translate(m_pos);
	Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, m_rot));
	Matrixf scaMat = Matrixf::Scale(m_sca);
	Matrixf pivotAdjust = Matrixf::Translate(Vec3f(-0.5f, -0.5f, 0.0f));

	Matrixf world = posMat * rotMat * scaMat * pivotAdjust; // transform into world space
	Matrixf view = Matrixf::Translate(Vec3f(0.0f, 0.0f, 0.0f)); // transform into camera space

	Matrixf projection = Matrixf::Orthographic(0.f, Graphics::GetContext()->m_windowWidth / Graphics::GetContext()->m_pixelScale, 0.0f, Graphics::GetContext()->m_windowHeight / Graphics::GetContext()->m_pixelScale, -1.0f, 10.0f); // transform into screen space
	
	Matrixf wvp = projection * view * world;

	perObject.m_wvp = wvp;
	perObject.m_lineThickness = m_lineThickness;
	Graphics::GetContext()->m_pDeviceContext->UpdateSubresource(m_pWVPBuffer, 0, nullptr, &perObject, 0, 0);
 	Graphics::GetContext()->m_pDeviceContext->VSSetConstantBuffers(0, 1, &(m_pWVPBuffer));
	Graphics::GetContext()->m_pDeviceContext->GSSetConstantBuffers(0, 1, &(m_pWVPBuffer));

	// do 3D rendering on the back buffer here
	Graphics::GetContext()->m_pDeviceContext->DrawIndexed((UINT)m_indices.size(), 0, 0);
}
