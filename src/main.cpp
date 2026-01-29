#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

// ==========================================
// TEMPLATE SYSTEM ONLY - No broken quick styles
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

// ==========================================
// TEMPLATE MANAGER
// ==========================================
class TemplateSystem {
public:
    static bool saveAsTemplate(EditorUI* ui, const std::string& name) {
        auto selected = ui->getSelectedObjects();
        if (!selected || selected->count() < 2) {
            Notification::create("Select: 1 BASE block + your decorations!", NotificationIcon::Warning)->show();
            return false;
        }

        // First selected = BASE block
        auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(0));
        CCPoint basePos = baseObj->getPosition();
        int baseZ = baseObj->getZOrder();

        Template templ;
        templ.name = name;

        // Rest = decorations (save relative to base)
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
            fmt::format("Saved '{}'! {} decorations", name, templ.patterns.size()).c_str(),
            NotificationIcon::Success
        )->show();
        return true;
    }

    static void applyTemplate(EditorUI* ui) {
        if (g_templates.empty()) {
            Notification::create("No templates! Create one first.", NotificationIcon::Warning)->show();
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
            fmt::format("Applied '{}': {} decorations!", templ.name, created).c_str(),
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

    static std::string current() {
        if (g_templates.empty()) return "(none)";
        return g_templates[g_selectedTemplate].name;
    }

    static int count() {
        return g_templates.size();
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

        auto info = CCLabelBMFont::create(
            "1. Select BASE block first\n"
            "2. Then select decorations\n"
            "3. Enter name and save!",
            "chatFont.fnt"
        );
        info->setPosition({center.width, center.height + 30});
        info->setScale(0.5f);
        info->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(info);

        m_input = TextInput::create(130, "Name");
        m_input->setPosition({center.width, center.height - 10});
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 60, true, "bigFont.fnt", "GJ_button_01.png", 28, 0.6f),
            this, menu_selector(SavePopup::onSave)
        );
        btn->setPosition({center.width, center.height - 45});
        menu->addChild(btn);

        return true;
    }

    void onSave(CCObject*) {
        std::string name = m_input->getString();
        if (name.empty()) name = fmt::format("Style{}", TemplateSystem::count() + 1);
        TemplateSystem::saveAsTemplate(m_ui, name);
        this->onClose(nullptr);
    }

public:
    static SavePopup* create(EditorUI* ui) {
        auto ret = new SavePopup();
        if (ret && ret->initAnchored(220.f, 150.f, ui)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// MAIN MENU - Templates only
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

        // Instructions
        auto info = CCLabelBMFont::create(
            "Create decoration manually,\nsave as template, apply to others!",
            "chatFont.fnt"
        );
        info->setPosition({center.width, center.height + 45});
        info->setScale(0.5f);
        info->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(info);

        // Current template
        auto tLabel = CCLabelBMFont::create("Template:", "goldFont.fnt");
        tLabel->setPosition({center.width, center.height + 15});
        tLabel->setScale(0.45f);
        m_mainLayer->addChild(tLabel);

        auto current = CCLabelBMFont::create(TemplateSystem::current().c_str(), "bigFont.fnt");
        current->setPosition({center.width, center.height - 5});
        current->setScale(0.4f);
        current->setColor({100, 255, 100});
        m_mainLayer->addChild(current);

        // Arrows (if templates exist)
        if (TemplateSystem::count() > 0) {
            auto leftArr = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onPrev)
            );
            leftArr->setPosition({center.width - 80, center.height - 5});
            leftArr->setScale(0.5f);
            menu->addChild(leftArr);

            auto rightArr = CCMenuItemSpriteExtra::create(
                CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
                this, menu_selector(MainMenu::onNext)
            );
            rightArr->setPosition({center.width + 80, center.height - 5});
            rightArr->setScale(0.5f);
            rightArr->setScaleX(-0.5f);
            menu->addChild(rightArr);
        }

        // APPLY button
        auto applyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY", 90, true, "bigFont.fnt", 
                TemplateSystem::count() > 0 ? "GJ_button_01.png" : "GJ_button_04.png", 30, 0.55f),
            this, menu_selector(MainMenu::onApply)
        );
        applyBtn->setPosition({center.width, center.height - 40});
        menu->addChild(applyBtn);

        // SAVE NEW button
        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE NEW", 90, true, "bigFont.fnt", "GJ_button_02.png", 30, 0.55f),
            this, menu_selector(MainMenu::onSave)
        );
        saveBtn->setPosition({center.width, center.height - 75});
        menu->addChild(saveBtn);

        return true;
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
        if (ret && ret->initAnchored(260.f, 180.f, ui)) {
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
        sprite->setScale(0.55f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onMenu));
        btn->setPosition({winSize.width - 70, winSize.height - 20});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.22f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v4.0 - Templates Only");
        return true;
    }

    void onMenu(CCObject*) {
        MainMenu::create(this)->show();
    }
};
