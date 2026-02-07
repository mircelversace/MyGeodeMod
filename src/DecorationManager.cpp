#include "DecorationManager.hpp"
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

// Helper to create objects relative to a target
void createDecoration(GameObject* target, int id, float xOffset, float yOffset, float scale, float rotation, int zLayer, int zOrder, bool useBaseColor) {
    auto editorUI = EditorUI::get();
    if (!editorUI) return;
    
    // Calculate absolute position
    auto pos = target->getPosition();
    pos.x += xOffset;
    pos.y += yOffset;

    // Create new object
    // Note: createObject returns void in some versions, or the object itself. 
    // We use createObject which typically places it in the level.
    // For safer object creation that returns the object:
    auto obj = editorUI->createObject(id, pos);
    if (obj) {
        obj->setScale(scale);
        obj->setRotation(rotation);
        obj->setZOrder(zOrder);
        obj->setZLayer(zLayer);
        
        if (useBaseColor) {
            // Apply color from target if needed, or use a specific color channel
            // logic here. For now, let's just make it share the base color ID if applicable.
            auto baseColor = target->getBaseColor();
            obj->setBaseColor(baseColor);
        }
    }
}

// ----------------------------------------------------------------------------
// Preset Logic
// ----------------------------------------------------------------------------

void DecorationManager::applyDecoration(CCArray* objects, int presetIndex) {
    if (!objects) return;
    
    // Process each object
    for (int i = 0; i < objects->count(); ++i) {
        auto obj = static_cast<GameObject*>(objects->objectAtIndex(i));
        decorateObject(obj, presetIndex);
    }
}

void DecorationManager::decorateObject(GameObject* target, int presetIndex) {
    // Simple heuristic to determine if it's a block or spike (or other)
    // We can check object ID ranges or types. 
    // For this prototype, we'll try to apply generic decorations based on bounding box.
    
    auto size = target->getContentSize();
    auto scale = target->getScale();
    float w = size.width * scale;
    float h = size.height * scale;
    float halfW = w / 2;
    float halfH = h / 2;

    // IDs used below are vanilla object IDs. 
    // 1000+ are usually decorations. 
    // 1 is a standard block.
    
    switch (presetIndex) {
        case 0: // Simple Glow Top
            createDecoration(target, 100, 0, halfH, 1.0f, 0, 1, 10, true); // Glow piece ID 100 (example)
            break;
        case 1: // Double Glow
            createDecoration(target, 100, 0, halfH, 1.0f, 0, 1, 10, true);
            createDecoration(target, 100, 0, -halfH, 1.0f, 180, 1, 10, true);
            break;
        case 2: // Outline Detail (Corner dots)
            createDecoration(target, 200, -halfW + 5, halfH - 5, 0.5f, 0, 2, 5, false); // Dot
            createDecoration(target, 200, halfW - 5, halfH - 5, 0.5f, 0, 2, 5, false);
            createDecoration(target, 200, -halfW + 5, -halfH + 5, 0.5f, 0, 2, 5, false);
            createDecoration(target, 200, halfW - 5, -halfH + 5, 0.5f, 0, 2, 5, false);
            break;
        case 3: // Inner Corner Detail
            createDecoration(target, 201, -halfW + 10, halfH - 10, 0.8f, 0, 2, 6, true);
            createDecoration(target, 201, halfW - 10, halfH - 10, 0.8f, 90, 2, 6, true);
            break;
        case 4: // Tech Debris
            createDecoration(target, 301, 0, 0, 1.0f, 45, 1, -1, false); // Gear inside
            break;
        case 5: // Nature/Leaves
            createDecoration(target, 401, -halfW, halfH, 0.8f, 0, 3, 20, false); // Leaf
            createDecoration(target, 401, halfW, halfH, 0.8f, 90, 3, 20, false);
            break;
        case 6: // Spike Caps (If spike)
             // Assumes target is spike-like. If block, places on top.
             createDecoration(target, 501, 0, halfH + 5, 0.5f, 0, 3, 50, true); 
             break;
        case 7: // Base Support Top
             createDecoration(target, 601, 0, halfH, 1.0f, 0, 0, -5, true); 
             break;
        case 8: // Base Support Bottom
             createDecoration(target, 601, 0, -halfH, 1.0f, 180, 0, -5, true); 
             break;
        case 9: // Neon Pulse
             createDecoration(target, 701, 0, 0, 1.2f, 0, 0, -10, true); // Big glow behind
             break;
        case 10: // Crystal Shards
             createDecoration(target, 801, -halfW/2, halfH/2, 0.4f, 15, 2, 10, false);
             createDecoration(target, 801, halfW/2, -halfH/2, 0.6f, -15, 2, 10, false);
             break;
        case 11: // Mechanical Gears
             createDecoration(target, 901, -halfW, 0, 0.5f, 0, 2, 5, false);
             createDecoration(target, 901, halfW, 0, 0.5f, 0, 2, 5, false);
             break;
        case 12: // Floating Orbs
             createDecoration(target, 1001, 0, halfH + 15, 0.3f, 0, 4, 100, true);
             break;
         case 13: // Chains Hanging
             createDecoration(target, 1101, 0, -halfH - 10, 1.0f, 0, 0, -5, false);
             break;
         case 14: // Toxic Drip
             createDecoration(target, 1201, 0, -halfH, 0.8f, 0, 3, 10, true);
             break;
         case 15: // Fire Embers
             createDecoration(target, 1301, 0, 0, 1.0f, 0, 4, 15, false);
             break;
         case 16: // Ice Icicles
             createDecoration(target, 1401, 0, -halfH, 1.0f, 180, 3, 20, false);
             break;
         case 17: // Shadow Depth
             createDecoration(target, 1501, 5, -5, 1.0f, 0, -1, -100, true); // Black block offset
             break;
         case 18: // Retro Pixel
             createDecoration(target, 1601, -halfW+5, halfH-5, 1.0f, 0, 2, 10, false);
             createDecoration(target, 1601, halfW-5, -halfH+5, 1.0f, 0, 2, 10, false);
             break;
         case 19: // Future Grid
             createDecoration(target, 1701, 0, 0, 1.0f, 0, 1, 1, true); // Grid texture overlay
             break;
         default:
             break;
    }
}
