#include "PluginEditor.h"
#include "BinaryData.h"
#include "NavegadorPresets.h"
#include "PosicaoPresets.h"
static juce::Colour cream(){return juce::Colour(0xfff0ead6);}
void VocaLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float start,float end,juce::Slider& slider){
 juce::ignoreUnused(start,end);
 static const auto atlas=juce::ImageCache::getFromMemory(BinaryData::knobs_png,BinaryData::knobs_pngSize);
 const auto visual=slider.getProperties().getWithDefault("posicaoAnimada",position);
 const float angle=(float)slider.getProperties()["startAngle"]+(float)visual*((float)slider.getProperties()["endAngle"]-(float)slider.getProperties()["startAngle"]);
 const float quadro=std::fmod(angle/juce::MathConstants<float>::twoPi*128.f+256.f,128.f);const int inferior=(int)std::floor(quadro),superior=(inferior+1)%128;const float mistura=quadro-inferior;
 g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
 for(int i=0;i<2;++i){const int frame=i?superior:inferior;g.setOpacity(i?mistura:1.f);g.drawImage(atlas,x,y,w,h,(frame%16)*264,(frame/16)*264,264,264);}g.setOpacity(1);
 if(slider.hasKeyboardFocus(false)){g.setColour(cream().withAlpha(.65f));g.drawEllipse((float)x+w*.14f,(float)y+h*.14f,w*.72f,h*.72f,1);}

}
void Lever::paintButton(juce::Graphics& g,bool over,bool){
 const auto cx=getWidth()*.5f;g.setGradientFill(juce::ColourGradient(cream(),cx-5,10,juce::Colour(0xff454b3d),cx+5,20,false));g.fillEllipse(cx-6,9,12,13);g.setColour(juce::Colours::black);g.drawEllipse(cx-4,11,8,9,1);
 const float tilt=spec.max>1?(value-1)*.38f:(value>.5f?0:2.88f);const auto top=juce::Point<float>(cx+std::sin(tilt)*10,15-std::cos(tilt)*10);
 g.setColour(over?juce::Colours::white:cream());g.drawLine(cx,16,top.x,top.y,4);g.setColour(juce::Colour(0xff666a5c));g.fillEllipse(top.x-2,top.y-2,4,4);
}
class BrowserWindow final:public juce::DocumentWindow {
public:
 explicit BrowserWindow(VocaProcessor& p):DocumentWindow("VocaBox Tags",juce::Colour(0xff45444b),closeButton){
  setUsingNativeTitleBar(false);setTitleBarHeight(0);setResizable(false,false);
  setContentOwned(new PresetBrowser(p),true);
  getContentComponent()->addMouseListener(this,true);
  limites.setMinimumOnscreenAmounts(30,60,30,60);
 }
 ~BrowserWindow()override{getContentComponent()->removeMouseListener(this);}
 void abrirAoLadoDe(juce::Component& editor){
  if(!isVisible()){
   const auto painel=editor.getScreenBounds();
   const auto* monitor=juce::Desktop::getInstance().getDisplays().getDisplayForRect(painel);
   const auto areaUtil=monitor?monitor->userArea:painel.expanded(getWidth()+6,getHeight()+6);
   setBounds(posicionarPresetsExternos(painel,{getWidth(),getHeight()},areaUtil));
  }
  setVisible(true);toFront(true);
 }
 void closeButtonPressed()override{setVisible(false);}
 void mouseDown(const juce::MouseEvent& evento)override{
  const auto ponto=evento.getEventRelativeTo(getContentComponent()).position;
  arrastando=evento.mods.isLeftButtonDown()&&ponto.y>=0&&ponto.y<30&&ponto.x>=0&&ponto.x<getContentComponent()->getWidth()-30;
  if(arrastando)arrastador.startDraggingComponent(this,evento);
 }
 void mouseDrag(const juce::MouseEvent& evento)override{if(arrastando)arrastador.dragComponent(this,evento,&limites);}
 void mouseUp(const juce::MouseEvent&)override{arrastando=false;}
private:
 juce::ComponentDragger arrastador;juce::ComponentBoundsConstrainer limites;bool arrastando=false;
};
VocaEditor::VocaEditor(VocaProcessor& p):AudioProcessorEditor(p),processor(p){
 setLookAndFeel(&look);face=juce::ImageCache::getFromMemory(BinaryData::face_png,BinaryData::face_pngSize);
 for(const auto& spec:specs){if(juce::String(spec.id)=="bypass")continue;
  if(spec.toggle){auto lever=std::make_unique<Lever>(spec);lever->value=processor.value(spec.id);lever->setTooltip(juce::String(spec.name)+" - click to cycle");lever->onClick=[this,id=juce::String(spec.id),maximum=spec.max]{processor.set(id.toRawUTF8(),std::fmod(processor.value(id.toRawUTF8())+1,maximum+1));};addAndMakeVisible(*lever);levers.push_back(std::move(lever));}
  else {auto knob=std::make_unique<NativeKnob>();knob->setRange(spec.min,spec.max,spec.step);knob->setDoubleClickReturnValue(true,spec.initial);knob->setRotaryParameters(0,juce::MathConstants<float>::pi*1.5f,true);knob->getProperties().set("startAngle",juce::degreesToRadians(spec.start));knob->getProperties().set("endAngle",juce::degreesToRadians(spec.end));knob->setTooltip(juce::String(spec.name)+"  |  drag / wheel  |  Shift: fine  |  double click: reset");knob->setName(spec.name);addAndMakeVisible(*knob);attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.params,spec.id,*knob));knobSpecs.push_back(&spec);knobs.push_back(std::move(knob));}
 }
 for(auto* button:{&previous,&next,&presetsButton,&bypass,&compare,&copy,&paste})addAndMakeVisible(button);
 presetsButton.onClick=[this]{if(!browserWindow)browserWindow=std::make_unique<BrowserWindow>(processor);static_cast<BrowserWindow&>(*browserWindow).abrirAoLadoDe(*this);};
 previous.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms());};next.onClick=[this]{processor.setCurrentProgram((processor.getCurrentProgram()+1)%processor.getNumPrograms());};
 bypass.onClick=[this]{processor.set("bypass",1-processor.value("bypass"));};
 comparison=processor.params.copyState();compare.onClick=[this]{auto old=processor.params.copyState();processor.params.replaceState(comparison);comparison=old;};copy.onClick=[this]{clipboard=processor.params.copyState();};paste.onClick=[this]{if(clipboard.isValid())processor.params.replaceState(clipboard.createCopy());};
 setSize(1192,420);startTimerHz(60);updateProgram();
}
VocaEditor::~VocaEditor(){stopTimer();browserWindow.reset();attachments.clear();setLookAndFeel(nullptr);}
void VocaEditor::resized(){
 previous.setBounds(8,9,27,28);next.setBounds(37,9,27,28);presetsButton.setBounds(68,9,270,28);bypass.setBounds(343,9,65,28);compare.setBounds(413,9,75,28);copy.setBounds(493,9,55,28);paste.setBounds(553,9,55,28);
 for(size_t i=0;i<knobs.size();++i){const auto& s=*knobSpecs[i];const auto ext=s.size*88.f/64.f;knobs[i]->setBounds(juce::roundToInt(s.x-ext/2),juce::roundToInt(46+s.y-ext/2),juce::roundToInt(ext),juce::roundToInt(ext));}
 for(auto& lever:levers){const auto& s=lever->spec;lever->setBounds((int)s.x-8,46+(int)s.y-12,16,25);}
}
void VocaEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff45444c));g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);g.drawImage(face,juce::Rectangle<float>(0,46,1192,330));g.setColour(cream());g.setFont(11);g.drawText("VOCABOX  |  OPTICAL CHANNEL STRIP",810,10,360,25,juce::Justification::centredRight);
 g.setColour(juce::Colour(0xff2d2c32));g.fillRect(1,377,1190,42);g.setColour(cream());g.drawText("VOCABOX  |  AUDIO ACTIVE  |  GR "+juce::String(processor.compressorGR.load(),1)+" dB  |  D-S "+juce::String(processor.deesserGR.load(),1)+" dB",8,379,1176,32,juce::Justification::centred);
 const bool power=processor.value("power")>.5f;g.setColour(power?juce::Colours::red:juce::Colour(0xff3d1716));g.fillEllipse(152,46+114,8,8);g.setColour(power&&processor.value("bypass")<.5f?juce::Colours::red:juce::Colour(0xff3d1716));g.fillEllipse(1053,46+63,8,8);
 // Meter clips to the amber window and reports actual DSP levels/reduction.
 juce::Graphics::ScopedSaveState saved(g);
 g.addTransform(juce::AffineTransform::scale(185.f/220.f).translated(285,46+31));
 juce::Path clip;clip.startNewSubPath(15,43);clip.quadraticTo(111,6,205,36);clip.quadraticTo(209,37,208,41);clip.lineTo(195,85);clip.quadraticTo(194,89,190,88);clip.quadraticTo(111,72,32,90);clip.quadraticTo(28,90,27,86);clip.lineTo(12,48);clip.quadraticTo(11,44,15,43);clip.closeSubPath();g.reduceClipRegion(clip);
 if(!power){g.setColour(juce::Colours::black.withAlpha(.65f));g.fillRect(0,0,220,106);}
 const auto pivot=juce::Point<float>(110,140);const auto end=pivot+juce::Point<float>(std::sin(juce::degreesToRadians(needle)),-std::cos(juce::degreesToRadians(needle)))*114;
 g.setColour(juce::Colour(0x405c3c12));g.drawLine({pivot.translated(.6f,.3f),end.translated(.6f,.3f)},1.1f);
 g.setColour(juce::Colour(0xff29220f));g.drawLine({pivot,end},1.1f);
}
void VocaEditor::updateProgram(){presetsButton.setButtonText(processor.getProgramName(processor.getCurrentProgram())+" v");bypass.setButtonText(processor.value("bypass")>.5f?"BYPASS":"IN");}
void VocaEditor::timerCallback(){
 updateProgram();for(auto& knob:knobs)knob->animar();for(auto& lever:levers){const auto v=processor.value(lever->spec.id);if(v!=lever->value){lever->value=v;lever->repaint();}}
 const int mode=(int)processor.value("meter");float target=-48;
 if(processor.value("power")>.5f){if(mode==0)target=juce::jlimit(-48.f,45.f,20.f-processor.compressorGR.load()*4.f);else if(mode==3)target=juce::jlimit(-48.f,45.f,20.f-processor.deesserGR.load()*3.f);else {const float level=processor.meterValues[mode==1?1:2].load();target=juce::jlimit(-48.f,45.f,20.f+(float)voca::gainToDb(level)*2.3f);}}
 needle+=(target-needle)*.22f;repaint();
}
