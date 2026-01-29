#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

// ==========================================
// GLOBALS
// ==========================================
enum class DecoStyle { Glow, Modern, Tech };
static DecoStyle g_currentStyle = DecoStyle::Glow;

// ==========================================
// LOGIC CLASS (The Auto-Builder)
// ==========================================
class AutoBuilder {
public:
    static void applyToSelection(EditorUI* ui) {
        auto selectedObjs = ui->getSelectedObjects();
        if (!selectedObjs || selectedObjs->count() == 0) {
            Notification::create("Please select blocks first!", NotificationIcon::Warning)->show();
            return;
        }

        // TODO: In a real compiled mod, here we would:
        // 1. Iterate selectedObjs
        // 2. Scan for neighbors (using LevelEditorLayer object map)
        // 3. Create new objects based on g_currentStyle
        
        // Simulating success for the template
        Notification::create("Auto-Decoration Applied! (Simulation)", NotificationIcon::Success)->show();
    }
};

// ==========================================
// POPUP MENU (Select Style)
// ==========================================
class AutoDecoMenu : public FLAlertLayer {
public:
    bool init() {
        if (!FLAlertLayer::init(nullptr, "Select Style", "Close", nullptr, 400.0f))
            return false;

        auto menu = CCMenu::create();
        m_mainLayer->addChild(menu);

        createBtn("Glow Style", 50, DecoStyle::Glow, menu);
        createBtn("Modern Style", 0, DecoStyle::Modern, menu);
        createBtn("Tech Style", -50, DecoStyle::Tech, menu);

        return true;
    }

    void createBtn(const char* txt, float y, DecoStyle style, CCMenu* menu) {
        auto sprite = ButtonSprite::create(txt);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(AutoDecoMenu::onSelect));
        btn->setUserObject(CCInteger::create((int)style));
        btn->setPositionY(y);
        menu->addChild(btn);
    }

    void onSelect(CCObject* sender) {
        auto val = static_cast<CCInteger*>(static_cast<CCNode*>(sender)->getUserObject())->getValue();
        g_currentStyle = (DecoStyle)val;
        
        const char* name = "Unknown";
        if(g_currentStyle == DecoStyle::Glow) name = "Glow";
        if(g_currentStyle == DecoStyle::Modern) name = "Modern";
        if(g_currentStyle == DecoStyle::Tech) name = "Tech";

        Notification::create(fmt::format("{} Selected", name).c_str(), NotificationIcon::Info)->show();
        this->onBtn1(sender); // Close
    }

    static AutoDecoMenu* create() {
        auto ret = new AutoDecoMenu();
        if (ret && ret->init()) { ret->autorelease(); return ret; }
        CC_SAFE_DELETE(ret); return nullptr;
    }
};

// ==========================================
// HOOKS (Modify EditorUI)
// ==========================================
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        // Find the menu to attach to (usually the left side menu or create a new one)
        auto menu = this->getChildByID("editor-buttons-menu");
        if (!menu) return true;

        // --- SHOW BUTTON ---
        // This is the main button visible in editor
        auto showSprite = ButtonSprite::create("Show", 40, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.8f);
        auto showBtn = CCMenuItemSpriteExtra::create(showSprite, this, menu_selector(MyEditorUI::onShowAutoDecoMenu));
        showBtn->setID("show-autodeco-btn"_spr);
        showBtn->setPosition({280, 80}); // Adjust position as needed
        menu->addChild(showBtn);

        // --- APPLY BUTTON (Hidden initially, or placed above Copy) ---
        // Creating a secondary menu for the tools
        auto toolsMenu = CCMenu::create();
        toolsMenu->setID("autodeco-tools-menu"_spr);
        toolsMenu->setPosition({0, 0});
        this->addChild(toolsMenu);

        auto applySprite = ButtonSprite::create("Apply", 40, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.6f);
        auto applyBtn = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(MyEditorUI::onApplyAutoDeco));
        applyBtn->setPosition({ winSize.width - 50, winSize.height - 100 }); // Top Right corner example
        toolsMenu->addChild(applyBtn);

        return true;
    }

    void onShowAutoDecoMenu(CCObject*) {
        AutoDecoMenu::create()->show();
    }

    void onApplyAutoDeco(CCObject*) {
        AutoBuilder::applyToSelection(this);
    }
};
