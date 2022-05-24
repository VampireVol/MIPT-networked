#pragma once
#include <cstdint>

struct Room
{
  char name[32];
  uint16_t id;
  uint8_t curPlayers;
  uint8_t maxPlayers;
  uint8_t type; // need squeeze
  uint8_t running; // need squeeze
};

struct AgarSettings
{
  uint16_t id;
  uint16_t botsCount = 0;
  float minStartRadius = 0.1f;
  float maxStartRadius = 1.0f;
  float weightLoss = 0.5f;
  float speedModif = 1.0f;
};

struct CarsSettings
{
  uint16_t id;
  float forwardAccel = 12.0f;
  float breakAccel = 3.0f;
  float speedRotation = 0.3f;
};

struct User
{
  char name[32];
  uint16_t roomId;
  uint16_t id;
};

struct ServerInfo
{
  uint16_t id;
  uint16_t port;
};