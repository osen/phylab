/***************************************************************
 * phy.h
 *
 * 1.0.3
 *
 * The client side component of PhyLab.
 *
 * In order to keep to a single header implementation, the C++
 * template system has been utilised to avoid errors due to
 * multiply defined symbols at the linker stage.
 * 
 * In order to avoid issues with objects being copied in a
 * potentially incorrect manner, the PIMPL pattern has been
 * utilised.
 *
 ***************************************************************/

#ifndef PHY_PHY_H
#define PHY_PHY_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include <memory>
#include <iostream>
#include <sstream>
#include <string>

#define SERVER_HOST "localhost"
#define SERVER_PORT 1987

#define PHY_TRIANGLE 3
#define PHY_SQUARE 4
#define PHY_CUBE 14
#define PHY_SPHERE 19

#pragma comment(lib, "Ws2_32.lib")

namespace phy
{

template <typename T>
struct Object
{
  Object()
  {
    impl = std::make_shared<Impl>();
  }

  void setPosition(float x, float y, float z)
  {
    impl->setPosition(x, y, z);
  }

  void setRotation(float x, float y, float z)
  {
    impl->setRotation(x, y, z);
  }

  void setScale(float x, float y, float z)
  {
    impl->setScale(x, y, z);
  }

  void setColor(float r, float g, float b)
  {
    impl->setColor(r, g, b);
  }

  void setType(int type)
  {
    impl->setType(type);
  }

private:
  struct Connection
  {
    bool initialized;
    bool connected;
    int id;
    WSADATA wsaData;
    SOCKET socket;
  };

  struct Impl
  {
    Impl()
    {
      initialize(connection);
      //std::cout << "Impl" << std::endl;
      //std::cout << "Id: " << connection.id << std::endl;
    }

    ~Impl()
    {
      //std::cout << "~Impl" << std::endl;
    }

    void setPosition(float x, float y, float z)
    {
      std::stringstream ss;

      ss << "position " << connection.id <<
        " " << x << " " << y << " " << z << std::endl;

      send(ss.str());
    }

    void setRotation(float x, float y, float z)
    {
      std::stringstream ss;

      ss << "rotation " << connection.id <<
        " " << x << " " << y << " " << z << std::endl;

      send(ss.str());
    }

    void setScale(float x, float y, float z)
    {
      std::stringstream ss;

      ss << "scale " << connection.id <<
        " " << x << " " << y << " " << z << std::endl;

      send(ss.str());
    }

    void setType(int type)
    {
      std::stringstream ss;

      ss << "type " << connection.id << " " << type << std::endl;

      send(ss.str());
    }

    void setColor(float r, float g, float b)
    {
      std::stringstream ss;

      ss << "color " << connection.id <<
        " " << r << " " << g << " " << b << " 1" << std::endl;

      send(ss.str());
    }

  private:
    static void initialize(Connection& connection)
    {
      static Connection sc = {0};

      sc.id++;
      connection = sc;
      if(sc.initialized) return;
      sc.initialized = true;

      //std::cout << "initialize" << std::endl;
      int res = WSAStartup(MAKEWORD(2,2), &sc.wsaData);
      // TODO: WSACleanup();

      if(res != 0)
      {
        return;
      }

      addrinfo hints = {0};
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;

      std::stringstream pss;
      pss << SERVER_PORT;
      addrinfo *result = NULL;
      res = getaddrinfo(SERVER_HOST, pss.str().c_str(), &hints, &result);

      if(res != 0)
      {        
        return;
      }

      sc.socket = INVALID_SOCKET;

      for(addrinfo *ptr = result; ptr != NULL;ptr = ptr->ai_next)
      {
        sc.socket = socket(ptr->ai_family, ptr->ai_socktype,  ptr->ai_protocol);

        if(sc.socket == INVALID_SOCKET)
        {
          return;
        }

        res = connect(sc.socket, ptr->ai_addr, (int)ptr->ai_addrlen);

        if(res == SOCKET_ERROR)
        {
          closesocket(sc.socket);
          sc.socket = INVALID_SOCKET;

          continue;
        }

        break;
      }

      freeaddrinfo(result);

      if(sc.socket == INVALID_SOCKET)
      {
        return;
      }

      std::cout << "PhyLab: Connected [" << SERVER_HOST << ":" << SERVER_PORT << "]" << std::endl;
      sc.connected = true;
      connection = sc;
    }

    void send(std::string message)
    {
      //std::cout << message;
      if(connection.connected == false) return;

      while(message.length() > 0)
      {
        int res = ::send(connection.socket, message.c_str(), message.length(), 0);

        if(res == SOCKET_ERROR)
        {
          std::cout << "PhyLab: Disconnected" << std::endl;
          connection.connected = false;

          return;
        }

        if(res < 1) continue;

        message = message.substr(res);
      }
    }

    Connection connection;
  };

  std::shared_ptr<Impl> impl;
};

}

typedef phy::Object<unsigned char> Object;

#endif
