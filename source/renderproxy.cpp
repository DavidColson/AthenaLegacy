
#include "renderproxy.h"

#include <DirectXPackedVector.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3d10.h>
#include <D3DCompiler.h>

#include "renderer.h"
#include "maths/maths.h"

struct cbPerObject
{
	mat4 wvp;
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

	gRenderer.m_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertBuffer);

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
	gRenderer.m_device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);



	// Create a constant buffer (uniform) for the WVP
	// **********************************************

	D3D11_BUFFER_DESC wvpBufferDesc;
	ZeroMemory(&wvpBufferDesc, sizeof(wvpBufferDesc));
	wvpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	wvpBufferDesc.ByteWidth = sizeof(cbPerObject);
	wvpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	wvpBufferDesc.CPUAccessFlags = 0;
	wvpBufferDesc.MiscFlags = 0;
	gRenderer.m_device->CreateBuffer(&wvpBufferDesc, nullptr, &m_wvpBuffer);
	
}

void RenderProxy::Draw()
{
	// Set vertex buffer as active
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	gRenderer.m_device_context->IASetVertexBuffers(0, 1, &m_vertBuffer, &stride, &offset);

	gRenderer.m_device_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	static float rotation = 0.0f;
	rotation += 0.5f;

	mat4 rotmat = MakeRotate(vec3(0.0f, 0.0f, rotation));

	mat4 world = rotmat; // transform into world space
	mat4 view = MakeTranslate(vec3(0.0f, 0.0f, 0.0f)); // transform into camera space

	float aR = gRenderer.m_width / gRenderer.m_height;
	mat4 projection = MakeOrthographic(-1.0f * aR, 1.0f * aR, -1.0f, 1.0f, 0.1f, 10.0f); // transform into screen space

	mat4 wvp = projection * view * world;

	perObject.wvp = wvp;
	gRenderer.m_device_context->UpdateSubresource(m_wvpBuffer, 0, nullptr, &perObject, 0, 0);
	gRenderer.m_device_context->VSSetConstantBuffers(0, 1, &(m_wvpBuffer));

	// do 3D rendering on the back buffer here
	gRenderer.m_device_context->DrawIndexed(3, 0, 0);
}
