#include "multiplayer.h"
#include "globals.h"
#include "string.h"

char _receiveMessage(TCPsocket socket, s_TCPpacket *packet);
void _removeDisconnectedSockets(s_SocketConnection *socketWrapper);
char _computePacket(s_TCPpacket packet, char *message);
void _parsePacket(s_TCPpacket *packet, char *message);

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip) {
	int success = SDLNet_ResolveHost(&socketWrapper->ipAddress, ip, MULTIPLAYER_PORT);

	if (success == -1) {
		printf("Failed to open port: %d\n", MULTIPLAYER_PORT);
		return 0;
	}

	// listen for new connections on 'port'
	socketWrapper->socket = SDLNet_TCP_Open(&socketWrapper->ipAddress);
	if (socketWrapper->socket == 0) {
		printf("Failed to open port for listening: %d\n", MULTIPLAYER_PORT);
		printf("Error: %s\n", SDLNet_GetError());
		return 0;
	}

	return 1;
}

void multiplayer_initClient(s_SocketConnection *socketWrapper) {
	socketWrapper->socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socketWrapper->socketSet, socketWrapper->socket);
	socketWrapper->type = CLIENT;
}

void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber) {
	socketWrapper->socketSet = SDLNet_AllocSocketSet(playersNumber);
	socketWrapper->nbMaxSockets = playersNumber - 1;
	socketWrapper->nbConnectedSockets = 0;
	socketWrapper->connectedSockets = (TCPsocket *) malloc(playersNumber * sizeof(TCPsocket));
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
	while (socketWrapper->nbConnectedSockets--) {
		SDLNet_TCP_DelSocket(
			socketWrapper->socketSet,
			socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets]
		);
		SDLNet_TCP_Close(
			socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets]
		);
	}

	if (socketWrapper->connectedSockets != 0) {
		free(socketWrapper->connectedSockets);
	}
	SDLNet_FreeSocketSet(socketWrapper->socketSet);
	socketWrapper->socketSet = NULL;
}

char multiplayer_is_room_full(s_SocketConnection socketWrapper) {
	return socketWrapper.nbConnectedSockets == socketWrapper.nbMaxSockets;
}

void multiplayer_broadcast(s_SocketConnection socketWrapper, s_TCPpacket packet) {
	int s;
	for (s = 0; s < socketWrapper.nbConnectedSockets; ++s) {
		multiplayer_send_message(socketWrapper, s, packet);
	}
}

void multiplayer_send_message(s_SocketConnection socketWrapper, int socketIndex, s_TCPpacket packet) {
	char message[TCP_PACKET_MAX_SIZE];
	if (_computePacket(packet, message) != 0) {
		printf("Packet size too large\n");
	}
	else {
		TCPsocket socket;
		if (socketIndex == -1) {
			socket = socketWrapper.socket;
		}
		else {
			socket = socketWrapper.connectedSockets[socketIndex];
		}
		SDLNet_TCP_Send(socket, message, TCP_PACKET_MAX_SIZE);
	}
}

void _parsePacket(s_TCPpacket *packet, char *message) {
	packet->type = message[0];
	packet->size = message[1];

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
	message[0] = packet.type;
	message[1] = packet.size;

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

void multiplayer_client_leave(s_SocketConnection *socketWrapper) {
	SDLNet_TCP_DelSocket(
		socketWrapper->socketSet,
		socketWrapper->socket
	);
	SDLNet_TCP_Close(socketWrapper->socket);
}
