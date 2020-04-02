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
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <strings.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdio.h>
#include "JsonFactory.h"
#include "JsonException.h"
#include "Utils.h"

bool Utils::isNewWDogAvailable() {
    int len = 0;
    char chunk[BUFFSIZE];
    FILE *fpSrc = NULL, *fpDst = NULL;

    std::string strNewWDog  = std::string(TECHNO_SPURS_ROOT_PATH) + std::string(TECHNO_SPURS_WDOG_FILE);
    fpSrc = fopen(strNewWDog.c_str(), "rb");
    if(!fpSrc) return false;

    //	Delete the existing one, otherwise fopen will fail
    unlink(TECHNO_SPURS_WDOG_ROOT);
    fpDst = fopen(TECHNO_SPURS_WDOG_ROOT, "wb");

    while((len = fread(chunk, 1, BUFFSIZE, fpSrc)) >= BUFFSIZE) {
        fwrite(chunk, 1, len, fpDst);
    }
    fwrite(chunk, 1, len, fpDst);
    fclose(fpSrc); fclose(fpDst);

    chmod(TECHNO_SPURS_WDOG_ROOT, S_IRWXU | S_IRGRP | S_IROTH);

    //	Delete it so that on next reboot, it will not copy
    unlink(strNewWDog.c_str());
    return true;
}

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

void Utils::sendPacket(std::string strPacket, int port, std::string strToIp) {
    struct hostent *he;

    struct sockaddr_in their_addr;
    int sockfd  = socket(AF_INET, SOCK_DGRAM, 0);
    he  = gethostbyname(strToIp.c_str());
    their_addr.sin_family   = AF_INET;
    their_addr.sin_port     = htons(port);
    their_addr.sin_addr     = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);
    sendto(sockfd, strPacket.c_str(), strPacket.length(), 0,
             (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
    close(sockfd);
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

struct in_addr Utils::getIpv4IpOfEthIF(std::string if_prefix) {
    struct ifaddrs *ifAddrs = NULL, *ifIter = NULL;
    std::string ifName;
    struct in_addr myip = {0};

    //  Gets the ip address of all the interfaces
    getifaddrs(&ifAddrs);

    for(ifIter  = ifAddrs; NULL != ifIter; ifIter = ifIter->ifa_next) {
        ifName  = ifIter->ifa_name;
        //  skip interfaces other than ipv4 and if_name
        if(ifIter->ifa_addr->sa_family == AF_INET && std::string::npos != ifName.find(if_prefix.c_str())) {
            myip    = ((struct sockaddr_in *)ifIter->ifa_addr)->sin_addr;
            break;
        }
    }
    if(NULL != ifAddrs) freeifaddrs(ifAddrs);
    return myip;
}

std::string Utils::getDotFormattedIp(struct in_addr ip) {
    //char szAddr[INET_ADDRSTRLEN];
    struct in_addr ipAddr;
    ipAddr = ip;
    //inet_ntoa(AF_INET, &ipAddr, szAddr, INET_ADDRSTRLEN);
    char* ipString = inet_ntoa(ip);
    return std::string(ipString);
}

