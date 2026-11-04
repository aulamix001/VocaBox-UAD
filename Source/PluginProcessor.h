#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Dsp.h"
#include "Parameters.h"
struct Preset {juce::String name,id,group,artist;juce::StringArray tags;juce::var settings;bool favorite=false,isUser=false;};
class VocaProcessor final:public juce::AudioProcessor {
public:
 explicit VocaProcessor(juce::File biblioteca={});
 const juce::String getName()const override{return "VocaBox";}
 void prepareToPlay(double,int)override; void releaseResources()override{}
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 juce::AudioProcessorEditor* createEditor()override; bool hasEditor()const override{return true;}
 double getTailLengthSeconds()const override{return 5;}
 bool acceptsMidi()const override{return false;} bool producesMidi()const override{return false;}
 int getNumPrograms()override{return (int)bank.size();}
 int getCurrentProgram()override{return currentProgram.load();}
 void setCurrentProgram(int)override;
 const juce::String getProgramName(int i)override{return juce::isPositiveAndBelow(i,bank.size())?bank[(size_t)i].name:juce::String{};}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 juce::AudioProcessorParameter* getBypassParameter()const override{return params.getParameter("bypass");}
 float value(const char* id)const;void set(const char* id,float value);
 juce::AudioProcessorValueTreeState params;
 std::vector<Preset> bank;
 void persistirBiblioteca();int salvarPreset(const juce::String&,const juce::StringArray&);
 bool renomearPreset(int,const juce::String&);bool excluirPreset(int);
 void alternarFavorito(int);static juce::File arquivoBiblioteca();

 std::atomic<bool> softwareGraphics{CHI_WINDOWS_SOFTWARE_GRAPHICS != 0};
 std::array<std::atomic<float>,5> meterValues{};std::atomic<float> compressorGR{0},deesserGR{0};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
 juce::File armazenamento;
 std::array<std::atomic<float>*,25> raw{};
 std::atomic<int> currentProgram{0};voca::Engine engine;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocaProcessor)
};
