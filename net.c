#include <arpa/inet.h>
#include "net.h"

int net_getIPs(struct ifaddrs **ifap) {
    if (getifaddrs(&*ifap) == -1) {
		return -1;
	}

    return 0;
}

struct ifaddrs *net_getNextIP(struct ifaddrs **ifap, char **interface, char **addr) {
    struct sockaddr_in *sa;

	if (*ifap) {
		if ((*ifap)->ifa_addr->sa_family != AF_INET) {
			*ifap = (*ifap)->ifa_next;
			return net_getNextIP(ifap, interface, addr);
		}

		sa = (struct sockaddr_in *) (*ifap)->ifa_addr;
		*interface = (*ifap)->ifa_name;
		*addr = inet_ntoa(sa->sin_addr);
		*ifap = (*ifap)->ifa_next;

		return *ifap;
	}

	return 0;
}

void net_freeIfAddr(struct ifaddrs *ifap) {
	freeifaddrs(ifap);
}
