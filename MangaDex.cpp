#include "MangaDex.h"

using namespace std;
MangaDex::MangaDex(string mangaID ,string outputDir) {
	this->mangaID = mangaID;
	this->outputDir = outputDir;
}
MangaDex::MangaDex(string mangaID) {
	this->mangaID = mangaID;
}

//Gets the title of the manga
string MangaDex::getTitle(std::string mangaID) {

}
string MangaDex::getTitle() {
	return	this->getTitle(this->mangaID);
}
