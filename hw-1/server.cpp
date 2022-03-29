#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include "socket_tools.h"


struct UserInfo {
    std::string name;
    struct sockaddr from;
    uint32_t addrlen;
};

std::vector<UserInfo> info;

int tryFindUser(const std::string &name)
{
  for (int i = 0; i < info.size(); ++i)
  {
    if (name == info[i].name)
      return i;
  }
  return -1;
}

void sendMessageFromUser(int sfd, const std::string &name, const std::string &message)
{
  for (int i = 0; i < info.size(); ++i)
  {
    if (name != info[i].name)
    {
      std::string output = name + ": " + message;
      sendto(sfd, output.c_str(), output.size(), 0, &info[i].from, info[i].addrlen);
    }
  }
}

int main(int argc, const char **argv)
{
  const char *port = "2022";

  int sfd = create_dgram_socket(nullptr, port, nullptr);

  if (sfd == -1)
    return 1;
  printf("listening!\n");

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd + 1, &readSet, NULL, NULL, &timeout);


    if (FD_ISSET(sfd, &readSet))
    {
      struct sockaddr from;
      uint32_t addrlen;
      constexpr size_t buf_size = 1000;
      static char buffer[buf_size];
      memset(buffer, 0, buf_size);
      addrlen = sizeof(from);
      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, &from, &addrlen);
      if (numBytes > 0)
      {
        char* name;
        char* code;
        char* save;
        code = strtok_r(buffer, " ", &save);
        name = strtok_r(nullptr, " ", &save);
        if (buffer[0] == '0')
        {
          int idx = tryFindUser(std::string(name));
          if (idx == -1)
          {
            printf("Add new user: %s\n", name);
            info.push_back({ std::string(name), from, addrlen });
          }
          else
          {
            printf("User info updated: %s\n", name);
            info[idx].from = from;
            info[idx].addrlen = addrlen;
          }
        }
        else if (buffer[0] == '1')
        {
          printf("From: %s get message: %s \n", name, save);
          sendMessageFromUser(sfd, std::string(name), save);
        }
        else if (buffer[0] == '2')
        {
          printf("Get keep alive packet from: %s\n", name);
        }
        else
        {
          printf("Read invalid message: %s\n", buffer);
        }

        //printf("%s %d \n", buffer, addrlen); // assume that buffer is a string
        //memmove(buffer, buffer + 2, strlen(buffer));
        //printf("new buffer: %s\n", buffer);
      }

    }
  }
  return 0;
}
