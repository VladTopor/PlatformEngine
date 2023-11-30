//
// Created by vladislav on 28.11.23.
//

#define PERPG_MULTIPLAYER_H
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "json.hpp"
using namespace std;
class ServerPlayer {
public:
    string username;
    float x;
    float y;
    string address;
    int id;
    string text_id;
    void join();
    void get_player();

};
class ClientPlayer {
    string username;
    float x;
    float y;
    int id;
    string text_id;
    void join();
    void get_player();
};
class multiplayer {

};
class Client {
public:
    int count;
    string address;
    int port;

    int get_count();
};
string sendRequest(string url);


