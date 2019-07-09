//
// Created by srai on 6/5/19.
//

#include <iomanip>
#include "Logger.hpp"

std::string Logger::fileName;
Logger* Logger::logger = nullptr;
std::ofstream Logger::out;
std::string getDate();

/*
 * 	Usage:
 *	At first, set log file name
 *		Logger::setLogFileName("fileName.txt");
 *		Logger::instance()->log("string to log")
 */

Logger* Logger::instance(){
	if (logger == nullptr){
		logger = new Logger();
		if(fileName.empty()){
			//std::cerr<<"FILENAME IS EMPTY"<<std::endl;
			return nullptr;
		}
		out.open(fileName.c_str(), std::ios::out | std::ios::app);
		if(out.fail()){
			std::cerr<<"FAILED TO OPEN LOG FILE : "<<fileName<<std::endl;
		}
	}
	return logger;
};

void Logger::setLogFileName(std::string logFileName){
	if(out.is_open()){
		out.close();
		fileName = logFileName;
		out.open(fileName.c_str(), std::ios::out | std::ios::app);
		if(out.fail()){
			std::cerr<<"FAILED TO OPEN LOG FILE : "<<fileName<<std::endl;
		}
	}else
		fileName = logFileName;
}

void Logger::log(const std::string &msg) {
	out << getDate() << ":	" << msg ;
};

std::string getDate(){
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}
