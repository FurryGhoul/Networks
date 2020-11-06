#include "Networks.h"
#include "ScreenMainMenu.h"

void ScreenMainMenu::enable()
{
	LOG("Example Info log entry...");
	DLOG("Example Debug log entry...");
	WLOG("Example Warning log entry...");
	ELOG("Example Error log entry...");
}

void ScreenMainMenu::gui()
{
	ImGui::Begin("Main Menu");
	
	Texture *banner = App->modResources->banner;
	ImVec2 bannerSize(400.0f, 400.0f * banner->height / banner->width);
	ImGui::Image(banner->shaderResource, bannerSize);

	ImGui::Spacing();

	ImGui::Text("Server");

	static int localServerPort = 8888;
	ImGui::InputInt("Local server port", &localServerPort);

	if (ImGui::Button("Start server"))
	{
		App->modScreen->screenGame->isServer = true;
		App->modScreen->screenGame->serverPort = localServerPort;
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Client");

	static char serverAddressStr[64] = "127.0.0.1";
	ImGui::InputText("Server address", serverAddressStr, sizeof(serverAddressStr));

	static int remoteServerPort = 8888;
	ImGui::InputInt("Remote server port", &remoteServerPort);

	static char playerNameStr[64] = "playername";
	ImGui::InputText("Player name", playerNameStr, sizeof(playerNameStr));

	if (ImGui::Button("Connect to server"))
	{
		App->modScreen->screenGame->isServer = false;
		App->modScreen->screenGame->serverPort = remoteServerPort;
		App->modScreen->screenGame->serverAddress = serverAddressStr;
		App->modScreen->screenGame->playerName = playerNameStr;
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenGame);
	}

	ImGui::End();
}
