#ifndef __NET__
#define __NET__

#include <ifaddrs.h>

int net_getIPs(struct ifaddrs **ifap);
struct ifaddrs *net_getNextIP(struct ifaddrs **ifap, char **interface, char **addr);
void net_freeIfAddr(struct ifaddrs *ifap);

#endif
