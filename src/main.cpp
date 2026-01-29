#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

// ==========================================
// TEMPLATE SYSTEM - Stores PATTERN not copies
// ==========================================
struct DecoPattern {
    int objectID;
    float relativeX;      // Position relative to base block
    float relativeY;
    float scale;
    float rotation;
    int mainColorChannel; // Color channel for main color
    int secColorChannel;  // Color channel for secondary color
    int zOrder;           // Z layer relative to base
    bool blending;        // Blending mode
};

struct Template {
    std::string name;
    std::vector<DecoPattern> patterns;
};

static std::vector<Template> g_templates;
static int g_selectedTemplate = 0;

// ==========================================
// BUILT-IN STYLES (Don't need saving)
// ==========================================
enum class BuiltInStyle { None, Glow, Outline, Shadow };
static BuiltInStyle g_builtInStyle = BuiltInStyle::None;

// ==========================================
// TEMPLATE MANAGER
// ==========================================
class TemplateSystem {
public:
    // Save selected decorations as a reusable pattern
    static bool saveAsTemplate(EditorUI* ui, const std::string& name) {
        auto selected = ui->getSelectedObjects();
        if (!selected || selected->count() < 2) {
            Notification::create("Select: 1 BASE block + decorations around it!", NotificationIcon::Warning)->show();
            return false;
        }

        // First object = BASE (the main block)
        auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(0));
        CCPoint basePos = baseObj->getPosition();

        Template templ;
        templ.name = name;

        // Remaining objects = decorations (store their pattern relative to base)
        for (int i = 1; i < selected->count(); i++) {
            auto obj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!obj) continue;

            DecoPattern pattern;
            pattern.objectID = obj->m_objectID;
            pattern.relativeX = obj->getPosition().x - basePos.x;
            pattern.relativeY = obj->getPosition().y - basePos.y;
            pattern.scale = obj->getScale();
            pattern.rotation = obj->getRotation();
            pattern.zOrder = obj->getZOrder() - baseObj->getZOrder();
            pattern.blending = obj->m_hasNoGlow; // Blending setting
            
            // Capture color channels (THIS IS THE KEY!)
            pattern.mainColorChannel = obj->m_baseColorID;
            pattern.secColorChannel = obj->m_detailColorID;

            templ.patterns.push_back(pattern);
        }

        g_templates.push_back(templ);
        g_selectedTemplate = g_templates.size() - 1;

        Notification::create(
            fmt::format("'{}' saved! {} deco patterns", name, templ.patterns.size()).c_str(),
            NotificationIcon::Success
        )->show();
        return true;
    }

    // Apply template pattern to selected blocks
    static void applyTemplate(EditorUI* ui) {
        if (g_templates.empty() || g_selectedTemplate < 0) {
            Notification::create("No template saved! Save one first.", NotificationIcon::Warning)->show();
            return;
        }

        auto lel = LevelEditorLayer::get();
        auto selected = ui->getSelectedObjects();
        if (!selected || selected->count() == 0) {
            Notification::create("Select blocks to decorate!", NotificationIcon::Warning)->show();
            return;
        }

        auto& templ = g_templates[g_selectedTemplate];
        int created = 0;

        // For each selected block, apply the pattern
        for (int i = 0; i < selected->count(); i++) {
            auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!baseObj) continue;

            CCPoint basePos = baseObj->getPosition();
            int baseZ = baseObj->getZOrder();

            // Create each decoration from the pattern
            for (auto& pattern : templ.patterns) {
                CCPoint newPos = {
                    basePos.x + pattern.relativeX,
                    basePos.y + pattern.relativeY
                };

                // Create new object
                auto newObj = lel->createObject(pattern.objectID, newPos, false);
                if (newObj) {
                    newObj->setScale(pattern.scale);
                    newObj->setRotation(pattern.rotation);
                    newObj->setZOrder(baseZ + pattern.zOrder);
                    
                    // Apply color channels from pattern!
                    newObj->m_baseColorID = pattern.mainColorChannel;
                    newObj->m_detailColorID = pattern.secColorChannel;
                    newObj->m_hasNoGlow = pattern.blending;
                    
                    // Update the object's appearance
                    newObj->updateObjectEditorColor();
                    
                    lel->addSpecial(newObj);
                    created++;
                }
            }
        }

        ui->deselectAll();
        Notification::create(
            fmt::format("Applied '{}': {} decorations!", templ.name, created).c_str(),
            NotificationIcon::Success
        )->show();
    }

    // Cycle through saved templates
    static void nextTemplate() {
        if (g_templates.empty()) return;
        g_selectedTemplate = (g_selectedTemplate + 1) % g_templates.size();
    }

    static std::string getCurrentTemplateName() {
        if (g_templates.empty()) return "None";
        return g_templates[g_selectedTemplate].name;
    }
};

// ==========================================
// SIMPLE BUILT-IN DECORATIONS
// ==========================================
class SimpleDecorator {
public:
    static void apply(EditorUI* ui) {
        auto lel = LevelEditorLayer::get();
        auto selected = ui->getSelectedObjects();
        if (!selected || selected->count() == 0) {
            Notification::create("Select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        int created = 0;
        for (int i = 0; i < selected->count(); i++) {
            auto obj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!obj) continue;

            CCPoint pos = obj->getPosition();
            int baseZ = obj->getZOrder();

            switch (g_builtInStyle) {
                case BuiltInStyle::Glow:
                    created += createGlow(lel, pos, baseZ);
                    break;
                case BuiltInStyle::Outline:
                    created += createOutline(lel, pos, baseZ);
                    break;
                case BuiltInStyle::Shadow:
                    created += createShadow(lel, pos, baseZ);
                    break;
                default:
                    break;
            }
        }

        ui->deselectAll();
        if (created > 0) {
            Notification::create(fmt::format("{} decorations added!", created).c_str(), NotificationIcon::Success)->show();
        }
    }

private:
    static int createGlow(LevelEditorLayer* lel, CCPoint pos, int baseZ) {
        // Object 1736 is a soft glow square
        auto obj = lel->createObject(1736, pos, false);
        if (obj) {
            obj->setScale(1.5f);
            obj->setZOrder(baseZ - 1);
            lel->addSpecial(obj);
            return 1;
        }
        return 0;
    }

    static int createOutline(LevelEditorLayer* lel, CCPoint pos, int baseZ) {
        int count = 0;
        float offset = 16.0f;
        
        // Object 1764 is outline glow
        std::vector<CCPoint> offsets = {
            {0, offset}, {0, -offset}, {-offset, 0}, {offset, 0}
        };
        
        for (auto& off : offsets) {
            auto obj = lel->createObject(1764, {pos.x + off.x, pos.y + off.y}, false);
            if (obj) {
                obj->setScale(0.5f);
                obj->setZOrder(baseZ + 1);
                lel->addSpecial(obj);
                count++;
            }
        }
        return count;
    }

    static int createShadow(LevelEditorLayer* lel, CCPoint pos, int baseZ) {
        auto obj = lel->createObject(1732, {pos.x + 5, pos.y - 5}, false);
        if (obj) {
            obj->setScale(1.2f);
            obj->setZOrder(baseZ - 2);
            lel->addSpecial(obj);
            return 1;
        }
        return 0;
    }
};

// ==========================================
// NAME INPUT POPUP
// ==========================================
class SaveTemplatePopup : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_ui;
    TextInput* m_nameInput;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("Save Template");

        auto center = m_mainLayer->getContentSize() / 2;

        auto infoLabel = CCLabelBMFont::create(
            "Select BASE block first,\nthen decorations around it.",
            "chatFont.fnt"
        );
        infoLabel->setPosition({center.width, center.height + 35});
        infoLabel->setScale(0.55f);
        infoLabel->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(infoLabel);

        m_nameInput = TextInput::create(140, "Template Name");
        m_nameInput->setPosition({center.width, center.height});
        m_mainLayer->addChild(m_nameInput);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 70, true, "bigFont.fnt", "GJ_button_01.png", 28, 0.6f),
            this, menu_selector(SaveTemplatePopup::onSave)
        );
        saveBtn->setPosition({center.width, center.height - 40});
        menu->addChild(saveBtn);

        return true;
    }

    void onSave(CCObject*) {
        std::string name = m_nameInput->getString();
        if (name.empty()) {
            name = fmt::format("Style {}", g_templates.size() + 1);
        }
        TemplateSystem::saveAsTemplate(m_ui, name);
        this->onClose(nullptr);
    }

public:
    static SaveTemplatePopup* create(EditorUI* ui) {
        auto ret = new SaveTemplatePopup();
        if (ret && ret->initAnchored(240.f, 150.f, ui)) {
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
class MainMenu : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_ui;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("AutoDecoration");

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto center = m_mainLayer->getContentSize() / 2;

        // === BUILT-IN STYLES ===
        auto styleLabel = CCLabelBMFont::create("Quick Styles:", "goldFont.fnt");
        styleLabel->setPosition({center.width, center.height + 65});
        styleLabel->setScale(0.5f);
        m_mainLayer->addChild(styleLabel);

        createStyleBtn("Glow", BuiltInStyle::Glow, {center.width - 50, center.height + 40}, menu);
        createStyleBtn("Outline", BuiltInStyle::Outline, {center.width, center.height + 40}, menu);
        createStyleBtn("Shadow", BuiltInStyle::Shadow, {center.width + 50, center.height + 40}, menu);

        // === TEMPLATES SECTION ===
        auto templLabel = CCLabelBMFont::create("Custom Templates:", "goldFont.fnt");
        templLabel->setPosition({center.width, center.height + 10});
        templLabel->setScale(0.5f);
        m_mainLayer->addChild(templLabel);

        // Current template display
        std::string templName = g_templates.empty() ? "(none saved)" : g_templates[g_selectedTemplate].name;
        auto currentLabel = CCLabelBMFont::create(templName.c_str(), "bigFont.fnt");
        currentLabel->setPosition({center.width, center.height - 15});
        currentLabel->setScale(0.4f);
        currentLabel->setColor({100, 255, 100});
        m_mainLayer->addChild(currentLabel);

        // Template buttons
        if (!g_templates.empty()) {
            auto prevBtn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onPrevTemplate)
            );
            prevBtn->setPosition({center.width - 80, center.height - 15});
            prevBtn->setScale(0.5f);
            menu->addChild(prevBtn);

            auto nextBtn = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onNextTemplate)
            );
            nextBtn->setPosition({center.width + 80, center.height - 15});
            nextBtn->setScale(0.5f);
            nextBtn->setScaleX(-0.5f);
            menu->addChild(nextBtn);
        }

        // Apply Template button
        auto applyTemplBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY TEMPLATE", 110, true, "bigFont.fnt", 
                g_templates.empty() ? "GJ_button_04.png" : "GJ_button_02.png", 25, 0.45f),
            this, menu_selector(MainMenu::onApplyTemplate)
        );
        applyTemplBtn->setPosition({center.width, center.height - 45});
        menu->addChild(applyTemplBtn);

        // Save Template button
        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE NEW TEMPLATE", 115, true, "bigFont.fnt", "GJ_button_05.png", 25, 0.45f),
            this, menu_selector(MainMenu::onSaveTemplate)
        );
        saveBtn->setPosition({center.width, center.height - 75});
        menu->addChild(saveBtn);

        return true;
    }

    void createStyleBtn(const char* name, BuiltInStyle style, CCPoint pos, CCMenu* menu) {
        bool sel = (g_builtInStyle == style);
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(name, 45, true, "bigFont.fnt", 
                sel ? "GJ_button_02.png" : "GJ_button_04.png", 22, 0.4f),
            this, menu_selector(MainMenu::onQuickStyle)
        );
        btn->setTag(static_cast<int>(style));
        btn->setPosition(pos);
        menu->addChild(btn);
    }

    void onQuickStyle(CCObject* sender) {
        g_builtInStyle = static_cast<BuiltInStyle>(static_cast<CCNode*>(sender)->getTag());
        SimpleDecorator::apply(m_ui);
        this->onClose(nullptr);
    }

    void onPrevTemplate(CCObject*) {
        if (g_templates.empty()) return;
        g_selectedTemplate = (g_selectedTemplate - 1 + g_templates.size()) % g_templates.size();
        this->onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onNextTemplate(CCObject*) {
        TemplateSystem::nextTemplate();
        this->onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onApplyTemplate(CCObject*) {
        TemplateSystem::applyTemplate(m_ui);
        this->onClose(nullptr);
    }

    void onSaveTemplate(CCObject*) {
        this->onClose(nullptr);
        SaveTemplatePopup::create(m_ui)->show();
    }

public:
    static MainMenu* create(EditorUI* ui) {
        auto ret = new MainMenu();
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
        sprite->setScale(0.65f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onOpenMenu));
        btn->setPosition({winSize.width - 80, winSize.height - 23});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.28f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v3.0 - Template System loaded!");
        return true;
    }

    void onOpenMenu(CCObject*) {
        MainMenu::create(this)->show();
    }
};
