#pragma once
#include "PluginProcessor.h"
inline void pintarEstrela(juce::Graphics& g,juce::Point<float> centro,float raio,bool preenchida){juce::Path desenho;desenho.addStar(centro,5,raio*.43f,raio);if(preenchida)g.fillPath(desenho);else g.strokePath(desenho,juce::PathStrokeType(1.1f));}
// Filtros cumulativos: todos os chips precisam corresponder ao mesmo preset.
class AparenciaPresets final:public juce::LookAndFeel_V4 {
public:
 AparenciaPresets(){setColour(juce::ScrollBar::thumbColourId,juce::Colour(0xff505055));setColour(juce::ScrollBar::backgroundColourId,juce::Colour(0xff262629));}
 void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool sobre,bool pressionado)override{const auto cor=b.getProperties().getWithDefault("chip",false)?juce::Colour(0xff4a4a4f):juce::Colour((bool)b.getProperties().getWithDefault("acao",false)?0xff3a3a3e:0xff2a2a2d);if(!b.getProperties().getWithDefault("semFundo",false)){g.setColour((sobre||pressionado)?juce::Colour(0xff38383c):cor);g.fillRoundedRectangle(b.getLocalBounds().toFloat().reduced(.5f),2);g.setColour(juce::Colour(0xff3c3c40));g.drawRoundedRectangle(b.getLocalBounds().toFloat().reduced(.5f),2,1);}}
 void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool)override{
  g.setColour(juce::Colours::white.withAlpha(b.isEnabled()?1.f:.3f));g.setFont(juce::Font(10.f,juce::Font::bold));auto texto=b.getButtonText();auto area=b.getLocalBounds();
  if((bool)b.getProperties().getWithDefault("estrela",false)){const auto centro=area.toFloat().getCentre();if(texto.containsChar(0x00d7)){g.drawLine(centro.x-4,centro.y-4,centro.x+4,centro.y+4,1.5f);g.drawLine(centro.x+4,centro.y-4,centro.x-4,centro.y+4,1.5f);}else pintarEstrela(g,centro,9,texto.containsChar(0x2605));return;}
  if(texto.startsWith("FAVORITES")){g.drawText("FAVORITES",area.withTrimmedRight(25),juce::Justification::centredRight);pintarEstrela(g,{(float)area.getRight()-12,area.getHeight()*.5f},7,texto.containsChar(0x2605));return;}
  if(texto.startsWith("TAGS")){g.drawText("TAGS",area.withTrimmedRight(12),juce::Justification::centred);juce::Path triangulo;const float x=area.getRight()-9.f,y=area.getHeight()*.5f;triangulo.addTriangle(x-3,y-2,x+3,y-2,x,y+2);g.fillPath(triangulo);return;}
  g.drawText(texto,area,juce::Justification::centred);
 }

};
class PresetBrowser final:public juce::Component,private juce::ListBoxModel {
public:
 explicit PresetBrowser(VocaProcessor&);
 ~PresetBrowser()override{setLookAndFeel(nullptr);}
 void paint(juce::Graphics&)override;void resized()override;
private:
 void atualizar();void desenharTags();bool corresponde(const Preset&)const;
 int getNumRows()override{return (int)linhas.size();}
 void paintListBoxItem(int,juce::Graphics&,int,int,bool)override;
 void selectedRowsChanged(int)override;
 AparenciaPresets aparencia;VocaProcessor& processador;juce::TextEditor busca,nome;
 juce::TextButton fechar{juce::String::fromUTF8("×")},tags{juce::String::fromUTF8("TAGS ▾")},limpar{juce::String::fromUTF8("×")},favoritos{juce::String::fromUTF8("FAVORITES ☆")},tipo{"TYPE"},artista{"ARTIST"},todos{"ALL"},estrela{juce::String::fromUTF8("☆")},excluir{"DELETE"},renomear{"RENAME"},salvar{"SAVE"};
 juce::Label titulo,aviso;juce::Viewport rolagem;juce::Component grade;
 juce::ListBox lista;std::vector<int> linhas;std::vector<std::unique_ptr<juce::TextButton>> botoesTags, chips;
 juce::StringArray filtros{"Line"};int aba=0;bool somenteFavoritos=false,mostrarTags=true,atualizando=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
