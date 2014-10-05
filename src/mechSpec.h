#pragma once
#include <string>
#include <map>

class mechSpec
{
public:
	mechSpec();

	std::string legName;
	std::string bodyName;
	std::string armName;
	std::string shoulderName;

	std::map<int,std::string> equipment;

	bool saveToFile(std::string fileName);
	bool loadFromFile(std::string fileName);
	bool deleteFile(std::string fileName);
};

