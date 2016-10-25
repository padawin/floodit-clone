#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

#include <SDL2/SDL_net.h>

#define OK 0
#define CONNECTION_LOST 1
#define ERROR 2
#define ERROR_CHECK_SERVER 3
#define TOO_MUCH_DATA_TO_RECEIVE 4

typedef enum {CLIENT, SERVER} e_SocketType;

typedef struct {
	IPaddress ipAddress;
	e_SocketType type;
	// current socket
	TCPsocket socket;
	SDLNet_SocketSet socketSet;
	TCPsocket *connectedSockets;
	int nbConnectedSockets;
	int nbMaxSockets;
} s_SocketConnection;

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip);
void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber);
void multiplayer_initClient(s_SocketConnection *socketWrapper);
void multiplayer_accept_client(s_SocketConnection *socket);
void multiplayer_check_clients(s_SocketConnection *socket);
char multiplayer_check_server(s_SocketConnection *socketWrapper);
void multiplayer_close_connection(TCPsocket socket);
void multiplayer_clean(s_SocketConnection *socketWrapper);

#endif
