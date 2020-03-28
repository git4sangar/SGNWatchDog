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

class Utils {
public:
    static void sendPacket(int port, std::string strPkt);
    static int prepareRecvSock(int port);
    static std::string prepareLogPacket(std::string strLog);
};

#endif /* UTILS_H_ */
