#include "multiplayer.h"
#include "globals.h"
#include "string.h"

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

void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber) {
	socketWrapper->socketSet = SDLNet_AllocSocketSet(playersNumber);
	socketWrapper->nbMaxSockets = playersNumber;
	socketWrapper->nbConnectedSockets = 0;
	socketWrapper->connectedSockets = (TCPsocket *) malloc(playersNumber * sizeof(TCPsocket));
}

void multiplayer_accept_client(s_SocketConnection *socketWrapper) {
	if (socketWrapper->nbConnectedSockets >= socketWrapper->nbMaxSockets) {
		return;
	}

	TCPsocket socket = SDLNet_TCP_Accept(socketWrapper->socket);
	if (socket != 0) {
		printf("Client found, send him a message\n");
		const char *message = "Hello World\n";
		SDLNet_TCP_Send(socket, message, strlen(message) + 1);
		SDLNet_TCP_AddSocket(socketWrapper->socketSet, socket);
		socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets] = socket;
		socketWrapper->nbConnectedSockets++;
	}
}

void multiplayer_check_clients(s_SocketConnection *socketWrapper) {
}

char multiplayer_check_server(s_SocketConnection *socketWrapper) {
}

void multiplayer_close_connection(TCPsocket socket) {
	SDLNet_TCP_Close(socket);
}

void multiplayer_clean(s_SocketConnection *socketWrapper) {
	multiplayer_close_connection(socketWrapper->socket);
	while (socketWrapper->nbConnectedSockets--) {
		SDLNet_TCP_DelSocket(
			socketWrapper->socketSet,
			socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets]
		);
		SDLNet_TCP_Close(
			socketWrapper->connectedSockets[socketWrapper->nbConnectedSockets]
		);
	}

	free(socketWrapper->connectedSockets);
	SDLNet_FreeSocketSet(socketWrapper->socketSet);
	socketWrapper->socketSet = NULL;
}
