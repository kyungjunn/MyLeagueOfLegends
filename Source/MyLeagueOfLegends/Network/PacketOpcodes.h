// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <cstdint>

// Wire protocol opcode table. Not a UENUM -- this is an internal
// transport-layer detail, never exposed to Blueprints directly.
//
// This is a manually-duplicated mirror of Server/src/Protocol/Opcodes.h.
// The server (CMake/Boost.Asio) and this client (UnrealBuildTool) are
// separate build systems, so the header itself can't be shared -- if you
// add/change a value on one side, update the other to match the same
// numeric values.
enum class EPacketOpcode : uint16_t
{
	ErrorResponse = 0,

	LoginRequest = 1,
	LoginResponse = 2,
	RegisterRequest = 3,
	RegisterResponse = 4,

	ChatJoinRoom = 5,
	ChatJoinRoomResponse = 6,
	ChatLeaveRoom = 7,
	ChatLeaveRoomResponse = 8,
	ChatMessage = 9,
	ChatMessageBroadcast = 10,
	ChatSystemMessage = 11,
};
