#pragma once

#include "ByteSwap.h"

enum class Endianness {
	LittleEndian,
	BigEndian
};

constexpr Endianness STREAM_ENDIANNESS = Endianness::BigEndian;

// NOTE(jesus): This should be set considering the current platform
constexpr Endianness PLATFORM_ENDIANNESS = Endianness::LittleEndian;

// Default size set to the minimum MSS (MTU - IP and TCP header sizes)
// Typical MTU over Ethernet is 1500 bytes
// Minimum IP and TCP header sizes are 20 bytes each
//constexpr uint32 DEFAULT_STREAM_SIZE = 1460;

// PACKET_SIZE is defined in Networks.h
constexpr uint32 DEFAULT_STREAM_SIZE = PACKET_SIZE;

class OutputMemoryStream
{
public:

	// Constructor
	OutputMemoryStream(uint32 inSize = DEFAULT_STREAM_SIZE):
		mBuffer(nullptr), mCapacity(0), mHead(0)
	{ ReallocBuffer(inSize); }

	// Destructor
	~OutputMemoryStream()
	{ std::free(mBuffer); }

	// Get pointer to the data in the stream
	const char *GetBufferPtr() const { return mBuffer; }
	uint32 GetCapacity() const { return mCapacity; }
	uint32 GetSize() const { return mHead; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Write method
	void Write(const void *inData, size_t inByteCount);

	// Generic write for arithmetic types
	template< typename T >
	void Write( T inData )
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Write only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Write( &inData, sizeof( inData ) );
		}
		else
		{
			T swappedData = ByteSwap( inData );
			Write( &swappedData, sizeof( swappedData ) );
		}
	}

	// Generic write for vectors of arithmetic types
	template< typename T >
	void Write( const std::vector< T >& inVector )
	{
		uint32 elementCount = static_cast<uint32>(inVector.size());
		Write( elementCount );
		for( const T& element : inVector )
		{
			Write( element );
		}
	}

	// Write for strings
	void Write( const std::string& inString )
	{
		uint32 elementCount = static_cast<uint32>(inString.size());
		Write( elementCount );
		Write( inString.data(), elementCount * sizeof( char ) );
	}

	// Write for C strings
	void Write(const char *inString)
	{
		uint32 elementCount = (uint32)strlen(inString);
		Write(elementCount);
		Write(inString, elementCount * sizeof(char));
	}

	// Generic operator <<
	template< typename T >
	OutputMemoryStream &operator<<(const T &data) {
		Write(data);
		return *this;
	}
	

private:

	// Resize the buffer
	void ReallocBuffer(uint32 inNewLength);

	char *mBuffer;
	uint32 mCapacity;
	uint32 mHead;
};

class InputMemoryStream
{
public:

	// Constructor
	InputMemoryStream(uint32 inSize = DEFAULT_STREAM_SIZE) :
		mBuffer(static_cast<char*>(std::malloc(inSize))), mCapacity(inSize), mSize(0), mHead(0)
	{ }

	// Destructor
	~InputMemoryStream()
	{ std::free(mBuffer); }

	// Get pointer to the data in the stream
	char *GetBufferPtr() const { return mBuffer; }
	uint32 GetCapacity() const { return mCapacity; }
	uint32 GetSize() const { return mSize; }
	uint32 RemainingByteCount() const { return mSize - mHead; }
	void SetSize(uint32 size) { mSize = size; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Read method
	void Read(void *outData, size_t inByteCount) const;

	// Generic read for arithmetic types
	template< typename T >
	void Read( T& outData ) const
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Read only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Read( &outData, sizeof( outData ) );
		}
		else
		{
			T unswappedData;
			Read( &unswappedData, sizeof( unswappedData ) );
			outData = ByteSwap(unswappedData);
		}
	}

	// Generic read for vectors of arithmetic types
	template< typename T >
	void Read( std::vector< T >& inVector ) const
	{
		uint32 elementCount;
		Read( elementCount );
		inVector.resize( elementCount );
		for( T& element : inVector)
		{
			Read( element );
		}
	}

	// Read for strings
	void Read( std::string& inString ) const
	{
		uint32 elementCount;
		Read( elementCount );
		inString.resize(elementCount);
		for (auto &character : inString) {
			Read(character);
		}
	}
	// Read for C strings
	void Read(char * inString) const
	{
		uint32 elementCount;
		Read(elementCount);
		for (uint32 i = 0; i < elementCount; ++i) {
			Read(inString[i]);
		}
	}

	// Generic operator >>
	template< typename T >
	const InputMemoryStream &operator>>(T &data) const {
		Read(data);
		return *this;
	}

private:

	char *mBuffer;
	uint32 mCapacity;
	uint32 mSize;
	mutable uint32 mHead;
};
