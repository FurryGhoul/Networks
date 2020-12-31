#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkID) 
{
	commands.push_back(ReplicationCommand(networkID, ReplicationAction::CREATE));
}

void ReplicationManagerServer::destroy(uint32 networkID) 
{
	commands.insert(commands.begin(), (ReplicationCommand(networkID, ReplicationAction::DESTROY)));
}

void ReplicationManagerServer::update(uint32 networkID) 
{
	commands.push_back(ReplicationCommand(networkID, ReplicationAction::UPDATE));
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, Delivery* delivery)
{
	for (auto item = commands.begin(); item != commands.end(); ++item)
	{
		packet << (*item).action;
		packet << (*item).networkID;

		if ((*item).action == CREATE)
		{
			GameObject* object = App->modLinkingContext->getNetworkGameObject((*item).networkID);

			packet << object->angle;
			packet << object->position.x;
			packet << object->position.y;
			packet << object->size.x;
			packet << object->size.y;

			std::string filename = object->sprite->texture->filename;
			packet << filename;

			bool hasAnimation = false;
			if (object->animation)
			{
				hasAnimation = true;
			}

			if (object->collider)
			{
				packet << true;
				packet << object->collider->type;
				packet << object->collider->isTrigger;
			}
			else
			{
				packet << false;
				packet << hasAnimation;
			}
		}
		if ((*item).action == UPDATE)
		{
			GameObject* object = App->modLinkingContext->getNetworkGameObject((*item).networkID);

			if (object)
			{
				packet << object->angle;
				packet << object->position.x;
				packet << object->position.y;
				packet << object->size.x;
				packet << object->size.y;

				object->behaviour->write(packet);
			}
		}
	}

	commands.clear();
}
