#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	// - Set address reuse
	int enable = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int)) == SOCKET_ERROR)
	{
		LOG("Error while binding socket");
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET; 
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	// - Bind the socket to a local interface
	int res = bind(listenSocket, (const struct sockaddr*)&address, sizeof(address));

	// - Enter in listen mode
	listen(listenSocket, SOMAXCONN);

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	// Set the player name of the corresponding connected socket proxy
	ClientMessage clientMessage;
	packet >> clientMessage;

	if (clientMessage == ClientMessage::Hello)
	{
		//If the received message is a client saying hello, welcome them and get their username
		std::string playerName;
		packet >> playerName;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.playerName = playerName;

				std::string welcomeMessage = "Welcome, " + playerName + "!\nType /help to see a list of available commands.";

				OutputMemoryStream welcomeStream;
				welcomeStream << ServerMessage::Welcome;
				welcomeStream << welcomeMessage;
				sendPacket(welcomeStream, socket);
			}

			//Also notify all other users that a new user has joined, and give them his/her username
			else if (connectedSocket.socket != socket)
			{
				std::string joinedMessage = playerName + " has joined!";

				OutputMemoryStream joinedStream;
				joinedStream << ServerMessage::Joined;
				joinedStream << joinedMessage;
				sendPacket(joinedStream, connectedSocket.socket);
			}
		}
	}

	else if (clientMessage == ClientMessage::Message)
	{
		//If the received message is a text from a client, add the sender's name and relay it to every other client
		std::string message;
		std::string senderName;
		packet >> senderName;
		packet >> message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket) //Find sending socket
			{
				if (connectedSocket.muted) //Respond to them if they're muted
				{
					std::string youreMutedMessage = "You're muted! No one will see that.";
					OutputMemoryStream relayPacket;
					relayPacket << ServerMessage::RelayedMessage;
					relayPacket << youreMutedMessage;
					sendPacket(relayPacket, socket);
				}

				else if (!connectedSocket.muted) //If they aren't, relay the message to everyone
				{
					for (auto& connectedSocket : connectedSockets)
					{
						if (connectedSocket.socket != socket)
						{
							std::string newChatMessage = senderName + ": " + message;
							OutputMemoryStream relayPacket;
							relayPacket << ServerMessage::RelayedMessage;
							relayPacket << newChatMessage;

							sendPacket(relayPacket, connectedSocket.socket);
						}
					}
				}

			}
		}
		
	}

	else if (clientMessage == ClientMessage::List)
	{
		//If the received message is asking for a list, add all usernames to a string and send it back to that client
		std::string playerList = "Current users in the chat:";
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket != socket)
				playerList += "\n" + connectedSocket.playerName;

			else
				playerList += "\n" + connectedSocket.playerName + " (that's you!)";
		}

		OutputMemoryStream listPacket;
		listPacket << ServerMessage::List;
		listPacket << playerList;

		sendPacket(listPacket, socket);
	}

	else if (clientMessage == ClientMessage::Kick)
	{
		std::string userToKick;
		packet >> userToKick;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == userToKick)
			{
				//TODO: Kick user (disconnect socket)
				OutputMemoryStream kickPacket;
				kickPacket << ServerMessage::Kick;
				sendPacket(kickPacket, connectedSocket.socket);
			}
			else
			{
				std::string kickMessage = userToKick + " has been kicked ";
				OutputMemoryStream kickPacket;
				kickPacket << ServerMessage::RelayedMessage;
				kickPacket << kickMessage;
				sendPacket(kickPacket, connectedSocket.socket);
			}
		}
	}

	else if (clientMessage == ClientMessage::Whisper)
	{
		std::string userToWhisper;
		std::string messageToWhisper;
		std::string senderName;
		packet >> userToWhisper;
		packet >> messageToWhisper;
		packet >> senderName;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == userToWhisper)
			{
				std::string whisperedMessage = senderName + " whispers: " + messageToWhisper;
				OutputMemoryStream whisperPacket;
				whisperPacket << ServerMessage::RelayedWhisper;
				whisperPacket << whisperedMessage;

				sendPacket(whisperPacket, connectedSocket.socket);
			}
		}
	}

	else if (clientMessage == ClientMessage::ChangeName)
	{
		std::string newName;
		std::string senderName;
		packet >> newName;
		packet >> senderName;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.playerName = newName;

				std::string changeNameMessage = "You've changed your name from " + senderName + " to " + newName + ".";
				OutputMemoryStream changeNamePacket;
				changeNamePacket << ServerMessage::ChangeName;
				changeNamePacket << changeNameMessage;

				sendPacket(changeNamePacket, socket);
			}

			else if (connectedSocket.socket != socket)
			{
				std::string changeNameMessage = senderName + " has changed their name to " + newName + ".";
				OutputMemoryStream changeNamePacket;
				changeNamePacket << ServerMessage::ChangeName;
				changeNamePacket << changeNameMessage;

				sendPacket(changeNamePacket, connectedSocket.socket);
			}
		}
	}

	else if (clientMessage == ClientMessage::KK)
	{
		std::string kkName;
		packet >> kkName;

		kkName += " activates their bubblegum!";

		for (auto& connectedSocket : connectedSockets)
		{
			OutputMemoryStream KKPacket;
			KKPacket << ServerMessage::KK;
			KKPacket << kkName;

			sendPacket(KKPacket, connectedSocket.socket);
		}
	}

	else if (clientMessage == ClientMessage::Mute)
	{
		std::string userToMute;
		std::string senderName;
		bool muted;
		packet >> userToMute;
		packet >> senderName;
		packet >> muted;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == userToMute)
			{
				connectedSocket.muted = muted;
				std::string mutedMessage;
				if (muted)
					mutedMessage = "You have been muted by " + senderName;
				else if (!muted)
					mutedMessage = "You have been unmuted by " + senderName;
				OutputMemoryStream mutedPacket;
				mutedPacket << ServerMessage::Mute;
				mutedPacket << mutedMessage;

				sendPacket(mutedPacket, connectedSocket.socket);
			}
		}
	}

	else if (clientMessage == ClientMessage::Roulette)
	{
		std::string senderName;
		packet >> senderName;
		//Set a random number from 0 to the max number of users and iterate up until that number
		int selected = rand() % (connectedSockets.size());
		std::string selectedUserMessage = senderName + " spun the roulette, and " + connectedSockets.at(selected).playerName + " won!";

		for (auto& connectedSocket : connectedSockets)
		{
			OutputMemoryStream roulettePacket;
			roulettePacket << ServerMessage::Roulette;
			roulettePacket << selectedUserMessage;

			sendPacket(roulettePacket, connectedSocket.socket);
		}
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

