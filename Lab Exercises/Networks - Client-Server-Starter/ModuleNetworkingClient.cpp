#include "ModuleNetworking.h"
#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	// - Create the remote address object
	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_port = htons(serverPort); // Port
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	// - Connect to the remote address
	connect(clientSocket, (const sockaddr*)&serverAddress, sizeof(sockaddr_in));

	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(clientSocket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, clientSocket))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::BeginChild("Chat", ImVec2(400, 750), true);

		for (int i = 0; i < ChatMessages.size(); i++)
		{
			ImGui::Text(ChatMessages[i].c_str());
		}

		
		ImGui::EndChild();

		char newMessage[50] = "";

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("Chat Message", newMessage, sizeof(newMessage), flags))
		{
			std::string message = newMessage;
			if (message.find_first_of("/") != 0)
			{
				OutputMemoryStream packet;
				packet << ClientMessage::Message;
				packet << playerName;
				packet << message;

				ChatMessages.push_back(playerName + ": " + message);
				sendPacket(packet, clientSocket);
			}
		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	if (serverMessage == ServerMessage::Welcome)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::RelayedMessage)
	{
		std::string message;
		packet >> message;
		ChatMessages.push_back(message);
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

