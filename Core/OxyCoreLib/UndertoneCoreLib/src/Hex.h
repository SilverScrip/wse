#include <string>
#include <iostream>

namespace Hex {

void stream2hex(const std::string str, std::string& hexstr, bool capital = false);
void hex2stream(const std::string hexstr, std::string& str);

}