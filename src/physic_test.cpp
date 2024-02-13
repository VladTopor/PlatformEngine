//
// Created by vladislav on 21.01.24.
//
//
// Created by vladislav on 03.02.24.
//

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include "EngineEditor.h"
#include <string>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_stdlib.h"
#include <Object.h>
#include <EngineFunctions.h>
#include <config.h>
#include <math.h>
#include <fstream>
#include <json.hpp>

using namespace std;
using json = nlohmann::json;
float max(float a, float b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
float min(float a, float b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
int object_count = 0;
int r_bg = 147;
int g_bg = 245;
int b_bg = 240;
bool smooth_x = false;
bool smooth_y = false;
static bool fixedCamera = false;
static float cameraScale = 1.0;
static float camera_x = 0;
static float camera_y = 0;
float cam_edit_x = 0;
float cam_edit_y = 0;
float cam_edit_scale = 1.0;
int screen_width = 1920;
int screen_height = 1080;


int main() {
    sf::RenderWindow window;
    window.setFramerateLimit(60);

    sf::err().rdbuf(NULL);

    sf::Clock imgui_clock;
    sf::Clock clock;


    int r_pr = 255;
    int g_pr = 255;
    int b_pr = 255;
    int a_pr = 255;




    bool tilemode = true;

    float camera_speed = 3;

    int width = 1920;
    int height = 1080;

    EngineObject preview;
    preview.isActive = true;
    preview.init();

    string texturePathPreview;

    string scriptContent;
    string scriptName;

    sf::View camera(sf::FloatRect(0.0f, 0.0f, width, height));
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                width = event.size.width;
                height = event.size.height;
                camera.setSize(width, height);
                window.setView(camera);
            }

        }



        window.clear(sf::Color(r_bg, g_bg, b_bg));
        window.setView(camera);
        float currentTime = clock.restart().asSeconds();
        float fps = 1.f / (currentTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera_x -= camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera_x += camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera_y -= camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera_y += camera_speed*getTickrateMultiplier(fps, 60);
        camera_x = max(camera_x, 960);
        camera_y = max(camera_y, 540);
        camera.setCenter(camera_x, camera_y);


        ImGui::SFML::Update(window, imgui_clock.restart());


        ImGui::SFML::Render(window);

        window.display();

    }
    return 0;
}
