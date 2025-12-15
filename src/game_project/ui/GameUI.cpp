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

void GameUI::render(float fuelPercent, float turboPercent)
{
    // Clamp percentages to 0-100
    fuelPercent = glm::clamp(fuelPercent, 0.0f, 100.0f);
    turboPercent = glm::clamp(turboPercent, 0.0f, 100.0f);
    
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
    
    // Render turbo bar display
    const float turboBarY = fuelBarY + barHeight + padding;
    const float turboBarHeight = 25.0f;
    
    // Turbo background (dark gray)
    renderQuad(fuelBarX, turboBarY, barWidth, turboBarHeight, glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Turbo border (light blue)
    renderQuad(fuelBarX, turboBarY, barWidth, borderThickness, glm::vec3(0.3f, 0.7f, 1.0f));
    renderQuad(fuelBarX, turboBarY + turboBarHeight - borderThickness, barWidth, borderThickness, glm::vec3(0.3f, 0.7f, 1.0f));
    renderQuad(fuelBarX, turboBarY, borderThickness, turboBarHeight, glm::vec3(0.3f, 0.7f, 1.0f));
    renderQuad(fuelBarX + barWidth - borderThickness, turboBarY, borderThickness, turboBarHeight, glm::vec3(0.3f, 0.7f, 1.0f));
    
    // Render turbo bar fill (light blue)
    float turboFillWidth = (barWidth - 8.0f) * (turboPercent / 100.0f);
    renderQuad(fuelBarX + 4.0f, turboBarY + 4.0f, turboFillWidth, turboBarHeight - 8.0f, glm::vec3(0.4f, 0.8f, 1.0f));
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Print fuel and turbo to console for debugging
    static int lastPrintedTurbo = -1;
    static int lastPrintedFuel = -1;
    int fuelInt = static_cast<int>(fuelPercent);
    int turboInt = static_cast<int>(turboPercent);
    if (turboInt != lastPrintedTurbo || fuelInt != lastPrintedFuel) {
        std::cout << "\rFuel: " << std::fixed << std::setprecision(1) << fuelPercent 
                  << "%  |  Turbo: " << turboPercent << "%     " << std::flush;
        lastPrintedTurbo = turboInt;
        lastPrintedFuel = fuelInt;
    }
}

void GameUI::renderGameOver(int finalScore, bool &continueButtonHovered, bool &exitButtonHovered)
{
    // Disable depth test for UI rendering
    glDisable(GL_DEPTH_TEST);
    
    const float padding = 20.0f;
    const float centerX = screenWidth / 2.0f;
    const float centerY = screenHeight / 2.0f;
    
    // Semi-transparent dark overlay
    renderQuad(0, 0, (float)screenWidth, (float)screenHeight, glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Game Over title box
    const float titleWidth = 400.0f;
    const float titleHeight = 80.0f;
    const float titleX = centerX - titleWidth / 2.0f;
    const float titleY = centerY - 200.0f;
    
    // Title background (dark red)
    renderQuad(titleX, titleY, titleWidth, titleHeight, glm::vec3(0.5f, 0.0f, 0.0f));
    // Title border (bright red)
    float borderThickness = 3.0f;
    renderQuad(titleX, titleY, titleWidth, borderThickness, glm::vec3(1.0f, 0.0f, 0.0f));
    renderQuad(titleX, titleY + titleHeight - borderThickness, titleWidth, borderThickness, glm::vec3(1.0f, 0.0f, 0.0f));
    renderQuad(titleX, titleY, borderThickness, titleHeight, glm::vec3(1.0f, 0.0f, 0.0f));
    renderQuad(titleX + titleWidth - borderThickness, titleY, borderThickness, titleHeight, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Score display box
    const float scoreWidth = 300.0f;
    const float scoreHeight = 60.0f;
    const float scoreX = centerX - scoreWidth / 2.0f;
    const float scoreY = titleY + titleHeight + 30.0f;
    
    // Score background (dark gray)
    renderQuad(scoreX, scoreY, scoreWidth, scoreHeight, glm::vec3(0.15f, 0.15f, 0.2f));
    // Score border (gold)
    renderQuad(scoreX, scoreY, scoreWidth, borderThickness, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(scoreX, scoreY + scoreHeight - borderThickness, scoreWidth, borderThickness, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(scoreX, scoreY, borderThickness, scoreHeight, glm::vec3(1.0f, 0.84f, 0.0f));
    renderQuad(scoreX + scoreWidth - borderThickness, scoreY, borderThickness, scoreHeight, glm::vec3(1.0f, 0.84f, 0.0f));
    
    // Score bars visualization
    int scoreBars = glm::min(finalScore / 5, 28);
    float scoreBarWidth = (scoreWidth - 16.0f) / 28.0f;
    for (int i = 0; i < scoreBars; ++i) {
        renderQuad(scoreX + 8.0f + i * scoreBarWidth + i * 1.0f, 
                  scoreY + 8.0f, 
                  scoreBarWidth - 1.0f, 
                  scoreHeight - 16.0f, 
                  glm::vec3(1.0f, 0.84f, 0.0f));
    }
    
    // Continue button
    const float buttonWidth = 250.0f;
    const float buttonHeight = 60.0f;
    const float continueButtonX = centerX - buttonWidth / 2.0f;
    const float continueButtonY = scoreY + scoreHeight + 50.0f;
    
    glm::vec3 continueColor = continueButtonHovered ? glm::vec3(0.3f, 0.8f, 0.3f) : glm::vec3(0.2f, 0.6f, 0.2f);
    renderQuad(continueButtonX, continueButtonY, buttonWidth, buttonHeight, continueColor);
    // Button border (white)
    renderQuad(continueButtonX, continueButtonY, buttonWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(continueButtonX, continueButtonY + buttonHeight - borderThickness, buttonWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(continueButtonX, continueButtonY, borderThickness, buttonHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(continueButtonX + buttonWidth - borderThickness, continueButtonY, borderThickness, buttonHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    
    // Exit button
    const float exitButtonX = centerX - buttonWidth / 2.0f;
    const float exitButtonY = continueButtonY + buttonHeight + 20.0f;
    
    glm::vec3 exitColor = exitButtonHovered ? glm::vec3(0.8f, 0.3f, 0.3f) : glm::vec3(0.6f, 0.2f, 0.2f);
    renderQuad(exitButtonX, exitButtonY, buttonWidth, buttonHeight, exitColor);
    // Button border (white)
    renderQuad(exitButtonX, exitButtonY, buttonWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(exitButtonX, exitButtonY + buttonHeight - borderThickness, buttonWidth, borderThickness, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(exitButtonX, exitButtonY, borderThickness, buttonHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    renderQuad(exitButtonX + buttonWidth - borderThickness, exitButtonY, borderThickness, buttonHeight, glm::vec3(1.0f, 1.0f, 1.0f));
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

bool GameUI::isPointInRect(double mouseX, double mouseY, float rectX, float rectY, float rectW, float rectH)
{
    return mouseX >= rectX && mouseX <= rectX + rectW &&
           mouseY >= rectY && mouseY <= rectY + rectH;
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
