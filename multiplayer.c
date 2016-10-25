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

void multiplayer_initClient(s_SocketConnection *socketWrapper) {
	socketWrapper->socketSet = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(socketWrapper->socketSet, socketWrapper->socket);
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
	int serverActive = SDLNet_CheckSockets(socketWrapper->socketSet, 0);
	if (serverActive == -1) {
		// an error occured, it can be read in SDLNet_GetError()
		return ERROR_CHECK_SERVER;
	}
	if (serverActive > 0) {
		int bufferSize = 255;
		char buffer[bufferSize];

		int byteCount = SDLNet_TCP_Recv(
			socketWrapper->socket,
			buffer,
			bufferSize
		);

		if (byteCount < 0) {
			// an error occured, it can be read in SDLNet_GetError()
			return ERROR;
		}
		else if (byteCount == 0) {
			return CONNECTION_LOST;
		}
		else if (byteCount > 0 && byteCount >= bufferSize) {
			return TOO_MUCH_DATA_TO_RECEIVE;
		}
	}

	return OK;
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

	if (socketWrapper->connectedSockets != 0) {
		free(socketWrapper->connectedSockets);
	}
	SDLNet_FreeSocketSet(socketWrapper->socketSet);
	socketWrapper->socketSet = NULL;
}
