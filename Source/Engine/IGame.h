#pragma once

struct IGame
{
	virtual void OnStart() = 0;
	virtual void OnFrame(float deltaTime) = 0;
	virtual void OnEnd() = 0;
};

