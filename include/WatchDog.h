/*sgn
 * WatchDog.h
 *
 *  Created on: 22-Mar-2020
 *      Author: tstone10
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <iostream>
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <map>
#include "JsonFactory.h"

#define HEART_BEAT      (0)
#define VERSION_REQ     (1)
#define REBOOT_RPi      (2)


#define UDP_Rx_PORT         (4951)
#define UDP_Tx_PORT         (4952)
#define BUFSIZE             (1024 * 10)
#define MAX_INTERVAL_SECs   (30)
#define CHECK_INTERVAL_SECs (15)

class Process {
    std::string strName, strRun;
    unsigned int pid, ver;
    unsigned int last_pet;
public:
    Process() : pid(0), ver(0), last_pet(0) {}
    virtual ~Process() {}

    std::string getName() { return strName; }
    int getVer() { return ver; }
    std::string getRunCmd() { return strRun; }
    int getPid() { return pid; }
    int getLastPet() { return last_pet; }


    void setName(std::string name) { strName = name; }
    void setVer(int iVer) { ver = iVer; }
    void setRunCmd(std::string cmd) { strRun = cmd; }
    void setPid(int id) { pid = id; }
    void setPet(int tm) { last_pet = tm; }

    void launchProc();
};

class WatchDog {
    pthread_mutex_t wLock;
    std::map<std::string, int> cmds;
    std::vector<Process *> processes;
    JsonFactory jsProcs;

    std::vector<Process *> getProcesses() { return processes; }

public:
    WatchDog();
    virtual ~WatchDog();

    std::string getAllVerAsJson();
    void parseHeartBeat(std::string strBeat);
    void pushIfNew(Process *pProc);
    void sendPacket(int port, std::string strPkt);

    static void *recvThread(void *);
    static void *wdogThread(void *);
};



#endif /* WATCHDOG_H_ */
