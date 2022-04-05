#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include "socket_tools.h"

enum State {
    INIT,
    VAR1,
    VAR2,
    VAR3
};

const std::string initStr = "0 ";
const std::string messageStr = "1 ";
const std::string keepAliveStr = "2 ";

std::vector<std::string> words1 { "Hello World!", "My message", "I started at " + std::to_string(time(nullptr)), "Some another message" };
std::vector<std::string> words2 { "Message #1", "Message #2", "Message #3", "Message #4", "Message #5" };
std::vector<std::string> words3 { "Test Message #1", "Test Message #2", "Test Message #3", "Test Message #4", "Test Message #5" };

int main(int argc, const char **argv)
{
  srand(time(nullptr));
  const char *port = "2022";
  State state = INIT;
  std::string userName;
  addrinfo resAddrInfo;
  int sfd = create_dgram_socket("localhost", port, &resAddrInfo);

  if (sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }

  uint32_t timeStart = time(nullptr);
  uint32_t lastSendTime = timeStart;
  uint32_t lastKeepAliveTime = timeStart;

  std::string output;
  printf("Hi what your name?\n");
  std::getline(std::cin, userName);
  printf("Choose message variant 1 or 2\n");
  std::getline(std::cin, output);
  if (output == "1")
    state = VAR1;
  else if (output == "2")
    state = VAR2;
  else {
    printf("Time to use variant 3 :)\n");
    state = VAR3;
  }
  output = initStr + userName;
  ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
  if (res == -1)
    std::cout << hstrerror(errno) << std::endl;

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd + 1, &readSet, NULL, NULL, &timeout);

    uint32_t curTime = time(nullptr);
    if (curTime - lastSendTime > 0)
    {
      lastSendTime = curTime;
      std::string input;
      std::string inputs;
      switch (state)
      {
        case VAR1:
          input = words1[rand() % words1.size()];
          output = messageStr + userName + " " + input;
          break;
        case VAR2:
          input = words2[rand() % words2.size()];
          output = messageStr + userName + " " + input;
          break;
        case VAR3:
          input = words3[rand() % words3.size()];
          output = messageStr + userName + " " + input;
          break;
      }
      printf(">%s \n", input.c_str());
      ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
      if (res == -1)
        std::cout << hstrerror(errno) << std::endl;
    }
    if (curTime - lastKeepAliveTime > 4)
    {
      lastKeepAliveTime = curTime;
      output = keepAliveStr + userName;
      ssize_t res = sendto(sfd, output.c_str(), output.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
      if (res == -1)
        std::cout << hstrerror(errno) << std::endl;
    }

    if (FD_ISSET(sfd, &readSet))
    {
      constexpr size_t buf_size = 1000;
      static char buffer[buf_size];
      std::memset(buffer, 0, buf_size);
      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, nullptr, nullptr);
      if (numBytes > 0)
      {
        printf("%s\n", buffer);
      }
    }
  }
  return 0;
}
