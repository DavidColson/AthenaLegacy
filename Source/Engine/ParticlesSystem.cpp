#include "ParticlesSystem.h"

#include "Scene.h"
#include "Maths/Matrix.h"

REFLECT_BEGIN(CParticleEmitter)
REFLECT_MEMBER(initial_count)
REFLECT_MEMBER(per_frame)
REFLECT_MEMBER(lifetime)
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
		pEmitter->particlePool = std::make_unique<ParticlePool>();

		// Create initial particles
		for (size_t i = 0; i < pEmitter->initial_count; i++)
		{
			Particle* pNewParticle = pEmitter->particlePool->NewParticle();

			Vec3f randomLocation = Vec3f(200.0f + float(rand() % 200), 200.0f + float(rand() % 200), 0.0f);
			Matrixf posMat = Matrixf::Translate(randomLocation);
			Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, 1.0f));
			Matrixf scaMat = Matrixf::Scale(10.0f);
			Matrixf world = posMat * rotMat * scaMat;
			pNewParticle->transform = world;
			pNewParticle->lifeRemaining = pEmitter->lifetime;
		}
		
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
		
		pEmitter->vertBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data(), "Particles Vert Buffer");
		pEmitter->transBuffer = GfxDevice::CreateConstantBuffer(sizeof(ParticlesTransform), "Particles Transform Constant Buffer");
	}
}

void ParticlesSystem::OnFrame(Scene& scene, float deltaTime)
{
	for (EntityID ent : SceneView<CParticleEmitter>(scene))
	{
		CParticleEmitter* pEmitter = scene.Get<CParticleEmitter>(ent);

		// Spawn new particles this frame
		// ******************************
		for (size_t i = 0; i < pEmitter->per_frame; i++)
		{
			Particle* pNewParticle = pEmitter->particlePool->NewParticle();
			if (pNewParticle)
			{
				Vec3f randomLocation = Vec3f(200.0f + float(rand() % 200), 200.0f + float(rand() % 200), 0.0f);
				Matrixf posMat = Matrixf::Translate(randomLocation);
				Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, 1.0f));
				Matrixf scaMat = Matrixf::Scale(10.0f);
				Matrixf world = posMat * rotMat * scaMat;
				pNewParticle->transform = world;
				pNewParticle->lifeRemaining = pEmitter->lifetime;
			}
		}

		// Simulate particles and update transforms
		// ****************************************
		std::vector<Matrixf> particleTransforms;
		for(int i = 0; i < pEmitter->particlePool->currentMaxParticleIndex; i++)
		{
			Particle* pParticle = &(pEmitter->particlePool->pPool[i]);
			if (!pParticle->bIsAlive)
				continue;

			pParticle->lifeRemaining -= deltaTime;
			if (pParticle->lifeRemaining < 0.0f)
			{
				pEmitter->particlePool->KillParticle(pParticle);
				continue;
			}

			particleTransforms.push_back(pParticle->transform);
		}


		// Render particles
		// ****************
		if (particleTransforms.empty())
			return;

		// Update instance data
		if (IsValid(pEmitter->instanceBuffer) || pEmitter->instanceBufferSize < particleTransforms.size())
		{
			pEmitter->instanceBufferSize = (int)particleTransforms.size() + 10;
			pEmitter->instanceBuffer = GfxDevice::CreateDynamicVertexBuffer(particleTransforms.size(), sizeof(Matrixf), "Particles Instance Buffer");
		}
		GfxDevice::UpdateDynamicVertexBuffer(pEmitter->instanceBuffer, particleTransforms.data(), particleTransforms.size() * sizeof(Matrixf));
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
		ParticlesTransform trans{ vp };
		GfxDevice::BindConstantBuffer(pEmitter->transBuffer, &trans, ShaderType::Vertex, 0);

		GfxDevice::DrawInstanced(4, (int)particleTransforms.size(), 0, 0);
	}
}