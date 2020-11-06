#include "Networks.h"

void ScreenGame::enable()
{
	if (isServer)
	{
		if (!App->modNetServer->start(serverPort))
		{
			WLOG("Could not start ModuleNetworkingServer");
		}
	}
	else
	{
		if (!App->modNetClient->start(serverAddress, serverPort, playerName))
		{
			WLOG("Could not start ModuleNetworkingClient");
		}
	}
}

void ScreenGame::update()
{
	if (isServer)
	{
		if (!App->modNetServer->isRunning())
		{
			App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);
		}
	}
	else
	{
		if (!App->modNetClient->isRunning())
		{
			App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);
		}
	}
}

void ScreenGame::gui()
{
}

void ScreenGame::disable()
{
}
