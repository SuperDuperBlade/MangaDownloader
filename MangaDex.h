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
#include <chrono>
#include <thread>
#include "util/FileHandler.h"
#include "util/Logger.h"
#include "libs/simdjson.h"
#include "libs/cmdParser.h"
struct chapterInfo {
	std::string id;
	std::string title;
	std::string hash;
	std::vector<std::string> fileNames_data;
	std::vector<std::string> fileNames_datasaver;
};
struct volumeInfo {
	std::string title;
	std::vector<chapterInfo> chapters;
};
struct mangaInfo {
	std::string title;
	std::vector<volumeInfo> vinfos;
};
class MangaDex
{
	
	public:
		MangaDex(CmdParser* parser, Logger* logger, int argc,char* argv[]);

		void init(int argc, char* argv[]);

	//	MangaDex();
	//	std::string getMangaInfo(std::string);
//		std::string getMangaInfo();
		std::string getTitle();
		std::string getCoverFileName(std::string mangaID);
		std::string getCoverFileName();
		std::string sendRequestUsingBASEDOWNLOAD_URL(std::string url);
		std::string getTitle(std::string title);
		
		bool writeMangaToDisk( std::string mode, std::string data_setting);
		bool writeMangaToDisk();

		bool compile(std::string baseDir);
		mangaInfo getMangaMetaData();
		std::string convertFromViewToString(std::string_view value);
		void getFilesInChapter(chapterInfo* cinfo,std::string chapterID);


		bool isChapterInDesiredLang(std::string chapterID, std::string lang, std::string* responce);

		//bool writeMangaToDisk(std::string mode);
		std::string sendRequestUsingBASEURL(std::string addonURL);
		
	private: 
		std::string mangaID{}, outputDir{}, mode{}, quality{}, desiredLanguage{};
		int rateLimit = 20; //defualt
		Logger* logg;
		CmdParser* parser;

		//URLS
		const std::string BASEURL = "https://api.mangadex.org";
		const std::string BASEURL_MANGA = "/manga/";
		const std::string BASEURL_CHAPTER = "/chapter/";
		const std::string BASEURL_CHAPTER_IMAGES = "/at-home/server/";;
		

		const std::string BASEDOWNLOAD_URL = "https://uploads.mangadex.org";
		const std::string FILEDOWNLOAD_URL_DATA = "/data/";
		const std::string FILEDOWNLOAD_URL_DATASAVER = "/data-saver/";

		//https clients
		httplib::Client baseCli{ BASEURL };
		httplib::Client baseDownloadCli{BASEDOWNLOAD_URL};

		//cmd arguments
		const std::string mangaID_identifier = "-i";
		const std::string outputDir_identifier = "-o";
		const std::string mode_identifier = "-m";
		const std::string quality_identifier = "-dt";
		const std::string language_identifier = "-l";
		const std::string baseURL_identifier = "-burl";
		const std::string downloadURL_identifier = "-durl";
};

