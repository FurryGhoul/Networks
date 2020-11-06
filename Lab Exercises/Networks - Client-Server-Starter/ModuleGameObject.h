#pragma once

struct Texture;
class Screen;

struct GameObject
{
	GameObject();

	float x = 0.0f;
	float y = 0.0f;
	float pivot_x = 0.5f;    // NOTE(jesus): 0.5 is the center
	float pivot_y = 0.5f;    // NOTE(jesus): 0.5 is the center
	float width   = 0.0f;    // NOTE(jesus): If equals 0, it takes the size of the texture
	float height  = 0.0f;    // NOTE(jesus): If equals 0, it takes the size of the texture
	float angle = 0.0f;
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // NOTE(jesus): The texture will tinted with this color
	Texture * texture = nullptr;
	int  order = 0;          // NOTE(jesus): determines the drawing order
	bool deleteFlag = false; // NOTE(jesus): set this to true to remove the game object

	uint32 networkId = 0; // NOTE(jesus): Only for network game objects
};

class ModuleGameObject : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool preUpdate() override;

	bool update() override;

	bool postUpdate() override;

	bool cleanUp() override;


	//void deleteGameObjectsInScene(Screen *);

	GameObject* gameObjects[MAX_GAME_OBJECTS] = {};
};

