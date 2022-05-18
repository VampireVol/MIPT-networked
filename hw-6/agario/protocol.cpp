#include "protocol.h"

void send_join(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  *packet->data = E_CLIENT_TO_SERVER_JOIN;

  enet_peer_send(peer, 0, packet);
}

void send_new_entity(ENetPeer *peer, const Entity &ent)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Entity),
                                                   ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_SERVER_TO_CLIENT_NEW_ENTITY);
  bs.Write(&ent);

  enet_peer_send(peer, 0, packet);
}

void send_set_controlled_entity(ENetPeer *peer, uint16_t eid)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                                   ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY);
  bs.Write(&eid);

  enet_peer_send(peer, 0, packet);
}

void send_entity_state(ENetPeer *peer, uint16_t eid, float x, float y, float r)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   3 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);
  BitStream bs(packet->data);
  bs.Write(E_CLIENT_TO_SERVER_STATE);
  bs.Write(&eid);
  bs.Write(&x);
  bs.Write(&y);
  bs.Write(&r);

  enet_peer_send(peer, 1, packet);
}

void send_snapshot(ENetPeer *peer, uint16_t eid, float x, float y, float r)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t) +
                                                   3 * sizeof(float),
                                                   ENET_PACKET_FLAG_UNSEQUENCED);
  BitStream bs(packet->data);
  bs.Write(E_SERVER_TO_CLIENT_SNAPSHOT);
  bs.Write(&eid);
  bs.Write(&x);
  bs.Write(&y);
  bs.Write(&r);

  enet_peer_send(peer, 1, packet);
}

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

void deserialize_new_entity(ENetPacket *packet, Entity &ent)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&ent);
}

void deserialize_set_controlled_entity(ENetPacket *packet, uint16_t &eid)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&eid);
}

void deserialize_entity_state(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&eid);
  bs.Read(&x);
  bs.Read(&y);
  bs.Read(&r);
}

void deserialize_snapshot(ENetPacket *packet, uint16_t &eid, float &x, float &y, float &r)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&eid);
  bs.Read(&x);
  bs.Read(&y);
  bs.Read(&r);
}

bool BitStream::CheckLenght(size_t addLenght)
{
  curLenght += addLenght;
  return curLenght <= lenght;
}
