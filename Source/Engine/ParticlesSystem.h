#pragma once

#include "Maths/Vec2.h"
#include "GraphicsDevice/GraphicsDevice.h"

struct Scene;

struct CParticleEmitter
{
	int count{ 0 };

	ProgramHandle shaderProgram;
  	VertexBufferHandle vertBuffer;
  	VertexBufferHandle instanceBuffer;
  	ConstBufferHandle transBuffer;

	REFLECT()
};

namespace ParticlesSystem
{
	void OnSceneStart(Scene& scene);

	void OnFrame(Scene& scene);
}