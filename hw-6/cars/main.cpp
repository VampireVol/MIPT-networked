// initial skeleton is a clone from https://github.com/jpcy/bgfx-minimal-example
//
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/timer.h>
#include <debugdraw/debugdraw.h>
#include <functional>
#include "app.h"
#include <enet/enet.h>
#include <math.h>
#include <sstream>
#include <iostream>

//for scancodes
#include <GLFW/glfw3.h>


#include <vector>
#include "entity.h"
#include "protocol.h"

struct Snapshot
{
  Snapshot() : x(0.0f), y(0.0f), ori(0.0f), t(0) {}
  Snapshot(const Entity &e, uint32_t t) : x(e.x), y(e.y), ori(e.ori), t(t) {}
  float x;
  float y;
  float ori;
  uint32_t t;
};

struct Input
{
  float thr;
  float steer;
  float speed;
  float dt;
  float x;
  float y;
  float ori;
  uint32_t t;
};

static std::vector<Entity> entities;
static uint16_t my_entity = invalid_entity;
static std::vector<std::vector<Snapshot>> history;
static std::vector<Input> inputHistroy;
static int offset = 0;
static Snapshot moveTo;
static int port;

void addSnapshot(uint16_t eid, const Entity &e, uint32_t t)
{
  if (history[eid].size() > 10)
    history[eid].erase(history[eid].begin());
  history[eid].push_back(Snapshot(e, t));
}

int findHistoryIdx(uint32_t t)
{
  for (int i = 0; i < inputHistroy.size(); ++i)
  {
    if (inputHistroy[i].t >= t)
      return i;
  }
  return -1;
}

void checkOffset(float x, float y, float ori, uint32_t t)
{
  float eps = 0.001f;
  int idx = findHistoryIdx(t);
  if (idx == -1)
  {
    inputHistroy.clear();
    return;
  }
  Input ih = inputHistroy[idx];
  if (abs(ih.x - x) > eps || abs(ih.y - y) > eps || abs(ih.ori - ori) > eps)
  {
    offset = 5;
    Entity e;
    e.x = x;
    e.y = y;
    e.ori = ori;
    e.speed = ih.speed;
    for (int i = idx; i < inputHistroy.size(); ++i)
    {
      e.steer = inputHistroy[i].steer;
      e.thr = inputHistroy[i].thr;
      simulate_entity(e, inputHistroy[i].dt);
    }
    Input last = inputHistroy.back();
    moveTo.x = (e.x - last.x) / offset;
    moveTo.y = (e.y - last.y) / offset;
    moveTo.ori = (e.ori - last.ori) / offset;
  }
  inputHistroy.clear();
}

void getInterpolate(uint16_t uid, float &x, float &y, float &ori)
{
  const double freq = double(bx::getHPFrequency());
  uint32_t now = enet_time_get() - 250;
  uint32_t idx = 1;
  if (history[uid].size() == 1)
  {
    x = history[uid][0].x;
    y = history[uid][0].y;
    ori = history[uid][0].ori;
    return;
  }
  while (idx < history[uid].size() - 1 && now > history[uid][idx].t)
  {
    ++idx;
  }
  Snapshot l = history[uid][idx];
  Snapshot p = history[uid][idx - 1];
  x   = (l.x - p.x)     * (now - p.t) / float(l.t - p.t) + p.x;
  y   = (l.y - p.y)     * (now - p.t) / float(l.t - p.t) + p.y;
  ori = (l.ori - p.ori) * (now - p.t) / float(l.t - p.t) + p.ori;
}

void on_new_entity_packet(ENetPacket *packet)
{
  Entity newEntity; uint32_t t;
  deserialize_new_entity(packet, newEntity, t);
  // TODO: Direct adressing, of course!
  for (const Entity &e : entities)
    if (e.eid == newEntity.eid)
      return; // don't need to do anything, we already have entity
  history.push_back(std::vector<Snapshot>({ Snapshot(newEntity, t) }));
  entities.push_back(newEntity);
}

void on_set_controlled_entity(ENetPacket *packet)
{
  uint32_t t;
  deserialize_set_controlled_entity(packet, my_entity, t);
  enet_time_set(t);
}

void on_snapshot(ENetPacket *packet)
{
  uint16_t eid = invalid_entity;
  float x = 0.f; float y = 0.f; float ori = 0.f; uint32_t t = 0;
  deserialize_snapshot(packet, eid, x, y, ori, t);
  // TODO: Direct adressing, of course!
  int i = 0;
  for (Entity& e : entities)
  {
    if (e.eid == eid)
    {
      if (e.eid == my_entity)
      {
        checkOffset(x, y, ori, t);
        continue;
      }
      e.x = x;
      e.y = y;
      e.ori = ori;
      addSnapshot(i, e, t);
    }
    ++i;
  }
}

template <typename T>
void read_arg(const char* str, T& out)
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

  ENetHost *client = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!client)
  {
    printf("Cannot create ENet client\n");
    return 1;
  }

  ENetAddress address;
  enet_address_set_host(&address, "localhost");
  address.port = port;

  ENetPeer *serverPeer = enet_host_connect(client, &address, 2, 0);
  if (!serverPeer)
  {
    printf("Cannot connect to server");
    return 1;
  }

  int width = 1280;
  int height = 720;
  if (!app_init(width, height))
    return 1;
  ddInit();

  bx::Vec3 eye(0.f, 0.f, -16.f);
  bx::Vec3 at(0.f, 0.f, 0.f);
  bx::Vec3 up(0.f, 1.f, 0.f);

  float view[16];
  float proj[16];
  bx::mtxLookAt(view, bx::load<bx::Vec3>(&eye.x), bx::load<bx::Vec3>(&at.x), bx::load<bx::Vec3>(&up.x) );


  bool connected = false;
  int64_t now = bx::getHPCounter();
  int64_t last = now;
  float dt = 0.f;
  while (!app_should_close())
  {
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0)
    {
      switch (event.type)
      {
      case ENET_EVENT_TYPE_CONNECT:
        printf("Connection with %x:%u established\n", event.peer->address.host, event.peer->address.port);
        send_join(serverPeer);
        connected = true;
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        switch (get_packet_type(event.packet))
        {
        case E_SERVER_TO_CLIENT_NEW_ENTITY:
          on_new_entity_packet(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SET_CONTROLLED_ENTITY:
          on_set_controlled_entity(event.packet);
          break;
        case E_SERVER_TO_CLIENT_SNAPSHOT:
          on_snapshot(event.packet);
          break;
        };
        break;
      default:
        break;
      };
    }
    if (my_entity != invalid_entity)
    {
      bool left = app_keypressed(GLFW_KEY_LEFT);
      bool right = app_keypressed(GLFW_KEY_RIGHT);
      bool up = app_keypressed(GLFW_KEY_UP);
      bool down = app_keypressed(GLFW_KEY_DOWN);
      // TODO: Direct adressing, of course!
      for (Entity &e : entities)
        if (e.eid == my_entity)
        {
          if (offset > 0)
          {
            e.x += moveTo.x;
            e.y += moveTo.y;
            e.ori += moveTo.ori;
            --offset;
          }
          
          // Update
          float thr = (up ? 1.f : 0.f) + (down ? -1.f : 0.f);
          float steer = (left ? 1.f : 0.f) + (right ? -1.f : 0.f);
          e.thr = thr;
          e.steer = steer;
          simulate_entity(e, dt);
          inputHistroy.push_back({ thr, steer, e.speed, dt, e.x, e.y, e.ori, enet_time_get() });
          
          // Send
          send_entity_input(serverPeer, my_entity, thr, steer);
        }
    }

    app_poll_events();
    // Handle window resize.
    app_handle_resize(width, height);
    bx::mtxProj(proj, 60.0f, float(width)/float(height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
    bgfx::setViewTransform(0, view, proj);

    // This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
    const bgfx::ViewId kClearView = 0;
    bgfx::touch(kClearView);

    DebugDrawEncoder dde;

    dde.begin(0);

    int i = 0;
    for (const Entity &e : entities)
    {
      dde.push();
      float x = 0.0f;
      float y = 0.0f;
      float ori = 0.0f;
      if (e.eid == my_entity)
      {
        x = e.x;
        y = e.y;
        ori = e.ori;
      }
      else
      {
        getInterpolate(i, x, y, ori);
      }

      dde.setColor(e.color);
      bx::Vec3 dir = { cosf(ori), sinf(ori), 0.f };
      bx::Vec3 pos = { x, y, -0.01f };
      dde.drawCapsule(sub(pos, dir), add(pos, dir), 1.f);

      dde.pop();
      ++i;
    }

    dde.end();

    // Advance to next frame. Process submitted rendering primitives.
    bgfx::frame();
    const double freq = double(bx::getHPFrequency());
    int64_t now = bx::getHPCounter();
    dt = (float)((now - last) / freq);
    last = now;
  }
  ddShutdown();
  bgfx::shutdown();
  app_terminate();
  return 0;
}
