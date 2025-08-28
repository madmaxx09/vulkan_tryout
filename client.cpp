	#include "client.hpp"
	#include <iostream>

	

	namespace wind
	{
		Client::Client(std::string &roomNumber)
		{
			std::cout << "CLient created" << std::endl;
			clientSocket = socket(AF_INET, SOCK_STREAM, 0);
			serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(port);
			
			if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
			{
				std::cout << "Connect failed on clientSocket" << std::endl;
				return;
			}
			
			std::cout << "TCP connection succesfull" << std::endl;

			if (send(clientSocket, roomNumber.c_str(), roomNumber.length(), 0) == -1)
			{
				std::cout << "Failed to send room number" << std::endl;
			}
			std::cout << "Room number sent" << std::endl;

			// while(1)
			// {
			// 	if (recv)
			// }



		}
		
		Client::~Client()
		{
		}

		void Client::connectUdpSocket()
		{

		}

		void Client::Send(Player &player)
		{
			//player.physicalEntity.transform.translation
		}

		void Client::Recv()
		{

		}
	}
