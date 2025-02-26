#include "MangaDex.h"


MangaDex::MangaDex(std::string mangaID, std::string outputDir, Logger* logger) {
	this->mangaID = mangaID;
	this->outputDir = outputDir;
	this->logg = logger;
	this->init();
}
MangaDex::MangaDex(std::string mangaID, Logger* logger) {
	this->mangaID = mangaID;
	this->logg = logger;
	this->init();
}

void MangaDex::init() {
	//	this->baseCli =  httplib::Client(this->BASEURL);
	//	this->baseCli.set_follow_location(true);
	//	this->baseDownloadCli = httplib::Client(this->BASEDOWNLOAD_URL);
}

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
std::string MangaDex::getTitle() {
	return	this->getTitle(this->mangaID);
}

std::string MangaDex::getCoverFileName(std::string mangaID) {


	return "title";
}
std::string MangaDex::getCoverFileName() {
	getCoverFileName(mangaID);
	return "";
}


bool MangaDex::writeMangaToDisk(std::string dir, std::string mode) {
	if (!FileHandler::checkIfExists(dir, true)) {
		logg->log("Dir:" + dir + " ,not found attempting to create");
	}




	return false;
}
//gets the correct chapters sorts them into their respective volumes and gets the files for the respective chapters needed for parsing
std::vector<volumeInfo> MangaDex::getMangaMetaData() {

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
					try {
						cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["title"].get_string().value());
					}
					catch (simdjson::simdjson_error e) {
						logg->errorLog(e.what());
						logg->log("Title not found leaving it blank");
						cinfo.title = "";
					}
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
							cinfo.title = convertFromViewToString(chapter_json["data"]["attributes"]["title"].get_string().value());
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
	}
	//volumes are in reverse order unreverse them
	std::reverse(volumes.begin(), volumes.end());
	return volumes;
}
std::string MangaDex::convertFromViewToString(std::string_view value) {
	std::string temp{ value };
	temp.erase(std::remove(temp.begin(), temp.end(), '"'), temp.end());
	return temp;
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
