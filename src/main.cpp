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

        const char* styleName = "Unknown";
        if(g_currentStyle == DecoStyle::Glow) styleName = "Glow";
        if(g_currentStyle == DecoStyle::Modern) styleName = "Modern";
        if(g_currentStyle == DecoStyle::Tech) styleName = "Tech";

        Notification::create(
            fmt::format("Applied {} style to {} objects!", styleName, selectedObjs->count()).c_str(), 
            NotificationIcon::Success
        )->show();
    }
};

// ==========================================
// POPUP MENU (Select Style)
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

        // Style buttons
        createStyleBtn("Glow", center.height + 40, DecoStyle::Glow, menu, center.width);
        createStyleBtn("Modern", center.height, DecoStyle::Modern, menu, center.width);
        createStyleBtn("Tech", center.height - 40, DecoStyle::Tech, menu, center.width);

        // Apply button at bottom
        auto applySprite = ButtonSprite::create("APPLY TO SELECTION", 150, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.5f);
        auto applyBtn = CCMenuItemSpriteExtra::create(applySprite, this, menu_selector(AutoDecoMenu::onApply));
        applyBtn->setPosition({center.width, 40});
        menu->addChild(applyBtn);

        return true;
    }

    void createStyleBtn(const char* txt, float y, DecoStyle style, CCMenu* menu, float centerX) {
        auto sprite = ButtonSprite::create(txt, 80, true, "bigFont.fnt", "GJ_button_04.png", 25, 0.6f);
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(AutoDecoMenu::onSelectStyle));
        btn->setTag(static_cast<int>(style));
        btn->setPosition({centerX, y});
        menu->addChild(btn);
    }

    void onSelectStyle(CCObject* sender) {
        int val = static_cast<CCNode*>(sender)->getTag();
        g_currentStyle = static_cast<DecoStyle>(val);
        
        const char* name = "Unknown";
        if(g_currentStyle == DecoStyle::Glow) name = "Glow";
        if(g_currentStyle == DecoStyle::Modern) name = "Modern";
        if(g_currentStyle == DecoStyle::Tech) name = "Tech";

        Notification::create(fmt::format("{} style selected!", name).c_str(), NotificationIcon::Info)->show();
    }

    void onApply(CCObject* sender) {
        // Get EditorUI from the game
        auto editorUI = LevelEditorLayer::get()->m_editorUI;
        if (editorUI) {
            AutoBuilder::applyToSelection(editorUI);
        }
        this->onClose(sender);
    }

public:
    static AutoDecoMenu* create() {
        auto ret = new AutoDecoMenu();
        if (ret && ret->initAnchored(280.f, 200.f)) {
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

        // Create button and add to the pause menu area (top right)
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        this->addChild(menu, 100); // High z-order to be on top

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Create the AutoDeco button - positioned near the pause button
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        sprite->setScale(0.8f);
        
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyEditorUI::onAutoDecoBtn));
        btn->setPosition({winSize.width - 90, winSize.height - 25});
        menu->addChild(btn);

        // Add a label on the button
        auto label = CCLabelBMFont::create("AD", "bigFont.fnt");
        label->setScale(0.4f);
        label->setPosition(sprite->getContentSize() / 2);
        sprite->addChild(label);

        log::info("AutoDecoration mod loaded! Button added.");

        return true;
    }

    void onAutoDecoBtn(CCObject*) {
        AutoDecoMenu::create()->show();
    }
};
