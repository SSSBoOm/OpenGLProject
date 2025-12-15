#include "GameUI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <glad/glad.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/filesystem.h>

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
    initText();
    
    // Load UI shader
    uiShader = std::make_unique<Shader>("ui.vs", "ui.fs");
    textShader = std::make_unique<Shader>("text.vs", "text.fs");
    
    // Setup orthographic projection
    projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    
    // Load font
    loadFont(FileSystem::getPath("resources/fonts/Arial.ttf"), 48);
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

void GameUI::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color)
{
    if (!textShader) return;
    
    // Activate corresponding render state
    textShader->use();
    textShader->setMat4("projection", projection);
    textShader->setVec3("textColor", color);
    textShader->setInt("text", 0); 
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y + (Characters['H'].Bearing.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos,     ypos,       0.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f }
        };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Now advance cursors for next glyph
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

float GameUI::getTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];
        width += (ch.Advance >> 6) * scale;
    }
    return width;
}

float GameUI::getTextHeight(const std::string& text, float scale)
{
    // Use 'H' as reference for typical capital letter height
    return Characters['H'].Size.y * scale;
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
    
    // "GAME OVER!" text - dynamically scale to fit
    std::string gameOverText = "GAME OVER!";
    float maxTitleScale = 1.2f;
    float titleTextWidth = getTextWidth(gameOverText, maxTitleScale);
    float titleAvailableWidth = titleWidth - 40.0f; // padding inside box
    float titleScale = (titleTextWidth > titleAvailableWidth) ? (titleAvailableWidth / titleTextWidth) * maxTitleScale : maxTitleScale;
    float actualTitleWidth = getTextWidth(gameOverText, titleScale);
    float textX = centerX - actualTitleWidth / 2.0f;
    // Y position: center of box minus half bearing for perfect centering
    float textY = titleY + titleHeight / 2.0f - (Characters['H'].Bearing.y * titleScale / 4.0f);
    renderText(gameOverText, textX, textY, titleScale, glm::vec3(1.0f, 1.0f, 0.0f));
    
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
    
    // "Score: XXX" text - dynamically scale to fit
    std::string scoreText = "Score: " + std::to_string(finalScore);
    float maxScoreScale = 0.8f;
    float scoreTextWidth = getTextWidth(scoreText, maxScoreScale);
    float scoreAvailableWidth = scoreWidth - 40.0f; // padding inside box
    float scoreScale = (scoreTextWidth > scoreAvailableWidth) ? (scoreAvailableWidth / scoreTextWidth) * maxScoreScale : maxScoreScale;
    float actualScoreWidth = getTextWidth(scoreText, scoreScale);
    float scoreTextX = centerX - actualScoreWidth / 2.0f;
    // Y position: center of box minus half bearing for perfect centering
    float scoreTextY = scoreY + scoreHeight / 2.0f - (Characters['H'].Bearing.y * scoreScale / 4.0f);
    renderText(scoreText, scoreTextX, scoreTextY, scoreScale, glm::vec3(1.0f, 0.84f, 0.0f));
    
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
    
    // "CONTINUE" button text - dynamically scale to fit
    std::string continueText = "CONTINUE";
    float maxContinueScale = 0.7f;
    float continueTextWidth = getTextWidth(continueText, maxContinueScale);
    float continueAvailableWidth = buttonWidth - 40.0f; // padding inside button
    float continueScale = (continueTextWidth > continueAvailableWidth) ? (continueAvailableWidth / continueTextWidth) * maxContinueScale : maxContinueScale;
    float actualContinueWidth = getTextWidth(continueText, continueScale);
    float continueTextX = continueButtonX + (buttonWidth - actualContinueWidth) / 2.0f;
    // Y position: center of button minus half bearing for perfect centering
    float continueTextY = continueButtonY + buttonHeight / 2.0f - (Characters['H'].Bearing.y * continueScale / 4.0f);
    renderText(continueText, continueTextX, continueTextY, continueScale, glm::vec3(1.0f, 1.0f, 1.0f));
    
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
    
    // "EXIT GAME" button text - dynamically scale to fit
    std::string exitText = "EXIT GAME";
    float maxExitScale = 0.7f;
    float exitTextWidth = getTextWidth(exitText, maxExitScale);
    float exitAvailableWidth = buttonWidth - 40.0f; // padding inside button
    float exitScale = (exitTextWidth > exitAvailableWidth) ? (exitAvailableWidth / exitTextWidth) * maxExitScale : maxExitScale;
    float actualExitWidth = getTextWidth(exitText, exitScale);
    float exitTextX = exitButtonX + (buttonWidth - actualExitWidth) / 2.0f;
    // Y position: center of button minus half bearing for perfect centering
    float exitTextY = exitButtonY + buttonHeight / 2.0f - (Characters['H'].Bearing.y * exitScale / 4.0f);
    renderText(exitText, exitTextX, exitTextY, exitScale, glm::vec3(1.0f, 1.0f, 1.0f));
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

bool GameUI::isPointInRect(double mouseX, double mouseY, float rectX, float rectY, float rectW, float rectH)
{
    return mouseX >= rectX && mouseX <= rectX + rectW &&
           mouseY >= rectY && mouseY <= rectY + rectH;
}

void GameUI::initText()
{
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GameUI::loadFont(const std::string& fontPath, unsigned int fontSize)
{
    // Initialize FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font at " << fontPath << std::endl;
        return;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        
        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    std::cout << "Font loaded successfully: " << fontPath << std::endl;
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
    if (textVAO != 0) {
        glDeleteVertexArrays(1, &textVAO);
        textVAO = 0;
    }
    if (textVBO != 0) {
        glDeleteBuffers(1, &textVBO);
        textVBO = 0;
    }
}
