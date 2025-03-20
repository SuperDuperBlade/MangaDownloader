#include  <iostream>

#include "util/Logger.h";

#include "MangaDex.h"

int main(int argc,char* argv[]) {

	


	//Handling and assining of the arguments
	Logger logger;
	CmdParser parser(false);

	MangaDex dex(&parser,&logger,argc,argv);
	dex.writeMangaToDisk();

}