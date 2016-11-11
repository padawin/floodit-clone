#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

#include <SDL2/SDL_net.h>

#define OK 0
#define CONNECTION_LOST 1
#define ERROR 2
#define ERROR_CHECK_SERVER 3
#define TOO_MUCH_DATA_TO_RECEIVE 4
#define MESSAGE_RECEIVED 5

#define TCP_PACKET_MAX_SIZE 198
#define TCP_PACKET_DATA_MAX_SIZE 196

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

typedef struct {
	uint8_t type;
	// a maximum of 196 bytes can be sent (whole grid),
	// so the size is stored in a uint8_t
	uint8_t size;
	// 196 * 8 = 1568
	char data[196];
} s_TCPpacket;

char multiplayer_create_connection(s_SocketConnection *socketWrapper, const char* ip);
void multiplayer_initHost(s_SocketConnection *socketWrapper, int playersNumber);
void multiplayer_initClient(s_SocketConnection *socketWrapper);
void multiplayer_accept_client(s_SocketConnection *socket);
char multiplayer_check_clients(
	s_SocketConnection *socketWrapper,
	s_TCPpacket *packet,
	int *fromIndex,
	char removeDisconnected
);
char multiplayer_check_server(s_SocketConnection *socketWrapper, s_TCPpacket *packet);
void multiplayer_close_connection(TCPsocket socket);
void multiplayer_clean(s_SocketConnection *socketWrapper);
char multiplayer_is_room_full(s_SocketConnection socketWrapper);
void multiplayer_broadcast(s_SocketConnection socketWrapper, s_TCPpacket packet);
void multiplayer_send_message(s_SocketConnection socketWrapper, int socketIndex, s_TCPpacket packet);
void multiplayer_client_leave(s_SocketConnection *socketWrapper);
void multiplayer_close_client(s_SocketConnection *socketWrapper, int socket);

#endif
