#include "DebugDraw.h"

#include <d3d11.h>
#include <d3d10.h>

#include "Renderer.h"

namespace
{
	struct DrawCall
	{
		int m_vertexCount{ 0 };
		int m_indexCount{ 0 };
	};
	std::vector<DrawCall> drawQueue;
	
	struct DebugVertex
	{
		vec3 m_pos{ vec3(0.0f, 0.0f, 0.0f) };
		vec3 m_col{ vec3(0.0f, 0.0f, 0.0f) };
	};
	std::vector<DebugVertex> vertBufferData;
	int vertBufferSize = 0;
	std::vector<int> indexBufferData;
	int indexBufferSize = 0;

	Graphics::Shader debugShader;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;

	struct cbTransform
	{
		mat4 m_wvp;
	};
	ID3D11Buffer* pConstBuffer;
}

void DebugDraw::Draw2DCircle(vec2 pos, float radius, vec3 color)
{
	float div = 6.282f / 20.f;
	for (int i = 0; i < 20; i++)
	{
		float x = div * (float)i;
		vec3 point = vec3(radius*cos(x), radius*sin(x), 0.0f) + embed<3>(pos);
		vertBufferData.emplace_back(DebugVertex{ point, color });
		indexBufferData.push_back(i);
	}
	indexBufferData.push_back(0);
	drawQueue.emplace_back(DrawCall{20, 21});
}

void DebugDraw::Draw2DLine(vec2 start, vec2 end, vec3 color)
{
	vertBufferData.emplace_back(DebugVertex{ embed<3>(start), color });
	vertBufferData.emplace_back(DebugVertex{ embed<3>(end), color });
	indexBufferData.push_back(0);
	indexBufferData.push_back(1);

	drawQueue.emplace_back(DrawCall{ 2, 2 });
}

void DebugDraw::Detail::Init()
{
	RenderContext* pCtx = Graphics::GetContext();

	std::string shaderSrc = "\
	cbuffer cbTransform\
	{\
		float4x4 WVP;\
	};\
	struct VS_OUTPUT\
	{\
		float4 Pos : SV_POSITION;\
		float4 Col : COLOR;\
	};\
	VS_OUTPUT VSMain(float4 inPos : POSITION, float4 inCol : COLOR)\
	{\
		VS_OUTPUT output;\
		output.Pos = mul(inPos, WVP);\
		output.Col = inCol;\
		return output;\
	}\
	float4 PSMain(VS_OUTPUT input) : SV_TARGET\
	{\
		return input.Col;\
	}";

	debugShader = Graphics::LoadShaderFromText(shaderSrc, false);

	// Create constant buffer for WVP
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(cbTransform);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		pCtx->m_pDevice->CreateBuffer(&desc, NULL, &pConstBuffer);
	}
}

void DebugDraw::Detail::DrawQueue()
{
	RenderContext* pCtx = Graphics::GetContext();

	if (pVertexBuffer == nullptr || vertBufferSize < vertBufferData.size())
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		vertBufferSize = (int)vertBufferData.size() + 1000;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * UINT(vertBufferSize);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		pCtx->m_pDevice->CreateBuffer(&desc, nullptr, &pVertexBuffer);
	}

	if (pIndexBuffer == nullptr || indexBufferSize < indexBufferData.size())
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		indexBufferSize = (int)indexBufferData.size() + 1000;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(Vertex) * UINT(indexBufferSize);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		pCtx->m_pDevice->CreateBuffer(&desc, nullptr, &pIndexBuffer);
	}

	// Update vert and index buffer data
	D3D11_MAPPED_SUBRESOURCE vertResource, indexResource;
	ZeroMemory(&vertResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&indexResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->m_pDeviceContext->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertResource);
	pCtx->m_pDeviceContext->Map(pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indexResource);

	if (!vertBufferData.empty())
		memcpy(vertResource.pData, vertBufferData.data(), vertBufferData.size() * sizeof(DebugVertex));
	if (!indexBufferData.empty())
		memcpy(indexResource.pData, indexBufferData.data(), indexBufferData.size() * sizeof(int));

	pCtx->m_pDeviceContext->Unmap(pVertexBuffer, 0);
	pCtx->m_pDeviceContext->Unmap(pIndexBuffer, 0);

	// Update constant buffer data
	D3D11_MAPPED_SUBRESOURCE constResource;
	ZeroMemory(&constResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->m_pDeviceContext->Map(pConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constResource);
	cbTransform* transform = (cbTransform*)constResource.pData;
	mat4 wvp = MakeOrthographic(0, pCtx->m_windowWidth / pCtx->m_pixelScale, 0.0f, pCtx->m_windowHeight / pCtx->m_pixelScale, 0.1f, 10.0f);
	memcpy(&transform->m_wvp, &wvp, sizeof(wvp));
	pCtx->m_pDeviceContext->Unmap(pConstBuffer, 0);

	// Bind shaders
	pCtx->m_pDeviceContext->IASetInputLayout(debugShader.m_pVertLayout);
	pCtx->m_pDeviceContext->VSSetShader(debugShader.m_pVertexShader, 0, 0);
	pCtx->m_pDeviceContext->PSSetShader(debugShader.m_pPixelShader, 0, 0);
	pCtx->m_pDeviceContext->GSSetShader(debugShader.m_pGeometryShader, 0, 0);
	pCtx->m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	pCtx->m_pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(DebugVertex);
	UINT offset = 0;
	pCtx->m_pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pCtx->m_pDeviceContext->VSSetConstantBuffers(0, 1, &pConstBuffer);


	int vertOffset = 0;
	int indexOffset = 0;
	for (DrawCall& draw : drawQueue)
	{
		pCtx->m_pDeviceContext->DrawIndexed(draw.m_indexCount, indexOffset, vertOffset);
		vertOffset += draw.m_vertexCount;
		indexOffset += draw.m_indexCount;
	}

	drawQueue.clear();
	vertBufferData.clear();
	indexBufferData.clear();
}
