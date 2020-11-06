#include "Networks.h"


#if defined(USE_TASK_MANAGER)
class TaskLoadTexture : public Task
{
public:

	const char *filename = nullptr;
	Texture *texture = nullptr;

	void execute() override
	{
		texture = App->modTextures->loadTexture(filename);
	}
};
#endif


bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	banner = App->modTextures->loadTexture("banner.jpg");
	client = App->modTextures->loadTexture("client.jpg");
	server = App->modTextures->loadTexture("server.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("banner.jpg", &banner);
	loadTextureAsync("client.jpg", &client);
	loadTextureAsync("server.jpg", &server);
#endif

	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress)
{
	TaskLoadTexture *task = new TaskLoadTexture;
	task->owner = this;
	task->filename = filename;
	App->modTaskManager->scheduleTask(task, this);

	taskResults[taskCount].texturePtr = texturePtrAddress;
	taskResults[taskCount].task = task;
	taskCount++;
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (int i = 0; i < taskCount; ++i)
	{
		if (task == taskResults[i].task)
		{
			*(taskResults[i].texturePtr) = taskLoadTexture->texture;
			finishedTaskCount++;
			delete task;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;
	}
}

#endif
