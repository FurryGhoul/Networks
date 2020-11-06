#include "Networks.h"

void OutputMemoryStream::Write(const void *inData, size_t inByteCount)
{
	// make sure we have space
	const uint32 resultHead = mHead + static_cast<uint32>(inByteCount);
	if (resultHead > mCapacity)
	{
		ReallocBuffer(max(mCapacity * 2, resultHead));
	}

	// Copy into buffer at head
	std::memcpy(mBuffer + mHead, inData, inByteCount);

	// Increment head for next write
	mHead = resultHead;
}

void OutputMemoryStream::ReallocBuffer(uint32 inNewLength)
{
	mBuffer = static_cast<char*>(std::realloc(mBuffer, inNewLength));
	ASSERT(mBuffer != nullptr && "OutputMemoryStream::ReallocBuffer() - std::realloc() failed.");
	mCapacity = inNewLength;
}

void InputMemoryStream::Read(void *outData, size_t inByteCount) const
{
	uint32 resultHead = mHead + static_cast<uint32>(inByteCount);
	ASSERT(resultHead <= mSize && resultHead <= mCapacity && "InputMemoryStream::Read() - trying to read more data than available.");
	std::memcpy(outData, mBuffer + mHead, inByteCount);
	mHead = resultHead;
}
