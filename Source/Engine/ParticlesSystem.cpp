#include "ParticlesSystem.h"

#include "Scene.h"
#include "Maths/Matrix.h"
#include "Maths/Maths.h"

REFLECT_BEGIN(CParticleEmitter)
REFLECT_MEMBER(looping)
REFLECT_MEMBER(lifetime)
REFLECT_MEMBER(initialCount)
REFLECT_MEMBER(initialVelocityMin)
REFLECT_MEMBER(initialVelocityMax)
REFLECT_MEMBER(initialRotationMin)
REFLECT_MEMBER(initialRotationMax)
REFLECT_MEMBER(initialScaleMin)
REFLECT_MEMBER(initialScaleMax)
REFLECT_END()

struct ParticlesTransform
{
	Matrixf vp;
};

void RestartEmitter(CParticleEmitter& emitter, CTransform& emitterTransform)
{
	// Create initial particles
	for (size_t i = 0; i < emitter.initialCount; i++)
	{
		Particle* pNewParticle = emitter.particlePool->NewParticle();
		pNewParticle->lifeRemaining = emitter.lifetime;

		auto randf = []() { return float(rand()) / float(RAND_MAX); };

		pNewParticle->position = Vec2f::Project3D(emitterTransform.pos);
		pNewParticle->rotation = LinearMap(randf(), 0.0f, 1.0f, emitter.initialRotationMin, emitter.initialRotationMax);
		pNewParticle->scale = Vec2f(5.0f, 5.0f);

		Vec2f initialVelocity;
		initialVelocity.x = LinearMap(randf(), 0.0f, 1.0f, emitter.initialVelocityMin.x, emitter.initialVelocityMax.x);
		initialVelocity.y = LinearMap(randf(), 0.0f, 1.0f, emitter.initialVelocityMin.y, emitter.initialVelocityMax.y);

		float scale = LinearMap(randf(), 0.0f, 1.0f, emitter.initialScaleMin, emitter.initialScaleMax); 
		pNewParticle->scale = Vec2f(scale);
		pNewParticle->velocity = initialVelocity;
	}
}

void ParticlesSystem::OnAddEmitter(Scene& scene, EntityID entity)
{
	CParticleEmitter& emitter = *(scene.Get<CParticleEmitter>(entity));

	// @TODO: debug names should be derivative of the entity name
	std::vector<VertexInputElement> particleLayout;
	particleLayout.push_back({"POSITION", AttributeType::Float3, 0 });
	particleLayout.push_back({"COLOR", AttributeType::Float3, 0 });
	particleLayout.push_back({"TEXCOORD", AttributeType::Float2, 0 });
	particleLayout.push_back({"INSTANCE_TRANSFORM", AttributeType::InstanceTransform, 1 });

	VertexShaderHandle vertShader = GfxDevice::CreateVertexShader(L"Shaders/Particles.hlsl", "VSMain", particleLayout, "Particles");
	PixelShaderHandle pixShader = GfxDevice::CreatePixelShader(L"Shaders/Particles.hlsl", "PSMain", "Particles");
	emitter.shaderProgram = GfxDevice::CreateProgram(vertShader, pixShader);

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
	
	emitter.vertBuffer = GfxDevice::CreateVertexBuffer(quadVertices.size(), sizeof(Vertex), quadVertices.data(), "Particles Vert Buffer");
	emitter.transBuffer = GfxDevice::CreateConstantBuffer(sizeof(ParticlesTransform), "Particles Transform Constant Buffer");

	emitter.particlePool = std::make_unique<ParticlePool>();
	RestartEmitter(emitter, *(scene.Get<CTransform>(entity)));
}

void ParticlesSystem::OnFrame(Scene& scene, float deltaTime)
{
	for (EntityID ent : SceneView<CParticleEmitter, CTransform>(scene))
	{
		CParticleEmitter* pEmitter = scene.Get<CParticleEmitter>(ent);
		CTransform* pTrans = scene.Get<CTransform>(ent);

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

			pParticle->position += pParticle->velocity * deltaTime;

			Matrixf posMat = Matrixf::Translate(Vec3f::Embed2D(pParticle->position));
			Matrixf rotMat = Matrixf::Rotate(Vec3f(0.0f, 0.0f, pParticle->rotation));
			Matrixf scaMat = Matrixf::Scale(Vec3f::Embed2D(pParticle->scale));
			Matrixf world = posMat * rotMat * scaMat;
			particleTransforms.push_back(world);
		}

		// If a looping emitter, and all particles are dead, reset it
		// **********************************************************
		if (particleTransforms.empty())
		{
			if (pEmitter->looping == true)
				RestartEmitter(*pEmitter, *pTrans);
			else if (pEmitter->destroyEntityOnEnd == true)
				scene.DestroyEntity(ent);

			continue;
		}

		// Render particles
		// ****************

		// Update instance data
		if (!IsValid(pEmitter->instanceBuffer) || pEmitter->instanceBufferSize < particleTransforms.size())
		{
			// TODO: release buffer before creating
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