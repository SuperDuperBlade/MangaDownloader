#include "MangaDex.h"


MangaDex::MangaDex(std::string mangaID ,std::string outputDir,Logger* logger) {
	this->mangaID = mangaID;
	this->outputDir = outputDir;
	this->logg = logger;
	this->init();
}
MangaDex::MangaDex(std::string mangaID,Logger* logger) {
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
	std::cout << "Sending request to: " << BASEURL + addonURl << '\n';
	auto res = this->baseCli.Get(addonURl);
	if (res) {
		this->logg->log("Success: " + res->body);
	}else {
		std::cout << "Encountered error" << to_string(res.error()) << '\n';
	}

	std::cout << "starting parsing \n";
	
	return res->body;
}


//Gets the title of the manga
std::string MangaDex::getTitle(std::string mangaID) {
	std::string responce = sendRequestUsingBASEURL(BASEURL_COVER + mangaID);
	std::string title = Fastson::fastSearchValue_fromFirst(responce, "title");

	std::cout << "Title:" << title << '\n';
	return title;
}
std::string MangaDex::getTitle() {
	return	this->getTitle(this->mangaID);
}

std::string MangaDex::getCoverFileName(std::string mangaID) {
	std::string responce = sendRequestUsingBASEURL(BASEURL_COVER + mangaID);
	std::string title = Fastson::fastSearchValue_fromFirst(responce, "title");

	std::cout << "Title: " << title << '\n';
	return title;
}
std::string MangaDex::getCoverFileName() {
	getCoverFileName(mangaID);
	return "";
}
bool MangaDex::writeMangaToDisk(std::string dir, std::string mode) {
	if (!FileHandler::checkIfExists(dir, true)) {
		
	}
	


	return false;
}

