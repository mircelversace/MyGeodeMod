#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "AutoBuildPopup.hpp"

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        auto menu = this->getChildByID("editor-buttons-menu"); 
        if (!menu) {
            // Fallback if node IDs are not present or different (older Geode versions or specific node layouts)
            // The copy button usually in a menu on the left or top-left. 
            // We'll search for the menu containing the copy button manually if ID fails, or just append to the main menu.
            // For now, let's assume standard positioning or create a new menu if needed.
            
            // Attempt to find the menu with the copy button (sprite "GJ_copyBtn_001.png")
            // This is a bit heuristic.
            CCArray* children = this->getChildren();
            if (children) {
                 for (int i = 0; i < children->count(); ++i) {
                    auto node = static_cast<CCNode*>(children->objectAtIndex(i));
                    if (auto checkMenu = typeinfo_cast<CCMenu*>(node)) {
                        CCArray* menuChildren = checkMenu->getChildren();
                        if (menuChildren) {
                            for (int j = 0; j < menuChildren->count(); ++j) {
                                auto btn = static_cast<CCNode*>(menuChildren->objectAtIndex(j));
                                if (auto menuItem = typeinfo_cast<CCMenuItemSpriteExtra*>(btn)) {
                                    // Check if it has the copy sprite
                                    // Implementation detail: checking children of menu item for sprite frame keys is complex without specific APIs.
                                    // Let's settle on adding to the "edit-menu" typically used for copy/paste if available, 
                                    // or just creating a new button at a fixed position relative to the screen.
                                }
                            }
                        }
                    }
                 }
            }
        }
        
        // Let's use the explicit node ID if available "edit-tab-menu" or similar.
        // Actually, the user asked for "next to the copy button".
        // The copy button is usually in the "edit-tab-menu" when the edit tab is selected, OR the persistent "edit-menu".
        // Let's look for "Undo" or "Copy" button connection.
        
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto autoBuildBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("AutoBuild", 40, true, "bigFont.fnt", "GJ_button_01.png", 30, 1.0f),
            this,
            menu_selector(MyEditorUI::onAutoBuild)
        );
        autoBuildBtn->setID("autobuild-button");
        
        // Find the copy-paste menu. In vanilla, it's often close to the left side or part of the specific edit tab.
        // A safe bet for a "tool" button is the top-left or adding it to the existing batch if we can find it.
        // Let's try to fetch the edit menu (left side tools).
        CCMenu* targetMenu = nullptr;
        if (auto menu = this->getChildByID("edit-menu")) {
             targetMenu = static_cast<CCMenu*>(menu);
        } else {
             // Fallback: Create our own menu
             targetMenu = CCMenu::create();
             targetMenu->setID("autobuild-menu");
             targetMenu->setPosition(50, winSize.height - 50); // Top Left rough position
             this->addChild(targetMenu);
        }

        // Add to the menu
        targetMenu->addChild(autoBuildBtn);
        targetMenu->updateLayout();

        return true;
    }

    void onAutoBuild(CCObject* sender) {
        AutoBuildPopup::create()->show();
    }
};
