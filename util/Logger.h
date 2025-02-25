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
	 void setErrorMessage(std::string);
	 void resetErrorMessage();
private:
	bool exitOnFailure;
	const std::string defualtErrorMessage{ "Encountred a error: " };
	std::string errorMessage = defualtErrorMessage;
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
	std::cout  << timeFormatted << message << '\n';
}
void Logger::errorLog(std::string message) {
	//Calls its child function
	this->errorLog(message,this->exitOnFailure);
}

//overides the defualt exitOnFailure setting
void Logger::errorLog(std::string message, bool exitOnFailure) {
	std::string timeFormatted{ getTimeFormatted() };
	std::cout << this->errorMessage << timeFormatted << message << "\n";
	if (exitOnFailure) {
		std::cerr << "Exiting... \n";
		exit(-1);
	}
}

void Logger::setErrorMessage(std::string errorMessage) {
	this->errorMessage = errorMessage;
}
void Logger::resetErrorMessage() {
	this->errorMessage = this->defualtErrorMessage;
}