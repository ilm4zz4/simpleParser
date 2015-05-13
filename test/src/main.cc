#include "Parser.h"
#include <iostream>
int main( int argc, char* argv[]){
Parser* cParser = new Parser();
	
	if ( cParser->setParameter("config.ini", '=') != true){
		std::cerr << "Error set parameter" << std::endl;
	}		

	char tmpBuffer[1024];
	cParser->getStr("Communication", "Comm Ip Grafics Interface", tmpBuffer, sizeof(tmpBuffer));
	std::cout << "tmpBuffer: " << tmpBuffer << std::endl;

	int value;
	cParser->getInt("Communication", "Comm CLC", &value);
	std::cout << "value: " << value << std::endl;

	bool boolean;
	cParser->getBool("Communication","Comm Graphics Interface", &boolean);
	if(boolean){
		std::cout << "true" << std::endl;
	}else{        
		std::cout << "false"  << std::endl;
	}

	double val;
	cParser->getDouble("Communication", "Comm freq", &val);
	std::cout << "double: " << val << std::endl;
}

