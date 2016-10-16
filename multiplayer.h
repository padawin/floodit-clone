#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

#include <SDL2/SDL_net.h>

typedef enum {CLIENT, SERVER} e_SocketType;

typedef struct {
	IPaddress ipAddress;
	e_SocketType type;
	// current socket
	TCPsocket socket;
	// socket which it is connected to
	TCPsocket connection;
} s_SocketConnection;

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip);
void multiplayer_check_connections(s_SocketConnection *socket);

#endif
