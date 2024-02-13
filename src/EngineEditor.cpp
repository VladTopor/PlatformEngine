//
// Created by vladislav on 21.01.24.
//

#include <SFML/Graphics/RenderWindow.hpp>
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
#if LAUNCH_EDITOR == true
    EngineObject *editorObjects = new EngineObject[OBJECT_LIMIT];
#else
    EngineObject *editorObjects = new EngineObject[1];
#endif
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

void load_edit(string levelName, bool fast) {
    std::ifstream level_file("assets/levels/"+levelName+"/level.json");
    for (int j = 0; j < object_count; j++) {
        editorObjects[j].isActive = false;
    }
    json level = json::parse(level_file);
    object_count = level["count"];
    r_bg = level["background"][0];
    g_bg = level["background"][1];
    b_bg = level["background"][2];
    /*try {
    for (int i = 0; i < level["staticTexturesCount"]; i++) {
        if (!staticTextures[i].loadFromFile(level["staticTextures"][i])){
            fatal("Failed to load static texture");
        }
    }
    } catch (json::exception::detail) {
        warn("123");
    }*/
    //speed = level["speed"];
    //fastSpeed = level["fastSpeed"];

    //b2Vec2 gravity(level["gravity_x"], level["gravity_y"]);
    //b2World world(gravity);
    if (!fast) {
        delete[] editorObjects;
        editorObjects = new EngineObject[object_count];
    }
    cam_edit_x = level["camera"]["pos"][0];
    cam_edit_y = level["camera"]["pos"][1];
    fixedCamera = level["camera"]["fixed"];
    cameraScale = level["camera"]["scale"];

    for (int i = 0; i < object_count; i++) {
        if (level["version"] == 3) { // Level 3 version parser
            //warn("Legacy level version! This version level will be removed later! Upgrade it to use without problems");
            //editorObjects[i].staticTexture = level["editorObjects"][i]["staticTexture"];
            editorObjects[i].x = level["objects"][i]["position"]["x"];
            editorObjects[i].y = level["objects"][i]["position"]["y"];
            editorObjects[i].z = level["objects"][i]["position"]["z"];
            editorObjects[i].id = i;
            editorObjects[i].r = level["objects"][i]["color"]["r"];
            editorObjects[i].g = level["objects"][i]["color"]["g"];
            editorObjects[i].b = level["objects"][i]["color"]["b"];
            editorObjects[i].a = level["objects"][i]["color"]["a"];
            editorObjects[i].isActive = true;
            editorObjects[i].ghost = !level["objects"][i]["collide"];
            try {
                editorObjects[i].texturePath = level["objects"][i]["texture"];
            } catch (nlohmann::json::exception) {
                editorObjects[i].texturePath = "assets/textures/emo/64x64.png";
            }



            editorObjects[i].width = level["objects"][i]["width"];
            editorObjects[i].height = level["objects"][i]["height"];

            editorObjects[i].isSmooth = level["objects"][i]["isSmooth"];
            editorObjects[i].isStatic = level["objects"][i]["isStatic"];
            if (level["objects"][i]["scripts"]["OnTick"] != false) {
                editorObjects[i].loop_script = level["objects"][i]["scripts"]["OnTick"];
            } else {
                editorObjects[i].loop_script = "assets/scripts/blank.lua";
            }

            editorObjects[i].rotation = level["objects"][i]["rotation"];
            editorObjects[i].tag = level["objects"][i]["tag"];
            //editorObjects[i].loop.enabled = false;
            if (!fixedCamera && editorObjects[i].tag == "player") {
                cam_edit_x = level["camera"]["pos"][0];
                cam_edit_y = level["camera"]["pos"][1];
                //cam_edit_x += editorObjects[i].x/2;
                //cam_edit_y += screen_height/2;

            }
            if (editorObjects[i].tag == "trigger") {
                editorObjects[i].tag = "trigger";
                editorObjects[i].collide_script = level["objects"][i]["scripts"]["OnCollide"];
            }
            editorObjects[i].init();

        }
        else
        {
            //fatal("Failed to load level "+levelName+", because LevelVersion is illegal");
        }
        if (level["smooth"]) {
            for (int j = 0; j <object_count; j++) {
                if (editorObjects[j].tag == "player") {
                    if (level["settings"]["smooth_x"] == true) {
                        smooth_x = true;
                    }
                    if (level["settings"]["smooth_y"] == true) {
                        smooth_y = true;
                    }
                }
            }
        }
    }
}

int editor(sf::RenderWindow &window, string levelName) {
    window.setTitle("Bark editor");
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
            if (event.type == sf::Event::MouseButtonPressed) {
                if (!ImGui::IsAnyItemHovered()) {
                    editorObjects[object_count].x = preview.x;
                    editorObjects[object_count].y = preview.y;
                    editorObjects[object_count].rotation = preview.rotation;
                    editorObjects[object_count].texturePath = preview.texturePath;
                    editorObjects[object_count].width = preview.width;
                    editorObjects[object_count].height = preview.height;
                    editorObjects[object_count].r = preview.r;
                    editorObjects[object_count].g = preview.g;
                    editorObjects[object_count].b = preview.b;
                    editorObjects[object_count].a = preview.a;
                    editorObjects[object_count].physic_enabled = preview.physic_enabled;
                    editorObjects[object_count].physic_static = preview.physic_static;
                    editorObjects[object_count].ghost = preview.ghost;
                    editorObjects[object_count].tag = preview.tag;
                    editorObjects[object_count].collide_script = preview.collide_script;
                    editorObjects[object_count].loop_script = preview.loop_script;
                    editorObjects[object_count].isStatic = preview.isStatic;
                    editorObjects[object_count].isSmooth = preview.isSmooth;
                    editorObjects[object_count].isActive = preview.isActive;
                    editorObjects[object_count].isActivated = preview.isActivated;
                    editorObjects[object_count].isFixed = preview.isFixed;
                    editorObjects[object_count].staticTexture = preview.staticTexture;



                    editorObjects[object_count].init();
                    object_count++;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Z) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        editorObjects[object_count].isActive = false;
                        if (object_count != 0) {
                            object_count--;
                        }
                    }
                }
            }

        }



        window.clear(sf::Color(r_bg, g_bg, b_bg));
        window.setView(camera);
        for (int i=0; i < object_count; i++) {
            //editoreditorObjects[i].init();
            editorObjects[i].apply();
            editorObjects[i].render(window, 0, 0);

        }
        float currentTime = clock.restart().asSeconds();
        float fps = 1.f / (currentTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera_x -= camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera_x += camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera_y -= camera_speed*getTickrateMultiplier(fps, 60);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera_y += camera_speed*getTickrateMultiplier(fps, 60);
        camera_x = max(camera_x, 960);
        camera_y = max(camera_y, 540);
        camera.setCenter(camera_x, camera_y);

        sf::RectangleShape cameraRect;


        cameraRect.setSize(sf::Vector2f(screen_width*cameraScale, screen_height*cameraScale));
        cameraRect.setOrigin(screen_width*cameraScale/2, screen_height*cameraScale/2);
        cameraRect.setPosition(sf::Vector2f(cam_edit_x, cam_edit_y));
        cameraRect.setFillColor(sf::Color(0, 0, 0, 0));
        cameraRect.setOutlineColor(sf::Color(255-r_bg, 255-g_bg, 255-g_bg, 128));
        cameraRect.setOutlineThickness(2);
        window.draw(cameraRect);

        ImGui::SFML::Update(window, imgui_clock.restart());
        //cout << object_count << endl;

        ImGui::Begin("Bark editor");

        ImGui::Text(floatToString(fps).c_str());
        ImGui::Checkbox("Tilemode", &tilemode);
        ImGui::SliderFloat("Camera speed", &camera_speed, 0.1, 100);
        ImGui::InputText("Level name", &levelName);
        if (ImGui::Button("Save")) {
            json level;
            level["version"] = 3;
            level["count"] = object_count;
            level["smooth"] = smooth_x||smooth_y;
            level["background"][0] = r_bg;
            level["background"][1] = g_bg;
            level["background"][2] = b_bg;
            level["camera"]["fixed"] = fixedCamera;
            level["camera"]["pos"][0] = cam_edit_x;
            level["camera"]["pos"][1] = cam_edit_y;
            level["camera"]["scale"] = cameraScale;
            level["OnLoad"] = "";
            level["speed"] = 10;
            level["fastSpeed"] = 20;
            level["settings"]["smooth_x"] = smooth_x;
            level["settings"]["smooth_y"] = smooth_y;
            for (int i=0; i < object_count; i++ ) {
                cout << (editorObjects[i].texturePath) << endl;
                level["objects"][i]["texture"] = editorObjects[i].texturePath;
                level["objects"][i]["position"]["x"] = editorObjects[i].x;
                level["objects"][i]["position"]["y"] = editorObjects[i].y;
                level["objects"][i]["position"]["z"] = editorObjects[i].z;
                level["objects"][i]["width"] = editorObjects[i].width;
                level["objects"][i]["height"] = editorObjects[i].height;
                level["objects"][i]["isSmooth"] = editorObjects[i].isSmooth;
                level["objects"][i]["isStatic"] = editorObjects[i].isStatic;
                level["objects"][i]["tag"] = editorObjects[i].tag;
                level["objects"][i]["rotation"] = editorObjects[i].rotation;
                level["objects"][i]["color"]["r"] = editorObjects[i].r;
                level["objects"][i]["color"]["g"] = editorObjects[i].g;
                level["objects"][i]["color"]["b"] = editorObjects[i].b;
                level["objects"][i]["color"]["a"] = editorObjects[i].a;
                level["objects"][i]["scripts"]["OnTick"] = editorObjects[i].loop_script;
                level["objects"][i]["scripts"]["OnCollide"] = editorObjects[i].collide_script;
                level["objects"][i]["collide"] = !editorObjects[i].ghost;
                level["objects"][i]["physics"]["enabled"] = editorObjects[i].physic_enabled;
                level["objects"][i]["physics"]["dynamic"] = !editorObjects[i].physic_static;
            }
            if (!filesystem::exists("assets/"+levelName)) {
                filesystem::create_directory("assets/levels/"+levelName);
            }
            ofstream file("assets/levels/"+levelName+"/level.json");
            file << level.dump(4);
            file.close();

        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            load_edit(levelName, false);
        }
        ImGui::SameLine();
        ImGui::Button("Play");

        ImGui::End();
        ImGui::Begin("Camera setting");
        ImGui::InputInt("Screen width", &screen_width);
        ImGui::InputInt("Screen height", &screen_height);
        ImGui::InputFloat("Camera X", &cam_edit_x);
        ImGui::InputFloat("Camera Y", &cam_edit_y);
        ImGui::InputFloat("Camera scale", &cameraScale);

        ImGui::Checkbox("Static camera", &fixedCamera);
        ImGui::End();
        ImGui::Begin("Level settings");
        ImGui::Text(("Objects count = "+floatToString(static_cast<float>(object_count))).c_str());
        ImGui::SliderInt("R", &r_bg, 0, 255);
        ImGui::SliderInt("G", &g_bg, 0, 255);
        ImGui::SliderInt("B", &b_bg, 0, 255);

        ImGui::Checkbox("Smooth X", &smooth_x);
        ImGui::Checkbox("Smooth Y", &smooth_y);

        ImGui::End();

        ImGui::Begin("Object editor");

        ImGui::InputFloat("Width", &preview.width);
        ImGui::InputFloat("Height", &preview.height);
        ImGui::Checkbox("Smooth", &preview.isSmooth);
        ImGui::InputText("Tag", &preview.tag);
        ImGui::SliderFloat("Rotation", &preview.rotation, -360, 360);

        ImGui::Text("Color");

        ImGui::SliderInt("R", &preview.r, 0, 255);
        ImGui::SliderInt("G", &preview.g, 0, 255);
        ImGui::SliderInt("B", &preview.b, 0, 255);
        ImGui::SliderInt("A", &preview.a, 0, 255);

        ImGui::Checkbox("Ghost", &preview.ghost);

        ImGui::Text("Scripts");

        ImGui::InputText("Every tick", &preview.loop_script);
        ImGui::InputText("Collide script", &preview.collide_script);
        ImGui::Text("Physics");
        ImGui::Checkbox("Enable physics", &preview.physic_enabled);
        ImGui::Checkbox("Static physics", &preview.physic_static);
        ImGui::Text("Texture");

        ImGui::InputText("Texture", &texturePathPreview);


        ImGui::Image(preview.texture);
        if (texturePathPreview == "") {
            texturePathPreview = "emo/64x64";
        }
        preview.texturePath = "assets/textures/"+texturePathPreview+".png";

        sf::Vector2i mousepos = sf::Mouse::getPosition(window);
        sf::Vector2f pos = window.mapPixelToCoords(mousepos, window.getView());
        sf::Texture texture = preview.texture;

        if (!tilemode) {
            preview.x = pos.x - texture.getSize().x / 2 * preview.width;
            preview.y = pos.y - texture.getSize().y / 2 * preview.height;
        }
        else {
            preview.x = pos.x;
            preview.y = pos.y;
            preview.x = static_cast<int>(preview.x/texture.getSize().x)*texture.getSize().x;
            preview.y = static_cast<int>(preview.y/texture.getSize().y)*texture.getSize().y;

        }
        preview.apply();
        preview.init();

        preview.render(window, 0, 0);


        //cout << pos.x << ' ' << pos.y << endl;

        preview.sprite.setScale(sf::Vector2f(preview.width, preview.height));

        ImGui::End();

        ImGui::Begin("Script editor");

        ImGui::InputText("Script name", &scriptName);
        ImGui::InputTextMultiline("Script content", &scriptContent);
        if (ImGui::Button("Save")) {
            ofstream file("assets/scripts/"+scriptName+".lua");
            file << scriptContent;
            file.close();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            ifstream file(scriptName);
            string content;
            scriptContent = "";
            while (getline (file, content)) {
                scriptContent += content;
            }
            file.close();
        }

        ImGui::End();


        ImGui::SFML::Render(window);

        window.display();

    }
}