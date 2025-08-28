#pragma once

#include "player.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>

namespace wind
{
	
	class Client
	{
		public:
		Client(std::string &roomNumber);
		~Client();
		
		
			const char* HMAC_SECRET_KEY = "MAXDOR\0";
			static constexpr int port = 666;

			void Send(Player &player);
			void Recv();
			void connectUdpSocket();

		private:
			sockaddr_in serverAddr;	
			int clientSocket;
			int clientUdpSocket;		
	};
	
	
}
