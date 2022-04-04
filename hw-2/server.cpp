#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <string>

struct UserInfo
{
  uint32_t id;
  std::string name;
  uint32_t lastTime;
};

UserInfo* get_user_info()
{
  static uint32_t gid = 0;
  gid++;
  return new UserInfo({ gid, "player" + std::to_string(gid), enet_time_get() });
}

void sent_user_info_packet(ENetPeer *peer, const UserInfo* info)
{
  const std::string msg = "Name: " + info->name + " id: " + std::to_string(info->id);
  ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}

void sent_users_list_packet(ENetPeer *peer, ENetPeer *peers, uint32_t peersCnt)
{
  std::string msg = "Users list: ";
  for (int i = 0; i < peersCnt - 1; ++i)
  {
    UserInfo* info = (UserInfo *) peers[i].data;
    msg += info->name + " ";
    ENetPacket *packet = enet_packet_create(msg.c_str(), msg.size() + 1, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, 1, packet);
  }
}

void send_time_packet(ENetPeer *peer, uint32_t time)
{
  const std::string msg = std::to_string(time + rand() % 200);
  ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()), ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}

int main(int argc, const char **argv)
{
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = 10007;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  uint32_t lastMicroSendTime = enet_time_get();
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(server, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        event.peer->data = get_user_info();
        for (int i = 0; i < server->connectedPeers - 1; ++i)
        {
          sent_user_info_packet(&server->peers[i], (UserInfo *)event.peer->data);
        }
        sent_users_list_packet(event.peer, server->peers, server->connectedPeers);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        printf("Packet received '%s'\n", event.packet->data);
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    uint32_t curTime = enet_time_get();
    if (curTime - lastMicroSendTime > 10000)
    {
      lastMicroSendTime = curTime;
      for (int i = 0; i < server->connectedPeers; ++i)
      {
        send_time_packet(&server->peers[i], curTime);
      }
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}

