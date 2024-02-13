//
// Created by vladislav on 20.01.24.
//

#include "Entity.h"

void Entity::render(sf::RenderWindow &window) {
    object.render(window);
}
void Entity::apply() {
    object.apply();
}
void Entity::spawn() {
    object.init();
    object.apply();
}