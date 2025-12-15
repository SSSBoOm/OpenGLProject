#ifndef GAME_UI_H
#define GAME_UI_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include <string>
#include <memory>

class GameUI {
public:
    GameUI();
    ~GameUI();
    
    void init(unsigned int screenWidth, unsigned int screenHeight);
    void render(float fuelPercent, float turboPercent);
    void renderGameOver(int finalScore, bool &continueButtonHovered, bool &exitButtonHovered);
    bool isPointInRect(double mouseX, double mouseY, float rectX, float rectY, float rectW, float rectH);
    void cleanup();
    
    void setScreenSize(unsigned int width, unsigned int height);
    
private:
    unsigned int screenWidth;
    unsigned int screenHeight;
    unsigned int quadVAO, quadVBO;
    std::unique_ptr<Shader> uiShader;
    glm::mat4 projection;
    
    void renderQuad(float x, float y, float width, float height, const glm::vec3& color);
    void initQuad();
};

#endif
