#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Coordenadas lógicas da tela: considera a área útil, sem cobrir o plugin
// quando houver espaço para a janela em algum dos quatro lados.
inline juce::Rectangle<int> posicionarPresetsExternos(juce::Rectangle<int> painel,
                                                   juce::Point<int> tamanho,
                                                   juce::Rectangle<int> areaUtil)
{
 constexpr int intervalo = 6;
 const auto limitar = [&](juce::Point<int> origem)
 {
  return juce::Rectangle<int>(
   juce::jlimit(areaUtil.getX(), juce::jmax(areaUtil.getX(), areaUtil.getRight() - tamanho.x), origem.x),
   juce::jlimit(areaUtil.getY(), juce::jmax(areaUtil.getY(), areaUtil.getBottom() - tamanho.y), origem.y),
   tamanho.x, tamanho.y);
 };
 const juce::Point<int> posicoes[] = {
  {painel.getX() - tamanho.x - intervalo, painel.getY()},
  {painel.getRight() + intervalo, painel.getY()},
  {painel.getX(), painel.getBottom() + intervalo},
  {painel.getX(), painel.getY() - tamanho.y - intervalo}
 };
 for (const auto& origem : posicoes)
 {
  const auto candidata = limitar(origem);
  if (areaUtil.contains(candidata) && !candidata.intersects(painel))
   return candidata;
 }
 // Em telas pequenas, mantém o cabeçalho acessível na borda com mais espaço.
 return limitar(painel.getX() - areaUtil.getX() >= areaUtil.getRight() - painel.getRight()
                 ? posicoes[0] : posicoes[1]);
}
