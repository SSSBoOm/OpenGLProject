#include "collectible.h"
#include <glad/glad.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/model.h>
#include <learnopengl/filesystem.h>
#include "../scene/Terrain.h"

static const int ITEM_SEGMENTS = 32;
static const float BASE_RADIUS = 0.5f;
static const float BASE_HALF_HEIGHT = 0.05f;
static const float DEFAULT_SCALE = 0.75f;

Collectibles::Collectibles()
{
}

void Collectibles::init()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

glm::vec3 Collectibles::getColor(CollectibleType type)
{
    switch (type) {
        case CollectibleType::COIN:
            return glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
        case CollectibleType::COIN_RARE:
            return glm::vec3(1.0f, 0.84f, 0.0f); // Gold/Orange
        case CollectibleType::TURBO:
            return glm::vec3(1.0f, 0.0f, 0.0f); // Red
        case CollectibleType::FUEL:
            return glm::vec3(0.0f, 1.0f, 0.0f); // Green
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f); // White
    }
}

float Collectibles::getScale(CollectibleType type)
{
    switch (type) {
        case CollectibleType::COIN:
            return DEFAULT_SCALE;
        case CollectibleType::COIN_RARE:
            return DEFAULT_SCALE * 2.0;
        case CollectibleType::TURBO:
            return DEFAULT_SCALE * 3.0f;
        case CollectibleType::FUEL:
            return DEFAULT_SCALE * 0.5;
        default:
            return DEFAULT_SCALE;
    }
}

int Collectibles::getDefaultValue(CollectibleType type)
{
    switch (type) {
        case CollectibleType::COIN:
            return 1;
        case CollectibleType::COIN_RARE:
            return 2;
        case CollectibleType::TURBO:
            return 0; // turbo boost amount
        case CollectibleType::FUEL:
            return 15; // fuel refill amount (15%)
        default:
            return 1;
    }
}

void Collectibles::spawnItem(const glm::vec3 &position, CollectibleType type)
{
    CollectibleItem item;
    item.position = position;
    item.collected = false;
    item.type = type;
    item.value = getDefaultValue(type);
    item.color = getColor(type);
    
    item.bobAmplitude = 0.04f + (std::rand() / (float)RAND_MAX) * 0.08f; // 0.04 - 0.12
    item.bobFrequency = 2.0f + (std::rand() / (float)RAND_MAX) * 3.0f;   // 2 - 5
    item.bobPhase = (std::rand() / (float)RAND_MAX) * 2.0f * (float)M_PI;
    
    items.push_back(item);
}

void Collectibles::spawnRandom(int count, CollectibleType type)
{
    const float desiredFaceHeight = 1.0f;
    const float baseLift = BASE_HALF_HEIGHT * getScale(type);
    const float storedY = desiredFaceHeight - baseLift;

    for (int i = 0; i < count; ++i) {
        float x = (std::rand() / (float)RAND_MAX) * 60.0f - 20.0f;
        float z = (std::rand() / (float)RAND_MAX) * 60.0f - 20.0f;
        
        CollectibleType itemType = type;
        // For coins, randomly make some rare
        if (type == CollectibleType::COIN && (std::rand() % 100) < 20) {
            itemType = CollectibleType::COIN_RARE;
        }
        
        spawnItem(glm::vec3(x, storedY, z), itemType);
    }
}

void Collectibles::spawnAlongDirection(int count, const glm::vec3 &origin, const glm::vec3 &forward, 
                                       const Terrain *terrain, CollectibleType type,
                                       float minForward, float maxForward, float lateralRange)
{
    const float baseLift = BASE_HALF_HEIGHT * getScale(type);

    glm::vec3 f = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    glm::vec3 right = glm::normalize(glm::vec3(-f.z, 0.0f, f.x));

    for (int i = 0; i < count; ++i) {
        float along = minForward + (std::rand() / (float)RAND_MAX) * (maxForward - minForward);
        float lateral = ((std::rand() / (float)RAND_MAX) - 0.5f) * lateralRange;

        glm::vec3 pos = origin + f * along + right * lateral;
        float sampledY = 1.0f;
        if (terrain) {
            sampledY = terrain->getHeight(pos.x, pos.z);
        }
        
        CollectibleType itemType = type;
        if (type == CollectibleType::COIN && (std::rand() % 100) < 20) {
            itemType = CollectibleType::COIN_RARE;
        }
        
        spawnItem(glm::vec3(pos.x, sampledY + baseLift, pos.z), itemType);
    }
}

int Collectibles::totalCount() const
{
    return static_cast<int>(items.size());
}

bool Collectibles::hasItemsInDirection(const glm::vec3 &origin, const glm::vec3 &forward,
                                       float minForward, float maxForward, float lateralRange, 
                                       int minCount, CollectibleType type) const
{
    if (minCount <= 0) return true;
    glm::vec3 f = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    int found = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].collected) continue;
        // Filter by type if checking for specific item type (or COIN includes both regular and rare)
        if (type == CollectibleType::COIN) {
            if (items[i].type != CollectibleType::COIN && items[i].type != CollectibleType::COIN_RARE) 
                continue;
        } else if (items[i].type != type) {
            continue;
        }
        
        glm::vec3 v = items[i].position - origin;
        glm::vec3 vXZ = glm::vec3(v.x, 0.0f, v.z);
        float forwardDist = glm::dot(vXZ, f);
        if (forwardDist < minForward || forwardDist > maxForward) continue;
        
        glm::vec3 proj = f * forwardDist;
        float lateralDist = glm::length(vXZ - proj);
        if (lateralDist <= lateralRange) {
            ++found;
            if (found >= minCount) return true;
        }
    }
    return false;
}

int Collectibles::updateCollect(const glm::vec3 &carPos, float carRadius, const glm::vec3 &carForward, 
                               float carSpeed, std::vector<CollectibleItem> &outCollected)
{
    int newly = 0;
    outCollected.clear();
    
    const float itemRadius = getScale(CollectibleType::COIN) * BASE_RADIUS; // Use default scale
    const float speedThreshold = 0.1f;
    const float minDot = 0.5f;
    const float minDistanceXZ = 0.3f;
    const float maxVerticalDist = 2.0f;

    glm::vec3 f = glm::normalize(glm::vec3(carForward.x, 0.0f, carForward.z));

    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].collected) continue;

        glm::vec2 carXZ = glm::vec2(carPos.x, carPos.z);
        glm::vec2 itemXZ = glm::vec2(items[i].position.x, items[i].position.z);
        float distXZ = glm::distance(carXZ, itemXZ);
        float threshXZ = carRadius + itemRadius + 0.05f;
        
        float verticalDist = std::abs(carPos.y - items[i].position.y);
        
        if (distXZ < threshXZ && verticalDist < maxVerticalDist) {
            if (carSpeed < speedThreshold) continue;
            if (distXZ < minDistanceXZ) continue;
            
            glm::vec3 toItem = glm::normalize(glm::vec3(itemXZ.x - carXZ.x, 0.0f, itemXZ.y - carXZ.y));
            float dot = glm::dot(f, toItem);
            if (dot < minDot) continue;

            items[i].collected = true;
            outCollected.push_back(items[i]);
            newly += items[i].value;
        }
    }
    return newly;
}

int Collectibles::remaining() const
{
    int rem = 0;
    for (size_t i = 0; i < items.size(); ++i) 
        if (!items[i].collected) ++rem;
    return rem;
}

void Collectibles::draw(Shader &shader, unsigned int fallbackTexture)
{
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].collected) continue;
        
        glm::mat4 m(1.0f);
        float t = static_cast<float>(glfwGetTime());

        float bounce = sinf(t * items[i].bobFrequency + items[i].bobPhase) * items[i].bobAmplitude;
        float bounceAbs = fabsf(bounce);
        float scale = getScale(items[i].type);
        float baseLift = BASE_HALF_HEIGHT * scale;
        m = glm::translate(m, items[i].position + glm::vec3(0.0f, baseLift + bounceAbs, 0.0f));

        m = glm::rotate(m, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        float spin = t * 180.0f;
        m = glm::rotate(m, glm::radians(spin), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(scale));

        shader.setMat4("model", m);
        
        // Use specific model for this item type, or fallback to COIN model
        Model* modelToUse = nullptr;
        CollectibleType typeForModel = items[i].type;
        
        // Rare coins use the same model as regular coins (just different color)
        if (typeForModel == CollectibleType::COIN_RARE) {
            typeForModel = CollectibleType::COIN;
        }
        
        auto it = models.find(typeForModel);
        if (it != models.end()) {
            modelToUse = it->second;
        } else {
            // Fallback to COIN model if specific model not found
            auto fallbackIt = models.find(CollectibleType::COIN);
            if (fallbackIt != models.end()) {
                modelToUse = fallbackIt->second;
            }
        }
        
        // Determine if this item type should use color override or model's own materials
        bool useColorOverride = (items[i].type == CollectibleType::COIN || 
                                 items[i].type == CollectibleType::COIN_RARE);
        
        if (useColorOverride) {
            // For coins: use simple texture with color override
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fallbackTexture);
            shader.setInt("texture_diffuse1", 0);
            shader.setBool("useColor", true);
            shader.setVec3("objectColor", items[i].color);
        } else {
            // For other models (fuel, turbo): use their embedded materials
            shader.setBool("useColor", false);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        if (modelToUse) {
            modelToUse->Draw(shader);
        }
        
        // Reset state after drawing
        shader.setBool("useColor", false);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    glBindVertexArray(0);
}
