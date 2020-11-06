#pragma once

#pragma comment(lib, "Ws2_32.lib")

#pragma comment (lib, "D3D11.lib")


////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) if ((x) == false) { *(int*)0 = 0; }

#ifdef PI
#undef PI
#endif
#define PI 3.14159265359f

#define Kilobytes(x) (1024L * x)
#define Megabytes(x) (1024L * Kilobytes(x))
#define Gigabytes(x) (1024L * Megabytes(x))
#define Terabytes(x) (1024L * Gigabytes(x))


////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////

#define MAX_SCREENS           32
#define MAX_TASKS            128
#define MAX_TEXTURES         512
#define MAX_GAME_OBJECTS    2048
#define MAX_CLIENTS            4

#define SCENE_TRANSITION_TIME      1.0f
#define PACKET_SIZE        Kilobytes(4)


////////////////////////////////////////////////////////////////////////
// BASIC TYPES
////////////////////////////////////////////////////////////////////////

// NOTE(jesus): These sizes are right for most platforms, but we should
// should be cautious about this because they could vary somewhere...

typedef char int8;
typedef short int int16;
typedef long int int32;
typedef long long int int64;

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef float real32;
typedef double real64;


////////////////////////////////////////////////////////////////////////
// FRAMEWORK TYPES
////////////////////////////////////////////////////////////////////////

struct GameObject;
struct Texture;

class Task;


////////////////////////////////////////////////////////////////////////
// WINDOW
////////////////////////////////////////////////////////////////////////

struct WindowStruct
{
	int width = 0;
	int height = 0;
};

// NOTE(jesus): Global objet to access window dimensions
extern WindowStruct Window;


////////////////////////////////////////////////////////////////////////
// TIME
////////////////////////////////////////////////////////////////////////

struct TimeStruct
{
	double time = 0.0f;      // NOTE(jesus): Time in seconds since the application started
	float deltaTime = 0.0f; // NOTE(jesus): Fixed update time step (use this for calculations)
	float frameTime = 0.0f; // NOTE(jesus): Time spend during the last frame
};

// NOTE(jesus): Global object to access the time
extern TimeStruct Time;


////////////////////////////////////////////////////////////////////////
// INPUT
////////////////////////////////////////////////////////////////////////

enum ButtonState { Idle, Press, Pressed, Release };

struct InputController
{
	bool isConnected = false;

	float verticalAxis = 0.0f;
	float horizontalAxis = 0.0f;

	union
	{
		ButtonState buttons[8] = {};
		struct
		{
			ButtonState actionUp;
			ButtonState actionDown;
			ButtonState actionLeft;
			ButtonState actionRight;
			ButtonState leftShoulder;
			ButtonState rightShoulder;
			ButtonState back;
			ButtonState start;
		};
	};
};

struct MouseController
{
	int16 x = 0;
	int16 y = 0;
	ButtonState buttons[5] = {};
};

// NOTE(jesus): Global object to access the input controller
extern InputController Input;

// NOTE(jesus): Global object to access the mouse
extern MouseController Mouse;


////////////////////////////////////////////////////////////////////////
// LOG
////////////////////////////////////////////////////////////////////////

// NOTE(jesus):
// Use log just like standard printf function.
// Example: LOG("New user connected %s\n", usernameString);
enum { LOG_TYPE_INFO, LOG_TYPE_WARN, LOG_TYPE_ERROR, LOG_TYPE_DEBUG };
#define LOG(format, ...)  log(__FILE__, __LINE__, LOG_TYPE_INFO,  format, __VA_ARGS__)
#define WLOG(format, ...) log(__FILE__, __LINE__, LOG_TYPE_WARN,  format, __VA_ARGS__)
#define ELOG(format, ...) log(__FILE__, __LINE__, LOG_TYPE_ERROR, format, __VA_ARGS__)
#define DLOG(format, ...) log(__FILE__, __LINE__, LOG_TYPE_DEBUG, format, __VA_ARGS__)
void log(const char file[], int line, int type, const char* format, ...);
uint32 getLogEntryCount();
struct LogEntry {
	int type;
	const char *message;
};
LogEntry getLogEntry(uint32 entryIndex);


////////////////////////////////////////////////////////////////////////
// MATH
////////////////////////////////////////////////////////////////////////

inline float radiansFromDegrees(float degrees)
{
	const float radians = PI * degrees / 180.0f;
	return radians;
}

inline float fractionalPart(float number)
{
	const float f = number - (int)number;
	return f;
}


////////////////////////////////////////////////////////////////////////
// FRAMEWORK HEADERS
////////////////////////////////////////////////////////////////////////

#include "Module.h"
#include "ModuleNetworking.h"
#include "ModuleNetworkingClient.h"
#include "ModuleNetworkingServer.h"
#include "ModuleGameObject.h"
#include "ModulePlatform.h"
#include "ModuleRender.h"
#include "ModuleResources.h"
#include "ModuleScreen.h"
#include "ModuleTaskManager.h"
#include "ModuleTextures.h"
#include "ModuleUI.h"
#include "Screen.h"
#include "ScreenLoading.h"
#include "ScreenBackground.h"
#include "ScreenOverlay.h"
#include "ScreenMainMenu.h"
#include "ScreenGame.h"
#include "Application.h"
