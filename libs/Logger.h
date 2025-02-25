#pragma once
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>



class Logger {
public:
	 void log(std::string);
	 //Uses cerr to print the message to the console 
	 void errorLog(std::string);
	 //overides the defualt exitOnFailure setting
	 void errorLog(std::string, bool exitOnFailure);
	 std::string getTimeFormatted();
private:
	bool exitOnFailure;
};
//gets the current time in a format
std::string Logger::getTimeFormatted() {
	std::time_t currentTime{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
	std::tm* local_time = std::localtime(&currentTime);
	std::stringstream timeStream;
	timeStream << std::put_time(local_time, "[%d-%m-Y] [%H:%M:%S] :");
	return timeStream.str();
}
//TODO add the option to write to a file
void Logger::log(std::string message) {
	std::string timeFormatted{ getTimeFormatted() };
	std::cout << timeFormatted << message << '\n';
}
void Logger::errorLog(std::string message) {
	std::string timeFormatted{ getTimeFormatted() };
	std::cout << timeFormatted << message << '\n';
}
//overides the defualt exitOnFailure setting
void Logger::errorLog(std::string message, bool exitOnFailure) {
	//Calls its parent function
	errorLog(message);
	if (exitOnFailure) {
		exit(-1);
	}
}