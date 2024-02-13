//
// Created by vladislav on 01.12.23.
//

#include "Object.h"
#include "EngineFunctions.h"
void EngineObject::apply(int camera_x = 0, int camera_y=0) {
        if(isActive) {
            sprite.setPosition(sf::Vector2f(x, y));
            if (isFixed) {
                sprite.setPosition(fixed_pos(camera_x, camera_y, 1920, 1080, fixedX, fixedY));
            }
            sprite.setRotation(rotation);
            sprite.setScale(sf::Vector2f(width, height));
        }
    }
    void EngineObject::apply() {
    apply(0,0);
}
    void EngineObject::render(sf::RenderWindow& window, int camera_x=0, int camera_y=0) {

        if (isActive) {
            window.draw(sprite);
        }
        //debug(y);
    }
    void EngineObject::render(sf::RenderWindow &window) {
        render(window, 0, 0);
    }
    void EngineObject::init() {
        //cout << "ENGINE OBJECTS" << endl;
        texture.setRepeated(false);
        //info("Loading textures...");
        //cout << texture.getSize().x << endl;
        if (staticTexture == -1) {
            if (!texture.loadFromFile(texturePath)) {
                texture.loadFromFile("assets/textures/emo/64x64.png");
                //cout << texture->getSize().x << ' ' <<texture->getSize().y << endl;
                //warn("Can`t find " + texturePath);
            }

            texture.setSmooth(isSmooth);

            sprite.setTexture(texture);
        }
        //texture->update(texture->copyToImage());

        sprite.setColor(sf::Color(r, g, b, a));
        if (physic_enabled) {
            if (physic_static) {
                //cout << x/UNIT_SIZE << " " << y/UNIT_SIZE << " " << width*texture.getSize().x/UNIT_SIZE << " " << height*texture.getSize().y/UNIT_SIZE;
                static_physic.start((x-1)/UNIT_SIZE, (y-1)/UNIT_SIZE, width*(texture.getSize().x-2)/UNIT_SIZE/2.0f, height*(texture.getSize().y-2)/UNIT_SIZE/2.0f, id);
            } else {
                dynamic_physic.start(x / (float)UNIT_SIZE, y / (float)UNIT_SIZE, width * (float)texture.getSize().x / (float)UNIT_SIZE/2.0f,
                                     height * (float)texture.getSize().y / (float)UNIT_SIZE/2.0f, id);
            }
        }

    }

    void EngineObject::setTexture(string path){
        texturePath = path;
        init();
    }
    b2Body* EngineObject::getBody() {
        if (physic_enabled) {
            if (physic_static) {
                return static_physic.body;
            }
            else {
                return dynamic_physic.body;
            }
        }

    }