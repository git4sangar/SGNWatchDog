/*sgn
 * WatchDog.cpp
 *
 *  Created on: 22-Mar-2020
 *      Author: tstone10
 */

#include <iostream>

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
#include "FileLogger.h"
#include "JsonFactory.h"
#include "WatchDog.h"
#include "Utils.h"

WatchDog :: WatchDog() : wLock(PTHREAD_MUTEX_INITIALIZER), info_log(Logger::getInstance()) {
    cmds["heart_beat"]  = HEART_BEAT;
    cmds["version_req"] = VERSION_REQ;
    cmds["reboot"]      = REBOOT_RPi;
    cmds["upload_logs"] = UPLOAD_LOGS;
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

        info_log << "WatchDog: Parsed HeartBeat " << jsRoot.getJsonString() << std::endl;
        pthread_mutex_lock(&wLock);
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
    if(newProc) {
        //  Process is just starting.
        //  So give enough time for it to get up & running
        newProc->setPet(time(0) + GET_UP_TIME_SECs);
        processes.push_back(newProc);
    }
}

/*
 * { "command" : "version_req",
 *      "processes" : [
 *              {"process_name" : "process1", "version" : 1, "isDead" : false},
 *              {"process_name" : "process2", "version" : 1, "isDead" : false}
 *           ]
 * }
 */
std::string WatchDog::getAllVerAsJson() {
    std::string allProcJson;
    JsonFactory jsProcs, jsRoot;
    pthread_mutex_lock(&wLock);
    for(Process *pProc : processes) {
        JsonFactory jsObj;
        bool isDead = (time(0) - pProc->getLastPet()) > DEAD_THRESHOLD_SECs;
        try {
            jsObj.addStringValue("process_name", pProc->getName());
            jsObj.addIntValue("version", pProc->getVer());
            jsObj.addBoolValue("isDead", isDead);
            jsProcs.appendToArray(jsObj);
        } catch(JsonException &je) {
            info_log << je.what() << std::endl;
        }
    }
    jsRoot.addStringValue("command", "version_req");
    jsRoot.addJsonObj("processes", jsProcs);
    allProcJson = jsRoot.getJsonString();
    pthread_mutex_unlock(&wLock);
    return allProcJson;
}

void *WatchDog::wdogThread(void *pUserData) {
    WatchDog *pThis     = reinterpret_cast<WatchDog *>(pUserData);
    Logger &info_log    = pThis->info_log;
    while(true) {
        pthread_mutex_lock(&pThis->wLock);
        for(Process *pProc : pThis->getProcesses()) {
            int delta = time(0) - pProc->getLastPet();
            if(delta > MAX_INTERVAL_SECs && delta < DEAD_THRESHOLD_SECs) {
                info_log << "WatchDog: Killing process " << pProc->getName() << std::endl;
                // We are going to kill the process & launch it again
                // So don't expect a heart within CHECK_INTERVAL_SECs
                // Give it a few more secs to get up & running
                pProc->setPet(time(0) + GET_UP_TIME_SECs);
                kill(pProc->getPid(), 9);
                pProc->launchProc();
            }
            if(0 > delta) {
                info_log << "WatchDog: As I'v killed " << pProc->getName()
                        << ", I'm waiting for " << (pProc->getLastPet() - time(0)) + MAX_INTERVAL_SECs
                        << " more secs for it to get up and send a heart-beat msg" << std::endl;
            }
        }
        pthread_mutex_unlock(&pThis->wLock);
        sleep(CHECK_INTERVAL_SECs);
    }
    return NULL;
}

void *WatchDog::recvThread(void *pUserData) {
    WatchDog *pThis     = reinterpret_cast<WatchDog *>(pUserData);
    Logger &info_log    = pThis->info_log;

    int sockfd = Utils::prepareRecvSock(UDP_Rx_PORT);
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
            info_log << "WatchDog: Got command " << strCmd << std::endl;
            switch(pThis->cmds[strCmd]) {
                case HEART_BEAT:
                    pThis->parseHeartBeat(strPkt);
                    break;

                case VERSION_REQ:
                    strPkt = pThis->getAllVerAsJson();
                    info_log << "WatchDog: Sending " << strPkt << std::endl;
                    Utils::sendPacket(UDP_Tx_PORT, strPkt);
                    break;

                case REBOOT_RPi:
                    info_log << "WatchDog: Rebooting... " << strPkt << std::endl;
                    sync();
                    reboot(RB_AUTOBOOT);
                    break;

                case UPLOAD_LOGS:
                    info_log << "WatchDog: Sending log data" << std::endl;
                    strPkt  = Utils::prepareLogPacket(info_log.getLogData());
                    Utils::sendPacket(UDP_Tx_PORT, strPkt);
                    break;
            }
        } catch(JsonException &je) {
            info_log << je.what() << std::endl;
        }
    }
    return NULL;
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





