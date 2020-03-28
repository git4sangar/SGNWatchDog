/*sgn
 * MainApp.cpp
 *
 *  Created on: 22-Mar-2020
 *      Author: tstone10
 */


#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "WatchDog.h"
#include "FileLogger.h"

int main() {
    WatchDog wDog;
    pthread_t wdog_thread, recv_thread;
    Logger &info_log = Logger::getInstance();

    info_log << "Starting WatchDog version 1" << std::endl;
    pthread_create(&wdog_thread, NULL, &WatchDog::wdogThread, &wDog);
    pthread_create(&recv_thread, NULL, &WatchDog::recvThread, &wDog);

    while(1) sleep(60);
    return 0;
}

