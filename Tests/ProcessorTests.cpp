#include "../Source/PluginProcessor.h"
#include "../Source/NavegadorPresets.h"
#include "../Source/PosicaoPresets.h"
#include <iostream>
#include <stdexcept>
int main(){juce::ScopedJuceInitialiser_GUI gui;try{
 // A janela fica fora do painel e dentro da área útil, inclusive em outro monitor.
 for(const auto& caso:std::initializer_list<std::pair<juce::Rectangle<int>,juce::Rectangle<int>>>{{{500,80,1192,420},{0,0,1920,1040}},{{8,80,1192,420},{0,0,1920,1040}},{{-1400,80,1000,420},{-1920,0,1920,1040}},{{0,20,1192,420},{0,0,1366,1080}}}){
  const auto janela=posicionarPresetsExternos(caso.first,{389,484},caso.second);
  if(janela.intersects(caso.first)||!caso.second.contains(janela))throw std::runtime_error("Floating browser overlaps editor or leaves display");
 }
 const auto esquerda=posicionarPresetsExternos({500,80,1192,420},{389,484},{0,0,1920,1040});if(esquerda.getRight()>500||esquerda.getY()!=80)throw std::runtime_error("Left placement not preferred");
 const auto pequena=posicionarPresetsExternos({0,0,1192,420},{389,484},{0,0,1024,768});if(!juce::Rectangle<int>(0,0,1024,768).contains(pequena))throw std::runtime_error("Floating browser off small screen");
 const auto fonte=fonteInterface();
 if(!fonte.getTypefacePtr()->getName().startsWith("Inter"))throw std::runtime_error("Embedded Inter font unavailable");
 if(std::abs(fonte.getStringWidthFloat(tema::fontes::amostraMetrica)-tema::fontes::tamanho*tema::fontes::larguraAmostraEm)>.1f)throw std::runtime_error("Native font differs from CSS pixel metrics");
 VocaProcessor p;p.prepareToPlay(48000,256);
 if(p.getNumPrograms()<50)throw std::runtime_error("Preset bank missing");
 for(int i=0;i<p.getNumPrograms();++i){p.setCurrentProgram(i);for(const auto& spec:specs){const auto expected=(float)p.bank[(size_t)i].settings.getProperty(spec.id,spec.initial);if(std::abs(p.value(spec.id)-expected)>0.001f)throw std::runtime_error("Preset mapping mismatch");}}
 p.set("threshold",8.25f);p.set("release",4);juce::MemoryBlock state;p.getStateInformation(state);VocaProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());if(std::abs(restored.value("threshold")-8.25f)>.001f||restored.value("release")!=4)throw std::runtime_error("Host state restore failed");
 juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer midi;audio.clear();restored.prepareToPlay(48000,256);restored.processBlock(audio,midi);for(int c=0;c<2;++c)for(int n=0;n<256;++n)if(!std::isfinite(audio.getSample(c,n)))throw std::runtime_error("Invalid output");
 auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());if(editor->getWidth()!=1192||editor->getHeight()!=420)throw std::runtime_error("Editor dimensions");
 auto image=editor->createComponentSnapshot(editor->getLocalBounds());if(image.isNull())throw std::runtime_error("Editor render failed");
 if(auto stream=juce::File::getCurrentWorkingDirectory().getChildFile("editor-preview.png").createOutputStream()){stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;png.writeImageToStream(image,*stream);}

 // Persistência isolada: não toca na biblioteca instalada do usuário.
 const auto arquivo=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("vocabox-biblioteca-teste",".json");
 {VocaProcessor editavel(arquivo);const int quantidade=editavel.getNumPrograms();if(editavel.excluirPreset(0)||editavel.renomearPreset(0,"Invalid"))throw std::runtime_error("Factory protection failed");
 editavel.set("input",8.25f);const int indice=editavel.salvarPreset("Teste de persistencia",{"Line","Vocal"});if(indice!=quantidade)throw std::runtime_error("Save failed");if(editavel.salvarPreset("Teste de persistencia",{})!=-1)throw std::runtime_error("Duplicate name accepted");editavel.alternarFavorito(indice);if(!editavel.renomearPreset(indice,"Teste renomeado"))throw std::runtime_error("Rename failed");
 VocaProcessor reaberto(arquivo);if(reaberto.getNumPrograms()!=quantidade+1||!reaberto.bank.back().favorite||reaberto.bank.back().name!="Teste renomeado")throw std::runtime_error("Library persistence failed");reaberto.setCurrentProgram(quantidade);if(std::abs(reaberto.value("input")-8.25f)>.001f)throw std::runtime_error("User snapshot lost");if(!reaberto.excluirPreset(quantidade)||reaberto.getNumPrograms()!=quantidade)throw std::runtime_error("Delete failed");
 PresetBrowser navegador(reaberto);
 juce::ListBox* lista=nullptr;juce::TextButton* limpar=nullptr;for(auto* c:navegador.getChildren()){if(auto* l=dynamic_cast<juce::ListBox*>(c))lista=l;if(auto* b=dynamic_cast<juce::TextButton*>(c))if(b->getButtonText()==juce::String::fromUTF8("×"))limpar=b;}
 if(!lista||lista->getListBoxModel()->getNumRows()!=28)throw std::runtime_error("Native initial Line filter failed");if(!limpar)throw std::runtime_error("Native clear control missing");limpar->onClick();if(lista->getListBoxModel()->getNumRows()!=quantidade)throw std::runtime_error("Native clear filters failed");
 auto selecionarTag=[&](const juce::String& tag){std::function<juce::TextButton*(juce::Component*)> achar=[&](juce::Component* pai)->juce::TextButton*{for(auto* c:pai->getChildren()){if(auto* b=dynamic_cast<juce::TextButton*>(c))if(b->getButtonText()==tag)return b;if(auto* encontrado=achar(c))return encontrado;}return nullptr;};auto* b=achar(&navegador);if(!b)throw std::runtime_error("Native tag missing");b->onClick();};
 selecionarTag("Vocal");selecionarTag("Heavy");int esperados=0;for(auto& item:reaberto.bank)if(item.tags.contains("Vocal")&&item.tags.contains("Heavy"))++esperados;if(lista->getListBoxModel()->getNumRows()!=esperados)throw std::runtime_error("Native AND filters failed");limpar->onClick();
const auto captura=navegador.createComponentSnapshot(navegador.getLocalBounds());if(auto stream=juce::File::getCurrentWorkingDirectory().getChildFile("presets-preview.png").createOutputStream()){stream->setPosition(0);stream->truncate();juce::PNGImageFormat formato;formato.writeImageToStream(captura,*stream);}
 }arquivo.deleteFile();
 std::cout<<"PASS: embedded Inter 14 px, browser placement and filtering, user library persistence; "<<p.getNumPrograms()<<" presets × 25 parameters, host state serialization, audio processing.\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
