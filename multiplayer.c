#include "multiplayer.h"
#include "globals.h"

char multiplayer_create_server(s_SocketConnection *socketWrapper) {
	IPaddress ipAddress;
	int success = SDLNet_ResolveHost(&ipAddress, 0, MULTIPLAYER_PORT);

	if (success == -1) {
		printf("Failed to open port: %d\n", MULTIPLAYER_PORT);
		return 0;
	}

	// listen for new connections on 'port'
	socketWrapper->socket = SDLNet_TCP_Open(&ipAddress);
	if (socketWrapper->socket == 0) {
		printf("Failed to open port for listening: %d\n", MULTIPLAYER_PORT);
		printf("Error: %s\n", SDLNet_GetError());
		return 0;
	}

	return 1;
}
