#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

const int MAX_PEERS = 32;

struct UserInfo
{
  uint32_t id;
  std::string name;
};

UserInfo *get_user_info()
{
  static uint32_t gid = 0;
  gid++;
  return new UserInfo({ gid, "player" + std::to_string(gid)});
}

template<typename F>
void sent_packet_to_connected_peers(ENetPeer *peers, enet_uint32 flags, F messageCompose)
{
  std::vector<ENetPeer *> connectedPeers;
  for (int i = 0; i < MAX_PEERS; ++i)
  {
    if (peers[i].state == ENET_PEER_STATE_CONNECTED)
    {
      connectedPeers.push_back(&peers[i]);
    }
  }

  std::string msg = messageCompose(connectedPeers);
  printf("Send message to connected users: '%s'\n", msg.c_str());
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, flags);
  for (const auto &peer : connectedPeers)
  {
    enet_peer_send(peer, flags == ENET_PACKET_FLAG_RELIABLE ? 0 : 1, packet);
  }
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet\n");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10007;

  ENetHost *server = enet_host_create(&address, MAX_PEERS, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  uint32_t startTime = enet_time_get();
  uint32_t lastTimeSendTime = startTime;
  uint32_t lastPingSendTime = startTime;

  auto usersListMessage = [](const std::vector<ENetPeer *> &connectedPeers) {
    std::string msg = "Users list: ";
    for (const auto &peer : connectedPeers)
    {
      UserInfo* info = (UserInfo *) peer->data;
      msg += info->name + " ";
    }
    return msg;
  };
  auto pingMessage = [](const std::vector<ENetPeer *> &connectedPeers) {
    std::string msg = "Ping list: ";
    for (const auto &peer : connectedPeers)
    {
      UserInfo* info = (UserInfo *) peer->data;
      msg += info->name + ": " + std::to_string(peer->roundTripTime) + "ms ";
    }
    return msg;
  };

  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
      {
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        event.peer->data = get_user_info();
        auto newUserMessage = [event](std::vector<ENetPeer *> &connectedPeers) {
          UserInfo* info = (UserInfo *) event.peer->data;
          std::string msg = "New user connected: Name: " + info->name + " id: " + std::to_string(info->id);
          connectedPeers.erase(std::remove_if(connectedPeers.begin(), connectedPeers.end(),
                                              [event](ENetPeer *peer) { return peer == event.peer; }),
                               connectedPeers.end());
          return msg;
        };
        sent_packet_to_connected_peers(server->peers, ENET_PACKET_FLAG_RELIABLE, newUserMessage);
        sent_packet_to_connected_peers(server->peers, ENET_PACKET_FLAG_RELIABLE, usersListMessage);
        break;
      }
      case ENET_EVENT_TYPE_RECEIVE:
      {
        UserInfo *info = (UserInfo *) event.peer->data;
        printf("From %s. Packet received '%s'\n", info->name.c_str(), event.packet->data);
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
      {
        UserInfo *info = (UserInfo *) event.peer->data;
        printf("User %s was disconnected\n", info->name.c_str());
        delete info;
        break;
      }
      default:
        break;
      };
    }
    if (server->connectedPeers)
    {
      uint32_t curTime = enet_time_get();
      if (curTime - lastTimeSendTime > 10000)
      {
        lastTimeSendTime = curTime;
        auto timeMessage = [curTime](std::vector<ENetPeer *> connectedPeers) {
          return "Server time: " + std::to_string(curTime + rand() % 200);
        };
        sent_packet_to_connected_peers(server->peers, ENET_PACKET_FLAG_RELIABLE, timeMessage);
      }
      if (curTime - lastPingSendTime > 5000)
      {
        lastPingSendTime = curTime;
        sent_packet_to_connected_peers(server->peers, ENET_PACKET_FLAG_UNSEQUENCED, pingMessage);
      }
    }
  }

  for (int i = 0; i < MAX_PEERS; ++i)
  {
    if (server->peers[i].state == ENET_PEER_STATE_CONNECTED)
    {
      UserInfo* ptr = (UserInfo *) server->peers[i].data;
      if (ptr)
        delete ptr;
    }
  }
  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}

