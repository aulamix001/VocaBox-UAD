#pragma once
#include "PluginProcessor.h"
class VocaLook final:public juce::LookAndFeel_V4 {
public: void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
};
class NativeKnob:public juce::Slider {
public:
 NativeKnob(){setSliderStyle(RotaryVerticalDrag);setTextBoxStyle(NoTextBox,false,0,0);setMouseDragSensitivity(140);setMouseCursor(juce::MouseCursor::UpDownResizeCursor);}
 void mouseDown(const juce::MouseEvent& e)override{if(e.mods.isPopupMenu()||e.mods.isCtrlDown()){entradaNumerica=true;auto* janela=new juce::AlertWindow(getName(),"Digite o valor",juce::MessageBoxIconType::NoIcon);janela->addTextEditor("valor",juce::String(getValue(),2));janela->addButton("Aplicar",1,juce::KeyPress(juce::KeyPress::returnKey));janela->addButton("Cancelar",0,juce::KeyPress(juce::KeyPress::escapeKey));auto seguro=juce::Component::SafePointer<NativeKnob>(this);janela->enterModalState(true,juce::ModalCallbackFunction::create([seguro,janela](int resultado){if(seguro&&resultado){const auto texto=janela->getTextEditorContents("valor").trim();if(texto.isNotEmpty()&&texto.containsOnly("0123456789.-+")){seguro->startedDragging();seguro->setValue(juce::jlimit(seguro->getMinimum(),seguro->getMaximum(),texto.getDoubleValue()),juce::sendNotificationSync);seguro->stoppedDragging();}}}),true);return;}entradaNumerica=false;startY=e.position.y;startValue=getValue();juce::Slider::mouseDown(e);}
 void mouseDrag(const juce::MouseEvent& e)override{if(entradaNumerica)return;const auto delta=(startY-e.position.y)/140.0*(getMaximum()-getMinimum())*(e.mods.isShiftDown()?.15:1);startValue+=delta;startY=e.position.y;setValue(startValue,juce::sendNotificationSync);}
 void mouseUp(const juce::MouseEvent& e)override{if(!entradaNumerica)juce::Slider::mouseUp(e);}
 void animar(){const double alvo=valueToProportionOfLength(getValue());auto& props=getProperties();double atual=props.getWithDefault("posicaoAnimada",alvo);atual+=(alvo-atual)*.33;if(std::abs(alvo-atual)<.0001)atual=alvo;props.set("posicaoAnimada",atual);repaint();}

private:bool entradaNumerica=false;float startY=0;double startValue=0;
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
 juce::TextButton presetsButton,previous{"<"},next{">"},bypass{"IN"},compare{"COMPARE"},copy{"COPY"},paste{"PASTE"};
 juce::ValueTree clipboard,comparison;std::unique_ptr<juce::DocumentWindow> browserWindow;
 juce::TooltipWindow tooltip{this,600};float needle=-45;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocaEditor)
};
