#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AutoBuildPopup : public geode::Popup<> {
protected:
    int m_currentPresetIndex = 0;
    CCLabelBMFont* m_presetLabel;
    
    // Internal state to track selected color if we add a picker
    ccColor3B m_selectedColor = {255, 255, 255}; 

    bool setup() override;
    
    void onNext(CCObject*);
    void onPrev(CCObject*);
    void onApply(CCObject*);
    void updateLabel();

public:
    static AutoBuildPopup* create();
};
