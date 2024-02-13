#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <PhysicEngine.hpp>
#include <fstream>
#include "json.hpp"
#include "LuaCpp.hpp"
//#include <PhysicEngine.hpp>
#include <math.h>
#include "multiplayer.h"
#include <sfeMovie/Movie.hpp>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui-SFML.h"
#include "ItemStack.h"
#include "Object.h"
#include "EngineFunctions.h"
#include "EngineEditor.h"
#include <Widget.h>
#include <config.h>
#include "json.hpp"
using json = nlohmann::json;

template <typename T>
void info(T log) {
    cout << "[\x1B[94mINFO\x1B[37m] " << log << '\n';}
template <typename T>
void warn(T log) {
    cout << "[\x1B[93mWARN\x1B[37m] " << log << '\n';}
template <typename T>
void fatal(T log) {
    cout << "[\x1B[91mFATAL\x1B[37m] " << log << '\n';
    exit(1);}
template <typename T>
void sucess(T log) {
    cout << "[\x1B[92mSUCCESS\x1B[37m] " << log << '\n';}
template <typename T>
void debug(T log) {
    if (DEBUG_LOGS) {
        cout << "[\x1B[95mDEBUG\x1B[37m] " << log << '\n';
    }
}
void error(string text, sf::RenderWindow &window);

using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;




float camera_x = 0;
float camera_y = 0;

json save;
unsigned long object_limit = OBJECT_LIMIT;
//unsigned long object_limit = 2147483647/4;
#if LAUNCH_EDITOR == false
    EngineObject *objects = new EngineObject[object_limit];
#else
    EngineObject *objects = new EngineObject[1];
#endif
Widget *widgets = new Widget[object_limit];
int objects_count = 0;
int widgets_count = 0;
bool fixedCamera;
float cameraScale = 1.0f;
string lang = "uk-UA";
sf::Color background;
bool isLoading = true;
string currentLoading;
sf::SoundBuffer *buffers = new sf::SoundBuffer[64];
sf::Sound *sounds = new sf::Sound[64];
string currentLevel = "";
ServerPlayer *players = new ServerPlayer[128];
EngineObject *playersEngineObjects = new EngineObject[128];
LuaCpp::LuaContext lua;
sf::Texture *staticTextures = new sf::Texture[64];
std::shared_ptr<LuaLibrary> engine_lib = std::make_shared<LuaLibrary>("engine");
float speed;
float fastSpeed;
bool isLeft = false;
bool isRight = false;
bool isUp = false;
bool isDown = false;
bool fixedPhysics = false;

void play_Sound(string path, int channel, bool loop) {
    if (!buffers[channel].loadFromFile(path)) {fatal("Failed to load sound");}
    sounds[channel].setBuffer(buffers[channel]);
    sounds[channel].setLoop(loop);
    sounds[channel].play();
}
bool run = false;
void load_ui(string levelName) {
    info("Loading UI layout "+levelName);

    std::ifstream level_file("assets/ui_layouts/"+levelName+".json");
    json level = json::parse(level_file);
    widgets_count = level["count"];

    //b2Vec2 gravity(level["gravity_x"], level["gravity_y"]);
    //b2World world(gravity);

    isLoading = true;
    for (int j = 0; j < widgets_count; j++) {
        objects[j].isActive = false;
    }
    for (int i = 0; i < widgets_count; i++) {
        if (true) { // Level 3 version parser
            //warn("Legacy level version! This version level will be removed later! Upgrade it to use without problems");
            //debug("Applying positions");
            widgets[i].fixedX = level["widgets"][i]["x"];
            widgets[i].fixedY = level["widgets"][i]["y"];
            widgets[i].isFixed = true;
            widgets[i].isActive = true;
            widgets[i].texturePath = "assets/textures/emo/2x2.png";
            if (level["widgets"][i]["type"] == "button") {
                widgets[i].texturePath = "assets/textures/button.png";

                widgets[i].width =  static_cast<float>(level["widgets"][i]["width"]) / 256.0f;
                widgets[i].height = static_cast<float>(level["widgets"][i]["height"]) / 64.0f;
                widgets[i].loop_script = "assets/scripts/blank.lua";
                widgets[i].collide_script = "assets/scripts/blank.lua";

                if (static_cast<bool>(level["widgets"][i]["OnMouseDown"]) != false) {
                    widgets[i].collide_script = level["widgets"][i]["OnMouseDown"];
                }
                if (level["widgets"][i]["OnMouseUp"] != false) {

                    widgets[i].loop_script = level["widgets"][i]["OnMouseUp"];
                }

            } else if(level["widgets"][i]["type"] == "label") {


            }
            else {

            }
            widgets[i].offsetX = level["widgets"][i]["offset"][0];
            widgets[i].offsetY = level["widgets"][i]["offset"][1];
            widgets[i].textSize = level["widgets"][i]["fontSize"];
            widgets[i].tag = level["widgets"][i]["type"];
            widgets[i].text = level["widgets"][i]["text"];
            //objects[i].loop.enabled = false
            widgets[i].physic_enabled = false;
            widgets[i].init();

        }

    }
    sucess(getLocalizedString("engine.sucess.done", lang));
    isLoading = false;

}
bool onBeginRunned = false;
string loaded_level_name = "";
void load_level(string levelName, bool fast) {
    loaded_level_name = levelName;
    info(getLocalizedString("engine.info.loadlevel", lang)+levelName);
    isRight=true;
    std::ifstream level_file("assets/levels/"+levelName+"/level.json");
    for (int j = 0; j < objects_count; j++) {
        objects[j].isActive = false;
    }
    json level = json::parse(level_file);
    int objects_count_old = objects_count;
    objects_count = level["count"];
    background.r = level["background"][0];
    background.g = level["background"][1];
    background.b = level["background"][2];
    //Areset();
    speed = level["speed"];
    fastSpeed = level["fastSpeed"];
    if (level["OnLoad"] != false) {
        lua.CompileFileAndRun(level["OnLoad"]);
    }
    float x = 0;
    float y = 0;
    if (level["smooth"]) {
        for (int j = 0; j <objects_count; j++) {
            if (objects[j].tag == "player") {
                x = objects[j].x;
                y = objects[j].y;
            }
        }
    }

    //b2Vec2 gravity(level["gravity_x"], level["gravity_y"]);
    //b2World world(gravity);
    for (int i = 0; i<objects_count_old; i++){
        if (objects[i].physic_enabled) {
            if (objects[i].physic_static) {
                objects[i].static_physic.remove();
            } else {
                objects[i].dynamic_physic.remove();
            }
        }
    }
    if (!fast) {
        //simulate();

        delete[] objects;
        objects = new EngineObject[object_limit];
    }
    isLoading = true;

    camera_x = level["camera"]["pos"][0];
    camera_y = level["camera"]["pos"][1];
    fixedCamera = level["camera"]["fixed"];
    cameraScale = level["camera"]["scale"];
    if (level["version"] == 3) {
        camera_x -= 32;
        camera_y -= 32;
    }
    for (int i = 0; i < objects_count; i++) {

        if (level["version"] > 0) { // Level 3 version parser
            //warn("Legacy level version! This version level will be removed later! Upgrade it to use without problems");
            //objects[i].staticTexture = level["objects"][i]["staticTexture"];
            objects[i].x = level["objects"][i]["position"]["x"];
            objects[i].y = level["objects"][i]["position"]["y"];
            objects[i].z = level["objects"][i]["position"]["z"];
            objects[i].id = i;
            objects[i].r = level["objects"][i]["color"]["r"];
            objects[i].g = level["objects"][i]["color"]["g"];
            objects[i].b = level["objects"][i]["color"]["b"];
            objects[i].a = level["objects"][i]["color"]["a"];
            objects[i].isActive = true;
            objects[i].ghost = !level["objects"][i]["collide"];
            try {
                objects[i].texturePath = level["objects"][i]["texture"];
            } catch (nlohmann::json::exception) {
                objects[i].texturePath = "assets/textures/emo/64x64.png";
            }



            objects[i].width = level["objects"][i]["width"];
            objects[i].height = level["objects"][i]["height"];

            objects[i].isSmooth = level["objects"][i]["isSmooth"];
            objects[i].isStatic = level["objects"][i]["isStatic"];
            if (level["objects"][i]["scripts"]["OnTick"] != false) {
                objects[i].loop_script = level["objects"][i]["scripts"]["OnTick"];
            } else {
                objects[i].loop_script = "assets/scripts/blank.lua";
            }

            objects[i].rotation = level["objects"][i]["rotation"];
            objects[i].tag = level["objects"][i]["tag"];
            //objects[i].loop.enabled = false;
            if (objects[i].tag == "trigger") {
                objects[i].tag = "trigger";
                objects[i].collide_script = level["objects"][i]["scripts"]["OnCollide"];
            }
            try {
                objects[i].physic_enabled = level["objects"][i]["physics"]["enabled"];
                if (level["objects"][i]["physics"]["dynamic"]) {
                    objects[i].physic_static = false;
                } else {
                    objects[i].physic_static = true;
                }

            } catch (json::exception) {
                warn("Physics settings not defined!");
                objects[i].physic_enabled = true;
                objects[i].physic_static = true;
                if (objects[i].tag == "player") {
                    setGravity(b2Vec2(0, 0));
                    objects[i].physic_static = false;


                }
            }

            objects[i].init();
            if (objects[i].tag == "player") {
                objects[i].getBody()->SetFixedRotation(true);
            }

        }
        else
        {
            fatal("Failed to load level "+levelName+", because LevelVersion is illegal");
        }
        fixedPhysics = true;
        if (level["smooth"]) {
            for (int j = 0; j <objects_count; j++) {
                if (objects[j].tag == "player") {
                    if (level["settings"]["smooth_x"] == true) {
                        objects[j].x = x;
                    }
                    if (level["settings"]["smooth_y"] == true) {
                        objects[j].y = y;
                    }
                }
            }
        }
    }
    isLoading = false;
    currentLevel = levelName;
}


extern "C" {

int modifyEngineObject(lua_State *L) {
    int n = lua_gettop(L);
    if (n!=3) {
        warn(getLocalizedString("engine.fatals.luafatal", lang, "modifyEngineObject(id, property, value)"));
    } else {
        string property = lua_tostring(L, 2);
        int id = lua_tonumber(L, 1);
        if (property == "x") {
            float value = lua_tonumber(L, 3);
            objects[id].getBody()->SetTransform(b2Vec2(value, objects[id].getBody()->GetPosition().y), objects[id].getBody()->GetAngle());
            objects[id].apply();
            //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].body->GetAngle());

        }

        if (property == "y") {
            float value = lua_tonumber(L, 3);
            objects[id].getBody()->SetTransform(b2Vec2(objects[id].getBody()->GetPosition().x, value), objects[id].getBody()->GetAngle());
            objects[id].apply();
            //info(value);
            //info(id);
            //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].body->GetAngle());

        }
        if (property == "rot") {
            float value = lua_tonumber(L, 3);
            objects[id].getBody()->SetTransform(objects[id].getBody()->GetPosition(), value);
            objects[id].apply();
            //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].rotation);
        }

    }
    return 0;
}
int loadLevel(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 3) {

        warn(getLocalizedString("engine.fatals.luafatal", lang, "loadLevel(level, fast)"));
        lua_pushliteral(L, "Failed to load this level!");
        return 0;
    }
    string level = lua_tostring(L, 1);
    load_level(level, lua_toboolean(L, 2));
    return 1;
}
int getLoadedLevel(lua_State *L) {
    lua_pushstring(L, currentLevel.c_str());
    return 0;
}
int loadUi(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 2) {
        warn(getLocalizedString("engine.fatals.luafatal", lang, "loadUi(ui_layout)"));
        lua_pushliteral(L, "Failed to load this layout!");
        return 0;
    }
    string level = lua_tostring(L, 1);
    load_ui(level);
    return 1;
}
int playSound(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 3) {
        warn(getLocalizedString("engine.fatals.luafatal", lang, "playSound(path, line, loop)"));
        lua_pushliteral(L, "Failed to load this level!");
        return 0;
    }
    int channel = lua_tonumber(L, 2);
    string level = lua_tostring(L, 1);
    bool loop = lua_toboolean(L, 3);
    play_Sound(level, channel, loop);
    return 1;
}
int getProperty(lua_State *L) {
    int n = lua_gettop(L);
    lua_Number id = lua_tonumber(L, 1);
    if (n != 2) {
        warn(getLocalizedString("engine.fatals.luafatal", lang, "getProperty(id, property)"));
        lua_pushliteral(L, "Usage: getProperty(id, property)");
    } else {
        string property = lua_tostring(L, 2);
        if (property == "x") {
            id = lua_tonumber(L, 1);
            int id_int = (int) id;
            lua_pushnumber(L, objects[id_int].x);
            return 1;
        }
        if (property == "rot") {
            id = lua_tonumber(L, 1);
            int id_int = (int) id;
            lua_pushnumber(L, objects[id_int].y);
            return 1;
        }
        if (property == "y") {
            id = lua_tonumber(L, 1);
            int id_int = (int) id;
            lua_pushnumber(L, objects[id_int].rotation);
            return 1;
        } else {
            return 0;
        }

    }
    return 0;
}
int setWidgetText(lua_State *L) {
    int n = lua_gettop(L);
    if (n != 2) {
        warn(getLocalizedString("engine.fatals.luafatal", lang, "setWidgetText(id, text)"));
        lua_pushliteral(L, "Usage: setWidgetText(id, text)");
    } else {
        int id = lua_tonumber(L, 1);
        string text = lua_tostring(L, 2);
        widgets[id].text = text;
}
}
}

bool collide(float x1, float y1, float width1, float height1, float x2, float y2, float width2, float height2, char direction, bool isActive) {
    if (!isActive) {
        return false;
    }
    if (direction == 'a') {
        return collide(x1, y1, width1, height1, x2, y2, width2, height2, 'l', isActive) || collide(x1, y1, width1, height1, x2, y2, width2, height2, 'r', isActive) || collide(x1, y1, width1, height1, x2, y2, width2, height2, 'u', isActive) || collide(x1, y1, width1, height1, x2, y2, width2, height2, 'd', isActive);
    }
    //cout << x1 << ' ' << y1 << ' ' << width1 << ' ' << height1 << ' ' << x2 << ' ' << y2 << ' ' << width2 << ' ' << height2 << endl;
    if (direction == 'l') {
        if (((y2 < y1 && y2+height2 > y1) || (y2 < y1+height1 && y2+height2 > y1+height1)) && ((x2 < x1)&&(x2+width2 > x1))) return true;
    }
    if (direction == 'r') {
        if (((y2 < y1 && y2+height2 > y1) || (y2 < y1+height1 && y2+height2 > y1+height1)) && ((x2+width2 > x1+width1)&&(x2 < x1+width1))) return true;
    }
    if (direction == 'u') {
        if (((x2 < x1 && x2+width2 > x1) || (x2 < x1+width1 && x2+width2 > x1+width1)) && ((y2 < y1)&&(y2+height2 > y1))) return true;
    }
    if (direction == 'd') {
        if (((x2 < x1 && x2+width2 > x1) || (x2 < x1+width1 && x2+width2 > x1+width1)) && ((y2+height2 > y1+height1)&&(y2 < y1+height1))) return true;
    }
    return false;
}
int width = 0;
int height = 0;
int beginWidth = 0;
int beginHeight = 0;
int framerate_limit = 0;
int tickrate;
float script_rate = 0.0f;
float script_now = 0;
bool runLoop = false;
bool debug_mode = false;
bool isMultiplayer = false;
string game_title;
string address = "127.0.0.1";
string version = "0.0.4 alpha";

void error(string text, sf::RenderWindow &window) {
    sf::Clock imgui_clock;
    /*ImGuiIO& io = ImGui::GetIO();
    ImFontConfig font_config;
    font_config.OversampleH = 1; //or 2 is the same
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;

    static const ImWchar ranges[] =
            {
                    0x0020, 0x00FF, // Basic Latin + Latin Supplement
                    0x0400, 0x044F, // Cyrillic
                    0,
            };
    io.Fonts->AddFontFromFileTTF("assets/fonts/font2.ttf", 14.0f, &font_config, ranges);
    */
    while (window.isOpen()) {
        ImGui::SFML::Update(window, imgui_clock.restart());
        window.clear(sf::Color(0, 0, 0));
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            ImGui::SFML::ProcessEvent(window, event);
        }
        ImGui::Begin("Platform engine error");
        ImGui::Text("Platform Engine was crashed!\nStop code:");
        ImGui::Text((text).c_str());
        ImGui::Text("To fix, you need to contact support, report bug or reinstall game");
        ImGui::Text("Technical info:");
        ImGui::InputText("Version", &version, ImGuiInputTextFlags_ReadOnly);
        ImGui::Checkbox("Is online", &isMultiplayer);
        ImGui::InputText("IP adress", &address, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Loaded level", &loaded_level_name, ImGuiInputTextFlags_ReadOnly);
        if (ImGui::Button("Exit")) {
            exit(1);
        }
        if (ImGui::Button("Report bug")) {
            openWebBrowser("http://vladtopor.dev/report?app_id=perpg");
        }
        if (ImGui::Button("Support")) {
            openWebBrowser("http://vladtopor.dev/support?app_id=perpg");
        }

        ImGui::End();
        ImGui::SFML::Render(window);
        window.display();
    }
}
int main()
{
    //gravity.Set(0.0f, -10.0f);

    //int32 velocityIterations = 6;
    //int32 positionIterations = 2;


    info("Loading engine...");
    info("Version: "+version);


    info("Loading config...");
    std::ifstream config_file("config.json");
    json config = json::parse(config_file);
    info("Loading key codes");
    std::ifstream codesFile("assets/configs/codes.json");
    json codes = json::parse(codesFile);
    info("Loading keys codes");
    std::ifstream keyFile("assets/configs/keys.json");
    json keys = json::parse(keyFile);
    info("Loading localization");
    initiateLocalization();

    string game_title = config["game_name"];
    width = config["window_width"];
    height = config["window_height"];
    beginWidth = config["window_width"];
    beginHeight = config["window_height"];

    sucess(getLocalizedString("engine.fatals.configloaded", lang));
    float beginAspectRatio = width/height;
    info(getLocalizedString("engine.loading.loadrender", lang));
    sf::RenderWindow window;
    bool fullscreen = false;
    if (!fullscreen) {
        window.create(sf::VideoMode(width, height), "Platform Engine 2D rpg " + version + " - " + game_title);
    } else {
        window.create(sf::VideoMode(width, height), "Platform Engine 2D rpg " + version + " - " + game_title, sf::Style::Fullscreen);
    }
    info("Starting ImGUI...");
    ImGui::SFML::Init(window);
    if (LAUNCH_EDITOR) {
        editor(window, "main");
    }
    sucess("Done");
    info("Starting physics...");
    startPhysics();
    sucess("Done");
    info("Setting icon...");
    sf::Image icon;
    icon.loadFromFile(config["icon"]); // File/Image/Pixel
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    sucess("Done");
    info("Showing intro...");
    //Launching introcameraScale


    //sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Platform Engine 2D rpg "+version+" - "+game_title);
    sucess(getLocalizedString("engine.sucess.done", lang));
    framerate_limit = config["framerate_limit"];
    tickrate = config["tickrate"];
    setTickrate(tickrate);
    float timeStep = 1.0f/(tickrate);
    window.setFramerateLimit(config["framerate_limit"]);

    sf::View camera(sf::FloatRect(0.0f, 0.0f, width, height));


    float lastTime = 0;
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/font.ttf"))
    {
        error(getLocalizedString("engine.fatals.engineasset", lang), window);
    }
    sf::Text fps_text;

    fps_text.setFont(font);
    fps_text.setCharacterSize(50);
    info(getLocalizedString("engine.loading.lua", lang));

    engine_lib->AddCFunction("getProperty", getProperty);
    engine_lib->AddCFunction("modifyEngineObject", modifyEngineObject);
    engine_lib->AddCFunction("loadLevel", loadLevel);
    engine_lib->AddCFunction("loadUi", loadUi);
    engine_lib->AddCFunction("playSound", playSound);
    engine_lib->AddCFunction("getLoadedLevel", getLoadedLevel);
    engine_lib->AddCFunction("setWidgetText", setWidgetText);


    lua.AddLibrary(engine_lib);
    sucess(getLocalizedString("engine.sucess.done", lang));

    sf::Clock imgui_clock;


    /*ServerPlayer serverplayer;
    serverplayer.address = address;
    serverplayer.username = "VladTopor";
    if (isMultiplayer) {
        serverplayer.join();
    }*/
    //playSound("assets/sounds/theme_player.wav", false);

    //load_ui("main");
    sf::Clock clock;
    sfe::Movie intro;
    intro.openFromFile("assets/videos/intro.mp4");
    intro.fit(0, 0, width, height, false);
    intro.play();
    bool isIntroPlaying = true;
    //error("Test error", window);
    //PhysicObjectDynamic object;

    //object.init(1.0f, 1.0f, 1.0f, 1.0f);

    while (window.isOpen() && isIntroPlaying) {
        if (intro.getStatus() == sfe::Status::Stopped) {
            isIntroPlaying = false;
        }
        sf::Event ev;
        while (window.pollEvent(ev))
        {
            // Window closure
            if (ev.type == sf::Event::Closed)
            {
                window.close();
            }

            if (ev.type == sf::Event::KeyPressed)
            {
                if (ev.key.code == sf::Keyboard::Escape) {
                    isIntroPlaying = false;
                }
            }
        }

        intro.update();

        // Render movie
        window.clear();
        window.draw(intro);
        window.display();

    }
    intro.pause();
    load_level(config["begin"], false);
    bool cameraDebug = false;
    string levelDebug = "";
    string scriptDebug = "";
    float cameraSpeed = 7.0f;

    int debugId = 0;
    int debugWidgetId = 0;
    bool showSelectedHitbox = false;
    bool showAllHitboxes = false;
    bool showImgui = false;
    float fixCameraScale = 1.0f;
    bool noclip = false;
    float joyMultiplyX = 1.0;
    float joyMultiplyY = 1.0;
    bool joyAnalogMovement = true;
    int deadzone = 5;
    string ip = "";
    int port = 0;
    int joySelectedWidget = 0;
    info("Checking controller...");
    bool joystickAvailable = sf::Joystick::isConnected(0);
    if (!joystickAvailable) {
        warn("Controller not available");
    } else {
        sucess("Controller available");
        if (sf::Joystick::hasAxis(0, sf::Joystick::X) && sf::Joystick::hasAxis(0, sf::Joystick::Y)) {
            joystickAvailable = true;
        } else {
            joystickAvailable = false;
        }
    }
    sf::Clock tickClock;
    while (window.isOpen())
    {
        //physicEngine.compute(tickrate);
        isUp = sf::Keyboard::isKeyPressed(codes[keys["up"]]);
        isDown = sf::Keyboard::isKeyPressed(codes[keys["down"]]);
        isLeft = sf::Keyboard::isKeyPressed(codes[keys["left"]]);
        isRight = sf::Keyboard::isKeyPressed(codes[keys["right"]]);
        camera.setSize(cameraScale*width*fixCameraScale, cameraScale*height*fixCameraScale);
        float currentTime = clock.restart().asSeconds();
        //info(tickClock.getElapsedTime().asSeconds());

        float fps = 1.f / (currentTime);
        //info(getTickrateMultiplier(fps, tickrate));
        lastTime = currentTime;
        if (cameraDebug) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera_x -= cameraSpeed*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera_x += cameraSpeed*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera_y -= cameraSpeed*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera_y += cameraSpeed*getTickrateMultiplier(fps, tickrate);
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                if (config["resize_mode"] == "height") {
                    fixCameraScale = (static_cast<float>(beginHeight) / static_cast<float>(event.size.height));
                }
                if (config["resize_mode"] == "width") {
                    fixCameraScale = (static_cast<float>(beginWidth) / static_cast<float>(event.size.width));
                }
                width = event.size.width;
                height = event.size.height;
//                double aspectRatio = static_cast<float>(event.size.width)/static_cast<int>(event.size.height);
//                info(aspectRatio);
//                width = beginWidth;
//                if (config["resize_mode"] == "width") {
//
//                    //width = static_cast<int>(static_cast<float>(static_cast<float>(event.size.width)));
//                    width *= aspectRatio;info(script_rate);
//                    height = static_cast<int>(static_cast<float>(static_cast<float>(event.size.height)));
//
//                }


                window.setView(camera);
            }
        }

        window.clear(background);
        window.setView(camera);

        if (isLoading or currentLoading != "") {

        }
        else {
            script_rate += 1.0f*getTickrateMultiplier(fps, tickrate);
            if (trunc(script_rate) > script_now) {
                script_now = trunc(script_rate);


            }
            //world.Step(timeStep, velocityIterations, positionIterations);

            for (int i = 0; i < objects_count; i++) {
                if (tickClock.getElapsedTime().asSeconds()*tickrate > 1) {
                    // Every tick run
                    simulate();
                    tickClock.restart();
                }
                if (showAllHitboxes) {
                    sf::RectangleShape rect;
                    rect.setPosition(objects[i].x+1, objects[i].y+1);
                    rect.setSize(sf::Vector2f((objects[i].width * objects[i].texture.getSize().x)-2,
                                              (objects[i].height * objects[i].texture.getSize().y)-2));
                    rect.setOrigin(objects[i].sprite.getOrigin());
                    rect.setRotation(objects[i].sprite.getRotation());
                    rect.setFillColor(sf::Color(255, 255, 255, 0));
                    rect.setOutlineThickness(2.0f);
                    window.draw(rect);
                }
                if (objects[i].loop_script != "assets/scripts/blank.lua") {
                    lua.CompileFileAndRun(objects[i].loop_script);
                }
                if (objects[i].physic_enabled) {
                        objects[i].x = objects[i].getBody()->GetPosition().x*UNIT_SIZE;
                        objects[i].y = objects[i].getBody()->GetPosition().y*UNIT_SIZE;
                        objects[i].sprite.setOrigin(objects[i].width*objects[i].texture.getSize().x/2.0f, objects[i].width*objects[i].texture.getSize().y/2.0f);
                        objects[i].rotation = toDegree(objects[i].getBody()->GetAngle());


                }
                //objects[i].sprite.setOrigin(0.0f, 0.0f);
                objects[i].apply();
                if (objects[i].tag == "player") {
                    b2Vec2 movementForce(0.0f, 0.0f);
                    if (!fixedPhysics) {
                        isRight = true;
                    }
                    if (isLeft)
                    {
                        b2Vec2 mov(-10.0f, 0.0f);
                        movementForce += mov;

                    }
                    if (isRight)
                    {
                        b2Vec2 mov(10.0f, 0.0f);
                        movementForce += mov;
                    }
                    if (isUp)
                    {
                        b2Vec2 mov(0.0f, -10.0f);
                        movementForce += mov;
//                        objects[i].y -= joyMultiplyY*speed*getTickrateMultiplier(tickrate, fps);
//                        for (int j = 0; j < objects_count; j++) {
//                            sf::Texture textureOther = objects[j].texture;
//                            if (objects[j].tag != "player") {
//
//                                if (collide(objects[i].x, objects[i].y, objects[i].width*objects[i].texture.getSize().x, objects[i].height*objects[i].texture.getSize().y,
//                                             objects[j].x, objects[j].y, objects[j].width*textureOther.getSize().x, objects[j].height*textureOther.getSize().y, 'u', objects[j].isActive&&!objects[j].ghost&&!noclip)) {
//
//                                    if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
//                                        lua.CompileFileAndRun(objects[j].collide_script);
//                                        objects[j].isActivated = true;
//                                        break;
//                                    }
//                                    if (!run) {
//                                        objects[i].y += speed * getTickrateMultiplier(tickrate, fps);
//                                    } else {
//                                        info(1);
//                                        objects[i].y += fastSpeed * getTickrateMultiplier(tickrate, fps);
//                                    }
//
//                                } else {
//                                    objects[j].isActivated = false;
//                                }
//                            }
//                        }
                    }
                    if (isDown)
                    {
                        b2Vec2 mov(0.0f, 10.0f);
                        movementForce += mov;
                    }
                    // for (auto )
                    for (int j = 0; j<objects_count;j++) {
                        if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
                            //info(j);
                            for (b2Contact* contact = getWorld()->GetContactList(); contact; contact = contact->GetNext()) {
                                b2Fixture* fixtureA = contact->GetFixtureA();
                                b2Fixture* fixtureB = contact->GetFixtureB();

                                if ((fixtureA->GetBody() == objects[i].getBody() && fixtureB->GetBody() == objects[j].getBody()) ||
                                    (fixtureA->GetBody() == objects[j].getBody() && fixtureB->GetBody() == objects[i].getBody())) {
                                    lua.CompileFileAndRun(objects[j].collide_script);
                                    objects[j].isActivated = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (objects[i].physic_enabled) {
                        if (!objects[i].physic_static) {
                            objects[i].getBody()->SetFixedRotation(true);
                            objects[i].getBody()->SetLinearVelocity(b2Vec2(0, 0));
                            movementForce.Normalize();
                            movementForce*=speed;
                            objects[i].getBody()->ApplyForceToCenter(movementForce, true);
                        }
                    }
                    if (!fixedPhysics) {
                        //isRight = false;
                        fixedPhysics = true;
                    }
                    if (!fixedCamera && !cameraDebug) {
                        camera_x = objects[i].x+(objects[i].texture.getSize().x*objects[i].width)/2;
                        camera_y = objects[i].y+(objects[i].texture.getSize().y*objects[i].height)/2;

                    }
                }

                if (runLoop) {

                    lua.CompileFileAndRun(objects[i].loop_script);

                }
                if (!debug_mode && objects[i].texturePath == "assets/textures/trigger.png") {
                    continue;
                }

                objects[i].render(window);
                runLoop = false;

            }
            if (script_rate > 1) {
                runLoop = true;
                script_rate = 0.0f;
                script_now = 0.0f;

            }
            camera.setCenter(sf::Vector2f(camera_x, camera_y));
            for (int i=0 ; i< widgets_count; i++) {
                sf::Texture texture = widgets[i].texture;
                if (i==joySelectedWidget && widgets[i].tag == "button") {
                    sf::RectangleShape rect;
                    rect.setPosition(widgets[i].fixedX+camera_x, widgets[i].fixedY+camera_y);
                    rect.setSize(sf::Vector2f(widgets[i].width * texture.getSize().x,
                                              widgets[i].height * texture.getSize().y));
                    rect.setFillColor(sf::Color(255, 255, 0, 128));
                    rect.setOutlineColor(sf::Color(255, 255, 0, 128));
                    rect.setOutlineThickness(2.0f);
                    window.draw(rect);
                }
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || (i==joySelectedWidget && joystickAvailable && sf::Joystick::isButtonPressed(0, 0))) {
                    sf::Vector2f pos = window.mapPixelToCoords(sf::Mouse::getPosition());
                    if ((collide(pos.x, pos.y, 1, 1, camera_x+widgets[i].fixedX, camera_y+widgets[i].fixedY, widgets[i].width*256/fixCameraScale, (widgets[i].height*64/fixCameraScale), 'a', true) || joystickAvailable )&& widgets[i].tag == "button") {
                        widgets[i].setTexture("assets/textures/button_pressed.png");
                        if (widgets[i].isActivated == false) {
                            lua.CompileFileAndRun(widgets[i].collide_script);
                        }
                        widgets[i].isActivated = true;

                    } else if (widgets[i].tag == "button") {
                        widgets[i].setTexture("assets/textures/button.png");
                        widgets[i].isActivated = false;
                    }
                } else {
                    if (widgets[i].tag == "button") {
                        if (widgets[i].isActivated == true) {
                            lua.CompileFileAndRun(widgets[i].loop_script);
                        }
                        widgets[i].setTexture("assets/textures/button.png");
                        widgets[i].isActivated = false;
                    }
                }
                widgets[i].apply();
                widgets[i].render(window, camera_x, camera_y);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F3))
        {
            showImgui = true;
        }
        bool joystickAvailable = sf::Joystick::isConnected(0);
        if (!joystickAvailable) {
        } else {
            if (sf::Joystick::hasAxis(0, sf::Joystick::X) && sf::Joystick::hasAxis(0, sf::Joystick::Y)) {
                joystickAvailable = true;
            } else {
                joystickAvailable = false;
            }
        }


        int joystickX = 0;
        int joystickY = 0;

        if (joystickAvailable) {
            joystickX = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
            joystickY = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
            if (sf::Joystick::isButtonPressed(0, 4)) {
                joySelectedWidget -= 1;
                if (joySelectedWidget == -1) {
                    joySelectedWidget = widgets_count-1;
                } else if (joySelectedWidget == widgets_count) {
                    joySelectedWidget = 0;
                }

            }
            if (sf::Joystick::isButtonPressed(0, 5)) {
                joySelectedWidget += 1;
                if (joySelectedWidget == -1) {
                    joySelectedWidget = widgets_count-1;
                } else if (joySelectedWidget == widgets_count) {
                    joySelectedWidget = 0;
                }

            }
            if (joyAnalogMovement) {
                if (joystickX > deadzone) {
                    isRight = true;
                }
                if (joystickX < -deadzone) {
                    isLeft = true;
                }
                if (joystickY > deadzone) {
                    isDown = true;
                }
                if (joystickY < -deadzone) {
                    isUp = true;
                }

                joyMultiplyX = abs(static_cast<float>(joystickX)/100);
                joyMultiplyY = abs(static_cast<float>(joystickY)/100);

            } else {
                if (joystickX < -50) {
                    isLeft = true;
                }
                if (joystickX > 50) {
                    isRight = true;
                }
                if (joystickY < -50) {
                    isUp = true;
                }
                if (joystickY > 50) {
                    isDown = true;
                }
                joyMultiplyX = 1.0f;
                joyMultiplyY = 1.0f;
            }

        }
        if (sf::Keyboard::isKeyPressed(codes[keys["speed"]])) {
             run = true;
        }
        else {
            run = false;
        }
        fps_text.setPosition(fixed_pos(camera_x, camera_y, width, height, cameraScale, cameraScale));
        fps_text.setString("FPS="+floatToString(fps));
        if (showImgui) {
            ImGui::SFML::Update(window, imgui_clock.restart());

            ImGui::Begin("Platform engine");
            ImGui::Text(version.c_str());
            ImGui::Text(("FPS=" + floatToString(fps)).c_str());
            ImGui::Text(("CX="+floatToString(camera_x)).c_str());
            ImGui::Text(("CY="+floatToString(camera_y)).c_str());

            ImGui::Checkbox("Show debug objects", &debug_mode);
            ImGui::Checkbox("Camera debug", &cameraDebug);
            ImGui::Checkbox("Show hitbox", &showSelectedHitbox);
            ImGui::Checkbox("Show ALL hitboxes", &showAllHitboxes);
            ImGui::Checkbox("Noclip", &noclip);
            ImGui::Checkbox("Joystick available", &joystickAvailable);

            //ImGui::Button("Restart");

            ImGui::InputText("Level name", &levelDebug);
            if (ImGui::Button("Load level")) {
                load_level(levelDebug, false);
            }
            ImGui::InputText("Script", &scriptDebug);
            if (ImGui::Button("Load script")) {
                lua.CompileFileAndRun("assets/scripts/" + scriptDebug + ".lua");
            }
            ImGui::SliderFloat("Camera scale", &cameraScale, 0.01f, 5.0f);
            ImGui::SliderFloat("Camera speed", &cameraSpeed, 0.2f, 30.0f);
            ImGui::InputFloat("Player speed", &speed);
            ImGui::InputFloat("Fixed camera scale", &fixCameraScale);
            if (ImGui::Button("Hide debug menu")) {
                showImgui = false;
            }
            ImGui::InputText("Language", &lang);
            int br = background.r;
            int bg = background.g;
            int bb = background.b;

            ImGui::SliderInt("R", &br, 0, 255);
            ImGui::SliderInt("G", &bg, 0, 255);
            ImGui::SliderInt("B", &bb, 0, 255);
            background.r = br;
            background.g = bg;
            background.b = bb;

            ImGui::End();
            ImGui::Begin("Object manager");
            if (debugId < 0) {
                debugId = 0;
            }
            if (debugId > objects_count - 1) {
                debugId = objects_count - 1;
            }

            ImGui::InputInt("ID", &debugId);
            ImGui::InputFloat("X", &objects[debugId].x);
            ImGui::InputFloat("Y", &objects[debugId].y);
            ImGui::InputFloat("Width", &objects[debugId].width);
            ImGui::InputFloat("Height", &objects[debugId].height);
            ImGui::InputInt("R", &objects[debugId].r);
            ImGui::InputInt("G", &objects[debugId].g);
            ImGui::InputInt("B", &objects[debugId].b);
            ImGui::InputInt("A", &objects[debugId].a);
            ImGui::InputText("Tag", &objects[debugId].tag);
            if (ImGui::Button("Reapply")) {
                objects[debugId].getBody()->SetTransform(b2Vec2(objects[debugId].x, objects[debugId].y), toRadian(objects[debugId].rotation));
                objects[debugId].init();
            }
            ImGui::Text("Texture");
            ImGui::Image(objects[debugId].texture);
            ImGui::End();

            ImGui::Begin("Widget manager");
            if (debugWidgetId < 0) {
                debugWidgetId = 0;
            }
            if (debugWidgetId > widgets_count - 1) {
                debugWidgetId = widgets_count - 1;
            }
            ImGui::InputInt("ID", &debugWidgetId);
            ImGui::InputFloat("X", &widgets[debugWidgetId].fixedX);
            ImGui::InputFloat("Y", &widgets[debugWidgetId].fixedY);
            ImGui::InputFloat("Width", &widgets[debugWidgetId].width);
            ImGui::InputFloat("Height", &widgets[debugWidgetId].height);
            ImGui::InputFloat("Offset X", &widgets[debugWidgetId].offsetX);
            ImGui::InputFloat("Offset Y", &widgets[debugWidgetId].offsetY);
            ImGui::InputInt("Font size", &widgets[debugWidgetId].textSize);
            //ImGui::InputText("Tag", &widgets[debugWidgetId].tag);

            if (ImGui::Button("Apply")) {
                widgets[debugWidgetId].init();
                widgets[debugWidgetId].apply();
            }
            ImGui::End();

            ImGui::Begin("Entity manager");
            ImGui::Button("Spawn entity");
            ImGui::End();

            ImGui::Begin("Physics");

            ImGui::End();

            ImGui::Begin("Network manager");
            ImGui::InputText("IP", &ip);
            ImGui::InputInt("Port", &port);
            ImGui::Button("Connect");

            ImGui::End();
            ImGui::Begin("Joystick manager");
            if (joystickAvailable) {
                ImGui::Checkbox("Joystick analog movement", &joyAnalogMovement);
                ImGui::SliderInt("Deadzones", &deadzone, 0, 100);
                ImGui::SliderInt("X", &joystickX, -100, 100);
                ImGui::SliderInt("Y", &joystickY, -100, 100);
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovX)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
                    ImGui::SliderInt("PovX", &joyval, -100, 100);
                }
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovY)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
                    ImGui::SliderInt("PovY", &joyval, -100, 100);
                }
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::Z)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z);
                    ImGui::SliderInt("Z", &joyval, -100, 100);
                }
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::R)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R);
                    ImGui::SliderInt("R", &joyval, -100, 100);
                }
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::U)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U);
                    ImGui::SliderInt("U", &joyval, -100, 100);
                }
                if (sf::Joystick::hasAxis(0, sf::Joystick::Axis::V)) {
                    int joyval = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V);
                    ImGui::SliderInt("V", &joyval, -100, 100);
                }
                for (int i=0; i<sf::Joystick::getButtonCount(0);i++) {
                    bool isEnabled = sf::Joystick::isButtonPressed(0, i);
                    stringstream ss;
                    ss << i;

                    ImGui::Checkbox(ss.str().c_str(), &isEnabled);
                }



            } else {
                ImGui::Text("Joystick is not available!");
            }
            ImGui::End();

            if (showSelectedHitbox) {
                sf::RectangleShape rect;
                rect.setPosition(objects[debugId].x, objects[debugId].y);
                rect.setSize(sf::Vector2f(objects[debugId].width * objects[debugId].texture.getSize().x,
                                          objects[debugId].height * objects[debugId].texture.getSize().y));
                rect.setOrigin(objects[debugId].sprite.getOrigin());
                rect.setRotation(objects[debugId].sprite.getRotation());
                rect.setFillColor(sf::Color(255, 255, 0, 128));
                rect.setOutlineColor(sf::Color(255, 255, 0, 128));
                rect.setOutlineThickness(2.0f);
                window.draw(rect);
            }
            ImGui::SFML::Render(window);
        }
        window.display();
    }
    info(getLocalizedString("engine.ui.exit", lang));
    return 0;
}