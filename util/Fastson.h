#pragma once
#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
class Fastson
{
public:
	Fastson();
	static std::string perpareJsonForParsing(std::string json);
	static std::string  fastSearchValue_fromFirst(std::string json, std::string parent);
	static std::vector<std::string> splitStringOnce(std::string);
	static std::vector<std::string> splitString(std::string, std::string token);
	
private:

};

std::string Fastson::perpareJsonForParsing(std::string json) {
	 json.erase(std::remove(json.begin(), json.end(), '"'), json.end());
	 json.erase(std::remove(json.begin(), json.end(), '['), json.end());
	 json.erase(std::remove(json.begin(), json.end(), ']'), json.end());
	 return json;
}
//prone to be buggy be carefull with this
std::string Fastson::fastSearchValue_fromFirst(std::string json, std::string parentKey) {
	json = perpareJsonForParsing(json);
	size_t pos = json.find(parentKey);

	std::vector<std::string> toReturn;
	if (pos != std::string::npos) {
		json = json.substr(pos + parentKey.length() + 2, json.length());
		json = json.substr(0, json.find("}"));
		//std::string key = json.substr(0, json.find(":"));
		//std::string value = json.substr(json.find(":") + 1, json.length());
		return json.substr(json.find(":")+1,json.length());
	}
	return NULL;
}

std::vector<std::string> Fastson::splitString(std::string token,std::string key) {
	std::vector<std::string> values;
	size_t pos = 0;
	
	while ((pos = token.find(key)) != std::string::npos)
	{
		std::string value = token.substr(0, pos);
		values.push_back(value);
		token.erase(0,pos+key.length());
	}
	return values;
}