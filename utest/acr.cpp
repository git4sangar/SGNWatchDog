//sgn

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <strings.h>

//  socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define UDP_Tx_PORT (4951)
void sendPacket(int port, std::string strPacket) {
    struct hostent *he;

    struct sockaddr_in their_addr;
    int sockfd  = socket(AF_INET, SOCK_DGRAM, 0);
    he  = gethostbyname("localhost");
    their_addr.sin_family   = AF_INET;
    their_addr.sin_port     = htons(port);
    their_addr.sin_addr     = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);
    sendto(sockfd, strPacket.c_str(), strPacket.length(), 0,
             (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
}
int main() {
	while(true) {
		std::stringstream ss;
		ss << "{ \"command\":\"heart_beat\", \"run_command\":\"/home/tstone10/sgn/bkup/private/projs/SGNBarc/technospurs/SmartMeter/acr\", \"version\":1, \"pid_of_process\":"
			<< getpid() << ",\"process_name\":\"acr\" }";
		std::cout << ss.str() << std::endl;

		sendPacket(UDP_Tx_PORT, ss.str());
		sleep(10);
	}
	return 0;
}
