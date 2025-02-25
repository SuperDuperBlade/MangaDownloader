#pragma once
#ifndef _FileHandler
#define _FileHandler
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#endif
 class FileHandler
{
public:
	static std::stringstream getFileContents_stream();
	static std::string getFileContents();
	//Checks if a file exits and if createIfNotFound is set to true then the file/directory will be created
	static bool checkIfExists(std::string filepath, bool createIfNotFound) {
		struct stat buffer;
		bool result = stat(filepath.c_str(), &buffer) == 0;

		if (!result && createIfNotFound) {
			if (buffer.st_mode & S_IFDIR) {
				std::filesystem::create_directories(filepath);
			}
			else if (buffer.st_mode & S_IFREG) {
				std::ofstream createdFile(filepath);
				createdFile.close();
			}
			else {
				//unkown item error
				std::cerr << "Unable to determine file type: " << filepath << "\n" << "Exiting... \n";
				exit(-1);
			}
		}
		return result;
	}


};
