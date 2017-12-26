//
//  initialize.cpp
//  Mycraft
//
//  Created by Clapeysron on 14/11/2017.
//  Copyright © 2017 Clapeysron. All rights reserved.
//

#include "Render.hpp"
#include "Stbi_load.hpp"


bool Render::firstMouse = true;
float Render::yaw   =  -90.0f;
float Render::pitch =  0.0f;
float Render::fov   =  45.0f;
float Render::lastX =  800.0f / 2.0;
float Render::lastY =  600.0 / 2.0;
float Render::deltaTime = 0.0f;
bool Render::tryRemove = false;
bool Render::tryPlace = false;
int Render::screen_width = SCREEN_WIDTH*2;
int Render::screen_height = SCREEN_HEIGHT*2;
glm::vec3 Render::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Render::cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

Render::Render() {
    lastFrame = 0.0f;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    window = glfwCreateWindow(screen_width/2, screen_height/2, "Mycraft", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
}

void Render::initial(Game &game) {
    view = glm::lookAt(game.steve_position, game.steve_position + cameraFront, cameraUp);
    projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
    Block_Shader = Shader("shader/Block.vs", "shader/Block.fs");
    Block_Shader.use();
    Block_Shader.setInt("texture_pic", 0);
    Block_Shader.setInt("shadowMap", 1);
    Steve_Shader = Shader("shader/Steve.vs", "shader/Steve.fs");
    Sky.Sky_init();
    Sky.Sky_Shader = Shader("shader/Skybox.vs", "shader/Skybox.fs");
    Gui.gui_init();
    Gui.Gui_Shader = Shader("shader/Gui.vs", "shader/Gui.fs");
    texture_init();
    Depth_Shader = Shader("shader/Depth.vs", "shader/Depth.fs");
    depthMap_init();
    Depth_debug_Shader = Shader("shader/Depth_debug.vs", "shader/Depth_debug.fs");
    Depth_debug_Shader.setInt("depthMap", 0);
    steve_model = Model("model/steve.obj");
    
}

void Render::depthMap_init() {
    glGenFramebuffers(1, &depthMap_fbo);
    glGenTextures(1, &depthMap_pic);
    glBindTexture(GL_TEXTURE_2D, depthMap_pic);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMap_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap_pic, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::texture_init() {
    glGenTextures(1, &texture_pic);
    glBindTexture(GL_TEXTURE_2D, texture_pic);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    int width, height, nrChannels;
    unsigned char *data = stbi_load_out("picture/texture_big.png", &width, &height, &nrChannels, STBI_rgb_alpha_out);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "****** Failed to load texture ******" << std::endl;
    }
    stbi_image_free_out(data);
}

// FOR DEBUG !
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
            1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
            1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Render::render(Game& game) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    printf("==========================\n");
    printf("fps: %.2f\n",1.0f/deltaTime);
    printf("view x:%.2f y:%.2f z:%.2f", cameraFront.x, cameraFront.y, cameraFront.z);
    processInput(window, game);
    if (game.game_mode == NORMAL_MODE) {
        game.gravity_move();
    }
    if(tryRemove){
        char type = game.visibleChunks.removeBlock(game.steve_position, cameraFront);
        tryRemove = false;
    }
    if(tryPlace){
        bool ret = game.visibleChunks.placeBlock(game.steve_position, cameraFront, DIAMAND_ORE);
        tryPlace = false;
    }
    
    // depth scene
    glm::mat4 lightProjection, lightView, lightSpaceMatrix;
    glm::vec3 lightDirection(-1.5f, -1.0f, 0.5f);
    glm::vec3 lightPos = game.steve_position;
    lightPos.y = 256;
    lightPos.x += 200;
    GLfloat near_plane = 0.0f, far_plane = 256.0f;
    lightProjection = glm::ortho(-120.0f, 120.0f, -120.0f, 120.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, lightPos + lightDirection, glm::vec3(-1.0f, 1.5f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    Depth_Shader.use();
    Depth_Shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMap_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    game.visibleChunks.drawDepth(Depth_Shader, texture_pic);
    steve_model.Draw(Depth_Shader);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Chunks render
    glViewport(0, 0, screen_width, screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
    if (game.game_perspective == FIRST_PERSON) {
        view = glm::lookAt(game.steve_position, game.steve_position + cameraFront, cameraUp);
    } else {
        view = glm::lookAt(game.steve_position - glm::vec3(5.0f)*cameraFront, game.steve_position + cameraFront, cameraUp);
    }
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glm::vec3 chosen_block_pos = game.visibleChunks.accessibleBlock(game.steve_position, cameraFront);
    game.visibleChunks.draw(game.steve_position, view, projection, Block_Shader, texture_pic, depthMap_pic, lightSpaceMatrix, lightDirection, chosen_block_pos);
    
    // steve render
    if (game.game_perspective == THIRD_PERSON) {
        Steve_Shader.use();
        Steve_Shader.setMat4("projection", projection);
        Steve_Shader.setMat4("view", view);
        Steve_Shader.setVec3("sunlight.lightDirection", lightDirection);
        Steve_Shader.setVec3("sunlight.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
        glm::mat4 model(1);
        model = glm::translate(model, game.steve_position);
        model = glm::translate(model, glm::vec3(0, -STEVE_HEIGHT+0.1, 0));
        model = glm::rotate(model, steve_turn_angle(cameraFront), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.3f * STEVE_HEIGHT));
        Steve_Shader.setMat4("model", model);
        steve_model.Draw(Steve_Shader);
    }
    
    // depth shadow draw DEBUG
//    Depth_debug_Shader.use();
//    Depth_debug_Shader.setFloat("near_plane", near_plane);
//    Depth_debug_Shader.setFloat("far_plane", far_plane);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, depthMap_pic);
//    glViewport(0, 0, 1024, 1024);
//    RenderQuad();

    // Draw sky box
    glViewport(0, 0, screen_width, screen_height);
    Sky.draw(game.steve_position, view, projection);
    
    // Draw gui
    Gui.draw(screen_width, screen_height);
    
    glfwSwapBuffers(window);
    glfwPollEvents();    
}

float Render::steve_turn_angle(glm::vec3 cameraFront) {
    if (cameraFront.z < 0) {
        return atan(cameraFront.x/cameraFront.z) + M_PI;
    } else {
        return atan(cameraFront.x/cameraFront.z);
    }
}

void Render::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    screen_width = width;
    screen_height = height;
}

void Render::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.9f)
        pitch = 89.9f;
    if (pitch < -89.9f)
        pitch = -89.9f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void Render::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_RELEASE){
        switch(button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                tryRemove = true;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                tryPlace = true;
                break;
            default:
                return;
        }
    }
    return;
}

void Render::processInput(GLFWwindow *window, Game &game)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    float cameraSpeed = 6.0 * deltaTime;
    glm::vec3 cameraFront_XZ = cameraFront;
    glm::vec3 new_position;
    cameraFront_XZ.y = 0;
    cameraFront_XZ = glm::normalize(cameraFront_XZ);
    glm::vec3 cameraRight_XZ = glm::cross(cameraFront, cameraUp);
    cameraRight_XZ.y = 0;
    cameraRight_XZ = glm::normalize(cameraRight_XZ);
    glm::vec3 cameraFront_Y = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        new_position = game.steve_position + cameraSpeed * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position + cameraSpeed * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        new_position = game.steve_position - cameraSpeed * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position - cameraSpeed * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        new_position = game.steve_position - cameraSpeed * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position - cameraSpeed * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        new_position = game.steve_position + cameraSpeed * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position + cameraSpeed * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        new_position = game.steve_position + cameraSpeed/10 * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position + cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        new_position = game.steve_position - cameraSpeed/10 * glm::vec3(cameraFront_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position - cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraFront_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        new_position = game.steve_position - cameraSpeed/10 * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position - cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        new_position = game.steve_position + cameraSpeed/10 * glm::vec3(cameraRight_XZ.x, 0.0f, 0.0f);
        game.move(new_position);
        new_position = game.steve_position + cameraSpeed/10 * glm::vec3(0.0, 0.0f, cameraRight_XZ.z);
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (game.game_mode == NORMAL_MODE) {
            if (game.vertical_v == 0) {
                game.vertical_v = JUMP_V/18.0f ;
            }
        } else {
            new_position = game.steve_position + cameraSpeed * cameraFront_Y;
            game.move(new_position);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        new_position = game.steve_position - cameraSpeed * cameraFront_Y;
        game.move(new_position);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        fov -= cameraSpeed*10;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        game.game_mode = GOD_MODE;
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        game.game_mode = NORMAL_MODE;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        fov += cameraSpeed*10;
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        game.game_perspective = FIRST_PERSON;
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        game.game_perspective = THIRD_PERSON;
    }
}


