#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include "multiplayer.h"
//#include "box2d.h"
#include "LuaCpp.hpp"
#include <math.h>
#define UNIT_SIZE 64
#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067
//#include "btBulletDynamicsCommon.h"
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;
using json = nlohmann::json;
using namespace std;

template <typename T>
void info(T log) {
 cout << "[\x1B[94mINFO\x1B[37m] " << log << '\n';}
template <typename T>
void warn(T log) {
 cout << "[\x1B[93mWARN\x1B[37m] " << log << '\n';}
template <typename T>
void error(T log) {
 cout << "[\x1B[91mERROR\x1B[37m] " << log << '\n';
 exit(1);}
template <typename T>
void sucess(T log) {
 cout << "[\x1B[92mSUCCESS\x1B[37m] " << log << '\n';}
template <typename T>
void debug(T log) {
 cout << "[\x1B[95mDEBUG\x1B[37m] " << log << '\n';
}
template<typename T>
T Vector2length(const sf::Vector2<T>& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

//b2Vec2 gravity(0.0f, 10.0f);
//b2World world(gravity);
sf::Vector2f fixed_pos(float camera_x, float camera_y, int screenWidth, int screenHeight, float fixed_x, float fixed_y) {
    return sf::Vector2f((camera_x-screenWidth/2+fixed_x), (camera_y-screenHeight/2+fixed_y));
}
float camera_x = 0;
float camera_y = 0;
class Object {
public: 
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float scriptX = -1.0f;
    float scriptY = -1.0f;
    int id = 0;
    float width = 1.0f;
    float height = 1.0f;

    int textSize = 0;

    float rotation = 0.0f;
    bool isSmooth = false;
    bool isStatic = false;
    bool ghost = false;
    bool isActive = false;
    bool isUI = false;
    int r = 255;
    int g = 255;
    int b = 255;
    int a = 255;

    float fixedX;
    float fixedY;

    float offsetX;
    float offsetY;

    bool isFixed = false;
    string tag;
    string loop_script;
    string collide_script;
    string text;
    bool isActivated = false;

    //FloatTriggerAction loop; //legacy

    string texturePath;
    sf::Texture texture;
    sf::Sprite sprite;
    void apply() {
        if(isActive) {
            sprite.setPosition(sf::Vector2f(x, y));
            if (isFixed) {
                sprite.setPosition(fixed_pos(camera_x, camera_y, 1920, 1080, fixedX, fixedY));
            }
            sprite.setRotation(rotation);
            sprite.setScale(sf::Vector2f(width, height));
        } 
    }
    void render(sf::RenderWindow& window) {
        
        if (isActive) {
            if (text != "") {
                sf::Text textW;
                sf::Font font;
                if (!font.loadFromFile("assets/fonts/font.ttf"))
                {
                    error("Failed to load font! Please check assets/fonts/font.ttf");
                }
                textW.setFont(font);
                textW.setCharacterSize(textSize);
                textW.setString(text);
                textW.setPosition(fixed_pos(camera_x, camera_y, 1920, 1080, fixedX+offsetX, fixedY+offsetY));
                window.draw(sprite);
                window.draw(textW);

            } else {
                window.draw(sprite);
            }
    }
        //debug(y);
    }

    void init() {
        //info("Loading textures...");
        if (!texture.loadFromFile(texturePath))
        {
            error("Can`t find "+texturePath);
        }

        texture.setSmooth(isSmooth);
        sprite.setTexture(texture);
        sprite.setColor(sf::Color(r, g, b, a));
        //sucess("Textures started");
        //info("Adding physic... (Skipped)");
        /*if (!ghost) {
            if (isStatic) {
                bodyDef.position.Set(x, y);
                body = world.CreateBody(&bodyDef);
                hitBox.SetAsBox(texture.getSize().x*width/2, texture.getSize().y*height/2);
                body->CreateFixture(&hitBox, 0.0f);
            } else {
                bodyDef.type = b2_dynamicBody;
                bodyDef.position.Set(x, y);
                body = world.CreateBody(&bodyDef);
                hitBox.SetAsBox(texture.getSize().x*width/2, texture.getSize().y*height/2);
                
                physProps.shape = &hitBox;
                physProps.density = 1.0f;
                physProps.friction = 0.3f;

                body->CreateFixture(&physProps);
            }
        }*/
        //info("Adding badass physic...");

    }
    void setTexture(string path){
        texturePath = path;
        init();
    }
};

unsigned long object_limit = 32768*50;
//unsigned long object_limit = 2147483647/4;
Object *objects = new Object[object_limit];
Object *widgets = new Object[object_limit];
int objects_count = 0;
int widgets_count = 0;

sf::Color background;
bool isLoading = true;
string currentLoading;
sf::SoundBuffer *buffers = new sf::SoundBuffer[64];
sf::Sound *sounds = new sf::Sound[64];

ServerPlayer *players = new ServerPlayer[128];
Object *playersObjects = new Object[128];
LuaCpp::LuaContext lua;
std::shared_ptr<LuaLibrary> engine_lib = std::make_shared<LuaLibrary>("engine");
float speed;
bool isLeft = false;
bool isRight = false;
bool isUp = false;
bool isDown = false;
bool fixedPhysics = false;
void playSound(string path, int channel, bool loop) {
    if (!buffers[channel].loadFromFile(path)) {error("Failed to load sound");}
    sounds[channel].setBuffer(buffers[channel]);
    sounds[channel].setLoop(loop);
    sounds[channel].play();
}
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
        debug("Applying positions");
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
        widgets[i].init();

        }

    }
    sucess("Loaded!");
    isLoading = false;

}
bool onBeginRunned = false;
void load_level(string levelName, bool fast) {
    info("Loading level "+levelName);
    isRight=true;
    std::ifstream level_file("assets/levels/"+levelName+"/level.json");
    json level = json::parse(level_file);
    objects_count = level["count"];
    background.r = level["background"][0];
    background.g = level["background"][1];
    background.b = level["background"][2];
    speed = level["speed"];
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
    if (!fast) {
        delete[] objects;
        objects = new Object[object_limit];
    }
    isLoading = true;
    for (int j = 0; j < objects_count; j++) {
        objects[j].isActive = false;
    }
    for (int i = 0; i < objects_count; i++) {
        if (level["version"] == 3) { // Level 3 version parser
        //warn("Legacy level version! This version level will be removed later! Upgrade it to use without problems");
        objects[i].x = level["objects"][i]["position"]["x"];
        objects[i].y = level["objects"][i]["position"]["y"];
        objects[i].z = level["objects"][i]["position"]["z"];
        objects[i].id = i;
        objects[i].r = level["objects"][i]["color"]["r"];
        objects[i].g = level["objects"][i]["color"]["g"];
        objects[i].b = level["objects"][i]["color"]["b"];
        objects[i].a = level["objects"][i]["color"]["a"];
        objects[i].isActive = true;

        objects[i].texturePath = level["objects"][i]["texture"];

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
        objects[i].init();

        }
        else
        {
            error("Failed to load level "+levelName+", because LevelVersion is illegal");
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
}
string floatToString(float d)
{
   stringstream ss;
   ss << d;
   return ss.str();
}
float getTickrateMultiplier(float fps, int target)
{
    return fps/target;
}

extern "C" {
    int modifyObject(lua_State *L) {
        int n = lua_gettop(L);
        if (n!=3) {
            warn("Lua script error: Need to use modifyObjecy(id, property, value)");
        } else {
            string property = lua_tostring(L, 2);
            int id = lua_tonumber(L, 1);
            if (property == "x") {
                float value = lua_tonumber(L, 3);
                objects[id].x = value;
                //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].body->GetAngle());

            }

            if (property == "y") {
                float value = lua_tonumber(L, 3);
                objects[id].y = value;
                //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].body->GetAngle());

            }
            if (property == "rot") {
                float value = lua_tonumber(L, 3);
                objects[id].rotation = value;
                //objects[id].body->SetTransform(b2Vec2(objects[id].x, objects[id].y), objects[id].rotation);
            }
            
        }
        return 0;
    }
    int loadLevel(lua_State *L) {
        int n = lua_gettop(L);
        if (n > 3) {

            warn("Lua script error: Need to use loadLevel(levelName, fast)");
            lua_pushliteral(L, "Failed to load this level!");
            return 0;
        }
        string level = lua_tostring(L, 1);
        load_level(level, lua_toboolean(L, 2));
        return 1;
    }
    int loadUi(lua_State *L) {
        int n = lua_gettop(L);
        if (n > 2) {
            warn("Lua script error: Need to use loadUi(uiName)");
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
            warn("Lua script error: Need to use playSound(path, line, loop)");
            lua_pushliteral(L, "Failed to load this level!");
            return 0;
        }
        int channel = lua_tonumber(L, 2);
        string level = lua_tostring(L, 1);
        bool loop = lua_toboolean(L, 2);
        playSound(level, channel, loop);
        return 1;
    }
   int getProperty(lua_State *L) {
    int n = lua_gettop(L);
    lua_Number id = lua_tonumber(L, 1);
    if (n != 2) {

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
        return collide(x2, y2, width2, height2, x1, y1, width1, height1, 'l', true);
    }
    if (direction == 'u') {
        if (((x2 < x1 && x2+width2 > x1) || (x2 < x1+width1 && x2+width2 > x1+width1)) && ((y2 < y1)&&(y2+height2 > y1))) return true;
    }
    if (direction == 'd') {
        return collide(x2, y2, width2, height2, x1, y1, width1, height1, 'u', true);
    }
    return false;
}
int main()
{
    
    

    float camera_scale = 1.0f;

    int framerate_limit = 0;
    int tickrate;
    float script_rate = 0.0f;
    float script_now = 0;
    bool runLoop = false;
    //int32 velocityIterations = 6;
    //int32 positionIterations = 2;

    string version = "0.1 alpha";
    info("Loading engine...");
    info("Version: "+version);
    
    
    info("Loading config...");
    std::ifstream config_file("config.json");
    json config = json::parse(config_file);
    
    int width = 0;
    int height = 0;
    string game_title = config["game_name"];
    width = config["window_width"];
    height = config["window_height"];
    sucess("Config loaded!");

    info("Rendering window...");
    sf::RenderWindow window(sf::VideoMode(width, height), "Platform Engine 2D rpg "+version+" - "+game_title);
    //sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Platform Engine 2D rpg "+version+" - "+game_title);
    sucess("Rendered!");
    framerate_limit = config["framerate_limit"];
    tickrate = config["tickrate"];
    float timeStep = 1.0f/(tickrate/4);
    window.setFramerateLimit(config["framerate_limit"]);
    info("Showing intro...");
    sf::Texture texture;
    sf::Sprite intro;
    texture.setSmooth(false);
    if (!texture.loadFromFile("assets/textures/intro_banner.png"))
    {
        error("Failed to load banner! Plese, check assets/textures/intro_banner.png");
    }
    intro.setTexture(texture);
    sf::View camera(sf::FloatRect(0.0f, 0.0f, width, height));

    sf::Clock clock;
    float lastTime = 0;
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/font.ttf"))
    {
        error("Failed to load font! Please check assets/fonts/font.ttf");
    }
    sf::Text fps_text;

    fps_text.setFont(font);
    fps_text.setCharacterSize(50);
    info("Starting lua...");

    engine_lib->AddCFunction("getProperty", getProperty);
    engine_lib->AddCFunction("modifyObject", modifyObject);
    engine_lib->AddCFunction("loadLevel", loadLevel);
    engine_lib->AddCFunction("loadUi", loadUi);
    engine_lib->AddCFunction("playSound", playSound);


    lua.AddLibrary(engine_lib);
    sucess("Loaded!");


    bool debug = true;
    bool isMultiplayer = true;
    string address = "127.0.0.1";
    /*ServerPlayer serverplayer;
    serverplayer.address = address;
    serverplayer.username = "VladTopor";
    if (isMultiplayer) {
        serverplayer.join();
    }*/
    //playSound("assets/sounds/theme_player.wav", false);
    load_level(config["begin"], false);
    //load_ui("main");

    while (window.isOpen())
    {

        camera.zoom(camera_scale);
        float currentTime = clock.restart().asSeconds();
        float fps = 1.f / (currentTime);
        lastTime = currentTime;
        if (debug) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera_x -= 7.0f*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera_x += 7.0f*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera_y -= 7.0f*getTickrateMultiplier(fps, tickrate);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera_y += 7.0f*getTickrateMultiplier(fps, tickrate);
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();  
            if (event.type == sf::Event::Resized) {
//                width = event.size.width;
//                height = event.size.height;
//
//                double aspectRatio = static_cast<double >(width)/static_cast<double>(height);
//                info(aspectRatio);
//                if (config["resize_mode"] == "width") {
//                    camera.setSize({
//                                           static_cast<float>(event.size.width),
//                                           static_cast<float>(event.size.height/aspectRatio)
//                    });
//                }
//
//
//                window.setView(camera);
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
                //debug(script_now);
                
            } 
            //world.Step(timeStep, velocityIterations, positionIterations);
            
            for (int i = 0; i < objects_count; i++) {
                if (objects[i].loop_script != "assets/scripts/blank.lua") {
                    lua.CompileFileAndRun(objects[i].loop_script);
                }
                objects[i].apply();
                if (objects[i].tag == "player") {

                    if (!fixedPhysics) {
                        isRight = true;
                    }
                    if (isLeft)
                    {
                        objects[i].x -= speed*getTickrateMultiplier(tickrate, fps);
                        for (int j = 0; j < objects_count; j++) {
                            if (objects[j].tag != "player") {

                            if (collide(objects[i].x, objects[i].y, objects[i].width*32, objects[i].height*32,
                                        objects[j].x, objects[j].y, objects[j].width*32, objects[j].height*32, 'l', objects[j].isActive)) {
                                if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
                                    lua.CompileFileAndRun(objects[j].collide_script);
                                    objects[j].isActivated = true;
                                    break;
                                }
                                objects[i].x += speed*2*getTickrateMultiplier(tickrate, fps);
                                   
                            } else {
                                objects[j].isActivated = false;
                            }
                        }
                        }
                    }
                    if (isRight)
                    {

                        objects[i].x += speed*getTickrateMultiplier(tickrate, fps);
                        for (int j = 0; j < objects_count; j++) {
                            if (objects[j].tag != "player") {

                                if (collide(objects[i].x, objects[i].y, objects[i].width*32, objects[i].height*32,
                                            objects[j].x, objects[j].y, objects[j].width*32, objects[j].height*32, 'r', objects[j].isActive)) {
                                    if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
                                        lua.CompileFileAndRun(objects[j].collide_script);
                                        objects[j].isActivated = true;
                                        break;
                                    }
                                    objects[i].x -= speed*2*getTickrateMultiplier(tickrate, fps);

                                } else {
                                    objects[j].isActivated = false;
                                }
                            }
                        }

                    }
                    if (isUp)
                    {

                        objects[i].y -= speed*getTickrateMultiplier(tickrate, fps);
                        for (int j = 0; j < objects_count; j++) {
                            if (objects[j].tag != "player") {

                                if (collide(objects[i].x, objects[i].y, objects[i].width*32, objects[i].height*32,
                                            objects[j].x, objects[j].y, objects[j].width*32, objects[j].height*32, 'u', objects[j].isActive)) {

                                    if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
                                        lua.CompileFileAndRun(objects[j].collide_script);
                                        objects[j].isActivated = true;
                                        break;
                                    }
                                    objects[i].y += speed*2*getTickrateMultiplier(tickrate, fps);

                                } else {
                                    objects[j].isActivated = false;
                                }
                            }
                        }
                    }
                    if (isDown)
                    {

                        objects[i].y += speed*getTickrateMultiplier(tickrate, fps);
                        for (int j = 0; j < objects_count; j++) {
                            if (objects[j].tag != "player") {

                                if (collide(objects[i].x, objects[i].y, objects[i].width*32, objects[i].height*32,
                                            objects[j].x, objects[j].y, objects[j].width*32, objects[j].height*32, 'd', objects[j].isActive)) {
                                    if (objects[j].tag == "trigger" && objects[j].isActivated == false) {
                                        lua.CompileFileAndRun(objects[j].collide_script);
                                        objects[j].isActivated = true;
                                        break;
                                    }
                                    objects[i].y -= speed*2*getTickrateMultiplier(tickrate, fps);

                                } else {
                                    objects[j].isActivated = false;
                                }
                            }
                        }
                    }
                    if (!fixedPhysics) {
                        //isRight = false;
                        fixedPhysics = true;
                    }
                    if (!debug) {
                        camera_x = objects[i].x;
                        camera_y = objects[i].y;
                           
                    }
                }
                if (runLoop) {
                    
                    try {
                        
                        lua.CompileFileAndRun(objects[i].loop_script);
                    }
                 
                    catch (std::runtime_error& e) {
                        std::cout << e.what()
                                  << '\n';
                    }
                    
                }
                if (!debug && objects[i].tag == "trigger") { 
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

                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    if (collide(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, 1, 1, widgets[i].fixedX, widgets[i].fixedY, widgets[i].width*256, widgets[i].height*64, 'a', true) && widgets[i].tag == "button") {
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
                widgets[i].render(window);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
        {
            debug = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
        {

            debug = false;
        }

        isUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        isDown = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        isLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        isRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

        fps_text.setPosition(fixed_pos(camera_x, camera_y, width, height, 0, 0));
        
        fps_text.setString("FPS="+floatToString(fps));

        
        
        if (debug) {
            window.draw(fps_text);
        }
        window.display();
    }

    return 0;
}
