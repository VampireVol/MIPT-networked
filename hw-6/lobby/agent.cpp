#include <enet/enet.h>
#include <string>
#include <vector>
#include "protocol.h"
#include <sys/types.h>

const int MAX_PEERS = 32;
const int AGENT_PORT = 10007;

static uint16_t curPort = 10100;

struct Info
{
  uint16_t id;
  uint16_t port;
  pid_t pid;
};

static std::vector<Info> infos;

bool check_port(uint16_t port)
{
  for (const auto &info : infos)
  {
    if (info.port == port)
      return false;
  }
  return true;
}

uint16_t get_free_port()
{
  const uint16_t startPort = 10100;
  const uint16_t endPort = 10200;
  while (!check_port(curPort))
  {
    if (curPort == endPort)
      curPort = startPort;
    else
      ++curPort;
  }
  return curPort;
}

void add_info(uint16_t id, uint16_t port, pid_t pid)
{
  infos.push_back({ id, port, pid });
}

void start_agario_server(uint16_t port, const AgarSettings &settings)
{
  const char *path = "agario/bin/server";
  pid_t pid;
  switch(pid=fork())
  {
    case 0:
      printf("This is child, starting agario\n");
      execl(path, "", std::to_string(port).c_str(), std::to_string(settings.botsCount).c_str(),
            std::to_string(settings.minStartRadius).c_str(), std::to_string(settings.maxStartRadius).c_str(),
            std::to_string(settings.weightLoss).c_str(), std::to_string(settings.speedModif).c_str(), nullptr);
      printf("ops\n");
      break;
    default:
      printf("I'm parent, my child: %d\n", pid);
      add_info(settings.id, port, pid);
  }
}

void start_cars_server(uint16_t port, const CarsSettings &settings)
{
  const std::string path = "cars/bin/server";
  pid_t pid;
  switch(pid=fork())
  {
    case 0:
      printf("This is child, starting cars port: %d\n", port);
      execl(path.c_str(), "", std::to_string(port).c_str(), std::to_string(settings.forwardAccel).c_str(),
            std::to_string(settings.breakAccel).c_str(), std::to_string(settings.speedRotation).c_str(), nullptr);
      break;
    default:
      printf("I'm parent, my child: %d\n", pid);
      add_info(settings.id, port, pid);
  }
}

void packet_receive(const ENetEvent &event)
{
  switch (get_packet_type(event.packet))
  {
    case E_LOBBY_SERVER_TO_AGENT_CREATE_AGAR_SERVER:
    {
      AgarSettings settings;
      deserialize_agario_server_settings(event.packet, settings);
      uint16_t port = get_free_port();
      start_agario_server(port, settings);
      send_server_created(event.peer, { settings.id, port });
      break;
    }
    case E_LOBBY_SERVER_TO_AGENT_CREATE_CARS_SERVER:
    {
      CarsSettings settings;
      deserialize_cars_server_settings(event.packet, settings);
      uint16_t port = get_free_port();
      start_cars_server(port, settings);
      send_server_created(event.peer, { settings.id, port });
      break;
    }
  }
}

int main(int argc, const char **argv)
{
  printf("Agent started\n");
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet\n");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = AGENT_PORT;

  ENetHost *server = enet_host_create(&address, MAX_PEERS, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

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
        break;
      }
      case ENET_EVENT_TYPE_RECEIVE:
      {
        printf("Packet received '%s'\n", event.packet->data);
        packet_receive(event);
        enet_packet_destroy(event.packet);
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
      {
        printf("User was disconnected\n");
        break;
      }
      default:
        break;
      };
    }
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}

