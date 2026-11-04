#pragma once
#include "PluginProcessor.h"
#include "FontesUI.h"
#include "Common/EditorLifetime.h"
inline void pintarEstrela(juce::Graphics& g,juce::Point<float> centro,float raio,bool preenchida){juce::Path desenho;desenho.addStar(centro,5,raio*.43f,raio);if(preenchida)g.fillPath(desenho);else g.strokePath(desenho,juce::PathStrokeType(1.1f));}
// Filtros cumulativos: todos os chips precisam corresponder ao mesmo preset.
class AparenciaPresets final:public juce::LookAndFeel_V4 {
public:
 AparenciaPresets(){
  setColour(juce::ScrollBar::thumbColourId,juce::Colour(tema::cores::scroll));
  setColour(juce::ScrollBar::backgroundColourId,juce::Colour(tema::cores::scrollFundo));
  setColour(juce::Label::textColourId,juce::Colour(tema::cores::browserTexto));
  setColour(juce::TextEditor::textColourId,juce::Colour(tema::cores::browserTexto));
  setColour(juce::TextEditor::highlightedTextColourId,juce::Colour(tema::cores::browserTexto));
  setColour(juce::TextEditor::highlightColourId,juce::Colour(tema::cores::tagSelecionada));
  setColour(juce::CaretComponent::caretColourId,juce::Colour(tema::cores::browserTexto));
 }
 juce::Typeface::Ptr getTypefaceForFont(const juce::Font&)override{return fonteInterface().getTypefacePtr();}
 void drawTextEditorOutline(juce::Graphics& g,int w,int h,juce::TextEditor& campo)override{
  juce::LookAndFeel_V4::drawTextEditorOutline(g,w,h,campo);
  if((bool)campo.getProperties().getWithDefault("campoBusca",false)){g.setColour(juce::Colour(tema::cores::browserSecundario));const float cy=h*.5f;g.drawEllipse(12,cy-6,10,10,1.2f);g.drawLine(13,cy+3,9,cy+7,1.2f);}
 }
 void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool sobre,bool pressionado)override{
  if((bool)b.getProperties().getWithDefault("semFundo",false))return;
  const bool selecionado=(bool)b.getProperties().getWithDefault("chip",false)||(bool)b.getProperties().getWithDefault("acao",false);
  g.setColour(juce::Colour(sobre||pressionado?tema::cores::tagHover:selecionado?tema::cores::tagSelecionada:tema::cores::tag));
  g.fillRoundedRectangle(b.getLocalBounds().toFloat().reduced(.5f),2);
  g.setColour(juce::Colour(tema::cores::tagBorda));g.drawRoundedRectangle(b.getLocalBounds().toFloat().reduced(.5f),2,1);
 }
 void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool)override{
  const bool inativo=b.getProperties().contains("ativo")&&!(bool)b.getProperties()["ativo"];
  g.setColour(juce::Colour(!b.isEnabled()?tema::cores::browserDesabilitado:inativo?tema::cores::browserSecundario:tema::cores::browserTexto));
  g.setFont(fonteInterface());const auto texto=b.getButtonText();const auto area=b.getLocalBounds();
  if((bool)b.getProperties().getWithDefault("estrela",false)){
   const auto centro=area.toFloat().getCentre();
   if(texto.containsChar(0x00d7)){g.drawLine(centro.x-4,centro.y-4,centro.x+4,centro.y+4,1.5f);g.drawLine(centro.x+4,centro.y-4,centro.x-4,centro.y+4,1.5f);}else pintarEstrela(g,centro,9,texto.containsChar(0x2605));return;
  }
  if(texto.startsWith("FAVORITES")){g.drawText("FAVORITES",area.withTrimmedRight(27),juce::Justification::centredRight);pintarEstrela(g,{(float)area.getRight()-12,area.getHeight()*.5f},8,texto.containsChar(0x2605));return;}
  if(texto.startsWith("TAGS")){g.drawText("TAGS",area.withTrimmedRight(14),juce::Justification::centredLeft);juce::Path triangulo;const float x=area.getRight()-9.f,y=area.getHeight()*.5f;triangulo.addTriangle(x-3,y-2,x+3,y-2,x,y+2);g.fillPath(triangulo);return;}
  g.drawText(texto,area.reduced(8,0),juce::Justification::centred);
 }
};
class PresetBrowser final:public juce::Component,private juce::ListBoxModel {
public:
 explicit PresetBrowser(VocaProcessor&);
 ~PresetBrowser()override{voca::cancelDialog(deleteDialog);setLookAndFeel(nullptr);}
 void paint(juce::Graphics&)override;void resized()override;
private:
 void atualizar();void desenharTags();bool corresponde(const Preset&)const;
 int getNumRows()override{return (int)linhas.size();}
 void paintListBoxItem(int,juce::Graphics&,int,int,bool)override;
 void selectedRowsChanged(int)override;
 std::unique_ptr<juce::AlertWindow> deleteDialog;
 AparenciaPresets aparencia;VocaProcessor& processador;juce::TextEditor busca,nome;
 juce::TextButton fechar{juce::String::fromUTF8("×")},tags{juce::String::fromUTF8("TAGS ▾")},limpar{juce::String::fromUTF8("×")},favoritos{juce::String::fromUTF8("FAVORITES ☆")},tipo{"TYPE"},artista{"ARTIST"},todos{"ALL"},estrela{juce::String::fromUTF8("☆")},excluir{"DELETE"},renomear{"RENAME"},salvar{"SAVE"};
 juce::Label titulo,aviso;juce::Viewport rolagem;juce::Component grade;
 juce::ListBox lista;std::vector<int> linhas;std::vector<std::unique_ptr<juce::TextButton>> botoesTags, chips;
 juce::StringArray filtros{"Line"};int aba=0;bool somenteFavoritos=false,mostrarTags=true,atualizando=false;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
