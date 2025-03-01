#pragma once
#ifndef _FileHandler
#define _FileHandler

//#define WIN32_LEAN_AND_MEAN 

#include <zip.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <vector>
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
		try {
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
		}catch (const std::exception& e) {
			
			std::cout << "Encountered error using filepath: "+filepath <<"\n" << e.what() << "\n";
			exit(-1);
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
		file.write(content.c_str(),content.size());
		file.close();
	}
	static void mkdir(std::string filepath) {
		std::filesystem::create_directories(filepath);
	}
	static void getFileInfo(std::string filename,std::vector<char>& content) {
		
		std::ifstream file(filename, std::ios::binary);
		file.seekg(0, SEEK_END);
		size_t size = file.tellg();
		file.seekg(0,SEEK_SET);

		content.resize(size);
		file.read(content.data(), size);
	}
	//lits all the folders ina a dir
	static std::vector<std::string> listAllFoldersInDir(std::string path) {
		std::vector<std::string> folders;
		for (const auto & entry: std::filesystem::directory_iterator(path)) {
			if (entry.is_directory()) {
				folders.push_back(entry.path().string());
			}
		}
		return folders;
	}
	static std::vector<std::string> listAllFilesInDir(std::string path) {
		std::vector<std::string> files;
		for (const auto & entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_regular_file()) {
				files.push_back(entry.path().string());
			}
		}
		return files;
	}
	//adds all the files in a dir to a .cbz archive
	static void zipAllFilesFromDir(std::vector<std::string> paths, std::string outputPath){

		std::vector<zip_source_t*> filesInBuffer;
		std::vector<std::vector<char>> wtfisthiscode;

		int err = 0;
		outputPath += ".zip";
		zip_t* archive = zip_open(outputPath.c_str(), ZIP_CREATE  ,&err);
		if (archive == nullptr) {
			zip_error_t ziperror;
			zip_error_init_with_code(&ziperror,err);
			std::cerr << "Failed to open file: " + outputPath + " , " + zip_error_strerror(&ziperror);
		}


		if (!archive) {
			std::cout << "encountered a error with making the archive! code : " << err << "\n";
			return;
		}
		for (std::string filepath : paths) {
			
			std::vector<char> fileContent;
			getFileInfo(filepath,fileContent);

			zip_source_t* source = zip_source_buffer(archive, fileContent.data(), fileContent.size(),0);
			
			if (zip_file_add(archive, filepath.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
				std::cerr << "Failed to add file to archive: " + filepath + "\n";
				zip_close(archive);
				return;
			}
			//zip_source_free(source);
			filesInBuffer.push_back(source);
			wtfisthiscode.push_back(fileContent);
		}
	
		if (zip_close(archive) < 0) {
			std::cerr << "Failed to close CBZ archive" << "\n";
			return;
		}
			
		std::cout << "Directory was sucessfully zipped and output in: " + outputPath + "\n";

		}
};
