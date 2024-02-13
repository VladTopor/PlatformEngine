//
// Created by vladislav on 20.01.24.
//

#ifndef PERPG_ENGINEFUNCTIONS_H
#define PERPG_ENGINEFUNCTIONS_H

#include <SFML/Graphics.hpp>
#include <string>
#include <math.h>
using namespace std;

sf::Vector2f fixed_pos(float camera_x, float camera_y, int screenWidth, int screenHeight, float fixed_x, float fixed_y);
void openWebBrowser(string url);
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);

string getLocalizedString(string key, string local);
string getLocalizedString(string key, string local, string a);
string getLocalizedString(string key, string local, string a, string b);
string getLocalizedString(string key, string local, string a, string b, string c);

void initiateLocalization();
string floatToString(float d);
float getTickrateMultiplier(float fps, int target);

template<typename T>
T Vector2length(const sf::Vector2<T>& v);
float toDegree(float radian);
float toRadian(float degree);

#endif //PERPG_ENGINEFUNCTIONS_H
