#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "FileHandler.h"

class Logger {
private:
	bool exitOnFailure;
	const std::string defualtErrorMessage{ "Encountred a error: " };
	std::string errorMessage{defualtErrorMessage};
	std::string logFilePath = FileHandler::getWorkingDirectory() + "\\log.txt";
public:
	
	void whereisLogFile() {
		log("Log file is at: " + logFilePath);
	}

	
	//TODO add the option to write to a file
	void log(std::string message) {
		//std::string timeFormatted{ getTimeFormatted() };

		FileHandler::writeToFile(this->logFilePath,"timeFormatted"+message+"\n");

		std::cout << "timeFormatted" << message << '\n';
	}
	void errorLog(std::string message) {
		//Calls its child function
		this->errorLog(message, this->exitOnFailure);
	}

	//overides the defualt exitOnFailure setting
	void errorLog(std::string message, bool exitOnFailure) {
	//	std::string timeFormatted{ "getTimeFormatted() "};
		//std::cout << this->errorMessage << timeFormatted << message << "\n";
		//if (exitOnFailure) {
	//	std::cerr << "Exiting... \n";
		//	exit(-1);
	//	}
	}

	void setErrorMessage(std::string errorMessage) {
		this->errorMessage = errorMessage;
	}
	void resetErrorMessage() {
		this->errorMessage = this->defualtErrorMessage;
	}

};