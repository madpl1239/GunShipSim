#include "NetCodec.hpp"
#include <cstring>


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
		if(size < sizeof(T)) return false;
		std::memcpy(&value, data, sizeof(T));
		data += sizeof(T);
		size -= sizeof(T);
		return true;
	}
}


std::vector<std::uint8_t> NetCodec::encode(const InputSnapshotPacket& packet)
{
	std::vector<std::uint8_t> out;
	out.reserve(1 + sizeof(packet.tick) + 4 * sizeof(float) + 2);
	
	appendBytes(out, static_cast<std::uint8_t>(NetPacketType::InputSnapshot));
	appendBytes(out, packet.tick);
	appendBytes(out, packet.throttle);
	appendBytes(out, packet.yaw);
	appendBytes(out, packet.pitch);
	appendBytes(out, packet.roll);
	appendBytes(out, static_cast<std::uint8_t>(packet.fire ? 1 : 0));
	appendBytes(out, static_cast<std::uint8_t>(packet.reset ? 1 : 0));
	
	return out;
}


bool NetCodec::decode(const std::uint8_t* data, std::size_t size, InputSnapshotPacket& outPacket)
{
	std::uint8_t type = 0;
	if(!readBytes(data, size, type)) return false;
	if(type != static_cast<std::uint8_t>(NetPacketType::InputSnapshot)) return false;
	
	std::uint8_t fire = 0;
	std::uint8_t reset = 0;
	
	return readBytes(data, size, outPacket.tick)
	&& readBytes(data, size, outPacket.throttle)
	&& readBytes(data, size, outPacket.yaw)
	&& readBytes(data, size, outPacket.pitch)
	&& readBytes(data, size, outPacket.roll)
	&& readBytes(data, size, fire)
	&& readBytes(data, size, reset)
	&& (outPacket.fire = (fire != 0), true)
	&& (outPacket.reset = (reset != 0), true);
}


std::vector<std::uint8_t> NetCodec::encode(const StateSnapshotPacket& packet)
{
	std::vector<std::uint8_t> out;
	out.reserve(1 + sizeof(packet.tick) + 6 * sizeof(float));
	
	appendBytes(out, static_cast<std::uint8_t>(NetPacketType::StateSnapshot));
	appendBytes(out, packet.tick);
	appendBytes(out, packet.x);
	appendBytes(out, packet.y);
	appendBytes(out, packet.z);
	appendBytes(out, packet.yaw);
	appendBytes(out, packet.pitch);
	appendBytes(out, packet.roll);
	
	return out;
}


bool NetCodec::decode(const std::uint8_t* data, std::size_t size, StateSnapshotPacket& outPacket)
{
	std::uint8_t type = 0;
	if(!readBytes(data, size, type)) return false;
	if(type != static_cast<std::uint8_t>(NetPacketType::StateSnapshot)) return false;
	
	return readBytes(data, size, outPacket.tick)
	&& readBytes(data, size, outPacket.x)
	&& readBytes(data, size, outPacket.y)
	&& readBytes(data, size, outPacket.z)
	&& readBytes(data, size, outPacket.yaw)
	&& readBytes(data, size, outPacket.pitch)
	&& readBytes(data, size, outPacket.roll);
}
