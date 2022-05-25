#pragma once
#include <cstdint>

struct Room
{
  char name[32];
  uint16_t id = 0;
  uint8_t curPlayers = 0;
  uint8_t maxPlayers = 1;
  uint8_t type = 0;
  uint8_t running = 0;
};

struct AgarSettings
{
  uint16_t id = 0;
  uint16_t botsCount = 0;
  float minStartRadius = 0.1f;
  float maxStartRadius = 1.0f;
  float weightLoss = 0.5f;
  float speedModif = 1.0f;
};

struct CarsSettings
{
  uint16_t id = 0;
  float forwardAccel = 12.0f;
  float breakAccel = 3.0f;
  float speedRotation = 0.3f;
};

struct User
{
  char name[32];
  uint16_t roomId = 0;
  uint16_t id = 0;
};

struct ServerInfo
{
  uint16_t id = 0;
  uint16_t port = 0;
};