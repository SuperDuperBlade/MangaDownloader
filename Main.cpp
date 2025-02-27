#include  <iostream>
#include "util/Logger.h";

#include "MangaDex.h"

int main(int argc,char* argv[]) {

	//Handling and assining of the arguments

	std::string mangaID, outputDir;
	const std::string mangaID_identifier = "-i";
	const std::string outputDir_identifier = "-o";
	Logger* logger = new Logger();


	CmdParser parser(true);
		
	parser.addOption(mangaID_identifier, "The id of the manga you want to download from mangaDex", true, true);
	parser.addOption(outputDir_identifier, "The directory you want to write the manga to (defualt is the dir the script is running in)", false, true);
	parser.passArguments(argc, argv);
	
	MangaDex dex(&parser,logger);

	dex.getTitle();
	dex.getMangaMetaData();
}