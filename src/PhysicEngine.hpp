//
// Created by vladislav on 20.01.24.
//

#ifndef PERPG_PHYSICENGINE_HPP

#define PERPG_PHYSICENGINE_HPP

#include <box2d/box2d.h>
#include <iostream>





class PhysicObjectStatic {
public:
    b2BodyDef bodyDef;
    b2Body *body;
    b2PolygonShape shape;
    int id;
    void start(float x, float y, float width, float height, int id_set);
    void remove();
};
class PhysicObjectDynamic {
public:
    b2BodyDef bodyDef;

    b2Body* body;
    b2PolygonShape dynamicBox;

    b2FixtureDef fixtureDef;
    int id;

    void start(float x, float y, float width, float height, int id_set);
    void remove();
};
void simulate();
void reset();
void setGravity(b2Vec2 grav);
void setTickrate(int tickrate);
bool haveContact(b2Body *body1, b2Body *body2);
void startPhysics();
b2World * getWorld();
#endif //PERPG_PHYSICENGINE_HPP
