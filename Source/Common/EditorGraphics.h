#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>
namespace voca {
class EditorGraphics final : private juce::ComponentListener {
public:
    EditorGraphics(juce::Component& editor, std::atomic<bool>& software)
        : component(editor), preference(software) { component.addComponentListener(this); }
    ~EditorGraphics() override { component.removeComponentListener(this); }
    void apply() {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
       #if JUCE_WINDOWS
        if (applying) return;
        const juce::ScopedValueSetter<bool> guard(applying, true);
        if (auto* peer = component.getPeer()) {
            const int wanted = preference.load() ? 0 : 1;
            if (wanted < peer->getAvailableRenderingEngines().size()
                && peer->getCurrentRenderingEngine() != wanted)
                peer->setCurrentRenderingEngine(wanted);
        }
       #endif
    }
private:
    void componentParentHierarchyChanged(juce::Component&) override { apply(); }
    void componentVisibilityChanged(juce::Component&) override { apply(); }
    juce::Component& component;
    std::atomic<bool>& preference;
    bool applying = false;
};
}
