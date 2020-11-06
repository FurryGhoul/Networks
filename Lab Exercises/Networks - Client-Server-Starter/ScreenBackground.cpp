#include "Networks.h"


void ScreenBackground::enable()
{
	background = new GameObject;
	background->texture = App->modResources->background;
	background->width = 1920;
	background->height = 1080;
	background->order = -1;
	//background->scene = this;
}

void ScreenBackground::update()
{
	background->width = (float)Window.width;
	background->height = (float)Window.height;
}

void ScreenBackground::disable()
{
	background->deleteFlag = true;
}
