
#include "multiplayer.h"
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;
string sendRequest(string url, int port, string req) {

    sf::Http http;
    http.setHost(url, port);
    sf::Http::Request request(req);
    sf::Http::Response response = http.sendRequest(request);
    return response.getBody();
}
void ServerPlayer::join()  {
        cout << "test";
        text_id = sendRequest("http://"+address, 5000, "/server/join?username="+username);
        istringstream (text_id) >> id;
}
void ServerPlayer::get_player() {
    auto json_object = json::parse(sendRequest("http://"+address, 5000, "/server/get/"+to_string(id)+"?x="+to_string(x)+"&y="+to_string(y)));
    x = json_object["x"];
    y = json_object["y"];

}
/*int ServerPlayer::get_count() {
    auto json_object = json::parse(sendRequest("http://"+address, 5000, "/server/count"));
    return json_object["count"];

}*/