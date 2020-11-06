#pragma once

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:

	Texture *background = nullptr;
	Texture *banner = nullptr;
	Texture *client = nullptr;
	Texture *server = nullptr;

	bool finishedLoading = false;
private:

	bool init() override;

#if defined(USE_TASK_MANAGER)
	void onTaskFinished(Task *task) override;

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress);
#endif

	struct LoadTextureResult {
		Texture **texturePtr = nullptr;
		Task *task = nullptr;
	};

	LoadTextureResult taskResults[1024] = {};
	int taskCount = 0;
	int finishedTaskCount = 0;
};

