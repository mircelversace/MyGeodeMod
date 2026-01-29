#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

// ==========================================
// GLOBALS
// ==========================================
enum class DecoStyle { Glow, Modern, Tech };
static DecoStyle g_currentStyle = DecoStyle::Glow;

// Object IDs for decorations
namespace DecoObjects {
    // Glow objects
    constexpr int GLOW_SQUARE = 1764;      // Square glow
    constexpr int GLOW_CIRCLE = 1766;      // Circle glow
    
    // Outline/Edge objects  
    constexpr int OUTLINE_SQUARE = 211;     // Square outline
    constexpr int SLOPE_DECO = 578;         // Slope decoration
    
    // Tech/Modern objects
    constexpr int TECH_SQUARE = 915;        // Tech square
    constexpr int TECH_CIRCLE = 917;        // Tech circle
    constexpr int GRID_DECO = 1338;         // Grid pattern
}

// ==========================================
// AUTO-DECORATION LOGIC
// ==========================================
class AutoDecorator {
public:
    static void decorateSelection(EditorUI* ui) {
        auto lel = LevelEditorLayer::get();
        if (!lel) {
            Notification::create("Error: No editor layer!", NotificationIcon::Error)->show();
            return;
        }

        auto selectedObjs = ui->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() == 0) {
            Notification::create("Select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        int decorationsAdded = 0;

        // Iterate through each selected object
        for (int i = 0; i < selectedObjs->count(); i++) {
            auto obj = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (!obj) continue;

            // Get object position and size
            CCPoint pos = obj->getPosition();
            CCSize size = obj->getContentSize() * obj->getScale();
            
            // Apply decoration based on style
            switch (g_currentStyle) {
                case DecoStyle::Glow:
                    decorationsAdded += addGlowDecoration(lel, ui, pos, size, obj);
                    break;
                case DecoStyle::Modern:
                    decorationsAdded += addModernDecoration(lel, ui, pos, size, obj);
                    break;
                case DecoStyle::Tech:
                    decorationsAdded += addTechDecoration(lel, ui, pos, size, obj);
                    break;
            }
        }

        // Deselect after decorating
        ui->deselectAll();

        const char* styleName = "Unknown";
        if(g_currentStyle == DecoStyle::Glow) styleName = "Glow";
        if(g_currentStyle == DecoStyle::Modern) styleName = "Modern";
        if(g_currentStyle == DecoStyle::Tech) styleName = "Tech";

        Notification::create(
            fmt::format("Added {} {} decorations!", decorationsAdded, styleName).c_str(),
            NotificationIcon::Success
        )->show();
    }

private:
    static GameObject* createObject(LevelEditorLayer* lel, int objectID, CCPoint pos) {
        // Create the object
        auto obj = lel->createObject(objectID, pos, false);
        if (obj) {
            // Add to editor
            lel->addSpecial(obj);
        }
        return obj;
    }

    static int addGlowDecoration(LevelEditorLayer* lel, EditorUI* ui, CCPoint pos, CCSize size, GameObject* original) {
        int count = 0;
        float offset = 15.0f;

        // Add glow behind the object (4 corners)
        std::vector<CCPoint> positions = {
            {pos.x - offset, pos.y - offset},
            {pos.x + offset, pos.y - offset},
            {pos.x - offset, pos.y + offset},
            {pos.x + offset, pos.y + offset}
        };

        for (auto& p : positions) {
            auto glow = createObject(lel, DecoObjects::GLOW_SQUARE, p);
            if (glow) {
                glow->setScale(0.5f);
                // Set to be behind (lower z-order)
                glow->setZOrder(original->getZOrder() - 1);
                count++;
            }
        }

        // Add center glow
        auto centerGlow = createObject(lel, DecoObjects::GLOW_CIRCLE, pos);
        if (centerGlow) {
            centerGlow->setScale(1.2f);
            centerGlow->setZOrder(original->getZOrder() - 1);
            centerGlow->setOpacity(150);
            count++;
        }

        return count;
    }

    static int addModernDecoration(LevelEditorLayer* lel, EditorUI* ui, CCPoint pos, CCSize size, GameObject* original) {
        int count = 0;
        float offset = 20.0f;

        // Add outlines on edges
        std::vector<std::pair<CCPoint, float>> edges = {
            {{pos.x, pos.y + offset}, 0.0f},   // Top
            {{pos.x, pos.y - offset}, 180.0f}, // Bottom
            {{pos.x - offset, pos.y}, 90.0f},  // Left
            {{pos.x + offset, pos.y}, 270.0f}  // Right
        };

        for (auto& edge : edges) {
            auto outline = createObject(lel, DecoObjects::OUTLINE_SQUARE, edge.first);
            if (outline) {
                outline->setRotation(edge.second);
                outline->setScale(0.4f);
                outline->setZOrder(original->getZOrder() + 1);
                count++;
            }
        }

        return count;
    }

    static int addTechDecoration(LevelEditorLayer* lel, EditorUI* ui, CCPoint pos, CCSize size, GameObject* original) {
        int count = 0;
        float offset = 25.0f;

        // Add tech grid pattern around object
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue; // Skip center

                CCPoint decoPos = {pos.x + dx * offset, pos.y + dy * offset};
                auto tech = createObject(lel, DecoObjects::TECH_SQUARE, decoPos);
                if (tech) {
                    tech->setScale(0.3f);
                    tech->setOpacity(100);
                    tech->setZOrder(original->getZOrder() - 1);
                    count++;
                }
            }
        }

        // Add corner circles
        std::vector<CCPoint> corners = {
            {pos.x - offset * 1.5f, pos.y - offset * 1.5f},
            {pos.x + offset * 1.5f, pos.y + offset * 1.5f}
        };

        for (auto& corner : corners) {
            auto circle = createObject(lel, DecoObjects::TECH_CIRCLE, corner);
            if (circle) {
                circle->setScale(0.25f);
                circle->setZOrder(original->getZOrder() - 1);
                count++;
            }
        }

        return count;
    }
};

// ==========================================
// POPUP MENU
// ==========================================
class AutoDecoMenu : public geode::Popup<> {
protected:
    bool setup() override {
        setTitle("AutoDecoration");
        
        auto menu = CCMenu::create();
        menu->setContentSize(m_mainLayer->getContentSize());
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto center = m_mainLayer->getContentSize() / 2;

        // Info label
        auto info = CCLabelBMFont::create("Select style, then Apply!", "chatFont.fnt");
        info->setPosition({center.width, center.height + 70});
        info->setScale(0.7f);
        m_mainLayer->addChild(info);

        // Style buttons
        createStyleBtn("GLOW", center.height + 30, DecoStyle::Glow, menu, center.width - 70, ccColor3B{255, 200, 100});
        createStyleBtn("MODERN", center.height + 30, DecoStyle::Modern, menu, center.width, ccColor3B{100, 200, 255});
        createStyleBtn("TECH", center.height + 30, DecoStyle::Tech, menu, center.width + 70, ccColor3B{200, 100, 255});

        // Apply button
        auto applySprite = ButtonSprite::create("APPLY", 120, true, "bigFont.fnt", "GJ_button_01.png", 35, 0.6f);
        auto applyBtn = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(AutoDecoMenu::onApply));
        applyBtn->setPosition({center.width, center.height - 40});
        menu->addChild(applyBtn);

        return true;
    }

    void createStyleBtn(const char* txt, float y, DecoStyle style, CCMenu* menu, float x, ccColor3B color) {
        bool isSelected = (g_currentStyle == style);
        const char* textureName = isSelected ? "GJ_button_02.png" : "GJ_button_04.png";
        
        auto sprite = ButtonSprite::create(txt, 60, true, "bigFont.fnt", textureName, 25, 0.5f);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(AutoDecoMenu::onSelectStyle));
        btn->setTag(static_cast<int>(style));
        btn->setPosition({x, y});
        menu->addChild(btn);
    }

    void onSelectStyle(CCObject* sender) {
        int val = static_cast<CCNode*>(sender)->getTag();
        g_currentStyle = static_cast<DecoStyle>(val);
        
        // Refresh menu to show selection
        this->onClose(nullptr);
        AutoDecoMenu::create()->show();
    }

    void onApply(CCObject* sender) {
        auto editorUI = LevelEditorLayer::get()->m_editorUI;
        if (editorUI) {
            AutoDecorator::decorateSelection(editorUI);
        }
        this->onClose(sender);
    }

public:
    static AutoDecoMenu* create() {
        auto ret = new AutoDecoMenu();
        if (ret && ret->initAnchored(300.f, 180.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// EDITOR HOOK
// ==========================================
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create button near pause
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        sprite->setScale(0.75f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onAutoDecoBtn));
        btn->setPosition({winSize.width - 85, winSize.height - 25});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.35f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v1.0 loaded!");

        return true;
    }

    void onAutoDecoBtn(CCObject*) {
        AutoDecoMenu::create()->show();
    }
};
