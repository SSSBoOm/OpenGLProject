#include "GameUI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <glad/glad.h>
#include <learnopengl/shader_m.h>

GameUI::GameUI() 
    : screenWidth(800), screenHeight(600), quadVAO(0), quadVBO(0), uiShader(nullptr)
{
}

GameUI::~GameUI()
{
    cleanup();
}

void GameUI::init(unsigned int width, unsigned int height)
{
    screenWidth = width;
    screenHeight = height;
    initQuad();
    
    // Load UI shader
    uiShader = std::make_unique<Shader>("ui.vs", "ui.fs");
    
    // Setup orthographic projection
    projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
}

void GameUI::initQuad()
{
    // Simple quad for rendering UI elements (normalized device coordinates)
    float vertices[] = {
        // positions (x, y)
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GameUI::setScreenSize(unsigned int width, unsigned int height)
{
    screenWidth = width;
    screenHeight = height;
    projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
}

void GameUI::renderQuad(float x, float y, float width, float height, const glm::vec3& color)
{
    if (!uiShader) return;
    
    uiShader->use();
    uiShader->setMat4("projection", projection);
    uiShader->setVec2("position", glm::vec2(x, y));
    uiShader->setVec2("size", glm::vec2(width, height));
    uiShader->setVec3("color", color);
    
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GameUI::render(float fuelPercent, int score)
{
    // Clamp fuel percent to 0-100
    fuelPercent = glm::clamp(fuelPercent, 0.0f, 100.0f);
    
    // Disable depth test for UI rendering
    glDisable(GL_DEPTH_TEST);
    
    // UI positions and sizes
    const float padding = 20.0f;
    const float barWidth = 200.0f;
    const float barHeight = 25.0f;
    const float fuelBarX = padding;
    const float fuelBarY = padding;
    
    // Render fuel bar background (dark gray)
    renderQuad(fuelBarX, fuelBarY, barWidth, barHeight, glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Render fuel bar border (white)
    float borderThickness = 2.0f;
    // Top border
    renderQuad(fuelBarX, fuelBarY, barWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    // Bottom border
    renderQuad(fuelBarX, fuelBarY + barHeight - borderThickness, barWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    // Left border
    renderQuad(fuelBarX, fuelBarY, borderThickness, barHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    // Right border
    renderQuad(fuelBarX + barWidth - borderThickness, fuelBarY, borderThickness, barHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    
    // Render fuel bar fill (color changes based on fuel level)
    float fillWidth = (barWidth - 8.0f) * (fuelPercent / 100.0f);
    glm::vec3 fuelColor;
    if (fuelPercent > 50.0f) {
        fuelColor = glm::vec3(0.2f, 0.8f, 0.2f); // Green
    } else if (fuelPercent > 25.0f) {
        fuelColor = glm::vec3(0.9f, 0.9f, 0.2f); // Yellow
    } else {
        fuelColor = glm::vec3(0.9f, 0.2f, 0.2f); // Red
    }
    renderQuad(fuelBarX + 4.0f, fuelBarY + 4.0f, fillWidth, barHeight - 8.0f, fuelColor);
    
    // Render score display
    const float scoreBarY = fuelBarY + barHeight + padding;
    const float scoreBarHeight = 30.0f;
    
    // Score background
    renderQuad(fuelBarX, scoreBarY, barWidth, scoreBarHeight, glm::vec3(0.1f, 0.1f, 0.15f));
    
    // Score border (gold)
    renderQuad(fuelBarX, scoreBarY, barWidth, borderThickness, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(fuelBarX, scoreBarY + scoreBarHeight - borderThickness, barWidth, borderThickness, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(fuelBarX, scoreBarY, borderThickness, scoreBarHeight, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(fuelBarX + barWidth - borderThickness, scoreBarY, borderThickness, scoreBarHeight, glm::vec3(1.0f, 0.84f, 0.0f));
    
    // Simple score visualization with bars (each bar represents 10 points)
    int scoreBars = glm::min(score / 10, 18); // Max 18 bars to fit in the width
    float scoreBarWidth = (barWidth - 8.0f) / 18.0f;
    for (int i = 0; i < scoreBars; ++i) {
        renderQuad(fuelBarX + 4.0f + i * scoreBarWidth + i * 1.0f, 
                  scoreBarY + 4.0f, 
                  scoreBarWidth - 1.0f, 
                  scoreBarHeight - 8.0f, 
                  glm::vec3(1.0f, 0.84f, 0.0f));
    }
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Print fuel and score to console for debugging
    static int lastPrintedScore = -1;
    static int lastPrintedFuel = -1;
    int fuelInt = static_cast<int>(fuelPercent);
    if (score != lastPrintedScore || fuelInt != lastPrintedFuel) {
        std::cout << "\rFuel: " << std::fixed << std::setprecision(1) << fuelPercent 
                  << "%  |  Score: " << score << "     " << std::flush;
        lastPrintedScore = score;
        lastPrintedFuel = fuelInt;
    }
}

void GameUI::cleanup()
{
    if (quadVAO != 0) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO != 0) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}
