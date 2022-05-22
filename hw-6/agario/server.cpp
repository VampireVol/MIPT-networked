#include <enet/enet.h>
#include <iostream>
#include "entity.h"
#include "protocol.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include <math.h>
#include <sstream>
#include <iostream>

struct Point2
{
  float x;
  float y;
};

static std::vector<Entity> entities;
static std::map<uint16_t, ENetPeer*> controlledMap;
static std::vector<Point2> moveTo;

const int WIGHT = 16;
const int HEIGHT = 8;

static int port = 10131;
static int aiSize = 10;
static float minStartRadius = 0.1f;
static float maxStartRadius = 1.0f;
static float weightLoss = 0.5f;
static float speedModif = 1.0f;

uint32_t gen_color()
{
  //best shade count is 2^n + 1
  const int shadeCount = 5;
  return 0xff000000 +
         0x00010000 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1) +
         0x00000100 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1) +
         0x00000001 * ((rand() % shadeCount) * 256 / (shadeCount - 1) - 1);
}

uint16_t gen_eid()
{
  // find max eid
  uint16_t maxEid = entities.empty() ? invalid_entity : entities[0].eid;
  for (const Entity &e : entities)
    maxEid = std::max(maxEid, e.eid);
  return maxEid + 1;
}

Point2 gen_point()
{
  float x = ((rand() % 100) / 50.0f - 1) * WIGHT;
  float y = ((rand() % 100) / 50.0f - 1) * HEIGHT;
  return { x, y };
}

float gen_r()
{
  return minStartRadius +
         static_cast<float> (rand()) / (static_cast<float> (RAND_MAX / (maxStartRadius - minStartRadius)));
}

void on_join(ENetPacket *packet, ENetPeer *peer, ENetHost *host)
{
  // send all entities
  for (const Entity &ent : entities)
    send_new_entity(peer, ent);

  uint16_t newEid = gen_eid();
  uint32_t color = gen_color();
  Point2 p = gen_point();
  float r = gen_r();
  Entity ent = {color, p.x, p.y, r, newEid};
  entities.push_back(ent);

  controlledMap[newEid] = peer;

  // send info about new entity to everyone
  for (size_t i = 0; i < host->peerCount; ++i)
    if (host->peers[i].state == ENET_PEER_STATE_CONNECTED)
      send_new_entity(&host->peers[i], ent);
  // send info about controlled entity
  send_set_controlled_entity(peer, newEid);
}

void on_state(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f; float r = 0.f;
  deserialize_entity_state(packet, eid, x, y, r);
  for (Entity &e : entities)
    if (e.eid == eid)
    {
      e.x = x;
      e.y = y;
      e.r = r;
    }
}

void create_ai()
{
  for (int i = 0; i < aiSize; ++i)
  {
    Point2 start = gen_point();
    Point2 next = gen_point();
    float r = gen_r();
    uint32_t color = gen_color();
    uint16_t newEid = gen_eid();
    Entity ent = {color, start.x, start.y, r, newEid};
    entities.push_back(ent);
    moveTo.push_back(next);
  }
}

void update_ai(float dt)
{
  for (int i = 0; i < aiSize; ++i)
  {
    Entity e = entities[i];
    Point2 t = moveTo[i];
    float speed = e.GetSpeed() * speedModif;
    float dx = t.x - e.x;
    float dy = t.y - e.y;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist < 1.f)
    {
      moveTo[i] = gen_point();
      t = moveTo[i];
      dx = e.x - t.x;
      dy = e.y - t.y;
      dist = sqrt(dx * dx + dy * dy);
    }
    entities[i].x += dx / dist * speed * dt;
    entities[i].y += dy / dist * speed * dt;
  }
}

void collide()
{
  for (auto &e1 : entities)
  {
    for (auto &e2 : entities)
    {
      if (e1.eid == e2.eid)
        continue;
      if ((abs(e1.x - e2.x) < (e1.r + e2.r)) && (abs(e1.y - e2.y) < (e1.r + e2.r)))
      {
        Entity &eMin = e1.r < e2.r ? e1 : e2;
        Entity &eMax = e1.r < e2.r ? e2 : e1;
        eMax.r += weightLoss * eMin.r;
        Point2 respawn = gen_point();
        eMin.x = respawn.x;
        eMin.y = respawn.y;
        eMin.r = std::max(weightLoss * eMin.r, 0.001f);
        if (auto search = controlledMap.find(e1.eid); search != controlledMap.end() &&
                                                      search->second->state == ENET_PEER_STATE_CONNECTED)
        {
          send_snapshot(search->second, e1.eid, e1.x, e1.y, e1.r);
        }
        if (auto search = controlledMap.find(e2.eid); search != controlledMap.end() &&
                                                      search->second->state == ENET_PEER_STATE_CONNECTED)
        {
          send_snapshot(search->second, e2.eid, e2.x, e2.y, e2.r);
        }
      }
    }
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
    read_arg(argv[2], aiSize);
    read_arg(argv[3], minStartRadius);
    read_arg(argv[4], maxStartRadius);
    read_arg(argv[5], weightLoss);
    read_arg(argv[6], speedModif);
    printf("get port: %d aiSize: %d minStartRadius: %f maxStartRadius: %f weightLoss: %f speedModif: %f \n", port, aiSize,
           minStartRadius, maxStartRadius, weightLoss, speedModif);
  }
}

int main(int argc, const char **argv)
{
  read_args(argc, argv);
  create_ai();
  if (enet_initialize() != 0)
  {
    printf("Cannot init ENet");
    return 1;
  }
  ENetAddress address;

  address.host = ENET_HOST_ANY;
  address.port = port;

  ENetHost *server = enet_host_create(&address, 32, 2, 0, 0);

  if (!server)
  {
    printf("Cannot create ENet server\n");
    return 1;
  }

  uint32_t start = enet_time_get();
  uint32_t last = start;
  float dt = 0.0f;
  while (true)
  {
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
          case E_CLIENT_TO_SERVER_STATE:
            on_state(event.packet);
            break;
        };
        enet_packet_destroy(event.packet);
        break;
      default:
        break;
      };
    }
    static int t = 0;
    for (const Entity &e : entities)
      for (size_t i = 0; i < server->peerCount; ++i)
      {
        ENetPeer *peer = &server->peers[i];
        auto search = controlledMap.find(e.eid);
        if (peer->state == ENET_PEER_STATE_CONNECTED && search->second != peer)
          send_snapshot(peer, e.eid, e.x, e.y, e.r);
      }
    update_ai(dt);
    collide();
    start = enet_time_get();
    dt = (start - last) / 1000.0f;
    last = start;
    usleep(10000);
  }

  enet_host_destroy(server);

  atexit(enet_deinitialize);
  return 0;
}


