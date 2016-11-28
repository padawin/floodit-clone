#include <arpa/inet.h>
#include <net/if.h>
#include "net.h"

int net_getIPs(struct ifaddrs **ifap) {
    if (getifaddrs(&*ifap) == -1) {
		return -1;
	}

    return 0;
}

char net_getNextIP(struct ifaddrs **ifap, char **interface, char **addr) {
    struct sockaddr_in *sa;
	*interface = 0;
	*addr = 0;

	if (!*ifap) {
		return 0;
	}

	if (
		(*ifap)->ifa_addr->sa_family != AF_INET
		|| IFF_LOOPBACK == ((*ifap)->ifa_flags & IFF_LOOPBACK)
	) {
		*ifap = (*ifap)->ifa_next;
		return net_getNextIP(ifap, interface, addr);
	}

	sa = (struct sockaddr_in *) (*ifap)->ifa_addr;
	*interface = (*ifap)->ifa_name;
	*addr = inet_ntoa(sa->sin_addr);
	*ifap = (*ifap)->ifa_next;

	return 1;
}

void net_freeIfAddr(struct ifaddrs *ifap) {
	freeifaddrs(ifap);
}
