#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace voca {
// Keep the slider, attachment and LookAndFeel alive until JUCE has ended its
// actual DragInProgress and destroyed the popup. Never fake APVTS callbacks.
inline void finishSliderInteraction(juce::Slider& slider, const juce::MouseEvent& lastEvent) {
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    juce::Component::SafePointer<juce::Slider> safe(&slider);
    auto source = lastEvent.source;
    if (source.isDragging()) {
        auto* target = source.getComponentUnderMouse();
        if (target == &slider || slider.isParentOf(target)) {
            if (auto* peer = slider.getPeer()) {
                const auto position = peer->globalToLocal(source.getScreenPosition());
                peer->handleMouseEvent(source.getType(), position,
                    source.getCurrentModifiers().withoutMouseButtons(), 1.0f, 0.0f,
                    juce::Time::currentTimeMillis());
            }
        }
    }
    // Also covers host/test events dispatched straight to Slider::mouseDown.
    if (safe) safe->juce::Slider::mouseUp(lastEvent);
}

inline void cancelDialog(std::unique_ptr<juce::AlertWindow>& dialog) {
    if (dialog) dialog->exitModalState(0);
    dialog.reset();
}
}
