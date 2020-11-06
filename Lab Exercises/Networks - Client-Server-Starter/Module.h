#pragma once

class Module
{
public:

	// Constructor and destructor

	Module() { }

	virtual ~Module() { }


	// Virtual functions

	virtual bool init() { return true; }

	virtual bool preUpdate() { return true; }

	virtual bool update() { return true; }

	virtual bool gui() { return true; }

	virtual bool postUpdate() { return true; }

	virtual bool cleanUp() { return true;  }


	// For tasks

	virtual void onTaskFinished(Task *) { }
};
