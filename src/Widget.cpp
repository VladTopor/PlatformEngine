//
// Created by vladislav on 20.01.24.
//

#include "Widget.h"
#include "EngineFunctions.h"
void Widget::render(sf::RenderWindow &window, int camera_x, int camera_y) {
    if (text != "") {
        sf::Text textW;
        sf::Font font;
        if (!font.loadFromFile("assets/fonts/font.ttf")) {
//fatal("Failed to load font! Please check assets/fonts/font.ttf");
        }
        textW.setFont(font);
        textW.setCharacterSize(textSize);
        textW.setString(text);
        textW.setPosition(fixed_pos(camera_x, camera_y, 1920, 1080, fixedX + offsetX, fixedY + offsetY));
        window.draw(sprite);
        window.draw(textW);

    } else {
        window.draw(sprite);
    }
}