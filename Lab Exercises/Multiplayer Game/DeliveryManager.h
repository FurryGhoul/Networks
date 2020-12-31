#pragma once

// TODO(you): Reliability on top of UDP lab session
#include <list>

class DeliveryManager;

class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager
{
public:

	//For senders to write a new seq. numbers into packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	//For receivers to process the seq. number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	//For receivers to write ack'ed seq. numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	//For senders to process ack'ed seq. numbers from a packet
	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	void forceSequenceNumber(uint32 num);

	void clear();

private:

	uint32 nextOutgoingSequenceNumber = 0;
	std::list<Delivery*> pendingDeliveries;

	uint32 nextExpectedSequenceNumber = 0;
	std::list<uint32> pendingNumsACK;
};