#pragma once

#include "libs/httplib.h"
#include "iostream"
#include <vector>

class MangaDex
{
	public:
		MangaDex(std::string, std::string);
		MangaDex(std::string);
		MangaDex();
		std::string getMangaInfo(std::string);
		std::string getMangaInfo();
		std::string getTitle();
		std::string getTitle(std::string title);
		std::vector<std::string> getAllChaptersInManga();
		std::vector<std::string> getAllChaptersInVolume();
		std::vector<std::string> getAllVolumes();
		std::string getVolumeName();
		std::string getChapterName();
		
		bool isChapterInLang(std::string);
		
		bool writeMangaToDisk(std::string dir, std::string mode);
		bool writeMangaToDisk(std::string mode);
	private: 
		std::string mangaID, outputDir;
};

