#include "../Source/PluginEditor.h"
#include "BinaryData.h"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct Gesture final : juce::AudioProcessorParameter::Listener {
    int starts=0, ends=0;
    void parameterValueChanged(int,float) override {}
    void parameterGestureChanged(int,bool beginning) override { if(beginning)++starts;else ++ends; }
};
class Lifecycle final : private juce::Timer {
public:
    Lifecycle() : file(juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("vocabox-lifecycle", ".json")) { startTimer(3); }
    ~Lifecycle() override { cleanup(); file.deleteFile(); }
    bool passed=false;
private:
    void cleanup() {
        editor.reset();
        running.store(false);
        if (worker.joinable()) worker.join(); // Host waits for the in-flight callback before releasing the processor.
        if(processor && parameter) parameter->removeListener(&gestures);
        parameter=nullptr;processor.reset();
    }
    void mouse(juce::ModifierKeys mods, juce::Point<float> point) {
        auto* peer=editor->getPeer();require(peer!=nullptr,"Native editor peer missing");
        peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse,point,mods,1,0,juce::Time::currentTimeMillis());
    }
    void timerCallback() override {
      try {
        if(!editor) {
            if(!processor) {
                processor=std::make_unique<VocaProcessor>(file);
                if(state.getSize())processor->setStateInformation(state.getData(),(int)state.getSize());
                processor->prepareToPlay(instance%2 ? 96000 : 44100,instance%2 ? 127 : 256);
                processor->setNonRealtime(true);
                juce::AudioBuffer<float> zero(2,0),offline(2,64);juce::MidiBuffer midi;offline.clear();
                processor->processBlock(zero,midi);processor->processBlock(offline,midi);processor->setNonRealtime(false);
                processor->getStateInformation(state);
                running.store(true);worker=std::thread([this]{
                    juce::AudioBuffer<float> audio(2,256);juce::MidiBuffer midi;
                    while(running.load()) { audio.clear();processor->processBlock(audio,midi);++blocks;std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
                });
            }
            editor.reset(processor->createEditor());editor->addToDesktop(juce::ComponentPeer::windowIsTemporary);editor->setVisible(true);
            for(auto* child:editor->getChildren())if(auto* candidate=dynamic_cast<NativeKnob*>(child)) {knob=candidate;break;}
            require(knob!=nullptr,"Native knob missing");
            for(auto& spec:specs)if(knob->getName()==spec.name)parameter=processor->params.getParameter(spec.id);
            require(parameter!=nullptr,"Knob attachment missing");parameter->addListener(&gestures);gestures.starts=gestures.ends=0;
            return;
        }
        if(!interacted) {
            const auto point=knob->getBounds().getCentre().toFloat();
            mouse({},point);
            if(cycle==1) {
                mouse(juce::ModifierKeys::rightButtonModifier,point);
                mouse({},point);
                require(juce::Component::getNumCurrentlyModalComponents()>0,"Numeric dialog did not open");
            } else if(cycle==2) {
                knob->setPopupDisplayEnabled(true,false,editor.get());
                mouse(juce::ModifierKeys::leftButtonModifier,point);
                mouse(juce::ModifierKeys::leftButtonModifier,point.translated(0,-12));
                require(gestures.starts==1 && gestures.ends==0,"Real drag did not open an automation gesture");
                require(knob->getCurrentPopupDisplay()!=nullptr,"Drag popup missing");
            }
            interacted=true;return;
        }
        if(cycle==1 && dialogStep==0) {
            auto* dialog=dynamic_cast<juce::AlertWindow*>(juce::Component::getCurrentlyModalComponent());
            require(dialog!=nullptr,"Numeric modal is not an AlertWindow");
            dialog->getTextEditor("valor")->setText(juce::String(knob->getMaximum()));dialog->exitModalState(1);numericDeadline=juce::Time::currentTimeMillis()+5000;dialogStep=1;return;
        }
        if(cycle==1 && dialogStep==1) {
            // JUCE delivers modal results asynchronously; wait for the actual result,
            // including on slower sanitizer builds, while the native event loop runs.
            if(gestures.ends==0 && juce::Time::currentTimeMillis()<numericDeadline)return;
            require(knob->getValue()==knob->getMaximum(),"Numeric dialog failed to apply value");
            require(gestures.starts==1 && gestures.ends==1,"Numeric entry gesture is unbalanced");
            const auto point=knob->getBounds().getCentre().toFloat();mouse(juce::ModifierKeys::rightButtonModifier,point);mouse({},point);
            require(juce::Component::getNumCurrentlyModalComponents()>0,"Numeric dialog failed to reopen");dialogStep=2;return;
        }
        // Close while the audio worker continues processing and, in cycle 2, while dragging.
        editor.reset();knob=nullptr;
        require(gestures.starts==gestures.ends,"Unbalanced automation gesture after editor destruction");
        require(juce::Component::getNumCurrentlyModalComponents()==0,"Dialog survived editor destruction");
        parameter->removeListener(&gestures);parameter=nullptr;
        interacted=false;dialogStep=0;
        if(++cycle==3) {
            cycle=0;running.store(false);worker.join();processor->releaseResources();processor.reset();
            std::cout<<"checkpoint instance "<<++instance<<" / 100, audio blocks "<<blocks.load()<<std::endl;
        }
        if(instance==100) { require(blocks.load()>0,"Audio worker never processed");passed=true;stopTimer();juce::MessageManager::getInstance()->stopDispatchLoop(); }
      } catch(const std::exception& e) {
        std::cerr<<"FAIL: "<<e.what()<<std::endl;cleanup();stopTimer();juce::MessageManager::getInstance()->stopDispatchLoop();
      }
    }
    juce::File file;juce::MemoryBlock state;
    std::unique_ptr<VocaProcessor> processor;std::unique_ptr<juce::AudioProcessorEditor> editor;
    NativeKnob* knob=nullptr;juce::AudioProcessorParameter* parameter=nullptr;Gesture gestures;
    std::atomic<bool> running{false};std::atomic<int> blocks{0};std::thread worker;
    juce::int64 numericDeadline=0;
    int instance=0,cycle=0,dialogStep=0;bool interacted=false;
};
int main() {
    juce::ScopedJuceInitialiser_GUI gui;
    std::mutex mutex;std::condition_variable condition;bool done=false;
    std::thread watchdog([&]{std::unique_lock<std::mutex> lock(mutex);if(!condition.wait_for(lock,std::chrono::seconds(120),[&]{return done;})){std::cerr<<"FAIL: lifecycle watchdog timeout"<<std::endl;std::_Exit(2);}});
    bool passed=false;
    {Lifecycle run;juce::MessageManager::getInstance()->runDispatchLoop();passed=run.passed;}
    {std::lock_guard<std::mutex> lock(mutex);done=true;}condition.notify_one();watchdog.join();
    if(passed)std::cout<<"PASS: 100 instances, 300 native editor cycles; owned numeric dialogs, real gestures/popups, active audio, state, rate/buffer changes and offline/zero buffers."<<std::endl;
    return passed?0:1;
}
