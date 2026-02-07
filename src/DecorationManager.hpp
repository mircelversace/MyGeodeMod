#pragma once

#include <Geode/Geode.hpp>

class DecorationManager {
public:
    static void applyDecoration(cocos2d::CCArray* objects, int presetIndex);
    
private:
    static void decorateObject(GameObject* obj, int presetIndex);
    // Add helpers for specific decoration types
};
