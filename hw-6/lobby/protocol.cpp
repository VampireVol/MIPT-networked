#include "protocol.h"

MessageType get_packet_type(ENetPacket *packet)
{
  return (MessageType)*packet->data;
}

bool BitStream::CheckLenght(size_t addLenght)
{
  curLenght += addLenght;
  return curLenght <= lenght;
}

void send_login(ENetPeer *peer, const User &user)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(User), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_LOGIN);
  bs.Write(&user);
  enet_peer_send(peer, 0, packet);
}

void send_user_id(ENetPeer *peer, uint16_t userId)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_USER_ID);
  bs.Write(&userId);
  enet_peer_send(peer, 0, packet);
}

void send_server_created(ENetPeer *peer, const ServerInfo &serverInfo)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(ServerInfo), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_AGENT_TO_LOBBY_SERVER_SERVER_CREATED);
  bs.Write(&serverInfo);
  enet_peer_send(peer, 0, packet);
}

void send_users_list(ENetPeer *peer, const std::vector<User> &users)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint32_t) + users.size() * sizeof(User),
                                          ENET_PACKET_FLAG_RELIABLE);
  uint32_t size = static_cast<uint32_t>(users.size());
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_USERS_LIST);
  bs.Write(&size);
  for (const auto &user : users)
  {
    bs.Write(&user);
  }
  enet_peer_send(peer, 0, packet);
}

void send_rooms_list(ENetPeer *peer, const std::vector<Room> &rooms)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint32_t) + rooms.size() * sizeof(Room),
                                          ENET_PACKET_FLAG_RELIABLE);
  uint32_t size = static_cast<uint32_t>(rooms.size());
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOMS_LIST);
  bs.Write(&size);
  for (const auto &room : rooms)
  {
    bs.Write(&room);
  }
  enet_peer_send(peer, 0, packet);
}

void send_room_info_agar(ENetPeer *peer, const Room &room, const AgarSettings &settings, const std::vector<User> &users)
{
  ENetPacket *packet = enet_packet_create(nullptr,
                                          sizeof(uint8_t) + sizeof(Room) + sizeof(AgarSettings) + sizeof(uint32_t) +
                                          users.size() * sizeof(User), ENET_PACKET_FLAG_RELIABLE);
  uint32_t size = static_cast<uint32_t>(users.size());
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_AGAR);
  bs.Write(&room);
  bs.Write(&settings);
  bs.Write(&size);
  for (const auto &user : users)
  {
    bs.Write(&user);
  }
  enet_peer_send(peer, 0, packet);
}
void send_room_info_cars(ENetPeer *peer, const Room &room, const CarsSettings &settings, const std::vector<User> &users)
{
  ENetPacket *packet = enet_packet_create(nullptr,
                                          sizeof(uint8_t) + sizeof(Room) + sizeof(CarsSettings) + sizeof(uint32_t) +
                                          users.size() * sizeof(User), ENET_PACKET_FLAG_RELIABLE);
  uint32_t size = static_cast<uint32_t>(users.size());
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_CARS);
  bs.Write(&room);
  bs.Write(&settings);
  bs.Write(&size);
  for (const auto &user : users)
  {
    bs.Write(&user);
  }
  enet_peer_send(peer, 0, packet);
}

void send_create_agar_room(ENetPeer *peer, const Room &room, const AgarSettings &settings)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Room) + sizeof(AgarSettings),
                                          ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_CREATE_AGAR_ROOM);
  bs.Write(&room);
  bs.Write(&settings);

  enet_peer_send(peer, 0, packet);
}

void send_create_cars_room(ENetPeer *peer, const Room &room, const CarsSettings &settings)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(Room) + sizeof(CarsSettings),
                                          ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_CREATE_CARS_ROOM);
  bs.Write(&room);
  bs.Write(&settings);

  enet_peer_send(peer, 0, packet);
}

void send_room_id(ENetPeer *peer, uint16_t roomId)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t),
                                          ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_ID);
  bs.Write(&roomId);

  enet_peer_send(peer, 0, packet);

}

void send_agario_server_settings(ENetPeer *peer, const AgarSettings &settings)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(AgarSettings), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_AGENT_CREATE_AGAR_SERVER);
  bs.Write(&settings);

  enet_peer_send(peer, 0, packet);
}

void send_cars_server_settings(ENetPeer *peer, const CarsSettings &settings)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(CarsSettings), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_AGENT_CREATE_CARS_SERVER);
  bs.Write(&settings);

  enet_peer_send(peer, 0, packet);
}

void send_server_port(ENetPeer *peer, uint16_t port)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_SERVER_TO_LOBBY_CLIENT_SERVER_INFO);
  bs.Write(&port);

  enet_peer_send(peer, 0, packet);
}

void send_start(ENetPeer *peer, uint16_t roomId)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_START);
  bs.Write(&roomId);

  enet_peer_send(peer, 0, packet);
}

void send_join(ENetPeer *peer, uint16_t userId, uint16_t roomId)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + 2 * sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_JOIN_ROOM);
  bs.Write(&userId);
  bs.Write(&roomId);

  enet_peer_send(peer, 0, packet);
}

void send_leave(ENetPeer *peer, uint16_t userId, uint16_t roomId)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t) + 2 * sizeof(uint16_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_LEAVE_ROOM);
  bs.Write(&userId);
  bs.Write(&roomId);

  enet_peer_send(peer, 0, packet);
}

void send_refresh(ENetPeer *peer)
{
  ENetPacket *packet = enet_packet_create(nullptr, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
  BitStream bs(packet->data);
  bs.Write(E_LOBBY_CLIENT_TO_LOBBY_SERVER_GET_ROOMS_LIST);

  enet_peer_send(peer, 0, packet);
}

void deserialize_login(ENetPacket *packet, User &user)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&user);
}

void deserialize_user_id(ENetPacket *packet, uint16_t &userId)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&userId);
}

void deserialize_server_created(ENetPacket *packet, ServerInfo &serverInfo)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&serverInfo);
}

void deserialize_users_list(ENetPacket *packet, std::vector<User> &users)
{
  uint32_t size;
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&size);
  for (int i = 0; i < size; ++i)
  {
    User user;
    bs.Read(&user);
    users.push_back(user);
  }
}

void deserialize_rooms_list(ENetPacket *packet, std::vector<Room> &rooms)
{
  uint32_t size;
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&size);
  for (int i = 0; i < size; ++i)
  {
    Room room;
    bs.Read(&room);
    rooms.push_back(room);
  }
}

void deserialize_room_info_agar(ENetPacket *packet, Room &room, AgarSettings &settings, std::vector<User> &users)
{
  uint32_t size;
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&room);
  bs.Read(&settings);
  bs.Read(&size);
  for (int i = 0; i < size; ++i)
  {
    User user;
    bs.Read(&user);
    users.push_back(user);
  }
}

void deserialize_room_info_cars(ENetPacket *packet, Room &room, CarsSettings &settings, std::vector<User> &users)
{
  uint32_t size;
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&room);
  bs.Read(&settings);
  bs.Read(&size);
  for (int i = 0; i < size; ++i)
  {
    User user;
    bs.Read(&user);
    users.push_back(user);
  }
}

void deserialize_create_agar_room(ENetPacket *packet, Room &room, AgarSettings &settings)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&room);
  bs.Read(&settings);
}

void deserialize_create_cars_room(ENetPacket *packet, Room &room, CarsSettings &settings)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&room);
  bs.Read(&settings);
}

void deserialize_room_id(ENetPacket *packet, uint16_t &roomId)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&roomId);
}

void deserialize_agario_server_settings(ENetPacket *packet, AgarSettings &settings)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&settings);
}

void deserialize_cars_server_settings(ENetPacket *packet, CarsSettings &settings)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&settings);
}

void deserialize_server_port(ENetPacket *packet, uint16_t &port)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&port);
}

void deserialize_start(ENetPacket *packet, uint16_t &roomId)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&roomId);
}

void deserialize_join(ENetPacket *packet, uint16_t &userId, uint16_t &roomId)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&userId);
  bs.Read(&roomId);
}

void deserialize_leave(ENetPacket *packet, uint16_t &userId, uint16_t &roomId)
{
  BitStream bs(packet->data, packet->dataLength);
  bs.Read(&userId);
  bs.Read(&roomId);
}
