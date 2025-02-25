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
		this->logg->log("Success: " + res->body);
	}
	else {
		std::cout << "Encountered error" << to_string(res.error()) << '\n';
	}

	std::cout << "starting parsing \n";

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
			std::string chapterID = convertFromViewToString(json["volumes"][vinfo.title]["chapters"][chatperN]["id"]);
			logg->log(chapterID);
		}
	}
	return volumes;
}
std::string MangaDex::convertFromViewToString(std::string_view value) {
	std::string temp{ value };
	temp.erase(std::remove(temp.begin(), temp.end(), '"'), temp.end());
	return temp;
}