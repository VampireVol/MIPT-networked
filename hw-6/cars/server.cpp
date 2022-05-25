#include <enet/enet.h>
#include <iostream>
#include "entity.h"
#include "protocol.h"
#include "mathUtils.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer*> controlledMap;
static int port = 10132;

uint32_t gen_color()
{
  //best shade count is 2^n + 1
  const int shadeCount = 5;
  return 0xff000000 +
         0x00010000 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1) +
         0x00000100 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1) +
         0x00000001 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1);
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent, enet_time_get());

  // find max eid
  uint16_t maxEid = entities.empty() ? invalid_entity : entities[0].eid;
  for (const Entity &e : entities)
    maxEid = std::max(maxEid, e.eid);
  uint16_t newEid = maxEid + 1;
  uint32_t color = gen_color();
  float x = (rand() % 4) * 2.f;
  float y = (rand() % 4) * 2.f;
  Entity ent = {color, x, y, 0.f, (rand() / RAND_MAX) * 3.141592654f, 0.f, 0.f, newEid};
  entities.push_back(ent);

  controlledMap[newEid] = peer;


  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
    if (host->peers[i].state == ENET_PEER_STATE_CONNECTED)
      send_new_entity(&host->peers[i], ent, enet_time_get());
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid, enet_time_get() - peer->roundTripTime / 2);
}

void on_input(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float thr = 0.f; float steer = 0.f;
  deserialize_entity_input(packet, eid, thr, steer);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.thr = thr;
      e.steer = steer;
    }
}

template <typename T>
void read_arg(const char *str, T &out)
{
  std::istringstream ss(str);
  T temp;
  if (ss >> temp)
    out = temp;
  else
    std::cerr << "Invalid number: " << str << std::endl;
}

void read_args(int argc, const char** argv)
{
  printf("count args: %d\n", argc);
  if (argc > 1)
  {
    read_arg(argv[1], port);
    read_arg(argv[2], forward_accel);
    read_arg(argv[3], break_accel);
    read_arg(argv[4], speed_rotation);
    printf("get port: %d forward_accel: %f break_accel: %f speed_rotation: %f \n", port, forward_accel,
           break_accel, speed_rotation);
  }
}

int main(int argc, const char **argv)
{
  read_args(argc, argv);
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = port;
  printf("Pre create server! Port: %d\n", port);
  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }
  printf("Crars server start! Port: %d\n", port);

  uint32_t lastTime = enet_time_get();
  uint32_t lastSendSnapshot = lastTime;
  while (true)
  {
    uint32_t curTime = enet_time_get();
    float dt = (curTime - lastTime) * 0.001f;
    lastTime = curTime;
    ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
          case E_CLIENT_TO_SERVER_JOIN:
            on_join(event.packet, event.peer, server);
            break;
          case E_CLIENT_TO_SERVER_INPUT:
            on_input(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    static int t = 0;
    for (Entity &e : entities)
    {
      // simulate
      simulate_entity(e, dt);
    }
    if (curTime - lastSendSnapshot > 200)
    {
      for (Entity &e : entities)
      {
        for (size_t i = 0; i < server->peerCount; ++i)
        {
          //send
          ENetPeer *peer = &server->peers[i];
          if (peer->state == ENET_PEER_STATE_CONNECTED)
            send_snapshot(peer, e.eid, e.x, e.y, e.ori, curTime);
        }
      }
      lastSendSnapshot = curTime;
    }
    usleep(10000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


