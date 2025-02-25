#pragma once

#define WIN32_LEAN_AND_MEAN
#define CPPHTTPLIB_NO_EXCEPTIONS
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <Windows.h>
#include "libs/httplib.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "util/FileHandler.h"
#include "util/Logger.h"
#include "libs/simdjson.h"

struct chapterInfo {
	std::string title;
	std::string hash;
	std::vector<std::string> fileNames_data;
	std::vector<std::string> fileNames_datasaver;
};
struct volumeInfo {
	std::string title;
	std::vector<chapterInfo> chapters;
};
class MangaDex
{
	
	public:
		MangaDex(std::string, std::string, Logger*);
		MangaDex(std::string,Logger*);
	//	MangaDex();
	//	std::string getMangaInfo(std::string);
//		std::string getMangaInfo();
		std::string getTitle();
		std::string getCoverFileName(std::string mangaID);
		std::string getCoverFileName();
		std::string getTitle(std::string title);
		//std::vector<std::string> getAllChaptersInManga();
		//std::vector<std::string> getAllChaptersInVolume();
		//std::vector<std::string> getAllVolumes();
		//std::string getVolumeName();
		//std::string getChapterName();
		
		//bool isChapterInLang(std::string);
		
		bool writeMangaToDisk(std::string dir, std::string mode);
		std::vector<volumeInfo> getMangaMetaData();
		std::string convertFromViewToString(std::string_view value);
		//bool writeMangaToDisk(std::string mode);
		std::string sendRequestUsingBASEURL(std::string addonURL);
		
	private: 
		std::string mangaID, outputDir;
		Logger* logg;

		//URLS
		void init();
		const std::string BASEURL = "https://api.mangadex.org";
		const std::string BASEURL_MANGA = "/manga/";

		

		const std::string BASEDOWNLOAD_URL = "https://uploads.mangadex.org";


		httplib::Client baseCli{ BASEURL };
		httplib::Client baseDownloadCli{BASEDOWNLOAD_URL};

		
};

