/*sgn
 * Utils.cpp
 *
 *  Created on: 28-Mar-2020
 *      Author: tstone10
 */

//  socket
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <strings.h>
#include "JsonFactory.h"
#include "JsonException.h"
#include "Utils.h"

std::string Utils::prepareLogPacket(std::string strLog) {
    std::string strPkt;
    JsonFactory jsRoot;
    try {
        jsRoot.addStringValue("command", "upload_logs");
        jsRoot.addStringValue("log_data", strLog);
        strPkt = jsRoot.getJsonString();
    } catch(JsonException &je) {}
    return strPkt;
}

void Utils::sendPacket(int port, std::string strPacket) {
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

int Utils::prepareRecvSock(int iPort) {
    int sockfd, optval;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    //  Prepare UDP
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serveraddr.sin_port         = htons((unsigned short)iPort);

    //  Bind the same
    bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    return sockfd;
}
