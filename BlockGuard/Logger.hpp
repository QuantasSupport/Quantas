
//
// Created by srai on 6/5/19.
//

#ifndef Logger_hpp
#define Logger_hpp


#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <string>
#include <sstream>

class Logger{
private:
	Logger																	()= default;;
	Logger																	(const Logger&)= default;
	Logger& 									operator=					(const Logger&)= default;

	static std::string 							fileName;
	static Logger* 								logger ;
	static std::ofstream 						out;

public:
	void 										log							(const std::string &msg);
	Logger& 									operator<<					(const std::string& msg);
	static void 								setLogFileName				(std::string);
	static Logger* 								instance					();
};


#endif //Logger_hpp
