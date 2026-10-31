#pragma once
#include <juce_graphics/juce_graphics.h>
#include "BinaryData.h"
#include "TemaVisual.h"

// A mesma fonte Inter SemiBold incorporada no HTML, sem depender das fontes do SO.
inline juce::Font fonteInterface(float altura=tema::fontes::tamanho)
{
 static const auto fonte=juce::Typeface::createSystemTypefaceFor(BinaryData::InterSemiBold_ttf,BinaryData::InterSemiBold_ttfSize);
 // JUCE mede a caixa ascendente/descendente; CSS mede o em. Normaliza
 // a métrica para que 14 px tenham o mesmo tamanho nos dois renderizadores.
 static const float correcao=tema::fontes::larguraAmostraEm/(juce::Font(fonte).withHeight(100.f).getStringWidthFloat(tema::fontes::amostraMetrica)/100.f);
 return juce::Font(fonte).withHeight(altura*correcao);
}
