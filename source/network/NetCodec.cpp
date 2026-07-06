/*
 * NetCodec.cpp
 * 
 * 06-07-2026 by madpl
 */
#include <cstring>
#include <network/NetCodec.hpp>


namespace
{
	template <typename T>
	void appendBytes(std::vector<std::uint8_t>& out, const T& value)
	{
		const auto* ptr = reinterpret_cast<const std::uint8_t*>(&value);
		out.insert(out.end(), ptr, ptr + sizeof(T));
	}
	
	template <typename T>
	bool readBytes(const std::uint8_t*& data, std::size_t& size, T& value)
	{
		if(size < sizeof(T))
			return false;
		
		std::memcpy(&value, data, sizeof(T));
		data += sizeof(T);
		size -= sizeof(T);
		
		return true;
	}
}


std::vector<std::uint8_t> NetCodec::encode(const InputSnapshotPacket& packet)
{
	std::vector<std::uint8_t> out;
	out.reserve(1 + sizeof(packet.tick) + 3 * sizeof(float) + sizeof(std::uint8_t));
	
	appendBytes(out, static_cast<std::uint8_t>(NetPacketType::InputSnapshot));
	appendBytes(out, packet.tick);
	appendBytes(out, packet.forwardInput);
	appendBytes(out, packet.yawInput);
	appendBytes(out, packet.verticalInput);
	appendBytes(out, static_cast<std::uint8_t>(packet.brake ? 1 : 0));
	
	return out;
}


bool NetCodec::decode(const std::uint8_t* data, std::size_t size, InputSnapshotPacket& outPacket)
{
	std::uint8_t type = 0;
	std::uint8_t brake = 0;
	
	if(not readBytes(data, size, type))
		return false;
	
	if(type != static_cast<std::uint8_t>(NetPacketType::InputSnapshot))
		return false;
	
	if(not readBytes(data, size, outPacket.tick))
		return false;
	
	if(not readBytes(data, size, outPacket.forwardInput))
		return false;
	
	if(not readBytes(data, size, outPacket.yawInput))
		return false;
	
	if(not readBytes(data, size, outPacket.verticalInput))
		return false;
	
	if(not readBytes(data, size, brake))
		return false;
	
	outPacket.brake = (brake != 0);
	
	return true;
}


std::vector<std::uint8_t> NetCodec::encode(const StateSnapshotPacket& packet)
{
	std::vector<std::uint8_t> out;
	out.reserve(1 + sizeof(packet.tick) + 9 * sizeof(float));
	
	appendBytes(out, static_cast<std::uint8_t>(NetPacketType::StateSnapshot));
	appendBytes(out, packet.tick);
	appendBytes(out, packet.x);
	appendBytes(out, packet.y);
	appendBytes(out, packet.z);
	appendBytes(out, packet.yawDegrees);
	appendBytes(out, packet.pitchDegrees);
	appendBytes(out, packet.rollDegrees);
	appendBytes(out, packet.speed);
	appendBytes(out, packet.verticalSpeed);
	appendBytes(out, packet.altitudeAboveGround);
	
	return out;
}


bool NetCodec::decode(const std::uint8_t* data, std::size_t size, StateSnapshotPacket& outPacket)
{
	std::uint8_t type = 0;
	
	if(not readBytes(data, size, type))
		return false;
	
	if(type != static_cast<std::uint8_t>(NetPacketType::StateSnapshot))
		return false;
	
	return (readBytes(data, size, outPacket.tick) &&
			readBytes(data, size, outPacket.x) &&
			readBytes(data, size, outPacket.y) &&
			readBytes(data, size, outPacket.z) &&
			readBytes(data, size, outPacket.yawDegrees) &&
			readBytes(data, size, outPacket.pitchDegrees) &&
			readBytes(data, size, outPacket.rollDegrees) &&
			readBytes(data, size, outPacket.speed) &&
			readBytes(data, size, outPacket.verticalSpeed) &&
			readBytes(data, size, outPacket.altitudeAboveGround));
}
