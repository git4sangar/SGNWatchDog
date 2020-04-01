/*sgn
 * MainApp.cpp
 *
 *  Created on: 22-Mar-2020
 *      Author: tstone10
 */


#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "Utils.h"
#include <sys/reboot.h>
#include "WatchDog.h"
#include "FileLogger.h"

int main() {
    Logger &info_log = Logger::getInstance();

    info_log << "Main: Starting WatchDog version 2" << std::endl;
    if(Utils::isNewWDogAvailable()) {
        info_log << "Main: Uploaded a new WatchDog. So rebooting in 30 secs..." << std::endl;
        sleep(30);
        reboot(RB_AUTOBOOT);
    }

    WatchDog wDog;
    pthread_t wdog_thread, recv_thread;
    pthread_create(&wdog_thread, NULL, &WatchDog::wdogThread, &wDog);
    pthread_create(&recv_thread, NULL, &WatchDog::recvThread, &wDog);

    while(1) sleep(60);
    return 0;
}

