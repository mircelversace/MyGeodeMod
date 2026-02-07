#include "AutoBuildPopup.hpp"
#include "DecorationManager.hpp"
#include <vector>
#include <string>

// Preliminary list of preset names to display.
// The actual logic will be in DecorationManager, but good to have names here.
static const std::vector<std::string> PRESET_NAMES = {
    "1. Simple Glow Top",
    "2. Double Glow",
    "3. Outline Detail",
    "4. Inner Corner Detail",
    "5. Tech Debris",
    "6. Nature/Leaves",
    "7. Spike Caps",
    "8. Base Support Top",
    "9. Base Support Bottom",
    "10. Neon Pulse",
    "11. Crystal Shards",
    "12. Mechanical Gears",
    "13. Floating Orbs",
    "14. Chains Hanging",
    "15. Toxic Drip",
    "16. Fire Embers",
    "17. Ice Icicles",
    "18. Shadow Depth",
    "19. Retro Pixel",
    "20. Future Grid",
    "21. Abstract Shapes"
};

AutoBuildPopup* AutoBuildPopup::create() {
    auto ret = new AutoBuildPopup();
    if (ret && ret->initAnchored(300, 200)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AutoBuildPopup::setup() {
    this->setTitle("Auto Decorate");

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    // Preset Label
    m_presetLabel = CCLabelBMFont::create(PRESET_NAMES[0].c_str(), "goldFont.fnt");
    m_presetLabel->setPosition(m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 + 20);
    m_mainLayer->addChild(m_presetLabel);

    // Initial update
    this->updateLabel();

    // Arrows
    auto prevSprite = ButtonSprite::create("Prev", 40, true, "goldFont.fnt", "GJ_button_01.png", 30, 0.8f); // Simplified sprite usage
    // Better to use actual arrow sprites if available, or just text for now.
    // Using standard controller arrows or similar: "edit_leftBtn_001.png"
    auto leftBtnSpr = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    if (!leftBtnSpr) leftBtnSpr = ButtonSprite::create("<", 40, true, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f);
    
    auto rightBtnSpr = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    if (!rightBtnSpr) rightBtnSpr = ButtonSprite::create(">", 40, true, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f);

    auto prevBtn = CCMenuItemSpriteExtra::create(
        leftBtnSpr,
        this,
        menu_selector(AutoBuildPopup::onPrev)
    );
    prevBtn->setPosition(m_mainLayer->getContentSize().width / 2 - 100, m_mainLayer->getContentSize().height / 2 + 20);

    auto nextBtn = CCMenuItemSpriteExtra::create(
        rightBtnSpr,
        this,
        menu_selector(AutoBuildPopup::onNext)
    );
    nextBtn->setPosition(m_mainLayer->getContentSize().width / 2 + 100, m_mainLayer->getContentSize().height / 2 + 20);

    // Apply Button
    auto applyBtn = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply", 60, true, "goldFont.fnt", "GJ_button_02.png", 30, 1.0f),
        this,
        menu_selector(AutoBuildPopup::onApply)
    );
    applyBtn->setPosition(m_mainLayer->getContentSize().width / 2, m_mainLayer->getContentSize().height / 2 - 50);

    auto menu = CCMenu::create();
    menu->addChild(prevBtn);
    menu->addChild(nextBtn);
    menu->addChild(applyBtn);
    menu->setPosition(0, 0);
    m_mainLayer->addChild(menu);

    return true;
}

void AutoBuildPopup::onNext(CCObject*) {
    m_currentPresetIndex++;
    if (m_currentPresetIndex >= PRESET_NAMES.size()) {
        m_currentPresetIndex = 0;
    }
    this->updateLabel();
}

void AutoBuildPopup::onPrev(CCObject*) {
    m_currentPresetIndex--;
    if (m_currentPresetIndex < 0) {
        m_currentPresetIndex = PRESET_NAMES.size() - 1;
    }
    this->updateLabel();
}

void AutoBuildPopup::updateLabel() {
    if (m_presetLabel) {
        m_presetLabel->setString(PRESET_NAMES[m_currentPresetIndex].c_str());
    }
}

void AutoBuildPopup::onApply(CCObject*) {
    // Get the editor layer
    auto editorUI = EditorUI::get();
    if (!editorUI) return;

    auto selectedObjs = editorUI->getSelectedObjects();
    if (!selectedObjs || selectedObjs->count() == 0) {
        FLAlertLayer::create("Info", "No objects selected!", "OK")->show();
        return;
    }

    DecorationManager::applyDecoration(selectedObjs, m_currentPresetIndex);
    this->onClose(nullptr);
}
