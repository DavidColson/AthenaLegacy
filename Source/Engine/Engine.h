#pragma once

struct IGame;

namespace Engine
{
	void Startup(IGame* pGame);
	void Run();
	void Shutdown();
}