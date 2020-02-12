#include "ParticlesSystem.h"

#include "Scene.h"
#include "Maths/Matrix.h"

REFLECT_BEGIN(CParticleEmitter)
REFLECT_MEMBER(count)
REFLECT_END()

struct ParticlesTransform
{
	Matrixf vp;
};

void ParticlesSystem::OnSceneStart(Scene& scene)
{
	// loop through particle emitters in the scene, initializing their data correctly
	for (EntityID ent : SceneView<CParticleEmitter>(scene))
	{
		CParticleEmitter* pEmitter = scene.Get<CParticleEmitter>(ent);

		// @TODO: debug names should be derivative of the entity name

		std::vector<VertexInputElement> particleLayout;
		particleLayout.push_back({"POSITION", AttributeType::Float3, 0 });
		particleLayout.push_back({"COLOR", AttributeType::Float3, 0 });
		particleLayout.push_back({"TEXCOORD", AttributeType::Float2, 0 });
		particleLayout.push_back({"INSTANCE_TRANSFORM", AttributeType::InstanceTransform, 1 });

		VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/Particles.hlsl", "VSMain", particleLayout, "Particles");
		PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/Particles.hlsl", "PSMain", "Particles");
		pEmitter->shaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

		std::vector<Vertex> quadVertices = {
		  Vertex(Vec3f(-1.0f, -1.0f, 0.5f)),
		  Vertex(Vec3f(-1.f, 1.f, 0.5f)),
		  Vertex(Vec3f(1.f, -1.f, 0.5f)),
		  Vertex(Vec3f(1.f, 1.f, 0.5f))
		};
		quadVertices[0].texCoords = Vec2f(0.0f, 1.0f);
		quadVertices[1].texCoords = Vec2f(0.0f, 0.0f);
		quadVertices[2].texCoords = Vec2f(1.0f, 1.0f);
		quadVertices[3].texCoords = Vec2f(1.0f, 0.0f);

		std::vector<Matrixf> particleTransforms;
		for(int i = 0; i < 50; i++)
		{
			Matrixf posMat = Matrixf::Translate(Vec3f(200.0f + 50.0f * i, 200.0f, 0.0f));
			Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, 1.0f));
			Matrixf scaMat = Matrixf::Scale(10.0f);
			Matrixf world = posMat * rotMat * scaMat;
			particleTransforms.push_back(world);
		}
		
		pEmitter->vertBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data(), "Particles Vert Buffer");
		pEmitter->instanceBuffer = GfxDevice::CreateVertexBuffer(particleTransforms.size(), sizeof(Matrixf), particleTransforms.data(), "Particles Instance Buffer");
		pEmitter->transBuffer = GfxDevice::CreateConstantBuffer(sizeof(ParticlesTransform), "Particles Transform Constant Buffer");
	}
}

void ParticlesSystem::OnFrame(Scene& scene)
{
	// simulate and render
	for (EntityID ent : SceneView<CParticleEmitter>(scene))
	{
		CParticleEmitter* pEmitter = scene.Get<CParticleEmitter>(ent);

		GfxDevice::BindProgram(pEmitter->shaderProgram);
		GfxDevice::SetTopologyType(TopologyType::TriangleStrip);

		// Set vertex buffer as active
		// Binding two buffers Here
		VertexBufferHandle buffers[2];
		buffers[0] = pEmitter->vertBuffer;
		buffers[1] = pEmitter->instanceBuffer;
		GfxDevice::BindVertexBuffers(2, buffers);

		Matrixf view = Matrixf::Translate(Vec3f(0.0f, 0.0f, 0.0f));
		Matrixf projection = Matrixf::Orthographic(0.f, GfxDevice::GetWindowWidth(), 0.0f, GfxDevice::GetWindowHeight(), -1.0f, 10.0f);
		Matrixf vp = projection * view;

		// TODO: Consider using a flag here for picking a shader, so we don't have to do memcpy twice
		ParticlesTransform trans{ vp };
		GfxDevice::BindConstantBuffer(pEmitter->transBuffer, &trans, ShaderType::Vertex, 0);

		GfxDevice::DrawInstanced(4, pEmitter->count, 0, 0);
	}
}