#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

// ==========================================
// TEMPLATE SYSTEM
// ==========================================
struct DecoPattern {
    int objectID;
    float relativeX;
    float relativeY;
    float scale;
    float rotation;
    int zOrderOffset;
};

struct Template {
    std::string name;
    std::vector<DecoPattern> patterns;
};

static std::vector<Template> g_templates;
static int g_selectedTemplate = 0;

// Built-in styles
enum class BuiltInStyle { None, Glow, Outline, Shadow };
static BuiltInStyle g_builtInStyle = BuiltInStyle::None;

// ==========================================
// TEMPLATE MANAGER
// ==========================================
class TemplateSystem {
public:
    static bool saveAsTemplate(EditorUI* ui, const std::string& name) {
        auto selected = ui->getSelectedObjects();
        if (!selected || selected->count() < 2) {
            Notification::create("Select: 1 BASE + decorations!", NotificationIcon::Warning)->show();
            return false;
        }

        auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(0));
        CCPoint basePos = baseObj->getPosition();
        int baseZ = baseObj->getZOrder();

        Template templ;
        templ.name = name;

        for (int i = 1; i < selected->count(); i++) {
            auto obj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!obj) continue;

            DecoPattern pattern;
            pattern.objectID = obj->m_objectID;
            pattern.relativeX = obj->getPosition().x - basePos.x;
            pattern.relativeY = obj->getPosition().y - basePos.y;
            pattern.scale = obj->getScale();
            pattern.rotation = obj->getRotation();
            pattern.zOrderOffset = obj->getZOrder() - baseZ;

            templ.patterns.push_back(pattern);
        }

        g_templates.push_back(templ);
        g_selectedTemplate = g_templates.size() - 1;

        Notification::create(
            fmt::format("'{}' saved! {} patterns", name, templ.patterns.size()).c_str(),
            NotificationIcon::Success
        )->show();
        return true;
    }

    static void applyTemplate(EditorUI* ui) {
        if (g_templates.empty() || g_selectedTemplate < 0) {
            Notification::create("No template! Save one first.", NotificationIcon::Warning)->show();
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

        for (int i = 0; i < selected->count(); i++) {
            auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!baseObj) continue;

            CCPoint basePos = baseObj->getPosition();
            int baseZ = baseObj->getZOrder();

            for (auto& pattern : templ.patterns) {
                CCPoint newPos = {
                    basePos.x + pattern.relativeX,
                    basePos.y + pattern.relativeY
                };

                auto newObj = lel->createObject(pattern.objectID, newPos, false);
                if (newObj) {
                    newObj->setScale(pattern.scale);
                    newObj->setRotation(pattern.rotation);
                    newObj->setZOrder(baseZ + pattern.zOrderOffset);
                    lel->addSpecial(newObj);
                    created++;
                }
            }
        }

        ui->deselectAll();
        Notification::create(
            fmt::format("Applied '{}': {} decos!", templ.name, created).c_str(),
            NotificationIcon::Success
        )->show();
    }

    static void nextTemplate() {
        if (g_templates.empty()) return;
        g_selectedTemplate = (g_selectedTemplate + 1) % g_templates.size();
    }

    static void prevTemplate() {
        if (g_templates.empty()) return;
        g_selectedTemplate = (g_selectedTemplate - 1 + g_templates.size()) % g_templates.size();
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
            Notification::create("Select blocks!", NotificationIcon::Warning)->show();
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
            Notification::create(fmt::format("{} decos added!", created).c_str(), NotificationIcon::Success)->show();
        }
    }

private:
    static int createGlow(LevelEditorLayer* lel, CCPoint pos, int baseZ) {
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
// SAVE POPUP
// ==========================================
class SavePopup : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_ui;
    TextInput* m_input;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("Save Template");

        auto center = m_mainLayer->getContentSize() / 2;

        auto info = CCLabelBMFont::create("Select BASE first, then decos", "chatFont.fnt");
        info->setPosition({center.width, center.height + 30});
        info->setScale(0.55f);
        m_mainLayer->addChild(info);

        m_input = TextInput::create(140, "Name");
        m_input->setPosition({center.width, center.height});
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 60, true, "bigFont.fnt", "GJ_button_01.png", 28, 0.6f),
            this, menu_selector(SavePopup::onSave)
        );
        btn->setPosition({center.width, center.height - 35});
        menu->addChild(btn);

        return true;
    }

    void onSave(CCObject*) {
        std::string name = m_input->getString();
        if (name.empty()) name = fmt::format("Style{}", g_templates.size() + 1);
        TemplateSystem::saveAsTemplate(m_ui, name);
        this->onClose(nullptr);
    }

public:
    static SavePopup* create(EditorUI* ui) {
        auto ret = new SavePopup();
        if (ret && ret->initAnchored(220.f, 130.f, ui)) {
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

        // Quick styles label
        auto label1 = CCLabelBMFont::create("Quick:", "goldFont.fnt");
        label1->setPosition({center.width - 70, center.height + 55});
        label1->setScale(0.45f);
        m_mainLayer->addChild(label1);

        createStyleBtn("Glow", BuiltInStyle::Glow, {center.width - 20, center.height + 55}, menu);
        createStyleBtn("Out", BuiltInStyle::Outline, {center.width + 25, center.height + 55}, menu);
        createStyleBtn("Shd", BuiltInStyle::Shadow, {center.width + 70, center.height + 55}, menu);

        // Templates section
        auto label2 = CCLabelBMFont::create("Templates:", "goldFont.fnt");
        label2->setPosition({center.width, center.height + 25});
        label2->setScale(0.45f);
        m_mainLayer->addChild(label2);

        // Template name
        std::string tName = g_templates.empty() ? "(none)" : g_templates[g_selectedTemplate].name;
        auto tLabel = CCLabelBMFont::create(tName.c_str(), "bigFont.fnt");
        tLabel->setPosition({center.width, center.height});
        tLabel->setScale(0.35f);
        tLabel->setColor({100, 255, 100});
        m_mainLayer->addChild(tLabel);

        // Arrows
        if (!g_templates.empty()) {
            auto leftArr = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onPrev)
            );
            leftArr->setPosition({center.width - 70, center.height});
            leftArr->setScale(0.4f);
            menu->addChild(leftArr);

            auto rightArr = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onNext)
            );
            rightArr->setPosition({center.width + 70, center.height});
            rightArr->setScale(0.4f);
            rightArr->setScaleX(-0.4f);
            menu->addChild(rightArr);
        }

        // Apply template
        auto applyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY", 80, true, "bigFont.fnt", 
                g_templates.empty() ? "GJ_button_04.png" : "GJ_button_02.png", 25, 0.5f),
            this, menu_selector(MainMenu::onApply)
        );
        applyBtn->setPosition({center.width, center.height - 30});
        menu->addChild(applyBtn);

        // Save new
        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE NEW", 85, true, "bigFont.fnt", "GJ_button_05.png", 25, 0.45f),
            this, menu_selector(MainMenu::onSave)
        );
        saveBtn->setPosition({center.width, center.height - 60});
        menu->addChild(saveBtn);

        return true;
    }

    void createStyleBtn(const char* name, BuiltInStyle style, CCPoint pos, CCMenu* menu) {
        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(name, 40, true, "bigFont.fnt", "GJ_button_04.png", 20, 0.4f),
            this, menu_selector(MainMenu::onStyle)
        );
        btn->setTag(static_cast<int>(style));
        btn->setPosition(pos);
        menu->addChild(btn);
    }

    void onStyle(CCObject* sender) {
        g_builtInStyle = static_cast<BuiltInStyle>(static_cast<CCNode*>(sender)->getTag());
        SimpleDecorator::apply(m_ui);
        this->onClose(nullptr);
    }

    void onPrev(CCObject*) {
        TemplateSystem::prevTemplate();
        this->onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onNext(CCObject*) {
        TemplateSystem::nextTemplate();
        this->onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onApply(CCObject*) {
        TemplateSystem::applyTemplate(m_ui);
        this->onClose(nullptr);
    }

    void onSave(CCObject*) {
        this->onClose(nullptr);
        SavePopup::create(m_ui)->show();
    }

public:
    static MainMenu* create(EditorUI* ui) {
        auto ret = new MainMenu();
        if (ret && ret->initAnchored(260.f, 170.f, ui)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// HOOK
// ==========================================
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        sprite->setScale(0.6f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onMenu));
        btn->setPosition({winSize.width - 75, winSize.height - 22});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.25f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v3.1 loaded!");
        return true;
    }

    void onMenu(CCObject*) {
        MainMenu::create(this)->show();
    }
};
