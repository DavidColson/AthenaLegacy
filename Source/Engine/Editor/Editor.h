#pragma once

struct Space;

namespace Editor
{
	void SetCurrentSpace(Space* pSpace);
	void ShowEditor(bool& shutdown);
}