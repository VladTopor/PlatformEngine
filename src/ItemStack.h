//
// Created by vladislav on 11.12.23.
//

#ifndef PERPG_ITEMSTACK_H
#define PERPG_ITEMSTACK_H

#include <SFML/Graphics/Texture.hpp>
#include "string"
using namespace std;

class ItemStack {
public:
    string id;
    int count;
    int cost;
    sf::Texture texture;
    string texturePath;
};


#endif //PERPG_ITEMSTACK_H
