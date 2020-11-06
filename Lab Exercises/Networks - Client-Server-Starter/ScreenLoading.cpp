#include "Networks.h"


void ScreenLoading::enable()
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		float progressRatio = (float)i / (float)BAR_COUNT;
		float radians = 2.0f * PI * progressRatio;

		loadingBars[i] = new GameObject;
		loadingBars[i]->color[0] = 1.0f;
		loadingBars[i]->color[1] = 1.0f;
		loadingBars[i]->color[2] = 1.0f;
		loadingBars[i]->color[3] = 1.0f;
		loadingBars[i]->pivot_x = 0.5f;
		loadingBars[i]->pivot_y = 0.5f;
		loadingBars[i]->width = 4;
		loadingBars[i]->height = 30;
		loadingBars[i]->x = 30.0f * sinf(radians);
		loadingBars[i]->y = 30.0f * cosf(radians);
		loadingBars[i]->angle = - 360.0f * progressRatio;
	}
}

void ScreenLoading::update()
{
	const float ROUND_TIME = 3.0f;
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		float progressRatio = (float)i / (float)BAR_COUNT;
		auto gameObject = loadingBars[i];
		gameObject->color[3] = 1.0f - fractionalPart(((float)Time.time + progressRatio * ROUND_TIME)/ ROUND_TIME);
	}

	if (App->modResources->finishedLoading)
	{
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);

		// NOTE(jesus): The following is equivalent to the previous line but without transition.
		//this->enabled = false;
		//App->modScene->scenePingPong->enabled = true;
	}
}

void ScreenLoading::disable()
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		loadingBars[i]->deleteFlag = true;
	}
}
