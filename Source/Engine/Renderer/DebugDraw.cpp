#include "DebugDraw.h"

#include <d3d11.h>
#include <d3d10.h>

#include "Maths/Matrix.h"
#include "Renderer.h"

namespace
{
	struct DrawCall
	{
		int vertexCount{ 0 };
		int indexCount{ 0 };
	};
	std::vector<DrawCall> drawQueue;
	
	struct DebugVertex
	{
		Vec3f pos{ Vec3f(0.0f, 0.0f, 0.0f) };
		Vec3f col{ Vec3f(0.0f, 0.0f, 0.0f) };
	};
	std::vector<DebugVertex> vertBufferData;
	int vertBufferSize = 0;
	std::vector<int> indexBufferData;
	int indexBufferSize = 0;

	GfxDevice::Shader debugShader;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;

	struct cbTransform
	{
		Matrixf wvp;
	};
	ID3D11Buffer* pConstBuffer;
}

void DebugDraw::Draw2DCircle(Vec2f pos, float radius, Vec3f color)
{
	float div = 6.282f / 20.f;
	for (int i = 0; i < 20; i++)
	{
		float x = div * (float)i;
		Vec3f point = Vec3f(radius*cos(x), radius*sin(x), 0.0f) + Vec3f::Embed2D(pos);
		vertBufferData.emplace_back(DebugVertex{ point, color });
		indexBufferData.push_back(i);
	}
	indexBufferData.push_back(0);
	drawQueue.emplace_back(DrawCall{20, 21});
}

void DebugDraw::Draw2DLine(Vec2f start, Vec2f end, Vec3f color)
{
	vertBufferData.emplace_back(DebugVertex{ Vec3f::Embed2D(start), color });
	vertBufferData.emplace_back(DebugVertex{ Vec3f::Embed2D(end), color });
	indexBufferData.push_back(0);
	indexBufferData.push_back(1);

	drawQueue.emplace_back(DrawCall{ 2, 2 });
}

void DebugDraw::Detail::Init()
{
	// #TODO: There should be no need for render proxies to have access to the GfxDevice context
	Context* pCtx = GfxDevice::GetContext();

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

	debugShader = GfxDevice::LoadShaderFromText(shaderSrc, false);
	debugShader.topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

	// Create constant buffer for WVP
	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(cbTransform);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		pCtx->pDevice->CreateBuffer(&desc, NULL, &pConstBuffer);
	}
}

void DebugDraw::Detail::DrawQueue()
{
	// #TODO: There should be no need for render proxies to have access to the GfxDevice context
	Context* pCtx = GfxDevice::GetContext();

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
		pCtx->pDevice->CreateBuffer(&desc, nullptr, &pVertexBuffer);
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
		pCtx->pDevice->CreateBuffer(&desc, nullptr, &pIndexBuffer);
	}

	// Update vert and index buffer data
	D3D11_MAPPED_SUBRESOURCE vertResource, indexResource;
	ZeroMemory(&vertResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	ZeroMemory(&indexResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->pDeviceContext->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertResource);
	pCtx->pDeviceContext->Map(pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indexResource);

	if (!vertBufferData.empty())
		memcpy(vertResource.pData, vertBufferData.data(), vertBufferData.size() * sizeof(DebugVertex));
	if (!indexBufferData.empty())
		memcpy(indexResource.pData, indexBufferData.data(), indexBufferData.size() * sizeof(int));

	pCtx->pDeviceContext->Unmap(pVertexBuffer, 0);
	pCtx->pDeviceContext->Unmap(pIndexBuffer, 0);

	// Update constant buffer data
	D3D11_MAPPED_SUBRESOURCE constResource;
	ZeroMemory(&constResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	pCtx->pDeviceContext->Map(pConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constResource);
	cbTransform* transform = (cbTransform*)constResource.pData;
	Matrixf wvp = Matrixf::Orthographic(0.f, pCtx->windowWidth, 0.0f, pCtx->windowHeight, 0.1f, 10.0f);
	memcpy(&transform->wvp, &wvp, sizeof(wvp));
	pCtx->pDeviceContext->Unmap(pConstBuffer, 0);

	// Bind shaders
	debugShader.Bind();

	pCtx->pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(DebugVertex);
	UINT offset = 0;
	pCtx->pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pCtx->pDeviceContext->VSSetConstantBuffers(0, 1, &pConstBuffer);


	int vertOffset = 0;
	int indexOffset = 0;
	for (DrawCall& draw : drawQueue)
	{
		pCtx->pDeviceContext->DrawIndexed(draw.indexCount, indexOffset, vertOffset);
		vertOffset += draw.vertexCount;
		indexOffset += draw.indexCount;
	}

	drawQueue.clear();
	vertBufferData.clear();
	indexBufferData.clear();
}
