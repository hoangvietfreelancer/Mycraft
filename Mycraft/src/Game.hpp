//
//  Game.hpp
//  Mycraft
//
//  Created by Clapeysron on 14/11/2017.
//  Copyright © 2017 Clapeysron. All rights reserved.
//

#ifndef Game_hpp
#define Game_hpp
#include "opengl_header.h"
#include "ChunkData.hpp"
#include "Block.hpp"
class Game {
public:
    VisibleChunks test = VisibleChunks(8, 135, 8);
    Chunk testchunk = Chunk(0, 0);
    Block block;
    glm::vec3 user_position;
};
#endif /* Game_hpp */