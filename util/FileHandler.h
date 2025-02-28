#pragma once
#ifndef _FileHandler
#define _FileHandler

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
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

	static std::string getWorkingDirectory() {
			return std::filesystem::temp_directory_path().generic_string();
	}


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
				//unkown item 
				std::filesystem::create_directories(filepath);
			
			}
		}
		return result;
	}
	static void writeToFile(std::string filepath, std::string content) {
		std::ofstream file;
		file.open(filepath, std::ios_base::app);
		file << content;
		file.close();
	}
	static void createImageFile(std::string filepath, std::string content) {
		std::ofstream file;
		file.open(filepath, std::ios_base::binary);
		file << content;
		file.close();
	}

};
