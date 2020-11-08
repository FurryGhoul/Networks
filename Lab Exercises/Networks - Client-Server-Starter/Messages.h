#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Message,
	List,
	Kick,
	Whisper,
	ChangeName,
	Mute,
	KK,
	Roulette
};

enum class ServerMessage
{
	Welcome,
	Joined,
	RelayedMessage,
	List,
	Kick,
	RelayedWhisper,
	ChangeName,
	Mute,
	KK,
	Roulette
};

