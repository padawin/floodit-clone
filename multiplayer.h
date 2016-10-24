#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

#include <SDL2/SDL_net.h>

typedef enum {CLIENT, SERVER} e_SocketType;

typedef struct {
	IPaddress ipAddress;
	e_SocketType type;
	// current socket
	TCPsocket socket;
	SDLNet_SocketSet serverSocketSet;
	TCPsocket *connectedSockets;
	int nbConnectedSockets;
	int nbMaxSockets;
} s_SocketConnection;

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip);
void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber);
void multiplayer_check_connections(s_SocketConnection *socket);
void multiplayer_close_connection(TCPsocket socket);
void multiplayer_clean(s_SocketConnection *socketWrapper);

#endif
