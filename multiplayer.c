#include "multiplayer.h"
#include "globals.h"
#include "string.h"
#include <ifaddrs.h>
#include "net.h"

#define SIZE_PING_PACKET 16

char _receiveMessage(TCPsocket socket, s_TCPpacket *packet);
void _removeDisconnectedSockets(s_SocketConnection *socketWrapper);
char _computePacket(s_TCPpacket packet, char *message);
void _parsePacket(s_TCPpacket *packet, char *message);
char _ping_server(s_SocketConnection *socketWrapper);

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip, E_ConnectionType type) {
	int success = SDLNet_ResolveHost(&socketWrapper->ipAddress, ip, MULTIPLAYER_PORT);

	if (success == -1) {
		printf("Failed to open port: %d\n", MULTIPLAYER_PORT);
		return 0;
	}

	if (type == TCP) {
		// listen for new connections on 'port'
		socketWrapper->socket = SDLNet_TCP_Open(&socketWrapper->ipAddress);
		if (socketWrapper->socket == 0) {
			printf("Failed to open port for listening: %d\n", MULTIPLAYER_PORT);
			printf("Error: %s\n", SDLNet_GetError());
			return 0;
		}
	}
	else if (type == PING) {
		printf("Opening port\n");
		// Sets our socket with our local port
		socketWrapper->pingSocket = SDLNet_UDP_Open(MULTIPLAYER_PORT_CLIENT);

		if (socketWrapper->pingSocket == 0) {
			printf("Error opening socket client\n");
			return 0;
		}

		printf("Set remote IP %s and port %d\n", ip, MULTIPLAYER_PORT_SERVER);
		if (SDLNet_ResolveHost(&socketWrapper->ipAddress, ip, MULTIPLAYER_PORT_SERVER) == -1) {
			printf("SDLNet_ResolveHost failed\n");
		}

		_ping_server(socketWrapper);
	}

	return 1;
}

char _ping_server(s_SocketConnection *socketWrapper) {
	printf("Creating packet\n");
	// Allocate memory for the packet
	UDPpacket* packet = SDLNet_AllocPacket(SIZE_PING_PACKET);
	if (packet == 0) {
		printf("SDLNet_AllocPacket failed : %s\n", SDLNet_GetError());
		return 0;
	}

	// Set the destination host and port
	// We got these from calling SetIPAndPort()
	packet->address.host = socketWrapper->ipAddress.host;
	packet->address.port = socketWrapper->ipAddress.port;

	struct ifaddrs *ifap;
	net_getIPs(&ifap);
	char *interface, *address;
	net_getNextIP(&ifap, &interface, &address);
	memcpy(packet->data, address, SIZE_PING_PACKET);
	packet->len = SIZE_PING_PACKET;

	// SDLNet_UDP_Send returns number of packets sent. 0 means error
	if (SDLNet_UDP_Send(socketWrapper->pingSocket, -1, packet) == 0) {
		printf("SDLNet_UDP_Send failed : %s\n", SDLNet_GetError());
		SDLNet_FreePacket(packet);
		return 0;
	}

	SDLNet_FreePacket(packet);
	return 1;
}

void multiplayer_initClient(s_SocketConnection *socketWrapper) {
	socketWrapper->nbConnectedSockets = 0;
	socketWrapper->socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socketWrapper->socketSet, socketWrapper->socket);
	socketWrapper->type = CLIENT;
}

void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber) {
	socketWrapper->socketSet = SDLNet_AllocSocketSet(playersNumber);
	socketWrapper->nbMaxSockets = playersNumber - 1;
	socketWrapper->nbConnectedSockets = 0;
	long unsigned nmemb = (long unsigned) playersNumber;
	socketWrapper->connectedSockets = (TCPsocket *) malloc(nmemb * sizeof(TCPsocket));
	socketWrapper->type = SERVER;
}

void multiplayer_accept_client(s_SocketConnection *socketWrapper) {
	if (socketWrapper->nbConnectedSockets >= socketWrapper->nbMaxSockets) {
		return;
	}

	TCPsocket socket = SDLNet_TCP_Accept(socketWrapper->socket);
	if (socket != 0) {
		SDLNet_TCP_AddSocket(socketWrapper->socketSet, socket);
		socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets] = socket;
		socketWrapper->nbConnectedSockets++;
	}
}

void multiplayer_reject_clients(s_SocketConnection *socketWrapper, int messageType) {
	// Accept the client connection to clear it from the incoming connections list
	TCPsocket socket = SDLNet_TCP_Accept(socketWrapper->socket);
	if (socket == 0) {
		return;
	}

	// Send a message to the client saying the server is full and to tell the
	// client to go away
	s_TCPpacket packet;
	packet.type = (unsigned char) messageType;
	packet.size = 0;
	char message[TCP_PACKET_MAX_SIZE];
	_computePacket(packet, message);
	SDLNet_TCP_Send(socket, message, TCP_PACKET_MAX_SIZE);

	// Shutdown, disconnect, and close the socket to the client
	SDLNet_TCP_Close(socket);
}

char multiplayer_check_clients(
	s_SocketConnection *socketWrapper,
	s_TCPpacket *packet,
	int *fromIndex,
	char removeDisconnected
) {
	int numSockets = SDLNet_CheckSockets(socketWrapper->socketSet, 0);
	if (numSockets == -1) {
		printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
	}
	else if (numSockets > 0) {
		int socket = 0;
		while (socket < socketWrapper->nbConnectedSockets) {
			// if this socket has nothing to say, skip it
			if (!SDLNet_SocketReady(socketWrapper->connectedSockets[socket])) {
				++socket;
				continue;
			}

			char responseCode = _receiveMessage(
				socketWrapper->connectedSockets[socket],
				packet
			);

			if (responseCode == CONNECTION_LOST) {
				multiplayer_close_client(socketWrapper, socket);
				if (removeDisconnected) {
					--socketWrapper->nbConnectedSockets;
					socketWrapper->connectedSockets[socket] = socketWrapper->connectedSockets[
						socketWrapper->nbConnectedSockets
					];
					socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets] = 0;
				}
			}
			else if (responseCode != ERROR) {
				*fromIndex = socket + 1;
				++socket;
				return responseCode;
			}
		}
	}

	return OK;
}

void multiplayer_close_client(s_SocketConnection *socketWrapper, int socket) {
	SDLNet_TCP_DelSocket(
		socketWrapper->socketSet,
		socketWrapper->connectedSockets[socket]
	);
	SDLNet_TCP_Close(socketWrapper->connectedSockets[socket]);
	socketWrapper->connectedSockets[socket] = 0;
}

char multiplayer_check_server(s_SocketConnection *socketWrapper, s_TCPpacket *packet) {
	int serverActive = SDLNet_CheckSockets(socketWrapper->socketSet, 0);
	if (serverActive == -1) {
		// an error occured, it can be read in SDLNet_GetError()
		return ERROR_CHECK_SERVER;
	}
	if (serverActive > 0) {
		return _receiveMessage(socketWrapper->socket, packet);
	}

	return OK;
}

void multiplayer_clean(s_SocketConnection *socketWrapper) {
	SDLNet_TCP_Close(socketWrapper->socket);
	if (socketWrapper->pingSocket) {
		SDLNet_UDP_Close(socketWrapper->pingSocket);
	}
	socketWrapper->socket = 0;
	while (socketWrapper->nbConnectedSockets--) {
		multiplayer_close_client(
			socketWrapper,
			socketWrapper->nbConnectedSockets
		);
	}

	if (socketWrapper->connectedSockets != 0) {
		free(socketWrapper->connectedSockets);
	}
	SDLNet_FreeSocketSet(socketWrapper->socketSet);
	socketWrapper->socketSet = 0;
}

char multiplayer_is_room_full(s_SocketConnection *socketWrapper) {
	return socketWrapper->nbConnectedSockets == socketWrapper->nbMaxSockets;
}

void multiplayer_broadcast(s_SocketConnection *socketWrapper, s_TCPpacket packet) {
	int s;
	for (s = 0; s < socketWrapper->nbConnectedSockets; ++s) {
		multiplayer_send_message(socketWrapper, s, packet);
	}
}

void multiplayer_send_message(s_SocketConnection *socketWrapper, int socketIndex, s_TCPpacket packet) {
	char message[TCP_PACKET_MAX_SIZE];
	if (_computePacket(packet, message) != 0) {
		printf("Packet size too large\n");
	}
	else {
		TCPsocket socket;
		if (socketIndex == -1) {
			socket = socketWrapper->socket;
		}
		else {
			socket = socketWrapper->connectedSockets[socketIndex];
		}

		if (socket != 0) {
			SDLNet_TCP_Send(socket, message, TCP_PACKET_MAX_SIZE);
		}
	}
}

int multiplayer_get_number_clients(s_SocketConnection *socketWrapper) {
	int nb = 0, client;
	for (client = 0; client < socketWrapper->nbConnectedSockets; ++client) {
		if (socketWrapper->connectedSockets[client]) {
			++nb;
		}
	}

	return nb;
}

int multiplayer_get_next_connected_socket_index(s_SocketConnection *socketWrapper, int currentIndex) {
	int next = -1, s = currentIndex + 1;
	while (next == -1 && s < socketWrapper->nbConnectedSockets) {
		if (socketWrapper->connectedSockets[s]) {
			next = s;
			break;
		}
		++s;
	}

	return next;
}

char multiplayer_is_client_connected(s_SocketConnection *socketWrapper, int clientIndex) {
	return socketWrapper->connectedSockets[clientIndex] != 0;
}

void _parsePacket(s_TCPpacket *packet, char *message) {
	packet->type = (unsigned char) message[0];
	packet->size = (unsigned char) message[1];

	int current;
	for (current = 0; current < TCP_PACKET_DATA_MAX_SIZE; ++current) {
		if (current < packet->size) {
			packet->data[current] = message[current + 2];
		}
		else {
			packet->data[current] = '\0';
		}
	}
}

char _computePacket(s_TCPpacket packet, char *message) {
	if (packet.size > TCP_PACKET_DATA_MAX_SIZE) {
		return -1;
	}

	// packet.size does not include the header (1 char for the type + 1 char
	// for the size)
	message[0] = (signed char) packet.type;
	message[1] = (signed char) packet.size;

	int current;
	for (current = 0; current < TCP_PACKET_DATA_MAX_SIZE; ++current) {
		if (current < packet.size) {
			message[current + 2] = packet.data[current];
		}
		else {
			message[current + 2] = 0;
		}
	}

	return 0;
}

char _receiveMessage(TCPsocket socket, s_TCPpacket *packet) {
	char message[TCP_PACKET_MAX_SIZE];
	int byteCount = SDLNet_TCP_Recv(socket, message, TCP_PACKET_MAX_SIZE);

	if (byteCount < 0) {
		// an error occured, it can be read in SDLNet_GetError()
		return ERROR;
	}
	else if (byteCount == 0) {
		return CONNECTION_LOST;
	}
	else {
		if (byteCount > TCP_PACKET_MAX_SIZE) {
			return TOO_MUCH_DATA_TO_RECEIVE;
		}
		else {
			_parsePacket(packet, message);
			return MESSAGE_RECEIVED;
		}
	}
}
