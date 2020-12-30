#pragma once

// TODO(you): World state replication lab session

enum ReplicationAction 
{
	NONE,
	CREATE,
	DESTROY,
	UPDATE
};

struct ReplicationCommand 
{
public:

	uint32 networkID = -1;
	ReplicationAction action = NONE;
	ReplicationCommand(uint32 networkID, ReplicationAction action) : networkID(networkID), action(action) {}
};