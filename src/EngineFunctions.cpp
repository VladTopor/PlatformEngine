//
// Created by vladislav on 20.01.24.
//

#include <fstream>
#include "EngineFunctions.h"
#include "json.hpp"
#include <config.h>

sf::Vector2f fixed_pos(float camera_x, float camera_y, int screenWidth, int screenHeight, float fixed_x, float fixed_y) {
    return sf::Vector2f((camera_x-screenWidth/2+fixed_x), (camera_y-screenHeight/2+fixed_y));
}
using json = nlohmann::json;
using namespace std;
json langJson;
enum Controlling {
    Keyboard,
    Joystick
};
enum Platforms {
    Windows,
    Linux,
    MacOS,
    Android,
    iOS,
    Web
};
Platforms platform = Linux;
void initiateLocalization() {
    std::ifstream ifs("assets/configs/langs.json");

    langJson = json::parse(ifs);
}
void openWebBrowser(string url) {
    if (platform == Linux) {
        system(("xdg-open "+url).c_str());
    } else if (platform == Windows) {
        system(("start "+url).c_str());
    }
}
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
string getLocalizedString(string key, string local) {
    string s = "";
    try {
        s = langJson[local][key];
    } catch (json::exception) {
        s = key;
    }
    return s;
}
string getLocalizedString(string key, string local, string a) {
    string s = "";
    try {
        s = langJson[local][key];
    } catch (json::exception) {
        s = key;
    }
    s = ReplaceAll(s, "{a}", a);
    return s;
}
string getLocalizedString(string key, string local, string a, string b) {
    string s = "";
    try {
        s = langJson[local][key];
    } catch (json::exception) {
        s = key;
    }
    s = ReplaceAll(s, "{a}", a);
    s = ReplaceAll(s, "{b}", b);
    return s;
}
string getLocalizedString(string key, string local, string a, string b, string c) {
    string s = "";
    try {
        s = langJson[local][key];
    } catch (json::exception) {
        s = key;
    }
    s = ReplaceAll(s, "{a}", a);
    s = ReplaceAll(s, "{b}", b);
    s = ReplaceAll(s, "{c}", c);
    return s;
}
template<typename T>
T Vector2length(const sf::Vector2<T>& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
string floatToString(float d)
{
    stringstream ss;
    ss << d;
    return ss.str();
}
float getTickrateMultiplier(float fps, int target)
{
    return (float)target/fps;
}
float toDegree(float radian) {
    return radian * (180/PI);
}
float toRadian(float degree) {
    return degree * PI/180;
}