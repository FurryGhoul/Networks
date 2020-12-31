#include "Networks.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	Delivery* delivery = new Delivery();

	packet << nextOutgoingSequenceNumber;
	delivery->sequenceNumber = nextOutgoingSequenceNumber;
	delivery->dispatchTime = Time.time;

	nextOutgoingSequenceNumber++;
	pendingDeliveries.push_back(delivery);

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	bool ret = false;

	uint32 sequenceNumber = 0;
	packet >> sequenceNumber;

	if (sequenceNumber == nextExpectedSequenceNumber)
	{
		pendingNumsACK.push_back(sequenceNumber);
		nextExpectedSequenceNumber++;
		ret = true;;
	}

	return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	bool ret = false;

	if (pendingNumsACK.size() > 0)
	{
		ret = true;
	}

	return ret;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	for (auto iter = pendingNumsACK.begin(); iter != pendingNumsACK.end(); ++iter)
	{
		packet << (*iter);
	}

	pendingNumsACK.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	std::vector<Delivery*> deliveriesToDelete;

	while (packet.RemainingByteCount() > 0)
	{
		uint32 currentSequenceNumber = 0;
		packet >> currentSequenceNumber;

		for (auto iter = pendingDeliveries.begin(); iter != pendingDeliveries.end(); ++iter)
		{
			if ((*iter)->sequenceNumber == currentSequenceNumber)
			{
				(*iter)->delegate->onDeliverySuccess(this);
				deliveriesToDelete.push_back((*iter));
			}
		}

		for (auto iter = deliveriesToDelete.begin(); iter != deliveriesToDelete.end(); ++iter)
		{
			pendingDeliveries.remove((*iter));
		}

		deliveriesToDelete.clear();
	}
}

void DeliveryManager::processTimedOutPackets()
{
	std::vector<Delivery*> deliveriesToDelete;

	for (auto iter = pendingDeliveries.begin(); iter != pendingDeliveries.end(); ++iter)
	{
		// Check if the packet is considered out of time and erase it if it is
		if (Time.time - (*iter)->dispatchTime > PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			if ((*iter)->delegate != nullptr)
			{
				(*iter)->delegate->onDeliveryFailure(this);
				deliveriesToDelete.push_back((*iter));
			}
		}
	}

	for (auto iter = deliveriesToDelete.begin(); iter != deliveriesToDelete.end(); ++iter)
	{
		pendingDeliveries.remove((*iter));
	}

	deliveriesToDelete.clear();
}

void DeliveryManager::clear()
{
	for (auto iter = pendingDeliveries.begin(); iter != pendingDeliveries.end(); ++iter)
	{
		(*iter) = nullptr;
	}

	pendingDeliveries.clear();
	pendingNumsACK.clear();

	nextOutgoingSequenceNumber = 0;
	nextExpectedSequenceNumber = 0;
}

void ReplicationDeliveryDelegate::onDeliverySuccess(DeliveryManager* deliveryManager)
{

}

void ReplicationDeliveryDelegate::onDeliveryFailure(DeliveryManager* deliveryManager)
{
	for (auto item = deliveryReplicationCommands.begin(); item != deliveryReplicationCommands.end(); ++item)
	{
		if ((*item) == ReplicationAction::CREATE)
		{
			replicationServer->create((*item));
		}
		if ((*item) == ReplicationAction::UPDATE)
		{
			replicationServer->update((*item));
		}
		if ((*item) == ReplicationAction::DESTROY)
		{
			replicationServer->destroy((*item));
		}
	}
}
