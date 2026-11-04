#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"
#include "TemaVisual.h"

// Release the embedded typeface at JUCE shutdown, before the framework's glyph
// and Path leak detectors are destroyed. A function-static Typeface outlives them.
struct FonteInterfaceResource final : juce::DeletedAtShutdown {
    inline static FonteInterfaceResource* instance=nullptr;
    juce::Typeface::Ptr typeface=juce::Typeface::createSystemTypefaceFor(BinaryData::InterSemiBold_ttf,BinaryData::InterSemiBold_ttfSize);
    float correction=tema::fontes::larguraAmostraEm/(juce::Font(typeface).withHeight(100.f).getStringWidthFloat(tema::fontes::amostraMetrica)/100.f);
    ~FonteInterfaceResource() override { instance=nullptr; }
    static FonteInterfaceResource& get() {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        if(instance==nullptr)instance=new FonteInterfaceResource;
        return *instance;
    }
};
// Same embedded Inter SemiBold as HTML; preserve CSS em/JUCE height calibration.
inline juce::Font fonteInterface(float altura=tema::fontes::tamanho) {
    auto& resource=FonteInterfaceResource::get();
    return juce::Font(resource.typeface).withHeight(altura*resource.correction);
}
