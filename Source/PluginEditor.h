#pragma once
#include "PluginProcessor.h"
#include "Common/EditorLifetime.h"
#include "Common/EditorGraphics.h"
class VocaLook final:public juce::LookAndFeel_V4 {
public: void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
};
class NativeKnob:public juce::Slider {
public:
 NativeKnob(){setSliderStyle(RotaryVerticalDrag);setTextBoxStyle(NoTextBox,false,0,0);setMouseDragSensitivity(140);setMouseCursor(juce::MouseCursor::UpDownResizeCursor);}
 ~NativeKnob() override { finishInteraction(); setLookAndFeel(nullptr); }
 void finishInteraction(){voca::cancelDialog(numericDialog);if(dragEvent){const auto event=*dragEvent;dragEvent.reset();voca::finishSliderInteraction(*this,event);}entradaNumerica=false;}
 void mouseDown(const juce::MouseEvent& e)override{
  if(e.mods.isPopupMenu()||e.mods.isCtrlDown()){
   finishInteraction();entradaNumerica=true;
   numericDialog=std::make_unique<juce::AlertWindow>(getName(),"Digite o valor",juce::MessageBoxIconType::NoIcon);
   auto* dialog=numericDialog.get();dialog->addTextEditor("valor",juce::String(getValue(),2));
   dialog->addButton("Aplicar",1,juce::KeyPress(juce::KeyPress::returnKey));dialog->addButton("Cancelar",0,juce::KeyPress(juce::KeyPress::escapeKey));
   auto safe=juce::Component::SafePointer<NativeKnob>(this);
   auto safeDialog=juce::Component::SafePointer<juce::AlertWindow>(dialog);
   dialog->enterModalState(true,juce::ModalCallbackFunction::create([safe,safeDialog](int result){
    if(!safe||!safeDialog||safe->numericDialog.get()!=safeDialog.getComponent())return;
    const auto text=safeDialog->getTextEditorContents("valor").trim();
    if(result&&text.isNotEmpty()&&text.containsOnly("0123456789.-+")){
     juce::Slider::ScopedDragNotification gesture(*safe);safe->setValue(juce::jlimit(safe->getMinimum(),safe->getMaximum(),text.getDoubleValue()),juce::sendNotificationSync);
    }
    if(safe){safe->numericDialog.reset();safe->entradaNumerica=false;}
   }),false);return;
  }
  entradaNumerica=false;startY=e.position.y;startValue=getValue();dragEvent=std::make_unique<juce::MouseEvent>(e);juce::Slider::mouseDown(e);
 }
 void mouseDrag(const juce::MouseEvent& e)override{if(entradaNumerica)return;const auto delta=(startY-e.position.y)/140.0*(getMaximum()-getMinimum())*(e.mods.isShiftDown()?.15:1);startValue+=delta;startY=e.position.y;setValue(startValue,juce::sendNotificationSync);}
 void mouseUp(const juce::MouseEvent& e)override{if(!entradaNumerica)juce::Slider::mouseUp(e);dragEvent.reset();}
 void animar(){const double alvo=valueToProportionOfLength(getValue());auto& props=getProperties();double atual=props.getWithDefault("posicaoAnimada",alvo);atual+=(alvo-atual)*.33;if(std::abs(alvo-atual)<.0001)atual=alvo;props.set("posicaoAnimada",atual);repaint();}

private:std::unique_ptr<juce::AlertWindow> numericDialog;std::unique_ptr<juce::MouseEvent> dragEvent;bool entradaNumerica=false;float startY=0;double startValue=0;
};
class Lever:public juce::Button {
public:
 Lever(const ParamSpec& p):Button(p.name),spec(p){}float value=0;
 void paintButton(juce::Graphics&,bool,bool)override;
 ParamSpec spec;
};
class PresetBrowser;
class VocaEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit VocaEditor(VocaProcessor&);~VocaEditor()override;
 void paint(juce::Graphics&)override;void resized()override;
private:
 void timerCallback()override;void updateProgram();
 VocaProcessor& processor;VocaLook look;juce::Image face;
 std::vector<std::unique_ptr<NativeKnob>> knobs;std::vector<const ParamSpec*> knobSpecs;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 std::vector<std::unique_ptr<Lever>> levers;
 juce::TextButton presetsButton,previous{"<"},next{">"},bypass{"IN"},compare{"COMPARE"},copy{"COPY"},paste{"PASTE"},options{"OPTIONS"};
 juce::ValueTree clipboard,comparison;std::unique_ptr<juce::DocumentWindow> browserWindow;
 juce::TooltipWindow tooltip{this,600};float needle=-45;
 voca::EditorGraphics graphics;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocaEditor)
};
