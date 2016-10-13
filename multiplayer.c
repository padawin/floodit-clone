#include "multiplayer.h"
#include "globals.h"
#include "string.h"

char multiplayer_create_server(s_SocketConnection *socketWrapper) {
	int success = SDLNet_ResolveHost(&socketWrapper->ipAddress, 0, MULTIPLAYER_PORT);

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

void multiplayer_check_connections(s_SocketConnection *socket) {
	socket->connection = SDLNet_TCP_Accept(socket->socket);
	if (socket->connection != 0) {
		printf("Client found, send him a message\n");
		const char *message = "Hello World\n";
		SDLNet_TCP_Send(socket->connection, message, strlen(message) + 1);
		SDLNet_TCP_Close(socket->connection);
	}
}
