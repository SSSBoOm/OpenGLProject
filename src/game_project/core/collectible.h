#ifndef ASSIGNMENT3_COLLECTIBLE_H
#define ASSIGNMENT3_COLLECTIBLE_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include <memory>

class Terrain;

enum class CollectibleType {
    COIN,
    COIN_RARE,
    TURBO,
    FUEL
};

struct CollectibleItem {
    glm::vec3 position;
    bool collected;
    CollectibleType type;
    int value;           // coin value, turbo amount, fuel amount, etc.
    float bobAmplitude;
    float bobFrequency;
    float bobPhase;
    glm::vec3 color;     // visual color of the item
};

class Collectibles {
public:
    Collectibles();
    void init();
    void spawnRandom(int count, CollectibleType type = CollectibleType::COIN);
    void spawnAlongDirection(int count, const glm::vec3 &origin, const glm::vec3 &forward, 
                            const Terrain *terrain, CollectibleType type = CollectibleType::COIN,
                            float minForward = 4.0f, float maxForward = 30.0f, float lateralRange = 8.0f);
    void spawnMixedGroup(int coinCount, const glm::vec3 &origin, const glm::vec3 &forward,
                        const Terrain *terrain, float minForward = 4.0f, float maxForward = 30.0f,
                        float lateralRange = 8.0f, int rareCoinChance = 15, int turboChance = 10,
                        int fuelChance = 5);
    int updateCollect(const glm::vec3 &carPos, float carRadius, const glm::vec3 &carForward, 
                     float carSpeed, std::vector<CollectibleItem> &outCollected);
    void draw(Shader &shader, unsigned int fallbackTexture);
    int remaining() const;
    int totalCount() const;
    bool hasItemsInDirection(const glm::vec3 &origin, const glm::vec3 &forward,
                            float minForward, float maxForward, float lateralRange, 
                            int minCount = 1, CollectibleType type = CollectibleType::COIN) const;
    void setModel(CollectibleType type, Model *m) { models[type] = m; }
    static glm::vec3 getColor(CollectibleType type);
    static float getScale(CollectibleType type);
    static int getDefaultValue(CollectibleType type);
    static float getYOffset(CollectibleType type);
    static int getMaxSpawnCount(CollectibleType type);
    
private:
    std::map<CollectibleType, Model*> models;
    std::vector<CollectibleItem> items;
    
    void spawnItem(const glm::vec3 &position, CollectibleType type);
};

#endif
