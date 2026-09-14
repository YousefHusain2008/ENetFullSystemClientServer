// ==========================================
// File:       Main.cpp
// Project:    eNetTraining1
// Author:     KING
// Created:    9/7/2026 5:58:38 PM
// Copyright:  © 2026 All rights reserved.
// ==========================================

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>
#include <string_view>

#include "enet.h"

#define newline "\n"

#ifdef _DEBUG
#define log(msg) std::cout << msg << newline
#define log2(first,second) std::cout << first << second << newline
#define log3(first, second, third) std::cout << first << second<< third<<newline
#define Debug(func) func
#else
#define log(msg)
#define log2(first,second)
#define log3(first, second, third)
#define Debug(func)
#endif

using std::cin;
using std::thread;
using std::atomic;

#define MAX_CLIENTS 32

static atomic<bool> EnterKeyPressed(false);

void ENetThread() {
	if (enet_initialize()) {
		log2("ENet", "ENet didnt initialize successfully!");
		return;
	}

	ENetAddress address = { ENET_HOST_ANY, 7777 };

	ENetHost* server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0, 0); // Max: 1048576

	if (server == nullptr) {
		log2("[ENet]: ", "ENet server didnt get created!");
		enet_deinitialize();
		return;
	}

	log2("[ENet]: ", "Server is up & running on port 7777");

	ENetEvent event;
	while (true) {
		while (enet_host_service(server, &event, 10) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				log2("[Server]: ", "Client connected!");
				auto eventpeeraddress = event.peer->address;

				char ip_buffer[64];
				
				enet_address_get_ip(&eventpeeraddress, ip_buffer, sizeof(ip_buffer));
				log3("[Server]: ", "The ip address of the client: ", ip_buffer);
				log3("[Server]: ", "The port: ", eventpeeraddress.port);

				const char connectionConfirmationMessage[] = "Connected to the server successfully!";
				ENetPacket* packet = enet_packet_create(connectionConfirmationMessage, sizeof(connectionConfirmationMessage), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(event.peer, 0, packet);
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: {
				std::string_view recievedText((char*)event.packet->data, event.packet->dataLength);
				log3("[Server]: ", "Recieved from client: ", recievedText);
				enet_packet_destroy(event.packet);

				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				log2("[Server]: ", "Client Disconnected!");
				char ip_buffer[64];
				enet_address_get_ip(&event.peer->address, ip_buffer, sizeof(ip_buffer));
				log3("[Server]: ", "The ip address: ", ip_buffer);
				log3("[Server]: ", "The port: ", event.peer->address.port);
				break;
			}
			case ENET_EVENT_TYPE_NONE:
				break;
			}
		}
		if (EnterKeyPressed) {
			for (int i = 0;i < server->peerCount; i++) {
				enet_peer_disconnect(&server->peers[i], 0);
			}
			break;
		}
	}
	enet_host_flush(server);

	enet_host_destroy(server);
	log2("[ENet]: ", "Server destroyed successfully");
	enet_deinitialize();
	log2("[ENet]: ", "ENet deinitialize");
}
#ifdef _WIN32
#include <windows.h>
inline void init_console_utf8() {
	SetConsoleOutputCP(CP_UTF8);   // console output uses UTF-8
	SetConsoleCP(CP_UTF8);         // console input uses UTF-8
}
#else
inline void init_console_utf8() {}
#endif
int main()
{
	init_console_utf8();

	thread eNetThread(ENetThread);

	log("Press Enter to stop the server & quit!");
	cin.get();
	EnterKeyPressed = true;

	eNetThread.join();

	system("pause");
	return 0;
}
