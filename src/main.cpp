#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/cocos.hpp>
#include <vector>

using namespace geode::prelude;

// ==========================================
// GLOBALS & TYPES
// ==========================================
enum class DecoStyle { 
    Glow,           // Soft glow behind blocks
    Outline,        // White outline around blocks  
    Shadow,         // Dark shadow beneath
    Neon,           // Bright neon effect
    Gradient,       // Gradient fade effect
    CustomTemplate  // User-saved template
};

static DecoStyle g_currentStyle = DecoStyle::Glow;

// Template storage - stores relative positions of decoration objects
struct DecoTemplate {
    std::string name;
    std::vector<std::tuple<int, float, float, float, float>> objects; // objectID, relX, relY, scale, rotation
};

static std::vector<DecoTemplate> g_templates;
static int g_selectedTemplate = -1;

// Better object IDs that actually look good
namespace DecoIDs {
    // Glows (work well)
    constexpr int GLOW_SQUARE_FULL = 1731;   // Full square glow
    constexpr int GLOW_GRADIENT = 1732;       // Gradient glow
    constexpr int GLOW_SOFT = 1736;           // Soft glow
    
    // Outlines
    constexpr int WHITE_SQUARE = 211;         // Simple white outline
    constexpr int THIN_LINE = 579;            // Thin line
    
    // Decorative
    constexpr int DECO_LINE = 580;            // Decorative line
    constexpr int CORNER_PIECE = 589;         // Corner decoration
    constexpr int ARROW_DECO = 1354;          // Arrow/tech deco
}

// ==========================================
// TEMPLATE MANAGER
// ==========================================
class TemplateManager {
public:
    static void saveSelectionAsTemplate(EditorUI* ui, const std::string& name) {
        auto selectedObjs = ui->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() < 2) {
            Notification::create("Select at least 2 objects (1 base + decorations)!", NotificationIcon::Warning)->show();
            return;
        }

        // First selected object is the "base" - others are decorations relative to it
        auto baseObj = static_cast<GameObject*>(selectedObjs->objectAtIndex(0));
        CCPoint basePos = baseObj->getPosition();

        DecoTemplate templ;
        templ.name = name;

        // Store all other objects relative to base
        for (int i = 1; i < selectedObjs->count(); i++) {
            auto obj = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (!obj) continue;

            float relX = obj->getPosition().x - basePos.x;
            float relY = obj->getPosition().y - basePos.y;
            float scale = obj->getScale();
            float rotation = obj->getRotation();
            int objID = obj->m_objectID;

            templ.objects.push_back({objID, relX, relY, scale, rotation});
        }

        g_templates.push_back(templ);
        g_selectedTemplate = g_templates.size() - 1;

        Notification::create(
            fmt::format("Template '{}' saved! ({} decorations)", name, templ.objects.size()).c_str(),
            NotificationIcon::Success
        )->show();
    }

    static void applyTemplate(EditorUI* ui, int templateIndex) {
        if (templateIndex < 0 || templateIndex >= (int)g_templates.size()) {
            Notification::create("No template selected!", NotificationIcon::Warning)->show();
            return;
        }

        auto lel = LevelEditorLayer::get();
        auto selectedObjs = ui->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() == 0) {
            Notification::create("Select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        auto& templ = g_templates[templateIndex];
        int created = 0;

        for (int i = 0; i < selectedObjs->count(); i++) {
            auto baseObj = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (!baseObj) continue;

            CCPoint basePos = baseObj->getPosition();

            for (auto& [objID, relX, relY, scale, rotation] : templ.objects) {
                CCPoint newPos = {basePos.x + relX, basePos.y + relY};
                auto newObj = lel->createObject(objID, newPos, false);
                if (newObj) {
                    newObj->setScale(scale);
                    newObj->setRotation(rotation);
                    lel->addSpecial(newObj);
                    created++;
                }
            }
        }

        ui->deselectAll();
        Notification::create(
            fmt::format("Applied '{}' - {} decorations added!", templ.name, created).c_str(),
            NotificationIcon::Success
        )->show();
    }
};

// ==========================================
// DECORATION LOGIC
// ==========================================
class AutoDecorator {
public:
    static void applyDecoration(EditorUI* ui) {
        if (g_currentStyle == DecoStyle::CustomTemplate) {
            TemplateManager::applyTemplate(ui, g_selectedTemplate);
            return;
        }

        auto lel = LevelEditorLayer::get();
        auto selectedObjs = ui->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() == 0) {
            Notification::create("Select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        int created = 0;
        for (int i = 0; i < selectedObjs->count(); i++) {
            auto obj = static_cast<GameObject*>(selectedObjs->objectAtIndex(i));
            if (!obj) continue;

            CCPoint pos = obj->getPosition();

            switch (g_currentStyle) {
                case DecoStyle::Glow:
                    created += addGlow(lel, pos, obj);
                    break;
                case DecoStyle::Outline:
                    created += addOutline(lel, pos, obj);
                    break;
                case DecoStyle::Shadow:
                    created += addShadow(lel, pos, obj);
                    break;
                case DecoStyle::Neon:
                    created += addNeon(lel, pos, obj);
                    break;
                case DecoStyle::Gradient:
                    created += addGradient(lel, pos, obj);
                    break;
                default:
                    break;
            }
        }

        ui->deselectAll();
        Notification::create(fmt::format("{} decorations added!", created).c_str(), NotificationIcon::Success)->show();
    }

private:
    static GameObject* create(LevelEditorLayer* lel, int id, CCPoint pos, float scale = 1.0f, float rotation = 0.0f) {
        auto obj = lel->createObject(id, pos, false);
        if (obj) {
            obj->setScale(scale);
            obj->setRotation(rotation);
            lel->addSpecial(obj);
        }
        return obj;
    }

    static int addGlow(LevelEditorLayer* lel, CCPoint pos, GameObject* base) {
        // Single large glow behind
        auto glow = create(lel, DecoIDs::GLOW_SOFT, pos, 1.5f);
        if (glow) {
            glow->setZOrder(base->getZOrder() - 1);
            return 1;
        }
        return 0;
    }

    static int addOutline(LevelEditorLayer* lel, CCPoint pos, GameObject* base) {
        int count = 0;
        float offset = 15.0f;
        
        // 4 lines around the block
        if (create(lel, DecoIDs::THIN_LINE, {pos.x, pos.y + offset}, 0.5f, 0)) count++;
        if (create(lel, DecoIDs::THIN_LINE, {pos.x, pos.y - offset}, 0.5f, 0)) count++;
        if (create(lel, DecoIDs::THIN_LINE, {pos.x - offset, pos.y}, 0.5f, 90)) count++;
        if (create(lel, DecoIDs::THIN_LINE, {pos.x + offset, pos.y}, 0.5f, 90)) count++;
        
        return count;
    }

    static int addShadow(LevelEditorLayer* lel, CCPoint pos, GameObject* base) {
        // Shadow offset down-right
        auto shadow = create(lel, DecoIDs::GLOW_GRADIENT, {pos.x + 8, pos.y - 8}, 1.2f);
        if (shadow) {
            shadow->setZOrder(base->getZOrder() - 2);
            return 1;
        }
        return 0;
    }

    static int addNeon(LevelEditorLayer* lel, CCPoint pos, GameObject* base) {
        int count = 0;
        
        // Multiple glows for neon effect
        if (create(lel, DecoIDs::GLOW_SQUARE_FULL, pos, 1.3f)) count++;
        if (create(lel, DecoIDs::GLOW_SOFT, pos, 1.6f)) count++;
        
        return count;
    }

    static int addGradient(LevelEditorLayer* lel, CCPoint pos, GameObject* base) {
        int count = 0;
        
        // Gradient glows at different positions
        if (create(lel, DecoIDs::GLOW_GRADIENT, {pos.x, pos.y + 20}, 0.8f)) count++;
        if (create(lel, DecoIDs::GLOW_GRADIENT, {pos.x, pos.y - 20}, 0.8f, 180)) count++;
        
        return count;
    }
};

// ==========================================
// TEMPLATE NAME INPUT POPUP
// ==========================================
class TemplateNamePopup : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_editorUI;
    TextInput* m_input;

    bool setup(EditorUI* ui) override {
        m_editorUI = ui;
        setTitle("Save Template");

        auto center = m_mainLayer->getContentSize() / 2;

        auto label = CCLabelBMFont::create("Enter template name:", "chatFont.fnt");
        label->setPosition({center.width, center.height + 30});
        label->setScale(0.7f);
        m_mainLayer->addChild(label);

        m_input = TextInput::create(150, "Template Name");
        m_input->setPosition({center.width, center.height});
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 80, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.6f),
            this, menu_selector(TemplateNamePopup::onSave)
        );
        saveBtn->setPosition({center.width, center.height - 40});
        menu->addChild(saveBtn);

        return true;
    }

    void onSave(CCObject*) {
        std::string name = m_input->getString();
        if (name.empty()) name = fmt::format("Template {}", g_templates.size() + 1);
        
        TemplateManager::saveSelectionAsTemplate(m_editorUI, name);
        this->onClose(nullptr);
    }

public:
    static TemplateNamePopup* create(EditorUI* ui) {
        auto ret = new TemplateNamePopup();
        if (ret && ret->initAnchored(250.f, 140.f, ui)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// MAIN MENU
// ==========================================
class AutoDecoMenu : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_editorUI;

    bool setup(EditorUI* ui) override {
        m_editorUI = ui;
        setTitle("AutoDecoration");

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto center = m_mainLayer->getContentSize() / 2;
        float startY = center.height + 60;
        float spacing = 32.0f;

        // Style buttons (2 rows)
        createStyleBtn("Glow", DecoStyle::Glow, {center.width - 60, startY}, menu);
        createStyleBtn("Outline", DecoStyle::Outline, {center.width, startY}, menu);
        createStyleBtn("Shadow", DecoStyle::Shadow, {center.width + 60, startY}, menu);
        
        createStyleBtn("Neon", DecoStyle::Neon, {center.width - 60, startY - spacing}, menu);
        createStyleBtn("Gradient", DecoStyle::Gradient, {center.width, startY - spacing}, menu);

        // Template button
        if (!g_templates.empty()) {
            auto templBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create(
                    fmt::format("T:{}", g_templates[g_selectedTemplate >= 0 ? g_selectedTemplate : 0].name.substr(0, 6)).c_str(),
                    50, true, "bigFont.fnt",
                    g_currentStyle == DecoStyle::CustomTemplate ? "GJ_button_02.png" : "GJ_button_04.png",
                    25, 0.4f
                ),
                this, menu_selector(AutoDecoMenu::onSelectTemplate)
            );
            templBtn->setPosition({center.width + 60, startY - spacing});
            menu->addChild(templBtn);
        }

        // Save Template button
        auto saveTemplBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE AS TEMPLATE", 140, true, "bigFont.fnt", "GJ_button_05.png", 25, 0.45f),
            this, menu_selector(AutoDecoMenu::onSaveTemplate)
        );
        saveTemplBtn->setPosition({center.width, startY - spacing * 2.2f});
        menu->addChild(saveTemplBtn);

        // Apply button
        auto applyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY", 100, true, "bigFont.fnt", "GJ_button_01.png", 35, 0.6f),
            this, menu_selector(AutoDecoMenu::onApply)
        );
        applyBtn->setPosition({center.width, 35});
        menu->addChild(applyBtn);

        return true;
    }

    void createStyleBtn(const char* name, DecoStyle style, CCPoint pos, CCMenu* menu) {
        bool selected = (g_currentStyle == style);
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(name, 55, true, "bigFont.fnt", 
                selected ? "GJ_button_02.png" : "GJ_button_04.png", 25, 0.4f),
            this, menu_selector(AutoDecoMenu::onStyle)
        );
        btn->setTag(static_cast<int>(style));
        btn->setPosition(pos);
        menu->addChild(btn);
    }

    void onStyle(CCObject* sender) {
        g_currentStyle = static_cast<DecoStyle>(static_cast<CCNode*>(sender)->getTag());
        this->onClose(nullptr);
        AutoDecoMenu::create(m_editorUI)->show();
    }

    void onSelectTemplate(CCObject*) {
        g_currentStyle = DecoStyle::CustomTemplate;
        // Cycle through templates
        if (!g_templates.empty()) {
            g_selectedTemplate = (g_selectedTemplate + 1) % g_templates.size();
        }
        this->onClose(nullptr);
        AutoDecoMenu::create(m_editorUI)->show();
    }

    void onSaveTemplate(CCObject*) {
        this->onClose(nullptr);
        TemplateNamePopup::create(m_editorUI)->show();
    }

    void onApply(CCObject*) {
        AutoDecorator::applyDecoration(m_editorUI);
        this->onClose(nullptr);
    }

public:
    static AutoDecoMenu* create(EditorUI* ui) {
        auto ret = new AutoDecoMenu();
        if (ret && ret->initAnchored(280.f, 200.f, ui)) {
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

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        sprite->setScale(0.7f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onAutoDecoBtn));
        btn->setPosition({winSize.width - 85, winSize.height - 25});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.3f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v2.0 with Templates loaded!");
        return true;
    }

    void onAutoDecoBtn(CCObject*) {
        AutoDecoMenu::create(this)->show();
    }
};
