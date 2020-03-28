/*sgn sgn
 * FileLogger.h
 *
 *  Created on: 20-Mar-2020
 *      Author: tstone10
 */

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include <pthread.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <queue>

#define MAX_LOG_SIZE    (1024 * 15)

class Logger {
    bool bTime;
    pthread_mutex_t writeLock;

	std::stringstream ss_log;
	Logger();
	void stampTime();
	static Logger *pLogger;

public:

	virtual ~Logger() {}
	std::string getLogData();

	Logger &operator<<(const std::string strLog);
	Logger &operator<<(const int val);

    // this is the type of std::cout
    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
    // this is the function signature of std::endl
    typedef CoutType& (*StandardEndLine)(CoutType&);
    Logger &operator << (StandardEndLine manip);

	static Logger &getInstance();
};


#endif /* FILELOGGER_H_ */
