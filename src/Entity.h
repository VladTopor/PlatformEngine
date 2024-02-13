//
// Created by vladislav on 20.01.24.
//

#ifndef PERPG_ENTITY_H
#define PERPG_ENTITY_H

#include "Object.h"
#include "ItemStack.h"
#include <string>

enum EntityType {
    Item,
    Mob,
    Decor,
    Interactive
};

class Entity {
public:
    EngineObject object;
    int maxHealth;
    int health;
    string id;
    EntityType type;
    ItemStack *inventory = new ItemStack[128];
    void render(sf::RenderWindow &window);
    void apply();
    void spawn();
};



#endif //PERPG_ENTITY_H
