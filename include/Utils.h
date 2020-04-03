/*sgn
 * Utls.h
 *
 *  Created on: 28-Mar-2020
 *      Author: tstone10
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>

//  socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define UDP_Rx_PORT         (4951)
#define UDP_Tx_PORT         (4952)
#define UDP_DROID_PORT      (4953)
#define BUFFSIZE            (1024 * 10)

#define TECHNO_SPURS_ROOT_PATH      "/home/pi/Technospurs/"
//#define TECHNO_SPURS_ROOT_PATH      "/home/tstone10/sgn/bkup/private/projs/SGNBarc/Technospurs/"
#define TECHNO_SPURS_WDOG_FILE      "WDog/WatchDog"
#define TECHNO_SPURS_WDOG_ROOT      "/root/WatchDog"
#define TECHNO_SPURS_WDOG_LOG       "Logs/wdog_log_"

class Utils {
public:
    static void sendPacket(std::string strPacket, int port, std::string strToIp = std::string("localhost"));
    static int prepareRecvSock(int port);
    static std::string prepareLogPacket(std::string strLog);
    static struct in_addr getIpv4IpOfEthIF(std::string if_prefix);
    static std::string getDotFormattedIp(struct in_addr sin_addr);
    static bool isNewWDogAvailable();
};

#endif /* UTILS_H_ */
