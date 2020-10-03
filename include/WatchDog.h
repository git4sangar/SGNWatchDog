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
#include "FileLogger.h"
#include "JsonFactory.h"

#define HEART_BEAT      (0)
#define VERSION_REQ     (1)
#define REBOOT_RPi      (2)
#define UPLOAD_LOGS     (3)
#define WiFi_SSID       (4)
#define WHERE_R_U       (5)
#define SMART_TV_MAC    (6)

#define MAX_INTERVAL_SECs   (5 * 60) // 60 to 5*60 as per Dinesh on 2020-09-08
#define GET_UP_TIME_SECs    (4 * MAX_INTERVAL_SECs)
#define CHECK_INTERVAL_SECs (15)
#define WATCHDOG_VERSION    (7)

#define WPA_SUPPLICANT_FILE     "/etc/wpa_supplicant/wpa_supplicant.conf"
//#define WPA_SUPPLICANT_FILE     "/home/tstone10/sgn/bkup/private/projs/SGNBundle/common/wpa_supplicant.conf"
#define WiFi_INTERFACE          "wlan"
//#define WiFi_INTERFACE          "wlp"

#define WDOG_PROC_NAME          "WatchDog"

class Process {
    std::string strName, strRun;
    unsigned int pid, ver;
    unsigned int last_pet;
    bool isPidValid;
public:
    Process() : pid(0), ver(0), last_pet(0), isPidValid(false) {}
    virtual ~Process() {}

    std::string getName() { return strName; }
    int getVer() { return ver; }
    std::string getRunCmd() { return strRun; }
    int getPid() { return pid; }
    int getLastPet() { return last_pet; }
    bool isValidPid() { return isPidValid; }

    void setName(std::string name) { strName = name; }
    void setVer(int iVer) { ver = iVer; }
    void setRunCmd(std::string cmd) { strRun = cmd; }
    void setPid(int id) { if(0 < id) {pid = id; isPidValid = true;}}
    void setPet(int tm) { last_pet = tm; }
    void invalidatePid() {isPidValid = false;}
    void launchProc();
};

class WatchDog {
    pthread_mutex_t wLock;
    std::map<std::string, int> cmds;
    std::vector<Process *> processes;
    Logger &info_log;

    std::vector<Process *> getProcesses() { return processes; }

public:
    WatchDog();
    virtual ~WatchDog();

    std::string getAllVerAsJson();
    void parseHeartBeat(std::string strBeat);
    void pushIfNew(Process *pProc);
    bool updateSSID(std::string strJson);
    std::string removeSSID(std::string strFile, std::string strSSID);
    void addMeToProcList();

    static void *recvThread(void *);
    static void *wdogThread(void *);
};



#endif /* WATCHDOG_H_ */
