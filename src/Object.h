//
// Created by vladislav on 01.12.23.
//

#ifndef PERPG_OBJECT_H
#define PERPG_OBJECT_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <iostream>

#include <config.h>


#include "PhysicEngine.hpp"
using namespace std;
class EngineObject {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float scriptX = -1.0f;
    float scriptY = -1.0f;
    int id = 0;
    float width = 1;
    float height = 1;



    float rotation = 0.0f;
    bool isSmooth = false;
    bool isStatic = false;
    bool ghost = false;
    bool isActive = false;

    int r = 255;
    int g = 255;
    int b = 255;
    int a = 255;
    int textureWidth = 32;
    int textureHeight = 32;
    float fixedX;
    float fixedY;

    PhysicObjectDynamic dynamic_physic;
    PhysicObjectStatic static_physic;
    bool physic_static = true;
    bool physic_enabled = true;

    bool isFixed = false;
    string tag;
    string loop_script = "assets/scripts/blank.lua";
    string collide_script = "assets/scripts/blank.lua";

    bool isActivated = false;
    //string animate = "";
    //float animationValue = 0.0f;
    string texturePath;
    sf::Texture texture;
    sf::Sprite sprite;
    int staticTexture = -1;

    sf::FloatRect hitbox;

    void init();
    void setTexture(string path);
    void render(sf::RenderWindow &window, int camera_x, int camera_y);
    void render(sf::RenderWindow &window);
    void apply(int camera_x, int camera_y);
    void apply();
    b2Body* getBody();
};


#endif //PERPG_OBJECT_H
