#pragma once
#include <enet/enet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "entity.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_STATE,
  E_SERVER_TO_CLIENT_SNAPSHOT
};

class BitStream {
public:
  explicit BitStream(uint8_t *data, size_t lenght) : data(data), lenght(lenght), curLenght(0)
  {
    this->data += sizeof(uint8_t);
  }

  explicit BitStream(uint8_t* data) : data(data), lenght(0), curLenght(0) {}

  template<typename T>
  bool Read(T* ptr)
  {
    if (CheckLenght(sizeof(T)))
    {
      memcpy(ptr, data, sizeof(T)); 
      data += sizeof(T);
      return true;
    }
    else
    {
      printf("Error out of bound!");
      return false;
    }    
  }

  template <typename T>
  void Write(T* ptr)
  {
    memcpy(data, ptr, sizeof(T));
    data += sizeof(T);
  }

  template <typename T>
  void Write(T val)
  {
    *data = val;
    data += sizeof(T);
  }
private:
  bool CheckLenght(size_t addLenght);
  uint8_t *data;
  size_t lenght;
  size_t curLenght;
};

void send_join(ENetPeer *peer);
void send_new_entity(ENetPeer *peer, const Entity &ent);
void send_set_controlled_entity(ENetPeer *peer, uint16_t eid);
void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y, float r);
void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float r);

MessageType get_packet_type(ENetPacket *packet);

void deserialize_new_entity(ENetPacket *packet, Entity &ent);
void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid);
void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r);
void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r);

