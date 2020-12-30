#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0) 
	{
		ReplicationAction action = ReplicationAction::NONE;
		uint32 networkID = 0;
		packet >> action;
		packet >> networkID;

		if (action == ReplicationAction::CREATE)
		{
			GameObject* object = App->modGameObject->Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(object, networkID);

			packet >> object->angle;
			packet >> object->position.x;
			packet >> object->position.y;
			packet >> object->size.x;
			packet >> object->size.y;

			std::string filename;
			packet >> filename;

			bool hasAnimation = false;
			bool hasCollider = false;
			packet >> hasCollider;

			object->sprite = App->modRender->addSprite(object);

			if (filename == "asteroid1.png")
			{
				object->sprite->texture = App->modResources->asteroid1;
			}
			if (filename == "asteroid2.png")
			{
				object->sprite->texture = App->modResources->asteroid2;
			}
			if (filename == "spacecraft1.png")
			{
				object->sprite->texture = App->modResources->spacecraft1;
			}
			if (filename == "spacecraft2.png")
			{
				object->sprite->texture = App->modResources->spacecraft2;
			}
			if (filename == "spacecraft3.png")
			{
				object->sprite->texture = App->modResources->spacecraft3;
			}
			if (filename == "laser.png")
			{
				object->sprite->texture = App->modResources->laser;
			}
			if (filename == "explosion1.png")
			{
				object->sprite->texture = App->modResources->explosion1;
			}

			ColliderType auxCollider = ColliderType::None;
			if (hasCollider)
			{
				packet >> auxCollider;
				object->collider = App->modCollision->addCollider(auxCollider, object);
				packet >> object->collider->isTrigger;
			}
			else
			{
				packet >> hasAnimation;
				if (hasAnimation)
				{
					object->animation = App->modRender->addAnimation(object);
					object->animation->clip = App->modResources->explosionClip;
					App->modSound->playAudioClip(App->modResources->audioClipExplosion);
				}
				else
				{
					auxCollider = ColliderType::Laser;
				}
			}

			if (auxCollider == ColliderType::Player)
			{
				Spaceship* playerBehaviour = App->modBehaviour->addSpaceship(object);
				object->behaviour = playerBehaviour;
				object->behaviour->isServer = false;
			}
			if (auxCollider == ColliderType::Laser)
			{
				Laser* laserBehaviour = App->modBehaviour->addLaser(object);
				object->behaviour = laserBehaviour;
				object->behaviour->isServer = false;
			}
		}
		if (action == ReplicationAction::UPDATE)
		{
			GameObject* object = App->modLinkingContext->getNetworkGameObject(networkID);

			if (object)
			{
				packet >> object->angle;
				packet >> object->position.x;
				packet >> object->position.y;
				packet >> object->size.x;
				packet >> object->size.y;

				object->behaviour->read(packet);
			}
		}
		if (action == ReplicationAction::DESTROY)
		{
			GameObject* object = App->modLinkingContext->getNetworkGameObject(networkID);
			if (object)
			{
				App->modLinkingContext->unregisterNetworkGameObject(object);
				App->modGameObject->Destroy(object);
			}
		}
	}
}
