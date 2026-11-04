#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
juce::AudioProcessorValueTreeState::ParameterLayout VocaProcessor::makeLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 for(const auto& p:specs){
  auto labels=juce::StringArray::fromTokens(p.labels,"|","");
  auto attributes=juce::AudioParameterFloatAttributes{}.withStringFromValueFunction([labels](float value,int){
   return !labels.isEmpty()?labels[juce::jlimit(0,labels.size()-1,juce::roundToInt(value))]:juce::String(value,1);
  });
  layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{p.id,1},p.name,juce::NormalisableRange<float>{p.min,p.max,p.step},p.initial,attributes));
 }
 return layout;
}
VocaProcessor::VocaProcessor(juce::File biblioteca):AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"VocaBoxState",makeLayout()),armazenamento(biblioteca==juce::File{}?arquivoBiblioteca():biblioteca){
 for(size_t i=0;i<specs.size();++i)raw[i]=params.getRawParameterValue(specs[i].id);
 const auto json=juce::JSON::parse(juce::String::fromUTF8(BinaryData::presets_json,BinaryData::presets_jsonSize));
 if(auto* presets=json["presets"].getArray())for(const auto& p:*presets){Preset preset;preset.name=p["name"].toString();preset.id=p["id"].toString();preset.group=p["group"].toString();preset.artist=p["artist"].toString();preset.settings=p["settings"];if(auto* tags=p["tags"].getArray())for(auto& t:*tags)preset.tags.add(t.toString());bank.push_back(preset);}
 const auto salvo=juce::JSON::parse(armazenamento);
 if(auto* usuarios=salvo["userPresets"].getArray())for(const auto& u:*usuarios){Preset novo;novo.name=u["name"].toString();novo.id=u["id"].toString();novo.group=u["group"].toString();novo.artist=u["artist"].toString();novo.isUser=true;novo.settings=u["settings"];if(auto* tags=u["tags"].getArray())for(auto& t:*tags)novo.tags.add(t.toString());bool valido=novo.name.isNotEmpty()&&novo.settings.isObject();for(const auto& spec:specs){const auto valor=novo.settings[spec.id];valido=valido&&(valor.isInt()||valor.isDouble())&&std::isfinite((double)valor)&&(double)valor>=spec.min&&(double)valor<=spec.max;}if(valido)bank.push_back(novo);}
 if(auto* favoritos=salvo["favorites"].getArray())for(auto& p:bank)for(const auto& id:*favoritos)if(p.id==id.toString())p.favorite=true;
 for(size_t i=0;i<bank.size();++i)if(bank[i].name==juce::String::fromUTF8("Compressão Forte"))setCurrentProgram((int)i);
}
float VocaProcessor::value(const char* id)const {if(auto* p=params.getRawParameterValue(id))return p->load();return 0;}
void VocaProcessor::set(const char* id,float v){if(auto* p=params.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}}
void VocaProcessor::setCurrentProgram(int i){if(!juce::isPositiveAndBelow(i,bank.size()))return;currentProgram.store(i);for(const auto& p:specs){const auto v=bank[(size_t)i].settings.getProperty(p.id,p.initial);if(auto* parameter=params.getParameter(p.id))parameter->setValueNotifyingHost(parameter->convertTo0to1((float)v));}}
void VocaProcessor::prepareToPlay(double fs,int){engine.prepare(fs);}
bool VocaProcessor::isBusesLayoutSupported(const BusesLayout& b)const {return b.getMainInputChannelSet()==b.getMainOutputChannelSet()&&(b.getMainInputChannelSet()==juce::AudioChannelSet::mono()||b.getMainInputChannelSet()==juce::AudioChannelSet::stereo());}
void VocaProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals noDenormals;midi.clear();std::array<float,25> v{};for(size_t i=0;i<v.size();++i)v[i]=raw[i]->load(std::memory_order_relaxed);
 voca::Settings s;s.power=v[0]>.5f;s.hpf=(int)v[1];s.phase=v[2]>.5f;s.mic=v[3]>.5f;s.input=v[4];s.gain=v[5];s.link=v[6]>.5f;s.compress=v[7]>.5f;s.threshold=v[8];s.attack=(int)v[9];s.release=(int)v[10];s.deess=v[12]>.5f;s.deFreq=(int)v[13];s.deThreshold=v[14];s.output=v[15];s.eq=v[16]>.5f;s.xfmr=v[17]>.5f;s.lowPeak=v[18];s.midDip=v[19];s.hiPeak=v[20];s.lowFreq=(int)v[21];s.midFreq=(int)v[22];s.hiFreq=(int)v[23];s.bypass=v[24]>.5f;
 engine.configure(s);if(b.getNumChannels()>0&&b.getNumSamples()>0)engine.process(b.getArrayOfWritePointers(),b.getNumChannels(),b.getNumSamples());
 const auto m=engine.getMeters();const float decay=.85f;
 const float levels[]={(float)m.input,(float)m.pre,(float)m.output,(float)m.gr,(float)m.ds};
 for(size_t i=0;i<5;++i)meterValues[i].store(std::max(levels[i],meterValues[i].load()*decay));compressorGR.store((float)m.gr);deesserGR.store((float)m.ds);
}
void VocaProcessor::getStateInformation(juce::MemoryBlock& data){
 auto state=params.copyState();const int program=juce::jlimit(0,(int)bank.size()-1,currentProgram.load());
 state.setProperty("program",program,nullptr);state.setProperty("programId",bank[(size_t)program].id,nullptr);
 state.setProperty("factoryCount",(int)std::count_if(bank.begin(),bank.end(),[](const Preset& p){return !p.isUser;}),nullptr);
 state.setProperty("graphicsMode",softwareGraphics.load()?0:1,nullptr);
 if(auto xml=state.createXml())copyXmlToBinary(*xml,data);
}
void VocaProcessor::setStateInformation(const void* data,int size){
 if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(params.state.getType())){
  auto state=juce::ValueTree::fromXml(*xml);softwareGraphics.store((int)state.getProperty("graphicsMode",CHI_WINDOWS_SOFTWARE_GRAPHICS?0:1)==0);
  int selected=-1;const auto id=state.getProperty("programId").toString();
  if(id.isNotEmpty())for(size_t i=0;i<bank.size();++i)if(bank[i].id==id){selected=(int)i;break;}
  if(selected<0){
   selected=(int)state.getProperty("program",0);
   // Older sessions stored user programs immediately after the original 50 factories.
   const int savedFactories=(int)state.getProperty("factoryCount",50);
   const int factories=(int)std::count_if(bank.begin(),bank.end(),[](const Preset& p){return !p.isUser;});
   if(id.isEmpty() && selected>=savedFactories){
    const int shifted=selected+factories-savedFactories;
    selected=juce::isPositiveAndBelow(shifted,bank.size())&&bank[(size_t)shifted].isUser?shifted:0;
   }
  }
  params.replaceState(state);currentProgram.store(juce::jlimit(0,(int)bank.size()-1,selected));
 }
}
juce::AudioProcessorEditor* VocaProcessor::createEditor(){return new VocaEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VocaProcessor();}

// Biblioteca editável separada dos snapshots de fábrica e do estado do host.
juce::File VocaProcessor::arquivoBiblioteca(){return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("VocaBox/biblioteca-v2.json");}
void VocaProcessor::persistirBiblioteca(){juce::DynamicObject::Ptr objeto=new juce::DynamicObject;juce::Array<juce::var> favoritos,usuarios;for(const auto& p:bank){if(p.favorite)favoritos.add(p.id);if(p.isUser){juce::DynamicObject::Ptr item=new juce::DynamicObject;item->setProperty("id",p.id);item->setProperty("name",p.name);item->setProperty("group",p.group);item->setProperty("artist",p.artist);item->setProperty("settings",p.settings);juce::Array<juce::var> tags;for(auto& t:p.tags)tags.add(t);item->setProperty("tags",tags);usuarios.add(juce::var(item.get()));}}objeto->setProperty("favorites",favoritos);objeto->setProperty("userPresets",usuarios);const auto arquivo=armazenamento;arquivo.getParentDirectory().createDirectory();arquivo.replaceWithText(juce::JSON::toString(juce::var(objeto.get())));}
int VocaProcessor::salvarPreset(const juce::String& nome,const juce::StringArray& tags){if(nome.trim().isEmpty())return -1;for(auto& p:bank)if(p.name.equalsIgnoreCase(nome.trim()))return -1;Preset novo;novo.name=nome.trim();novo.id=juce::Uuid().toString();novo.group=bank[(size_t)getCurrentProgram()].group;novo.artist=bank[(size_t)getCurrentProgram()].artist;novo.tags=tags;novo.isUser=true;juce::DynamicObject::Ptr valores=new juce::DynamicObject;for(auto& s:specs)valores->setProperty(s.id,value(s.id));novo.settings=juce::var(valores.get());bank.push_back(novo);currentProgram.store((int)bank.size()-1);persistirBiblioteca();return getCurrentProgram();}
bool VocaProcessor::renomearPreset(int i,const juce::String& nome){if(!juce::isPositiveAndBelow(i,bank.size())||!bank[(size_t)i].isUser||nome.trim().isEmpty())return false;for(size_t n=0;n<bank.size();++n)if((int)n!=i&&bank[n].name.equalsIgnoreCase(nome.trim()))return false;bank[(size_t)i].name=nome.trim();persistirBiblioteca();return true;}
bool VocaProcessor::excluirPreset(int i){if(!juce::isPositiveAndBelow(i,bank.size())||!bank[(size_t)i].isUser)return false;const int atual=currentProgram.load();bank.erase(bank.begin()+i);if(atual==i)setCurrentProgram(0);else if(atual>i)currentProgram.store(atual-1);persistirBiblioteca();return true;}
void VocaProcessor::alternarFavorito(int i){if(juce::isPositiveAndBelow(i,bank.size())){bank[(size_t)i].favorite=!bank[(size_t)i].favorite;persistirBiblioteca();}}
