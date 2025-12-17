#ifndef GAME_UI_H
#define GAME_UI_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader_m.h>
#include <string>
#include <memory>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class GameUI {
public:
    GameUI();
    ~GameUI();
    
    void init(unsigned int screenWidth, unsigned int screenHeight);
    void render(float fuelPercent, float turboPercent, int score, float speed = 0.0f, float maxSpeed = 40.0f);
    void renderGameOver(int finalScore, bool &continueButtonHovered, bool &exitButtonHovered);
    bool isPointInRect(double mouseX, double mouseY, float rectX, float rectY, float rectW, float rectH);
    void cleanup();
    
    void setScreenSize(unsigned int width, unsigned int height);
    
    // Public rendering methods for external UI
    void renderQuad(float x, float y, float width, float height, const glm::vec3& color);
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    float getTextWidth(const std::string& text, float scale);
    float getTextHeight(const std::string& text, float scale);
    
private:
    unsigned int screenWidth;
    unsigned int screenHeight;
    unsigned int quadVAO, quadVBO;
    unsigned int textVAO, textVBO;
    unsigned int fuelIconTexture, turboIconTexture, coinIconTexture, speedometerTexture;
    std::unique_ptr<Shader> uiShader;
    std::unique_ptr<Shader> textShader;
    std::unique_ptr<Shader> iconShader;
    glm::mat4 projection;
    std::map<char, Character> Characters;
    
    void renderIcon(unsigned int textureID, float x, float y, float size);
    void renderRotatedIcon(unsigned int textureID, float x, float y, float size, float angleDegrees);
    void initQuad();
    void initText();
    void loadFont(const std::string& fontPath, unsigned int fontSize);
    unsigned int loadIcon(const std::string& imagePath);
};

#endif
