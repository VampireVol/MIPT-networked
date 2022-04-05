#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include <string>

void send_fragmented_packet(ENetPeer *peer)
{
  const char *baseMsg = "Stay awhile and listen. ";
  const size_t msgLen = strlen(baseMsg);

  const size_t sendSize = 2500;
  char *hugeMessage = new char[sendSize];
  for (size_t i = 0; i < sendSize; ++i)
    hugeMessage[i] = baseMsg[i % msgLen];
  hugeMessage[sendSize-1] = '\0';

  ENetPacket *packet = enet_packet_create(hugeMessage, sendSize, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);

  delete[] hugeMessage;
}

void send_time_packet(ENetPeer *peer, uint32_t time)
{
  const std::string msg = std::to_string(time + rand() % 200);
  ENetPacket *packet = enet_packet_create(msg.c_str(), strlen(msg.c_str()), ENET_PACKET_FLAG_UNSEQUENCED);
  enet_peer_send(peer, 1, packet);
}

void send_start(ENetPeer *peer)
{
  const char *msg = "start";
  ENetPacket *packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
  enet_peer_send(peer, 0, packet);
}

int main(int argc, const char **argv)
{
  srand(time(nullptr));
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }

  uint32_t startMatchTime = 0;
  if (argc == 2)
  {
    printf("Start session after %s seconds\n", argv[1]);
    startMatchTime = atoi(argv[1]) * 1000;
  }

  ENetHost *client = enet_host_create(nullptr, 2, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = 10887;

  ENetPeer *lobbyPeer = enet_host_connect(client, &address, 2, 0);
  ENetPeer *serverPeer = nullptr;
  if (!lobbyPeer)
  {
    printf("Cannot connect to lobby");
    return 1;
  }

  uint32_t timeStart = enet_time_get();
  uint32_t lastFragmentedSendTime = timeStart;
  uint32_t lastMicroSendTime = timeStart;
  bool connected = false;
  while (true)
  {
    ENetEvent event;
    while (enet_host_service(client, &event, 10) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        if (!serverPeer && event.peer->address.port == address.port) //==
        {
          char *buffer = (char *)event.packet->data;
          char *save;
          char *host = strtok_r(buffer, " ", &save);
          char *port = strtok_r(nullptr, " ", &save);
          ENetAddress serverAddress;
          enet_address_set_host(&serverAddress, host);
          serverAddress.port = atoi(port);
          serverPeer = enet_host_connect(client, &serverAddress, 2, 0);
          startMatchTime = 0;
          if (!serverPeer)
          {
            printf("Cannot connect to server\n");
            return 1;
          }
        }
        printf("Packet received '%s' From:%x\n", event.packet->data, event.peer->address.host);
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    if (connected)
    {
      uint32_t curTime = enet_time_get();
      if (curTime - lastMicroSendTime > 1000 && serverPeer)
      {
        lastMicroSendTime = curTime;
        send_time_packet(serverPeer, curTime);
      }
      if (curTime - timeStart > startMatchTime && startMatchTime > 0)
      {
        printf("Send start\n");
        send_start(lobbyPeer);
        startMatchTime = 0;
      }
    }
  }
  return 0;
}
