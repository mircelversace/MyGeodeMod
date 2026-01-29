#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

struct DecoItem {
    int objectID;
    float offsetX;
    float offsetY;
    float scale;
    float rotation;
};

struct DecoTemplate {
    std::string name;
    std::vector<DecoItem> items;
};

static std::vector<DecoTemplate> g_templates;
static int g_idx = 0;

void initTemplates() {
    if (!g_templates.empty()) return;

    // Glow
    g_templates.push_back({"Glow", {{1736, 0, 0, 1.3f, 0}}});
    
    // Corners
    g_templates.push_back({"Corners", {
        {1764, -12, -12, 0.3f, 0},
        {1764, 12, -12, 0.3f, 0},
        {1764, -12, 12, 0.3f, 0},
        {1764, 12, 12, 0.3f, 0}
    }});
    
    // Shadow
    g_templates.push_back({"Shadow", {{1732, 4, -4, 1.1f, 0}}});
    
    // DblGlow
    g_templates.push_back({"DblGlow", {
        {1736, 0, 0, 1.2f, 0},
        {1736, 0, 0, 1.6f, 0}
    }});
    
    // HLines
    g_templates.push_back({"HLines", {
        {579, 0, 15, 0.8f, 0},
        {579, 0, -15, 0.8f, 0}
    }});
    
    // VLines
    g_templates.push_back({"VLines", {
        {579, 15, 0, 0.8f, 90},
        {579, -15, 0, 0.8f, 90}
    }});
    
    // Frame
    g_templates.push_back({"Frame", {
        {579, 0, 15, 0.8f, 0},
        {579, 0, -15, 0.8f, 0},
        {579, 15, 0, 0.8f, 90},
        {579, -15, 0, 0.8f, 90}
    }});
    
    // BigBG
    g_templates.push_back({"BigBG", {{211, 0, 0, 1.5f, 0}}});
}

class DecoSystem {
public:
    static void apply(EditorUI* ui) {
        initTemplates();
        
        auto lel = LevelEditorLayer::get();
        auto sel = ui->getSelectedObjects();
        
        if (!sel || sel->count() == 0) {
            Notification::create("Select blocks!", NotificationIcon::Warning)->show();
            return;
        }

        auto& t = g_templates[g_idx];
        int n = 0;

        for (int i = 0; i < sel->count(); i++) {
            auto base = static_cast<GameObject*>(sel->objectAtIndex(i));
            if (!base) continue;

            CCPoint pos = base->getPosition();
            int z = base->getZOrder();

            for (auto& item : t.items) {
                auto obj = lel->createObject(item.objectID, {pos.x + item.offsetX, pos.y + item.offsetY}, false);
                if (obj) {
                    obj->setScale(item.scale);
                    obj->setRotation(item.rotation);
                    obj->setZOrder(z - 1);
                    lel->addSpecial(obj);
                    n++;
                }
            }
        }

        ui->deselectAll();
        Notification::create(fmt::format("{}: +{}", t.name, n).c_str(), NotificationIcon::Success)->show();
    }

    // NEW: Select ONLY decorations, calculates center automatically
    static void saveCustom(EditorUI* ui, const std::string& name) {
        auto sel = ui->getSelectedObjects();
        
        if (!sel || sel->count() == 0) {
            Notification::create("Select decorations only!", NotificationIcon::Warning)->show();
            return;
        }

        // Calculate center of all selected objects
        float sumX = 0, sumY = 0;
        for (int i = 0; i < sel->count(); i++) {
            auto obj = static_cast<GameObject*>(sel->objectAtIndex(i));
            if (obj) {
                sumX += obj->getPosition().x;
                sumY += obj->getPosition().y;
            }
        }
        float centerX = sumX / sel->count();
        float centerY = sumY / sel->count();

        DecoTemplate t;
        t.name = name;

        // Save all objects relative to calculated center
        for (int i = 0; i < sel->count(); i++) {
            auto obj = static_cast<GameObject*>(sel->objectAtIndex(i));
            if (!obj) continue;

            DecoItem item;
            item.objectID = obj->m_objectID;
            item.offsetX = obj->getPosition().x - centerX;
            item.offsetY = obj->getPosition().y - centerY;
            item.scale = obj->getScale();
            item.rotation = obj->getRotation();
            t.items.push_back(item);
        }

        g_templates.push_back(t);
        g_idx = g_templates.size() - 1;

        Notification::create(fmt::format("'{}' saved! {} items", name, t.items.size()).c_str(), NotificationIcon::Success)->show();
    }

    static void next() { initTemplates(); g_idx = (g_idx + 1) % g_templates.size(); }
    static void prev() { initTemplates(); g_idx = (g_idx - 1 + g_templates.size()) % g_templates.size(); }
    static std::string name() { initTemplates(); return g_templates[g_idx].name; }
    static int total() { initTemplates(); return g_templates.size(); }
};

class SavePopup : public geode::Popup<EditorUI*> {
    EditorUI* m_ui;
    TextInput* m_input;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("Save Style");

        auto c = m_mainLayer->getContentSize() / 2;

        auto info = CCLabelBMFont::create("Select ONLY decorations\n(NOT the block!)", "chatFont.fnt");
        info->setPosition({c.width, c.height + 20});
        info->setScale(0.55f);
        info->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(info);

        m_input = TextInput::create(100, "Name");
        m_input->setPosition({c.width, c.height - 10});
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto btn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 50, true, "bigFont.fnt", "GJ_button_01.png", 22, 0.6f),
            this, menu_selector(SavePopup::onSave)
        );
        btn->setPosition({c.width, c.height - 40});
        menu->addChild(btn);

        return true;
    }

    void onSave(CCObject*) {
        std::string n = m_input->getString();
        if (n.empty()) n = "Custom" + std::to_string(DecoSystem::total());
        DecoSystem::saveCustom(m_ui, n);
        onClose(nullptr);
    }

public:
    static SavePopup* create(EditorUI* ui) {
        auto ret = new SavePopup();
        if (ret && ret->initAnchored(180.f, 120.f, ui)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class MainMenu : public geode::Popup<EditorUI*> {
    EditorUI* m_ui;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("AutoDeco");

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto c = m_mainLayer->getContentSize() / 2;

        // Style name
        auto styleName = CCLabelBMFont::create(DecoSystem::name().c_str(), "bigFont.fnt");
        styleName->setPosition({c.width, c.height + 30});
        styleName->setScale(0.5f);
        styleName->setColor({100, 255, 100});
        m_mainLayer->addChild(styleName);

        // Arrows
        auto prev = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(MainMenu::onPrev)
        );
        prev->setPosition({c.width - 70, c.height + 30});
        prev->setScale(0.5f);
        menu->addChild(prev);

        auto next = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(MainMenu::onNext)
        );
        next->setPosition({c.width + 70, c.height + 30});
        next->setScale(-0.5f);
        menu->addChild(next);

        // Counter
        auto cnt = CCLabelBMFont::create(fmt::format("{}/{}", g_idx + 1, DecoSystem::total()).c_str(), "chatFont.fnt");
        cnt->setPosition({c.width, c.height + 8});
        cnt->setScale(0.5f);
        m_mainLayer->addChild(cnt);

        // Apply button
        auto applyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY", 80, true, "bigFont.fnt", "GJ_button_01.png", 28, 0.6f),
            this, menu_selector(MainMenu::onApply)
        );
        applyBtn->setPosition({c.width, c.height - 20});
        menu->addChild(applyBtn);

        // Save button
        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save New", 70, true, "bigFont.fnt", "GJ_button_05.png", 20, 0.5f),
            this, menu_selector(MainMenu::onSave)
        );
        saveBtn->setPosition({c.width, c.height - 50});
        menu->addChild(saveBtn);

        return true;
    }

    void onPrev(CCObject*) { DecoSystem::prev(); onClose(nullptr); MainMenu::create(m_ui)->show(); }
    void onNext(CCObject*) { DecoSystem::next(); onClose(nullptr); MainMenu::create(m_ui)->show(); }
    void onApply(CCObject*) { DecoSystem::apply(m_ui); onClose(nullptr); }
    void onSave(CCObject*) { onClose(nullptr); SavePopup::create(m_ui)->show(); }

public:
    static MainMenu* create(EditorUI* ui) {
        auto ret = new MainMenu();
        if (ret && ret->initAnchored(220.f, 130.f, ui)) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* el) {
        if (!EditorUI::init(el)) return false;
        initTemplates();

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto ws = CCDirector::sharedDirector()->getWinSize();

        auto spr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        spr->setScale(0.45f);
        
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyEditorUI::openMenu));
        btn->setPosition({ws.width - 60, ws.height - 16});
        menu->addChild(btn);

        auto lbl = CCLabelBMFont::create("AD", "bigFont.fnt");
        lbl->setScale(0.18f);
        lbl->setPosition(spr->getContentSize() / 2);
        spr->addChild(lbl);

        return true;
    }

    void openMenu(CCObject*) { MainMenu::create(this)->show(); }
};
