#pragma once
#include <string>
std::string generateSalt();
std::string sha256(const std::string& password , const std::string& salt);
