#pragma once
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
static class FileHandler
{
public:
	std::stringstream getFileContents_stream();
	std::string getFileContents();
	bool checkIfExists(std::string filepath, bool createIfNotFound);
private:

};
//Checks if a file exits and if createIfNotFound is set to true then the file/directory will be created
bool FileHandler::checkIfExists(std::string filepath, bool createIfNotFound) {
	struct stat buffer;
	bool result = stat(filepath.c_str(), &buffer) == 0;

	if (!result && createIfNotFound) {
		if (buffer.st_mode & S_IFDIR) {
			std::filesystem::create_directories(filepath);
		}else if (buffer.st_mode & S_IFREG) {
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