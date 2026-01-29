#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

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
// POPUP MENU (Select Style) - Using Geode Popup
// ==========================================
class AutoDecoMenu : public geode::Popup<> {
protected:
    bool setup() override {
        setTitle("Select Style");
        
        auto menu = CCMenu::create();
        menu->setContentSize(m_mainLayer->getContentSize());
        menu->setPosition({0, 0});
        m_mainLayer->addChild(menu);

        createBtn("Glow Style", 50, DecoStyle::Glow, menu);
        createBtn("Modern Style", 0, DecoStyle::Modern, menu);
        createBtn("Tech Style", -50, DecoStyle::Tech, menu);

        return true;
    }

    void createBtn(const char* txt, float y, DecoStyle style, CCMenu* menu) {
        auto sprite = ButtonSprite::create(txt);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(AutoDecoMenu::onSelect));
        btn->setTag(static_cast<int>(style));
        auto center = m_mainLayer->getContentSize() / 2;
        btn->setPosition({center.width, center.height + y});
        menu->addChild(btn);
    }

    void onSelect(CCObject* sender) {
        int val = static_cast<CCNode*>(sender)->getTag();
        g_currentStyle = static_cast<DecoStyle>(val);
        
        const char* name = "Unknown";
        if(g_currentStyle == DecoStyle::Glow) name = "Glow";
        if(g_currentStyle == DecoStyle::Modern) name = "Modern";
        if(g_currentStyle == DecoStyle::Tech) name = "Tech";

        Notification::create(fmt::format("{} Selected", name).c_str(), NotificationIcon::Info)->show();
        this->onClose(sender);
    }

public:
    static AutoDecoMenu* create() {
        auto ret = new AutoDecoMenu();
        if (ret && ret->initAnchored(300.f, 200.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ==========================================
// HOOKS (Modify EditorUI)
// ==========================================
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        // Get window size properly
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Find the menu to attach to (usually the left side menu or create a new one)
        auto menu = this->getChildByID("editor-buttons-menu");
        if (!menu) {
            // Create our own menu if the expected one doesn't exist
            auto newMenu = CCMenu::create();
            newMenu->setID("autodeco-menu"_spr);
            newMenu->setPosition({0, 0});
            this->addChild(newMenu);
            menu = newMenu;
        }

        // --- SHOW BUTTON ---
        // This is the main button visible in editor
        auto showSprite = ButtonSprite::create("AutoDeco", 60, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.6f);
        auto showBtn = CCMenuItemSpriteExtra::create(showSprite, this, menu_selector(MyEditorUI::onShowAutoDecoMenu));
        showBtn->setID("show-autodeco-btn"_spr);
        showBtn->setPosition({winSize.width - 70, winSize.height - 50});
        static_cast<CCMenu*>(menu)->addChild(showBtn);

        return true;
    }

    void onShowAutoDecoMenu(CCObject*) {
        AutoDecoMenu::create()->show();
    }
};
