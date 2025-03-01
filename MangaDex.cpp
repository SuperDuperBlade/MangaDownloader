#include "MangaDex.h"

MangaDex::MangaDex(CmdParser* parser, Logger* logger) {
	
	this->parser = parser;
	this->logg = logger;
	this->init();
}

void MangaDex::init() {
	this->mangaID = parser->getArgument("-i");
	if (parser->doesArgExist("-o")) {
		this->outputDir = parser->getArgument("-o");
	}
	else {
		logg->errorLog("Working directory not found setting to the working directory", false);
		this->outputDir = FileHandler::getWorkingDirectory();
	}
	if (parser->doesArgExist("-m")) {
		this->mode = parser->getArgument("-m");
	}
	else {
		//defualt
		logg->errorLog("mode not set setting it to defualt (volumes)", false);
		this->mode = "volumes";
	}
	if (parser->doesArgExist("-dt")) {
		this->quality = parser->getArgument("-dt");
	}else {
		logg->errorLog("quality option not found setting it to defualt (data)", false);
		//defualt
		this->quality = "data";
	}
	logg->whereisLogFile();
}
//Sends a get request using the base url
std::string MangaDex::sendRequestUsingBASEURL(std::string addonURl) {
	this->logg->log("Sending get request to: " + this->BASEURL + addonURl);
	auto res = this->baseCli.Get(addonURl);
	if (res) {
		this->logg->log("Success: " + this->BASEURL+addonURl);
	}
	else if (res->status == 429) {
		logg->log("Encountered rate limit waiting 20 seconds... before trying again");
		std::this_thread::sleep_for(std::chrono::seconds(this->rateLimit));
		sendRequestUsingBASEURL(addonURl);
	}else {
		logg->errorLog( "Encountered error: " + std::to_string(res->status) + " , " + to_string(res.error()) + '\n');
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


	return "title";
}
std::string MangaDex::getCoverFileName() {
	getCoverFileName(mangaID);
	return "";
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
bool MangaDex::writeMangaToDisk( std::string mode,std::string data_setting) {

	
	if (!FileHandler::checkIfExists(this->outputDir, true)) {
		logg->log("Dir:" + this->outputDir + " ,not found attempting to create");
	}
	
	mangaInfo manga = getMangaMetaData();
	manga.title = getTitle();

	//TODO sanitise
	std::string manga_dir = this->outputDir + "\\" + manga.title;
	const std::string name_prefix = manga.title;
	const std::string base_DIR{ this->outputDir+"\\"+manga.title};
	
	compile(base_DIR);

	FileHandler::checkIfExists(manga_dir,true);
		
	
	 //if the mode is not volumes or chapter then it will defualt to manga
		for (volumeInfo vinfo : manga.vinfos) {
			
			//all the files in a specific volume go to the corrasponding directory
			if(mode == "volume") manga_dir = base_DIR+"\\" +"v" + vinfo.title + "_" + name_prefix;
			
			FileHandler::mkdir(manga_dir);
			long chapterCounter{ 0 };
			long fileCounter{ 0 };

			for (chapterInfo cinfo: vinfo.chapters) {
				chapterCounter++;

				std::string chapterHash = cinfo.hash;
				//if chapter is empty then it will be set to its index
				if (cinfo.title == "") {
					cinfo.title = std::to_string(chapterCounter);
				}
				//all the files in a chapter go to a corrasponding directory
				if (mode == "chapter") manga_dir = base_DIR+"\\"+"v" + vinfo.title+"_" + "c" + cinfo.title + "_" + name_prefix;

				//Creates the dir in advance
				FileHandler::mkdir(manga_dir);

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
			}
			compile(base_DIR);
		}
	return false;
}
//uses varibles provided via cmd instead
bool MangaDex::writeMangaToDisk() {
	this->writeMangaToDisk(this->mode,this->quality);
	return true;
}

bool MangaDex::compile(std::string baseDir) {
	std::vector<std::string> dirs = FileHandler::listAllFoldersInDir(baseDir);
	logg->log("Compiling downloaded files into cbz archives");
	for (std::string dir: dirs) {
		logg->log("Now compiling: " + dir);
		std::string dirOut = baseDir + "\\" + std::filesystem::path(dir).filename().string();
		FileHandler::zipAllFilesFromDir(FileHandler::listAllFilesInDir(dir), dirOut);
		logg->log("Compiled: " + dir + " into: " + dirOut);
	}
	logg->log("Finished Compiling all files!");
	return true;
}

//gets the correct chapters sorts them into their respective volumes and gets the files for the respective chapters needed for parsing
mangaInfo MangaDex::getMangaMetaData() {
	
	mangaInfo mngInfo;
	mngInfo.title = getTitle();

	std::vector<volumeInfo> volumes;
	std::string mangaTitle = getTitle(this->mangaID);
	std::string responce = sendRequestUsingBASEURL(BASEURL_MANGA + this->mangaID + "/aggregate");

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
			
				if (isChapterInDesiredLang(chapterID, "en", &chapter_responce)) {
					
					auto chapter_json = chapter_parser.iterate(chapter_responce);

					chapterInfo cinfo;
					cinfo.id = chapterID;
					try {
						//most of the time mangaDex does not give the title so for now will just be using the chapter number
						cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["chapter"].get_string().value());
					}
					catch (simdjson::simdjson_error e) {
						logg->errorLog(e.what());
						logg->log("Title not found setting the title to blank");
						cinfo.title = "";
					}
					getFilesInChapter(&cinfo, chapterID);

					vinfo.chapters.push_back(cinfo);
					wasChapterFound = true;
				}
				else {
					logg->log("Unable to find the chapter for the correct lang checking other IDs \n");

					for (auto id : json["volumes"][vinfo.title]["chapters"][chatperN]["others"].get_array()) {
						std::string sID = convertFromViewToString(id.get_string().value());
						if (isChapterInDesiredLang(chapterID, "en", &chapter_responce)) {
						
							auto chapter_json = chapter_parser.iterate(chapter_responce);

							chapterInfo cinfo;
							cinfo.id = sID;
							//most of the time mangaDex does not give the title so for now will just be using the chapter number
							try {
								cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["chapter"].get_string().value());
							}
							catch (simdjson::simdjson_error e) {
								logg->errorLog(e.what());
								logg->log("Title not found leaving setting the title to blank");
								cinfo.title = "";
							}
							getFilesInChapter(&cinfo, sID);
							vinfo.chapters.push_back(cinfo);
							wasChapterFound = true;
							break;
						}
					}
					if (!wasChapterFound) {
						logg->log("Unable to find the relevant chapter for the lang requested skipping...");
					}

				}
			
		}
		
		
		//chapters are in reverse order unreverse them
		std::reverse(vinfo.chapters.begin(),vinfo.chapters.end());
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

void MangaDex::getFilesInChapter(chapterInfo* cinfo,std::string chapterID) {
	std::string responce = sendRequestUsingBASEURL(this->BASEURL_CHAPTER_IMAGES+chapterID);
	simdjson::ondemand::parser parser;

	auto json = parser.iterate(responce);
	cinfo->hash = convertFromViewToString(json["chapter"]["hash"].get_string().value());
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
		if (language == lang) {
			logg->log("Chapter:" + chapterID + " matches the target language");
			return true;
		}
		return false;
}
