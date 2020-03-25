/*sgn
 * WatchDog.cpp
 *
 *  Created on: 22-Mar-2020
 *      Author: tstone10
 */

#include <iostream>

//  socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/reboot.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <cstdlib>
#include <vector>
#include <errno.h>
#include "JsonFactory.h"
#include "WatchDog.h"

WatchDog :: WatchDog() : wLock(PTHREAD_MUTEX_INITIALIZER) {
    cmds["heart_beat"]  = HEART_BEAT;
    cmds["version_req"] = VERSION_REQ;
    cmds["reboot"]      = REBOOT_RPi;
}

WatchDog :: ~WatchDog() {
    for(Process *pProc : processes) {
        delete pProc;
    }
    processes.clear();
}

void WatchDog::parseHeartBeat(std::string strPkt) {
    JsonFactory jsRoot;
    std::string strName, strRun;
    int pid, ver;

    if(strPkt.empty()) return;

    try {
        jsRoot.setJsonString(strPkt);
        jsRoot.validateJSONAndGetValue("process_name", strName);
        jsRoot.validateJSONAndGetValue("run_command", strRun);
        jsRoot.validateJSONAndGetValue("pid_of_process", pid);
        jsRoot.validateJSONAndGetValue("version", ver);

        Process *pProc  = new Process();
        pProc->setName(strName);
        pProc->setPid(pid);
        pProc->setPet(time(0));
        pProc->setRunCmd(strRun);
        pProc->setVer(ver);

        pthread_mutex_lock(&wLock);
        std::cout << "WatchDog: Parsed JSON " << jsRoot.getJsonString() << std::endl;
        pushIfNew(pProc);   // pProc might be deleted beyond this point
        pthread_mutex_unlock(&wLock);
    } catch(JsonException &je) {}
}

void WatchDog::pushIfNew(Process *newProc) {
    if(!newProc) return;
    Process *pProc = NULL;

    //  Don't acquire mutex, it would'v been acquired by now
    std::vector<Process *> :: iterator itr;
    for(itr = processes.begin(); itr != processes.end(); itr++) {
        pProc   = *itr;
        if(0 == pProc->getName().compare(newProc->getName())) {
            pProc->setPet(newProc->getLastPet());
            pProc->setVer(newProc->getVer());
            pProc->setPid(newProc->getPid());
            delete newProc; newProc = NULL;
            break;
        }
    }
    if(newProc) processes.push_back(newProc);
}

std::string WatchDog::getAllVerAsJson() {
    std::string allProcJson;
    JsonFactory jsRoot, jsObj;
    pthread_mutex_lock(&wLock);
    for(Process *pProc : processes) {
        try {
            jsObj.addStringValue("process_name", pProc->getName());
            jsObj.addIntValue("version", pProc->getVer());
            jsRoot.addArray(jsObj);
        } catch(JsonException &je) {
            std::cout << je.what() << std::endl;
        }
    }
    allProcJson = jsRoot.getJsonString();
    pthread_mutex_unlock(&wLock);
    return allProcJson;
}

void *WatchDog::wdogThread(void *pUserData) {
    WatchDog *pThis = reinterpret_cast<WatchDog *>(pUserData);
    while(true) {
        pthread_mutex_lock(&pThis->wLock);
        for(Process *pProc : pThis->getProcesses()) {
            if(time(0) - pProc->getLastPet() > MAX_INTERVAL_SECs) {
                std::cout << "WatchDog: Killing process " << pProc->getName() << std::endl;
                kill(pProc->getPid(), 9);
                pProc->launchProc();
            }
        }
        pthread_mutex_unlock(&pThis->wLock);
        sleep(CHECK_INTERVAL_SECs);
    }
    return NULL;
}

void *WatchDog::recvThread(void *pUserData) {
    WatchDog *pThis = reinterpret_cast<WatchDog *>(pUserData);

    int sockfd, optval;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    //  Prepare UDP
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serveraddr.sin_port         = htons((unsigned short)UDP_Rx_PORT);

    //  Bind the same
    bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    struct sockaddr_in clientaddr;
    int clientlen, recvd;
    char buf[BUFSIZE];

    std::string strCmd, strPkt;
    while(true) {
        JsonFactory jsRoot;
        recvd       = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, (socklen_t*)&clientlen);
        buf[recvd]  = '\0';
        strPkt      = std::string(buf);
        try {
            jsRoot.setJsonString(std::string(buf));
            jsRoot.validateJSONAndGetValue("command", strCmd);
            std::cout << "Got command " << strCmd << std::endl;
            switch(pThis->cmds[strCmd]) {
                case HEART_BEAT:
                    pThis->parseHeartBeat(strPkt);
                    break;

                case VERSION_REQ:
                    strPkt = pThis->getAllVerAsJson();
                    pThis->sendPacket(UDP_Tx_PORT, strPkt);
                    break;

                case REBOOT_RPi:
                    sync();
                    reboot(RB_AUTOBOOT);
                    break;
            }
        } catch(JsonException &je) {
            std::cout << je.what() << std::endl;
        }
    }
    return NULL;
}

void WatchDog::sendPacket(int port, std::string strPacket) {
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

void Process::launchProc() {
    pid_t   pid;

    //  Change the user id & group id of process to 1000 (pi)
    //  We cannot run the process with "root" privileges
    chown(strRun.c_str(), 1000, 1000);

    //  In case of script, strRun -> name with absolute path of the python script with "execute permission"
    //  In case of script, strRun -> first line of the python script shall be "#!/usr/bin/python3.5" (path important)
    //  In case of executable, the name with absolute path of binary with execute permission

    //  Fork a child process
    if(0 == (pid= fork())) {
        // Now on child process is running
        if(-1 == execve(strRun.c_str(), NULL , NULL)) {
            printf("WatchDog: Error: %d, could not launch process %s\n", errno, strName.c_str());
        }
    }
}





