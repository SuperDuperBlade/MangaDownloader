#include "MangaDex.h"

MangaDex::MangaDex(CmdParser* parser, Logger* logger,int argc,char* argv[]) {
	this->parser = parser;
	this->logg = logger;
	this->init(argc, argv);
}

void MangaDex::init(int argc,char* argv[]) {


	//parser configuration
	parser->addOption(mangaID_identifier, "The id of the manga you want to download from mangaDex", true, true);
	parser->addOption(outputDir_identifier, "The directory you want to write the manga to (defualt is the dir the script is running in)", false, true);
	parser->addOption(mode_identifier, "The mode you want to download the manga in can be either volume , chapter or manga.", false, true);
	parser->addOption(quality_identifier, "The quality of the downloaded images , lower quality will take up less space. options: data , datasaver, both", false, true);
	parser->addOption(language_identifier, "The language the mangaWill be downlaoded in (shorthand) defualt is en (english) ", false, true);
	parser->addOption(baseURL_identifier, "The base url used for iteracting with the api", false, true);
	parser->addOption(downloadURL_identifier, "The base url used to download the files", false, true);
	parser->addOption(retrivalMethod_identifier, "The retrival method to use to get the files", false, true);
	parser->addOption(dontCompile_identifier, "Does not compile the images into a cbz file", false, false);
	parser->addOption(range_identifier, "The range of (chapters/volumes) the method will go through", false, true);
	
	//The executable itself does not count
	bool notEnoughArgs = parser->getNumberOfRequiredArguments() > argc - 1;
	
	parser->passArguments(argc, argv);
	if(notEnoughArgs) logg->log(" Entering promt mode.., ");
	
	if(!notEnoughArgs) this->mangaID = parser->getArgument(mangaID_identifier);
	else {
		std::cout << "Enter your ID: ";
		std::cin >> this->mangaID;
	}

	if (parser->doesArgExist(outputDir_identifier)) {
		this->outputDir = parser->getArgument(outputDir_identifier);
	}
	else if (notEnoughArgs) {
		std::cout << "Enter the output Dir: ";
		std::cin >> this->outputDir;
	}else {
		logg->errorLog("Working directory not found ,setting to the working directory", false);
		this->outputDir = FileHandler::getWorkingDirectory();
	}

	if (parser->doesArgExist(mode_identifier)) {
		this->mode = parser->getArgument(mode_identifier);
	}
	else if (notEnoughArgs) {
		std::cout << "Enter the mode: ";
		std::cin >> this->mode;
	}else {
		//defualt
		logg->errorLog("mode not set setting it to defualt (volumes)", false);
		this->mode = "volumes";
	}

	if (parser->doesArgExist(quality_identifier)) {
		this->quality = parser->getArgument(quality_identifier);
	}
	else if (notEnoughArgs) {
		std::cout << "Enter the quality you want: ";
		std::cin >> this->quality;
	}else {
		logg->errorLog("quality option not found setting it to defualt (data)", false);
		//defualt
		this->quality = "data";
	}
	
	if (parser->doesArgExist(language_identifier)) {
		this->desiredLanguage = parser->getArgument(language_identifier);
	}else if(notEnoughArgs){
		std::cout << "Enter the language you want (shorthand): ";
		std::cin >> this->desiredLanguage;
	}
	else {
		logg->errorLog("Language option not found setting it to defualt (en)",false);
		this->desiredLanguage = "en";
	}

	if (parser->doesArgExist(baseURL_identifier)) {
		baseCli = httplib::Client{parser->getArgument(baseURL_identifier)};
	}
	else if (notEnoughArgs) {
		std::cout << "Enter the baseUrl (type skip to use defualt): ";
		std::string baseURL_t;
		std::cin >> baseURL_t;
		if (baseURL_t != "skip" && baseURL_t != "SKIP") {
			baseCli = httplib::Client{ baseURL_t };
		}
	}

	if (parser->doesArgExist(downloadURL_identifier)) {
		baseDownloadCli = httplib::Client{parser->getArgument(downloadURL_identifier)};
	}
	else if (notEnoughArgs) {
		std::string baseDownloadUrl_t;
		std::cout << "Enter the base Download Url (type skip to use defualt): ";
		std::cin >> baseDownloadUrl_t;
		if (baseDownloadUrl_t != "skip" && baseDownloadUrl_t != "SKIP") {
			baseCli = httplib::Client{ baseDownloadUrl_t };
		}
	}

	if (parser->doesArgExist(retrivalMethod_identifier)) {
		method = std::stoi(parser->getArgument(retrivalMethod_identifier));
	}
	else if (notEnoughArgs) {
		std::string result{};
		
		std::cout << "Enter the method you would like to choose (options 0 , 1) (1 is perfferred): ";
		std::cin >> result;

		this->method = std::stoi(result);
	}
	
	if (parser->doesArgExist(dontCompile_identifier)) {
		dontCompile = true;
	}else if (notEnoughArgs) {
		std::string result;

		std::cout << "Should we compile the manga into .cbz files after it is downloaded( 0 / no for no , 1 / yes for yes ): ";
		std::cin >> result;

		if (result == "0" || result == "no") {
			dontCompile = true;
		}
	}
	if (parser->doesArgExist(range_identifier)) {
		isRangeEnabled = true;
		std::string value = parser->getArgument(range_identifier);
		size_t pos = value.find(":");
		if (pos == std::string::npos) {
			rangeMin = stol(value);
		}
		else {
			isRangeSettingMax = true;
			rangeMin = std::stol(value.substr(0, pos));
			rangeMax = std::stol(value.substr(pos+1,value.size()));
		}
	}
	

	logg->whereisLogFile();
}
//Sends a get request using the base url
std::string MangaDex::sendRequestUsingBASEURL(std::string addonURL) {

    this->logg->log("Sending get request to: " + this->BASEURL + addonURL);
	auto res = this->baseCli.Get(addonURL);
	simdjson::ondemand::parser parser;
	auto json = parser.iterate(res->body);
	std::string status = convertFromViewToString(json["result"].get_string().value());

	
	if (res && status != "error") {
		this->logg->log("Success : " + this->BASEURL + addonURL);
	}
	else {
		for (auto obj : json["errors"].get_array()) {
			int status = obj["status"].get_int64().value();

			logg->errorLog("Encountered error code: "+status,false);

			if (status == 429) {
				logg->log("Encountered rate limit waiting 20 seconds... before trying again");
				std::this_thread::sleep_for(std::chrono::seconds(this->rateLimit));
				return sendRequestUsingBASEURL(addonURL);
			}
		}
	}

	return res->body;
}

//Gets the title of the manga
std::string MangaDex::getTitle(std::string mangaID) {
	std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + mangaID);
	simdjson::ondemand::parser parser;
	auto result = parser.iterate(responce);

	auto obj = result["data"]["attributes"]["title"].get_object();
	for (auto o_title : obj) {
		simdjson::ondemand::value  value = o_title.value();
		//covert from string view to a string
		std::string_view title_up = value.raw_json_token();
		std::string title{ title_up };

		title.erase(std::remove(title.begin(), title.end(), '"'), title.end());

		logg->log("Retrived Title: " + title);
		return title;
	}
	logg->errorLog("Title for mangaID:" + mangaID + " was not found");
	return "unkown";
}
//returns the title of the manga using the id provided in the cmd
std::string MangaDex::getTitle() {
	return	this->getTitle(this->mangaID);
}
//TODO
std::string MangaDex::getCoverFileName(std::string mangaID) {
	try {
		//first finds the cover id
		std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + mangaID);
		simdjson::ondemand::parser parser;

		auto json = parser.iterate(responce);
		auto array = json["data"]["relationships"].get_array();

		for (auto obj : array) {
			std::string type = convertFromViewToString(obj["type"].get_string().value());
			if (type == "cover_art") {
				std::string id = convertFromViewToString(obj["id"].get_string().value());
				//sends a request to get more info about the coverID so we can get the file name
				responce = sendRequestUsingBASEURL(BASEURL_COVER + id);
				json = parser.iterate(responce);
				std::string fileName = convertFromViewToString(json["data"]["attributes"]["fileName"].get_string().value());

				return fileName;
				break;
			}
		}
	}
	catch (...) {
		logg->errorLog("Unable to retrive cover skipping...", false);
	}


	return "";
}
std::string MangaDex::getCoverFileName() {
	return getCoverFileName(mangaID);
}
//used for downloading files from the api
std::string MangaDex::sendRequestUsingBASEDOWNLOAD_URL(std::string addonURL) {
	this->logg->log("Sending get request to: " + this->BASEDOWNLOAD_URL + addonURL);
	auto res = this->baseDownloadCli.Get(addonURL);
	if (res) {
		this->logg->log("Success: " + this->BASEDOWNLOAD_URL + addonURL);
	}
	else if (res->status == 429) {
		logg->log("Encountered rate limit waiting 20 seconds... before trying again");
		std::this_thread::sleep_for(std::chrono::seconds(this->rateLimit));
		sendRequestUsingBASEDOWNLOAD_URL(addonURL);
	}
	else {
		logg->errorLog("Encountered error: " + std::to_string(res->status) + " , " + to_string(res.error()) + '\n');
	}

	return res->body;
}

//downloads the images from mangaDex and converts them to cbz files
//if the mode is not volumes or chapter then it will defualt to manga
bool MangaDex::writeMangaToDisk( std::string mode,std::string data_setting) {

	
	if (!FileHandler::checkIfExists(this->outputDir, true)) {
		logg->log("Dir:" + this->outputDir + " ,not found attempting to create");
	}
	mangaInfo manga;

	switch (method) {
		case 0:
			manga = getMangaMetaData();
			break;
		case 1:
			manga = getMangaMetaDataSecondMethod();
			break;
		default:
			logg->errorLog("Incorrect retrival method: " + method,true);

	}
	manga.title = getTitle();
	
	
	const std::string name_prefix = FileHandler::sanitiseFileName(manga.title);
	std::string manga_dir = this->outputDir + "\\" + name_prefix;
	const std::string base_DIR{ this->outputDir + "\\" + name_prefix };
	

	FileHandler::checkIfExists(manga_dir,true);


	long volumeCounter{ 1 };
	if (manga.hasUnorderedVolume) volumeCounter == 0 ;

	
	long fileCounter{ 0 };

	//makes it go in another directory if mode is set to manga in order to make the compiler work
		if (mode == "manga") {
			manga_dir += +"//" + name_prefix;
			FileHandler::mkdir(manga_dir);
		}

	
		//Gets the cover of the manga
		logg->log("Retriving Cover");
		std::string cover{ getCoverFileName() };
		if (cover != "") {
			std::string filename = "00_cover_" + cover;
			std::string coverBuffer = sendRequestUsingBASEDOWNLOAD_URL(this->FILEDOWNLOAD_URL_COVER + mangaID + "/" + cover);
			FileHandler::createImageFile(base_DIR + "\\" + filename, coverBuffer);
		}

		
		for (volumeInfo vinfo : manga.vinfos) {
	
			//all the files in a specific volume go to the corrasponding directory
			if (mode == "volume") {

				if (volumeCounter < rangeMin) {
					volumeCounter++;
					continue;
				}
				if (isRangeSettingMax) {
					if (volumeCounter > rangeMax) {
						break;
					}
				}

				//checks if there are prevolumes (volumes labled 0)


				//yes i know they are the same
				if (method == 0) manga_dir = base_DIR + "\\" + "v" + vinfo.title + "_" + name_prefix;
				else if (method == 1) manga_dir = base_DIR + "\\" + "v" + std::to_string(volumeCounter) + "_" + name_prefix;
				FileHandler::mkdir(manga_dir);

			}
			
			long chapterCounter{ 1 };
			
			if (mode != "volume") fileCounter = 0;

			
			bool temp = true;

			for (chapterInfo cinfo: vinfo.chapters) {
				logg->log("volume: "+cinfo.volume+ " chapter: "+cinfo.chapter);
				std::string chapterHash = cinfo.hash;
				//if chapter is empty then it will be set to its index
				if (cinfo.title == "") {
					cinfo.title = std::to_string(chapterCounter);
				}
				//all the files in a chapter go to a corrasponding directory
				if (mode == "chapter") {

					if (chapterCounter < rangeMin) {
						chapterCounter++;
						continue;
					}
					if (isRangeSettingMax) {
						if (chapterCounter > rangeMax) {
							break;
						}
					}

					if (method == 0) manga_dir = base_DIR + "\\" + "v" + vinfo.title + "_" + "c" + cinfo.title + "_" + name_prefix;
					else if (method == 1) manga_dir = base_DIR + "\\" + "v" + std::to_string(volumeCounter) + "c" + cinfo.chapter + "_" + FileHandler::sanitiseFileName(cinfo.title);

						//Creates the dir in advance
						FileHandler::mkdir(manga_dir);
				}
			
				


				if (data_setting == "data" ) {
					
					for (std::string file : cinfo.fileNames_data) {
						
						//retrives file and writes to directory
						std::string filepath = manga_dir+ "\\" + std::to_string(fileCounter) + "_" + file;
						std::string addonURL = this->FILEDOWNLOAD_URL_DATA+chapterHash + "/"+file;
						std::string responce = sendRequestUsingBASEDOWNLOAD_URL(addonURL);
						FileHandler::createImageFile(filepath, responce);
						fileCounter++;
					}
				}else if (data_setting == "saver") {
					
					for (std::string file : cinfo.fileNames_datasaver) {
						//retrives file and writes to directory
						std::string filepath = manga_dir + "\\" + std::to_string(fileCounter) + "_" + file;
						std::string addonURL = this->FILEDOWNLOAD_URL_DATA + chapterHash + "/" + file;
						std::string responce = sendRequestUsingBASEDOWNLOAD_URL(addonURL);
				    	FileHandler::createImageFile(filepath, responce);
						fileCounter++;
					}
				}
				else {
					//No data setting found exiting
					logg->errorLog("Wrong data setting check -help for details", true);
				}
				chapterCounter++;
			}
			volumeCounter++;
		}
		if(!dontCompile) compile(base_DIR);
	return false;
}
//uses varibles provided via cmd instead
bool MangaDex::writeMangaToDisk() {
	this->writeMangaToDisk(this->mode,this->quality);
	return true;
}
//compresses all the files in a zip to .cbz files which can be read by manga readers
bool MangaDex::compile(std::string baseDir) {
	std::vector<std::string> dirs = FileHandler::listAllFoldersInDir(baseDir);
	logg->log("Compiling downloaded files into cbz archives");

	for (std::string dir: dirs) {
		logg->log("Now compiling: " + dir);
		std::string dirOut = baseDir + "\\" + std::filesystem::path(dir).filename().string();
		FileHandler::zipAllFilesFromDir(dir,FileHandler::listAllFilesInDir(dir), dirOut);
		logg->log("Compiled: " + dir + " into: " + dirOut);
		
		//directory is no longer needed so can be removed
		FileHandler::removeDir(dir);
	}
	logg->log("Finished Compiling all files!");
	return true;
}
//fast and accurate
mangaInfo MangaDex::getMangaMetaDataSecondMethod() {
	mangaInfo mngInfo;
	mngInfo.title = MangaDex::getTitle();


	std::vector<chapterInfo> cinfos;
	float highestChapter{ 0 }, highestVolume{ 1 };
	std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + this->mangaID + "/feed?translatedLanguage[]=" + desiredLanguage);
	simdjson::ondemand::parser parserb;
	auto json = parserb.iterate(responce);


	//max limit
	long limit = json["total"].get_int64().value();

	 responce = sendRequestUsingBASEURL(BASEURL_MANGA + this->mangaID + "/feed?translatedLanguage[]=" + desiredLanguage+"&limit="+std::to_string(limit));
	simdjson::ondemand::parser parser;
	 json = parser.iterate(responce);


	for (auto chap : json["data"].get_array()) {
		chapterInfo cinfo;
		auto volObj = chap["attributes"]["volume"];


		cinfo.id = convertFromViewToString(chap["id"].get_string().value());
		cinfo.title = convertFromViewToString(chap["attributes"]["title"].get_string().value());

		//If the volume attribute is set to null .If so we shall replace it with 0 meaning that the volume is unordered
		if (!volObj.is_null())	cinfo.volume = convertFromViewToString(volObj.get_string().value());
		else { 
			cinfo.volume = "0"; 
			mngInfo.hasUnorderedVolume = true;
		}
		
		cinfo.chapter = convertFromViewToString(chap["attributes"]["chapter"].get_string().value());

		getFilesInChapter(&cinfo, cinfo.id);

		//looks if the current chapter has a volume greater than the current one
		if (std::stof(cinfo.volume) > highestVolume) {
			highestVolume = std::stof(cinfo.volume);
		}
		cinfos.push_back(cinfo);
	}
	//sorts the chapters in order
	std::sort(cinfos.begin(), cinfos.end(), &isChapterLargerThanTheOther);

	//volume starts at the volume the first chapter is in
	int currentVolIter{ std::stoi(cinfos.at(0).volume)};
	volumeInfo vinfo;
	for (chapterInfo cinfo : cinfos) {
		logg->log("Volume: " + cinfo.volume + " , chapter: " + cinfo.chapter);

		if (currentVolIter == stoi(cinfo.volume)) {
			vinfo.chapters.push_back(cinfo);
		}
		else {
			mngInfo.vinfos.push_back(vinfo);
			currentVolIter = currentVolIter+1;
			vinfo.chapters.clear();
			vinfo.title = std::to_string(currentVolIter);
		}
	}
	//returns the last volume
	mngInfo.vinfos.push_back(vinfo);

	return mngInfo;
}
//inefficcent and buggy
//gets the correct chapters sorts them into their respective volumes and gets the files for the respective chapters needed for parsing
mangaInfo MangaDex::getMangaMetaData() {
	
	mangaInfo mngInfo;
	mngInfo.title = getTitle();

	std::vector<volumeInfo> volumes;
	std::string mangaTitle = getTitle(this->mangaID);
	std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + this->mangaID + "/aggregate?translatedLanguage[]=" + desiredLanguage);

	//std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + this->mangaID + "/feed?translatedLanguage[]=" + desiredLanguage);
	
		simdjson::ondemand::parser parser;
		auto json = parser.iterate(responce);
		simdjson::ondemand::object volumes_c = json["volumes"].get_object();

		for (auto volume : volumes_c) {
			volumeInfo vinfo;
			vinfo.title = convertFromViewToString(volume.key_raw_json_token());

			for (auto chapter : json["volumes"][vinfo.title]["chapters"].get_object()) {

				std::string chatperN = convertFromViewToString(chapter.key_raw_json_token());
				std::string chapterID = convertFromViewToString(json["volumes"][vinfo.title]["chapters"][chatperN]["id"].get_string().value());
				std::string chapter_responce{};
				simdjson::ondemand::parser chapter_parser;

				bool wasChapterFound = false;
				//checks if the chapter is in the right language

			//	if (isChapterInDesiredLang(chapterID, desiredLanguage, &chapter_responce)) {

					auto chapter_json = chapter_parser.iterate(chapter_responce);

					chapterInfo cinfo;
					cinfo.id = chapterID;
					try {
						//most of the time mangaDex does not give the title so for now will just be using the chapter number
						cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["chapter"].get_string().value());
					}
					catch (simdjson::simdjson_error e) {
						logg->errorLog(e.what(), false);
						logg->log("Title not found setting the title to blank");
						cinfo.title = "";
					}
					getFilesInChapter(&cinfo, chapterID);

					vinfo.chapters.push_back(cinfo);
					wasChapterFound = true;
			//	}
				/*
				else {
					logg->log("Unable to find the chapter for the correct lang checking other IDs \n");

					for (auto id : json["volumes"][vinfo.title]["chapters"][chatperN]["others"].get_array()) {
						std::string sID = convertFromViewToString(id.get_string().value());

						//lanuage chcking is removed for now

					//	if (isChapterInDesiredLang(chapterID, desiredLanguage, &chapter_responce)) {

							auto chapter_json = chapter_parser.iterate(chapter_responce);

							chapterInfo cinfo;
							cinfo.id = sID;
							//most of the time mangaDex does not give the title so for now will just be using the chapter number
							try {
								cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["chapter"].get_string().value());
							}
							catch (simdjson::simdjson_error e) {
								logg->errorLog(e.what(), false);
								logg->log("Title not found leaving setting the title to blank");
								cinfo.title = "";
							}
							getFilesInChapter(&cinfo, sID);
							vinfo.chapters.push_back(cinfo);
							wasChapterFound = true;
							break;
						}
					//}
					if (!wasChapterFound) {
						logg->log("Unable to find the relevant chapter for the lang requested skipping...");
					}
					*/
			//	}
			}
			//chapters are in reverse order unreverse them
			std::reverse(vinfo.chapters.begin(), vinfo.chapters.end());
			volumes.push_back(vinfo);

			
		}
		
	//volumes are in reverse order unreverse them
	std::reverse(volumes.begin(), volumes.end());
	mngInfo.vinfos = volumes;
	return mngInfo;
}

std::string MangaDex::convertFromViewToString(std::string_view value) {
	std::string temp{ value };
	temp.erase(std::remove(temp.begin(), temp.end(), '"'), temp.end());
	return temp;
}
//gets all the files in a chapter
void MangaDex::getFilesInChapter(chapterInfo* cinfo,std::string chapterID) {
	std::string responce = sendRequestUsingBASEURL(this->BASEURL_CHAPTER_IMAGES+chapterID);
	simdjson::ondemand::parser parser;

	auto json = parser.iterate(responce);
	try {
		cinfo->hash = convertFromViewToString(json["chapter"]["hash"].get_string().value());
	}
	catch(...){
		//debugging pourposes
		logg->log(responce);
		exit(-1);
	}
	for (auto obj : json["chapter"]["data"].get_array()) {
		std::string filename = convertFromViewToString(obj.get_string().value());
		cinfo->fileNames_data.push_back(filename);
	}
	for (auto obj : json["chapter"]["dataSaver"].get_array()) {
		std::string filename = convertFromViewToString(obj.get_string().value());
		cinfo->fileNames_datasaver.push_back(filename);
	}
}

//Checks if the chapter is in the desired language
bool MangaDex::isChapterInDesiredLang(std::string chapterID, std::string lang,std::string* responce) {
	std::string request = sendRequestUsingBASEURL(this->BASEURL_CHAPTER + chapterID);
	responce = new std::string(request);
	simdjson::ondemand::parser parser;
	auto json = parser.iterate(request);
		std::string language = convertFromViewToString(json["data"]["attributes"]["translatedLanguage"].get_string().value());
		logg->log(language);
		if (language == lang) {
			logg->log("Chapter:" + chapterID + " matches the target language");
			return true;
		}
		return false;
}
//returns if a chapter is in front or not than the other
bool MangaDex::isChapterLargerThanTheOther(chapterInfo const& cinfo1,chapterInfo const& cinfo2) {
	if (cinfo1.id != cinfo2.id) {
		return std::stof(cinfo1.chapter) < std::stof(cinfo2.chapter);
	}
	return false;
}