//
// Created by vladislav on 20.01.24.
//

#ifndef PERPG_WIDGET_H
#define PERPG_WIDGET_H

#include "Object.h"

class Widget: public EngineObject {
public:
    float offsetX;
    float offsetY;
    string text;
    int textSize = 0;
    void render(sf::RenderWindow &window, int camera_x, int camera_y );
};


#endif //PERPG_WIDGET_H
