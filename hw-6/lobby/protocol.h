#pragma once
#include <enet/enet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "structs.h"
//#include "entity.h"

enum MessageType : uint8_t
{
  E_CLIENT_TO_SERVER_JOIN = 0,
  E_SERVER_TO_CLIENT_NEW_ENTITY,
  E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY,
  E_CLIENT_TO_SERVER_STATE,
  E_SERVER_TO_CLIENT_SNAPSHOT,

  E_LOBBY_SERVER_TO_AGENT_CREATE_AGAR_SERVER, //command+eid+settings
  E_LOBBY_SERVER_TO_AGENT_CREATE_CARS_SERVER,

  E_LOBBY_CLIENT_TO_LOBBY_SERVER_LOGIN,
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_CREATE_AGAR_ROOM, //command+room id
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_CREATE_CARS_ROOM,
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_GET_ROOMS_LIST,
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_START,
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_JOIN_ROOM,
  E_LOBBY_CLIENT_TO_LOBBY_SERVER_LEAVE_ROOM,

  E_AGENT_TO_LOBBY_SERVER_SERVER_CREATED, //eid+port

  E_LOBBY_SERVER_TO_LOBBY_CLIENT_USER_ID,
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_ID,
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOMS_LIST,
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_AGAR, //players list + settings
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_ROOM_INFO_CARS,
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_USERS_LIST,
  E_LOBBY_SERVER_TO_LOBBY_CLIENT_SERVER_INFO,
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

void send_login(ENetPeer *peer, const User &user);
void send_user_id(ENetPeer *peer, uint16_t userId);
void send_server_created(ENetPeer *peer, const ServerInfo &serverInfo);
void send_users_list(ENetPeer *peer, const std::vector<User> &users);
void send_rooms_list(ENetPeer *peer, const std::vector<Room> &rooms);
void send_room_info_agar(ENetPeer *peer, const Room &room, const AgarSettings &settings, const std::vector<User> &users);
void send_room_info_cars(ENetPeer *peer, const Room &room, const CarsSettings &settings, const std::vector<User> &users);
void send_create_agar_room(ENetPeer *peer, const Room &room, const AgarSettings &settings);
void send_create_cars_room(ENetPeer *peer, const Room &room, const CarsSettings &settings);
void send_room_id(ENetPeer *peer, uint16_t roomId);
void send_agario_server_settings(ENetPeer *peer, const AgarSettings &settings);
void send_cars_server_settings(ENetPeer *peer, const CarsSettings &settings);
void send_server_port(ENetPeer *peer, uint16_t port);
void send_start(ENetPeer *peer, uint16_t roomId);
void send_join(ENetPeer *peer, uint16_t userId, uint16_t roomId);
void send_leave(ENetPeer *peer, uint16_t userId, uint16_t roomId);
void send_refresh(ENetPeer *peer);

MessageType get_packet_type(ENetPacket *packet);

//void deserialize_ (ENetPacket *packet, );
void deserialize_login(ENetPacket *packet, User &user);
void deserialize_user_id(ENetPacket *packet, uint16_t &userId);
void deserialize_server_created(ENetPacket *packet, ServerInfo &serverInfo);
void deserialize_users_list(ENetPacket *packet, std::vector<User> &users);
void deserialize_rooms_list(ENetPacket *packet, std::vector<Room> &rooms);
void deserialize_room_info_agar(ENetPacket *packet, Room &room, AgarSettings &settings, std::vector<User> &users);
void deserialize_room_info_cars(ENetPacket *packet, Room &room, CarsSettings &settings, std::vector<User> &users);
void deserialize_room_id(ENetPacket *packet, uint16_t &roomId);
void deserialize_create_agar_room(ENetPacket *packet, Room &room, AgarSettings &settings);
void deserialize_create_cars_room(ENetPacket *packet, Room &room, CarsSettings &settings);
void deserialize_agario_server_settings(ENetPacket *packet, AgarSettings &settings);
void deserialize_cars_server_settings(ENetPacket *packet, CarsSettings &settings);
void deserialize_server_port(ENetPacket *packet, uint16_t &port);
void deserialize_start(ENetPacket *packet, uint16_t &roomId);
void deserialize_join(ENetPacket *packet, uint16_t &userId, uint16_t &roomId);
void deserialize_leave(ENetPacket *packet, uint16_t &userId, uint16_t &roomId);

