#include <Windows.h>
#include <stdlib.h>
#include <shellapi.h>
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
		//Send a Hello message to the server just upon joining
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

		ImGui::BeginChild("Chat", ImVec2(400, 450), true);

		//Print all chat messages
		for (int i = 0; i < ChatMessages.size(); i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(r, g, b, 1.0f));
			ImGui::Text(ChatMessages[i].c_str());
			ImGui::PopStyleColor();
		}

		
		ImGui::EndChild();

		char newMessage[50] = "";

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		if (ImGui::InputText("Chat Message", newMessage, sizeof(newMessage), flags))
		{
			r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			//If the inputted text doesn't have a / in pos 0, send it as a Message
			std::string message = newMessage;
			if (message.find_first_of("/") != 0)
			{
				OutputMemoryStream packet;
				packet << ClientMessage::Message;
				packet << playerName;
				packet << message;

				packet << r;
				packet << g;
				packet << b;

				//Add the sent text to the local chat log and then send it to the server to relay to other clients
				ChatMessages.push_back(playerName + ": " + message);
				sendPacket(packet, clientSocket);
			}

			//If the text has a / in pos 0, check which command it is
			else if (message.find("/") == 0)
			{
				if (message.find("/help") == 0)
				{
					std::string commandList = "Available commands:\n'/list': Displays all connected users.\n'/kick username': Kicks a specific user from the chat.\n'/whisper username msg': Sends a message to a specific user.\n'/change_name newname': Changes your username to a new one.\n'/clear': Deletes all messages on the chat window, but just for you.";
					ChatMessages.push_back(commandList);
				}

				else if (message.find("/list") == 0)
				{
					OutputMemoryStream packet;
					packet << ClientMessage::List;

					sendPacket(packet, clientSocket);
				}

				else if (message.find("/kick") == 0)
				{
					size_t pos1 = message.find_first_of(" ");
					std::string kickMessage = message.substr(pos1 + 1);

					//std::string userToKick = message;
					OutputMemoryStream packet;
					packet << ClientMessage::Kick;
					packet << kickMessage;

					sendPacket(packet, clientSocket);
				}

				else if (message.find("/whisper") == 0)
				{
					size_t pos1 = message.find_first_of(" ");
					size_t pos2 = message.find(" ", pos1 + 1);
					std::string userToWhisper = message.substr(pos1+1, pos2-pos1-1);
					std::string messageToWhisper = message.substr(pos2+1);

					OutputMemoryStream packet;
					packet << ClientMessage::Whisper;
					packet << userToWhisper;
					packet << messageToWhisper;
					packet << playerName;

					ChatMessages.push_back(playerName + " (whispered to " + userToWhisper + "): " + messageToWhisper);

					sendPacket(packet, clientSocket);
				}

				else if (message.find("/change_name") == 0)
				{
					size_t pos1 = message.find_first_of(" ");
					size_t pos2 = message.find(" ", pos1 + 1);

					std::string newName = message.substr(pos1 + 1, pos2 - pos1 - 1); //Ignore all text after the second space

					OutputMemoryStream packet;
					packet << ClientMessage::ChangeName;
					packet << newName;
					packet << playerName;

					playerName = newName;
					sendPacket(packet, clientSocket);
				}

				else if (message.find("/clear") == 0)
				{
					ChatMessages.clear();
				}

				else if (message.find("/kk") == 0)
				{
					OutputMemoryStream packet;
					packet << ClientMessage::KK;
					packet << playerName;

					sendPacket(packet, clientSocket);
				}

				else if (message.find("/mute") == 0)
				{
					size_t pos1 = message.find_first_of(" ");
					size_t pos2 = message.find(" ", pos1 + 1);

					std::string userToMute = message.substr(pos1 + 1, pos2 - pos1 - 1); //Ignore all text after the second space

					OutputMemoryStream packet;
					packet << ClientMessage::Mute;
					packet << userToMute;
					packet << playerName;
					packet << true;

					sendPacket(packet, clientSocket);
				}

				else if (message.find("/unmute") == 0)
				{
					size_t pos1 = message.find_first_of(" ");
					size_t pos2 = message.find(" ", pos1 + 1);

					std::string userToMute = message.substr(pos1 + 1, pos2 - pos1 - 1); //Ignore all text after the second space

					OutputMemoryStream packet;
					packet << ClientMessage::Mute;
					packet << userToMute;
					packet << playerName;
					packet << false;

					sendPacket(packet, clientSocket);
				}
			}
		}
		ImGui::PopStyleColor();
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	// Server Welcome messages are added to the chat log as a regular message, but it's separated for potential future use
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

	else if (serverMessage == ServerMessage::List)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::Joined)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::RelayedWhisper)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::ChangeName)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::KK)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);

		ShellExecute(0, 0, "https://www.youtube.com/watch?v=022CdArz5oM", 0, 0, SW_SHOW);
	}

	else if (serverMessage == ServerMessage::Mute)
	{
		std::string message;
		packet >> message;

		ChatMessages.push_back(message);
	}

	else if (serverMessage == ServerMessage::Kick)
	{
		kickUser();
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::kickUser()
{
	disconnect(); 
	state = ClientState::Stopped;
	ChatMessages.clear();
}
