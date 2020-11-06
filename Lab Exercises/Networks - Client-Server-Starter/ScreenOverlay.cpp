#include "Networks.h"


void ScreenOverlay::enable()
{
	ASSERT(oldScene != nullptr && newScene != nullptr);
	oldScene->shouldUpdate = false;
	newScene->shouldUpdate = false;

	overlay = new GameObject();
	//overlay->color[0] = 0.0f;
	//overlay->color[1] = 0.0f;
	//overlay->color[2] = 0.0f;
	//overlay->color[3] = 1.0f;
	overlay->texture = App->modResources->background;
	overlay->order = 9999;
	//overlay->scene = this;

	transitionTimeElapsed = 0.0f;
}

void ScreenOverlay::update()
{
	ASSERT(oldScene != nullptr && newScene != nullptr);

	overlay->width = (float)Window.width;
	overlay->height = (float)Window.height;

	transitionTimeElapsed += Time.deltaTime;

	const float halfTransitionTime = transitionTimeMax * 0.5f;

	if (transitionTimeElapsed < halfTransitionTime)
	{
		overlay->color[3] = transitionTimeElapsed / halfTransitionTime;
	}
	else
	{
		oldScene->enabled = false;
		newScene->enabled = true;

		overlay->color[3] = 1.0f - (transitionTimeElapsed - halfTransitionTime) / halfTransitionTime;
		if (overlay->color[3] < 0.0f) { overlay->color[3] = 0.0f; }
	}

	if (transitionTimeElapsed > transitionTimeMax)
	{
		enabled = false;
		oldScene->shouldUpdate = true;
		newScene->shouldUpdate = true;
		oldScene = nullptr;
		newScene = nullptr;
	}
}

void ScreenOverlay::disable()
{
	overlay->deleteFlag = true;
}
