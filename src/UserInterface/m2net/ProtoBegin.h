// m2net/ProtoBegin.h + ProtoEnd.h — se pun in jurul oricarui #include "....pb.h" din client.
//
// De ce: windows.h si headerele vechi ale clientului definesc macro-uri cu nume obisnuite
// (min, max, GetMessage, SendMessage) care pot strica headerele Protobuf si Abseil.
// Aici le ascundem temporar; ProtoEnd.h le readuce exact cum erau.
//
// Fara #pragma once: fisierul trebuie sa poata fi inclus de mai multe ori.

#pragma push_macro("min")
#pragma push_macro("max")
#pragma push_macro("GetMessage")
#pragma push_macro("SendMessage")
#pragma push_macro("number")
#undef min
#undef max
#undef GetMessage
#undef SendMessage
#undef number
