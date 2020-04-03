/*sgn
 * FileLogger.cpp
 *
 *  Created on: 20-Mar-2020
 *      Author: tstone10
 */

#include <sstream>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "Utils.h"
#include "FileLogger.h"

Logger *Logger::pLogger = NULL;

Logger &Logger:: getInstance() {
	if(NULL == pLogger) {
		pLogger = new Logger();
	}
	return *pLogger;
}

Logger::Logger() : bTime (true), writeLock(PTHREAD_MUTEX_INITIALIZER) {
	std::string strFile	= std::string(TECHNO_SPURS_ROOT_PATH) + std::string(TECHNO_SPURS_WDOG_LOG);
	fp = fopen( (char *)strFile.c_str(), "w");
}

void Logger::stampTime() {
	struct timeval st;
	gettimeofday(&st,NULL);

	//	why do i need secs since epoch? get secs from now
	//	1584718500 => secs since epoch till now (31-Mar-20 12:30)
	unsigned long secs	= st.tv_sec - 1585638057;
	secs = secs % 36000;	// reset secs every 10 hours
	unsigned long msecs	= st.tv_usec / 1000;
	unsigned long usecs	= st.tv_usec % 1000;
	std::cout << secs << ":" << msecs << ":" << usecs << ": ";
	ss_log << secs << ":" << msecs << ":" << usecs << ": ";
}

std::string Logger::getLogData() {
    std::string strLog;

	pthread_mutex_lock(&writeLock);
	strLog = ss_log.str();
	ss_log.str(""); ss_log.clear();
	pthread_mutex_unlock(&writeLock);

	return strLog;
}

Logger &Logger::operator << (StandardEndLine manip) {
	pthread_mutex_lock(&writeLock);
	fprintf(fp, "\n"); fflush(fp);
	ss_log << std::endl; std::cout << std::endl; bTime = true;

	ss_log.seekg(0, std::ios::end);
	if(ss_log.tellg() > MAX_LOG_SIZE) {
		std::string strLog = ss_log.str();
		ss_log.str(""); ss_log.clear();
		Utils::sendPacket(Utils::prepareLogPacket(strLog), UDP_Tx_PORT);
	}
	pthread_mutex_unlock(&writeLock);
	return *this;
}

Logger &Logger::operator <<(const std::string strMsg) {
	pthread_mutex_lock(&writeLock);
	if(bTime) { stampTime(); bTime = false; }
	fprintf(fp, "%s", strMsg.c_str()); fflush(fp);
	ss_log << strMsg; std::cout << strMsg;
	pthread_mutex_unlock(&writeLock);
	return *this;
}

Logger &Logger::operator <<(int iVal) {
	pthread_mutex_lock(&writeLock);
	if(bTime) { stampTime(); bTime = false; }
	fprintf(fp, "%d", iVal); fflush(fp);
	ss_log << iVal; std::cout << iVal;
	pthread_mutex_unlock(&writeLock);
	return *this;
}
