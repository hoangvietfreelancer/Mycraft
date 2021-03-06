//
//  Sky.hpp
//  Mycraft
//
//  Created by Clapeysron on 08/12/2017.
//  Copyright © 2017 Clapeysron. All rights reserved.
//

#ifndef Sky_hpp
#define Sky_hpp
#include "opengl_header.h"
#include <vector>

class Sky {
public:
    ~Sky();
    void draw(glm::vec3 position, glm::mat4 view, glm::mat4 projection, float dayTime, float starIntensity);
    Shader Sky_Shader;
    void Sky_init();
    void Star_init();
    unsigned int Skybox_pic;
private:
    unsigned int VBO, VAO;
    unsigned int Star_pic;
};
#endif /* Sky_hpp */
