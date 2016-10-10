#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

#include <SDL2/SDL_net.h>

typedef enum {CLIENT, SERVER} e_SocketType;

typedef struct {
	e_SocketType type;
	// current socket
	TCPsocket socket;
	// socket which it is connected to
	TCPsocket connection;
} s_SocketConnection;

char multiplayer_create_server();

#endif
