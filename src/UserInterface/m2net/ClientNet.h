#pragma once
// m2net — partea de client a pachetului SYSTEM (0x0C80), pereche cu core/net pe server.
//
//   pachet:  [header:2 = SYSTEM][lungime:2][ system:2 ][ type:2 ][ mesaj Protobuf ]
//
// Membrii CPythonNetworkStream definiti in ClientNet.cpp:
//   RecvSystemPacket()                  primeste GC::SYSTEM si il da sistemului destinatar
//   SendSystemMessage(system, type, b)  trimite CG::SYSTEM (doar in faza de joc)
//   SendHeartbeatPing()                 trimite m2.heartbeat.Ping
//
// Comanda locala de chat (nu ajunge la server ca text):
//   /ping   trimite un Ping; raspunsul (Pong) apare in chat cu latenta si verificarea protocolului

class CPythonNetworkStream;

namespace m2net
{
	// Apelat din CPythonNetworkStream::ClientCommand. true = comanda a fost tratata local.
	bool HandleClientCommand(CPythonNetworkStream& net, const char* command);
}
