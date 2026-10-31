// Minimal ABI smoke host: loads the compiled VST3, creates its audio component,
// processes stereo buffers, and exercises session state through the VST3 API.
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
using namespace Steinberg;using namespace Steinberg::Vst;
void check(bool b,const char* text){if(!b)throw std::runtime_error(text);}
struct Stream final:IBStream {
 std::vector<char> bytes;int64 position=0;
 tresult PLUGIN_API queryInterface(const TUID id,void** out)override{if(!std::memcmp(id,IBStream_iid,16)||!std::memcmp(id,FUnknown_iid,16)){*out=this;return kResultOk;}*out=nullptr;return kNoInterface;}
 uint32 PLUGIN_API addRef()override{return 1;}uint32 PLUGIN_API release()override{return 1;}
 tresult PLUGIN_API read(void* buffer,int32 n,int32* readBytes)override{const auto count=(int32)std::min<int64>(n,(int64)bytes.size()-position);if(count<0)return kResultFalse;std::memcpy(buffer,bytes.data()+position,(size_t)count);position+=count;if(readBytes)*readBytes=count;return kResultOk;}
 tresult PLUGIN_API write(void* buffer,int32 n,int32* written)override{bytes.resize((size_t)std::max<int64>(bytes.size(),position+n));std::memcpy(bytes.data()+position,buffer,(size_t)n);position+=n;if(written)*written=n;return kResultOk;}
 tresult PLUGIN_API seek(int64 pos,int32 mode,int64* result)override{position=mode==kIBSeekSet?pos:mode==kIBSeekCur?position+pos:(int64)bytes.size()+pos;position=std::max<int64>(0,position);if(result)*result=position;return kResultOk;}
 tresult PLUGIN_API tell(int64* result)override{*result=position;return kResultOk;}
};
int main(int argc,char** argv){try{check(argc==2,"Pass the VST3 module file path");
#ifdef _WIN32
 auto module=LoadLibraryA(argv[1]);check(module!=nullptr,"LoadLibrary failed");
 auto init=(bool(*)())GetProcAddress(module,"InitDll");auto exit=(bool(*)())GetProcAddress(module,"ExitDll");
 auto getFactory=(IPluginFactory*(*)())GetProcAddress(module,"GetPluginFactory");
#else
 auto module=dlopen(argv[1],RTLD_NOW);if(!module)std::cerr<<dlerror()<<'\n';check(module!=nullptr,"dlopen failed");
 auto init=(bool(*)(void*))dlsym(module,"ModuleEntry");auto exit=(bool(*)())dlsym(module,"ModuleExit");
 auto getFactory=(IPluginFactory*(*)())dlsym(module,"GetPluginFactory");
#endif
 check(getFactory,"Missing GetPluginFactory");
#ifdef _WIN32
 if(init)check(init(),"InitDll failed");
#else
 if(init)check(init(module),"ModuleEntry failed");
#endif
 auto* factory=getFactory();check(factory&&factory->countClasses()>=2,"Missing component/controller classes");IComponent* component=nullptr;
 for(int32 i=0;i<factory->countClasses();++i){PClassInfo info{};factory->getClassInfo(i,&info);if(std::strcmp(info.category,kVstAudioEffectClass)==0)check(factory->createInstance(info.cid,IComponent_iid,(void**)&component)==kResultOk,"Component creation failed");}
 check(component,"Audio component missing");check(component->initialize(nullptr)==kResultOk,"Component initialization failed");
 IAudioProcessor* processor=nullptr;check(component->queryInterface(IAudioProcessor_iid,(void**)&processor)==kResultOk,"IAudioProcessor missing");
 SpeakerArrangement stereo=SpeakerArr::kStereo;check(processor->setBusArrangements(&stereo,1,&stereo,1)==kResultOk,"Stereo buses rejected");component->activateBus(kAudio,kInput,0,true);component->activateBus(kAudio,kOutput,0,true);
 ProcessSetup setup{};setup.processMode=kRealtime;setup.symbolicSampleSize=kSample32;setup.sampleRate=48000;setup.maxSamplesPerBlock=256;check(processor->setupProcessing(setup)==kResultOk,"setupProcessing failed");check(component->setActive(true)==kResultOk,"setActive failed");processor->setProcessing(true);
 float left[256]{},right[256]{},outLeft[256]{},outRight[256]{};float* ins[]={left,right};float* outs[]={outLeft,outRight};AudioBusBuffers input{},output{};input.numChannels=output.numChannels=2;input.channelBuffers32=ins;output.channelBuffers32=outs;
 ProcessData data{};data.processMode=kRealtime;data.symbolicSampleSize=kSample32;data.numSamples=256;data.numInputs=data.numOutputs=1;data.inputs=&input;data.outputs=&output;double energy=0,difference=0;
 for(int block=0;block<750;++block){for(int i=0;i<256;++i)left[i]=right[i]=float(.1*std::sin((block*256+i)*6.283185307179586*1000/48000));check(processor->process(data)==kResultOk,"process failed");for(int i=0;i<256;++i){check(std::isfinite(outLeft[i])&&std::isfinite(outRight[i]),"Nonfinite VST3 output");if(block>500){energy+=outLeft[i]*outLeft[i];difference+=std::abs(outLeft[i]-left[i]);}}}
 check(energy>1e-6&&difference>1,"VST3 did not process audio");Stream state;check(component->getState(&state)==kResultOk&&!state.bytes.empty(),"VST3 state save failed");state.position=0;check(component->setState(&state)==kResultOk,"VST3 state restore failed");
 processor->setProcessing(false);component->setActive(false);processor->release();component->terminate();component->release();factory->release();if(exit)exit();
#ifdef _WIN32
 FreeLibrary(module);
#else
 dlclose(module);
#endif
 std::cout<<"PASS: VST3 factory, component lifecycle, stereo processing, finite/nonzero altered audio, session save/restore.\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
