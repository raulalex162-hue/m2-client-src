// Fisierul sta in src/UserInterface/m2net, deci headerele clientului se includ cu "../".
#include "../StdAfx.h"
#include "../PythonNetworkStream.h"
#include "../AbstractChat.h"
#include "../Packet.h"
#include "ClientNet.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "m2/ProtocolHash.h"

// Headerele Protobuf se includ mereu intre ProtoBegin.h si ProtoEnd.h.
#include "ProtoBegin.h"
#include "m2/heartbeat.pb.h"
#include "m2/system_ids.pb.h"
#include "ProtoEnd.h"

namespace
{
	// Prefixul [system:2][type:2] din payload.
	constexpr size_t kPrefixSize = 4;
	// Limita pachetului (lungimea e pe 2 octeti) minus framing-ul si prefixul.
	constexpr size_t kMaxBodySize = 65535 - PACKET_HEADER_SIZE - kPrefixSize;

	uint64_t NowMs()
	{
		using namespace std::chrono;
		return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
	}

	uint16_t ReadU16(const uint8_t* p)
	{
		return static_cast<uint16_t>(p[0] | (p[1] << 8));
	}

	void WriteU16(std::vector<uint8_t>& out, uint16_t value)
	{
		out.push_back(static_cast<uint8_t>(value & 0xFF));
		out.push_back(static_cast<uint8_t>(value >> 8));
	}

	void Chat(const std::string& text)
	{
		IAbstractChat::GetSingleton().AppendChat(CHAT_TYPE_INFO, text.c_str());
	}

	void OnHeartbeatPong(const uint8_t* body, size_t size)
	{
		m2::heartbeat::Pong pong;
		if (!pong.ParseFromArray(body, static_cast<int>(size)))
		{
			TraceError("m2net: Pong invalid (%u octeti)", static_cast<unsigned>(size));
			return;
		}

		const uint64_t now = NowMs();
		const uint64_t rtt = now >= pong.client_time_ms() ? now - pong.client_time_ms() : 0;

		std::string text = "[heartbeat] Pong: " + std::to_string(rtt) + " ms, protocol ";
		if (pong.protocol_ok())
			text += std::string("OK (") + m2::kProtocolHash + ")";
		else
			text += std::string("DIFERIT: client ") + m2::kProtocolHash + ", server " + pong.server_protocol_hash();
		Chat(text);
	}

	// Trimite mesajul la sistemul lui. Clientul are deocamdata un singur sistem (heartbeat);
	// cand vor fi mai multe, aici devine un tabel, ca routerul de pe server.
	void Dispatch(uint16_t system, uint16_t type, const uint8_t* body, size_t size)
	{
		if (system == m2::SYSTEM_HEARTBEAT && type == m2::heartbeat::PONG)
		{
			OnHeartbeatPong(body, size);
			return;
		}
		TraceError("m2net: mesaj necunoscut %u:%u (%u octeti)", system, type, static_cast<unsigned>(size));
	}
}

bool CPythonNetworkStream::RecvSystemPacket()
{
	// DispatchPacket a verificat deja ca pachetul a sosit intreg.
	TDynamicSizePacketHeader frame;
	if (!Recv(sizeof(frame), &frame))
		return false;
	if (frame.length < PACKET_HEADER_SIZE)
		return false;

	const size_t payloadSize = frame.length - PACKET_HEADER_SIZE;
	std::vector<uint8_t> payload(payloadSize);
	if (payloadSize > 0 && !Recv(static_cast<int>(payloadSize), payload.data()))
		return false;

	if (payloadSize < kPrefixSize)
	{
		TraceError("m2net: pachet SYSTEM prea scurt (%u octeti)", static_cast<unsigned>(payloadSize));
		return true;
	}

	Dispatch(ReadU16(payload.data()), ReadU16(payload.data() + 2), payload.data() + kPrefixSize, payloadSize - kPrefixSize);
	return true;
}

bool CPythonNetworkStream::SendSystemMessage(uint16_t system, uint16_t type, const std::string& body)
{
	// Serverul accepta pachetul SYSTEM doar in faza de joc; in alte faze ar inchide conexiunea.
	if (m_strPhase != "Game")
		return false;

	if (body.size() > kMaxBodySize)
	{
		TraceError("m2net: mesaj %u:%u prea mare (%u octeti)", system, type, static_cast<unsigned>(body.size()));
		return false;
	}

	const uint16_t length = static_cast<uint16_t>(PACKET_HEADER_SIZE + kPrefixSize + body.size());
	std::vector<uint8_t> packet;
	packet.reserve(length);
	WriteU16(packet, CG::SYSTEM);
	WriteU16(packet, length);
	WriteU16(packet, system);
	WriteU16(packet, type);
	packet.insert(packet.end(), body.begin(), body.end());

	return Send(static_cast<int>(packet.size()), packet.data());
}

bool CPythonNetworkStream::SendHeartbeatPing()
{
	m2::heartbeat::Ping ping;
	ping.set_client_time_ms(NowMs());
	ping.set_protocol_hash(m2::kProtocolHash);
	return SendSystemMessage(m2::SYSTEM_HEARTBEAT, m2::heartbeat::PING, ping.SerializeAsString());
}

namespace m2net
{
	bool HandleClientCommand(CPythonNetworkStream& net, const char* command)
	{
		if (!command || std::strcmp(command, "/ping") != 0)
			return false;

		if (net.SendHeartbeatPing())
			Chat("[heartbeat] Ping trimis...");
		else
			Chat("[heartbeat] Ping netrimis (doar in joc)");
		return true;
	}
}
