#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <vector>

using namespace geode::prelude;

// ==========================================
// TEMPLATE STRUCTURE
// ==========================================
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

static std::vector<DecoTemplate> g_savedTemplates;
static int g_currentIndex = 0;

// ==========================================
// PRE-MADE TEMPLATES (Built-in styles)
// ==========================================
void initBuiltInTemplates() {
    if (!g_savedTemplates.empty()) return;

    // Style 1: Simple Glow (centered behind)
    DecoTemplate glow;
    glow.name = "Glow";
    glow.items.push_back({1736, 0, 0, 1.3f, 0}); // Soft glow centered
    g_savedTemplates.push_back(glow);

    // Style 2: Corner Dots
    DecoTemplate corners;
    corners.name = "Corners";
    corners.items.push_back({1764, -12, -12, 0.3f, 0});
    corners.items.push_back({1764, 12, -12, 0.3f, 0});
    corners.items.push_back({1764, -12, 12, 0.3f, 0});
    corners.items.push_back({1764, 12, 12, 0.3f, 0});
    g_savedTemplates.push_back(corners);

    // Style 3: Shadow offset
    DecoTemplate shadow;
    shadow.name = "Shadow";
    shadow.items.push_back({1732, 4, -4, 1.1f, 0});
    g_savedTemplates.push_back(shadow);

    // Style 4: Double Glow
    DecoTemplate dblGlow;
    dblGlow.name = "DblGlow";
    dblGlow.items.push_back({1736, 0, 0, 1.2f, 0});
    dblGlow.items.push_back({1736, 0, 0, 1.6f, 0});
    g_savedTemplates.push_back(dblGlow);

    // Style 5: Horizontal lines
    DecoTemplate hLines;
    hLines.name = "HLines";
    hLines.items.push_back({579, 0, 15, 0.8f, 0});
    hLines.items.push_back({579, 0, -15, 0.8f, 0});
    g_savedTemplates.push_back(hLines);

    // Style 6: Vertical lines
    DecoTemplate vLines;
    vLines.name = "VLines";
    vLines.items.push_back({579, 15, 0, 0.8f, 90});
    vLines.items.push_back({579, -15, 0, 0.8f, 90});
    g_savedTemplates.push_back(vLines);

    // Style 7: X pattern
    DecoTemplate xPat;
    xPat.name = "XPattern";
    xPat.items.push_back({579, 0, 0, 0.6f, 45});
    xPat.items.push_back({579, 0, 0, 0.6f, -45});
    g_savedTemplates.push_back(xPat);

    // Style 8: Big Behind
    DecoTemplate bigBg;
    bigBg.name = "BigBG";
    bigBg.items.push_back({211, 0, 0, 1.5f, 0});
    g_savedTemplates.push_back(bigBg);
}

// ==========================================
// DECORATION LOGIC
// ==========================================
class DecoSystem {
public:
    static void applyCurrentTemplate(EditorUI* ui) {
        initBuiltInTemplates();
        
        if (g_savedTemplates.empty()) {
            Notification::create("No templates!", NotificationIcon::Warning)->show();
            return;
        }

        auto lel = LevelEditorLayer::get();
        auto selected = ui->getSelectedObjects();
        
        if (!selected || selected->count() == 0) {
            Notification::create("Select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        auto& templ = g_savedTemplates[g_currentIndex];
        int count = 0;

        // Apply template to EACH selected object
        for (int i = 0; i < selected->count(); i++) {
            auto baseObj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!baseObj) continue;

            CCPoint basePos = baseObj->getPosition();
            int baseZ = baseObj->getZOrder();

            // Create each decoration item
            for (auto& item : templ.items) {
                CCPoint newPos = {basePos.x + item.offsetX, basePos.y + item.offsetY};
                
                auto newObj = lel->createObject(item.objectID, newPos, false);
                if (newObj) {
                    newObj->setScale(item.scale);
                    newObj->setRotation(item.rotation);
                    newObj->setZOrder(baseZ - 1); // Behind the block
                    lel->addSpecial(newObj);
                    count++;
                }
            }
        }

        ui->deselectAll();
        Notification::create(
            fmt::format("{}: {} decorations!", templ.name, count).c_str(),
            NotificationIcon::Success
        )->show();
    }

    static void saveSelectionAsTemplate(EditorUI* ui, const std::string& name) {
        auto selected = ui->getSelectedObjects();
        
        if (!selected || selected->count() < 2) {
            Notification::create("Select 1 block + decorations!", NotificationIcon::Warning)->show();
            return;
        }

        // FIRST = reference block (we use its position but DON'T copy it)
        auto refBlock = static_cast<GameObject*>(selected->objectAtIndex(0));
        CCPoint refPos = refBlock->getPosition();

        DecoTemplate newTempl;
        newTempl.name = name;

        // REST = decorations to save (starting from index 1)
        for (int i = 1; i < selected->count(); i++) {
            auto obj = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!obj) continue;

            DecoItem item;
            item.objectID = obj->m_objectID;
            item.offsetX = obj->getPosition().x - refPos.x;
            item.offsetY = obj->getPosition().y - refPos.y;
            item.scale = obj->getScale();
            item.rotation = obj->getRotation();

            newTempl.items.push_back(item);
        }

        if (newTempl.items.empty()) {
            Notification::create("No decorations found!", NotificationIcon::Error)->show();
            return;
        }

        g_savedTemplates.push_back(newTempl);
        g_currentIndex = g_savedTemplates.size() - 1;

        Notification::create(
            fmt::format("'{}' saved ({} items)", name, newTempl.items.size()).c_str(),
            NotificationIcon::Success
        )->show();
    }

    static void next() {
        initBuiltInTemplates();
        if (g_savedTemplates.empty()) return;
        g_currentIndex = (g_currentIndex + 1) % g_savedTemplates.size();
    }

    static void prev() {
        initBuiltInTemplates();
        if (g_savedTemplates.empty()) return;
        g_currentIndex = (g_currentIndex - 1 + g_savedTemplates.size()) % g_savedTemplates.size();
    }

    static std::string currentName() {
        initBuiltInTemplates();
        if (g_savedTemplates.empty()) return "None";
        return g_savedTemplates[g_currentIndex].name;
    }

    static int total() {
        initBuiltInTemplates();
        return g_savedTemplates.size();
    }
};

// ==========================================
// SAVE POPUP
// ==========================================
class SavePopup : public geode::Popup<EditorUI*> {
    EditorUI* m_ui;
    TextInput* m_input;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("Save Custom Style");

        auto c = m_mainLayer->getContentSize() / 2;

        auto info = CCLabelBMFont::create(
            "Select BLOCK first,\nthen DECORATIONS around it",
            "chatFont.fnt"
        );
        info->setPosition({c.width, c.height + 25});
        info->setScale(0.55f);
        info->setAlignment(kCCTextAlignmentCenter);
        m_mainLayer->addChild(info);

        m_input = TextInput::create(120, "Style Name");
        m_input->setPosition({c.width, c.height - 5});
        m_mainLayer->addChild(m_input);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAVE", 60, true, "bigFont.fnt", "GJ_button_01.png", 25, 0.6f),
            this, menu_selector(SavePopup::onSave)
        );
        saveBtn->setPosition({c.width, c.height - 40});
        menu->addChild(saveBtn);

        return true;
    }

    void onSave(CCObject*) {
        std::string name = m_input->getString();
        if (name.empty()) name = "Custom" + std::to_string(DecoSystem::total());
        DecoSystem::saveSelectionAsTemplate(m_ui, name);
        onClose(nullptr);
    }

public:
    static SavePopup* create(EditorUI* ui) {
        auto ret = new SavePopup();
        if (ret && ret->initAnchored(200.f, 130.f, ui)) {
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
    EditorUI* m_ui;

    bool setup(EditorUI* ui) override {
        m_ui = ui;
        setTitle("AutoDecoration");

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        auto c = m_mainLayer->getContentSize() / 2;

        // Style selector
        auto styleLabel = CCLabelBMFont::create("Style:", "goldFont.fnt");
        styleLabel->setPosition({c.width - 50, c.height + 35});
        styleLabel->setScale(0.5f);
        m_mainLayer->addChild(styleLabel);

        auto styleName = CCLabelBMFont::create(DecoSystem::currentName().c_str(), "bigFont.fnt");
        styleName->setPosition({c.width + 20, c.height + 35});
        styleName->setScale(0.45f);
        styleName->setColor({100, 255, 100});
        m_mainLayer->addChild(styleName);

        // Navigation arrows
        auto prevBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(MainMenu::onPrev)
        );
        prevBtn->setPosition({c.width - 90, c.height + 35});
        prevBtn->setScale(0.5f);
        menu->addChild(prevBtn);

        auto nextBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"),
            this, menu_selector(MainMenu::onNext)
        );
        nextBtn->setPosition({c.width + 90, c.height + 35});
        nextBtn->setScale(-0.5f);
        menu->addChild(nextBtn);

        // Counter
        auto counter = CCLabelBMFont::create(
            fmt::format("{}/{}", g_currentIndex + 1, DecoSystem::total()).c_str(),
            "chatFont.fnt"
        );
        counter->setPosition({c.width, c.height + 10});
        counter->setScale(0.6f);
        m_mainLayer->addChild(counter);

        // APPLY button (big, green)
        auto applyBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("APPLY TO SELECTION", 150, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.5f),
            this, menu_selector(MainMenu::onApply)
        );
        applyBtn->setPosition({c.width, c.height - 25});
        menu->addChild(applyBtn);

        // Save custom button
        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save Custom Style", 130, true, "bigFont.fnt", "GJ_button_05.png", 22, 0.45f),
            this, menu_selector(MainMenu::onSaveCustom)
        );
        saveBtn->setPosition({c.width, c.height - 60});
        menu->addChild(saveBtn);

        return true;
    }

    void onPrev(CCObject*) {
        DecoSystem::prev();
        onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onNext(CCObject*) {
        DecoSystem::next();
        onClose(nullptr);
        MainMenu::create(m_ui)->show();
    }

    void onApply(CCObject*) {
        DecoSystem::applyCurrentTemplate(m_ui);
        onClose(nullptr);
    }

    void onSaveCustom(CCObject*) {
        onClose(nullptr);
        SavePopup::create(m_ui)->show();
    }

public:
    static MainMenu* create(EditorUI* ui) {
        auto ret = new MainMenu();
        if (ret && ret->initAnchored(280.f, 150.f, ui)) {
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

        initBuiltInTemplates();

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100);

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        sprite->setScale(0.5f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::openMenu));
        btn->setPosition({winSize.width - 65, winSize.height - 18});
        menu->addChild(btn);

        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.2f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration v5.0 loaded - 8 built-in styles!");
        return true;
    }

    void openMenu(CCObject*) {
        MainMenu::create(this)->show();
    }
};
