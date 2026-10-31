#include "../Source/Dsp.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <random>
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Render {double rms=0,gr=0,ds=0,peak=0;};
Render run(voca::Settings s,double hz,double amplitude,double fs=48000,int seconds=4){
 voca::Engine engine;engine.prepare(fs);engine.configure(s);std::array<std::vector<float>,2> b;for(auto& c:b)c.resize(256);float* data[]={b[0].data(),b[1].data()};double sum=0;int count=0;Render out;
 for(int start=0;start<int(seconds*fs);start+=256){const int n=std::min(256,int(seconds*fs)-start);for(int i=0;i<n;++i)b[0][i]=b[1][i]=(float)(amplitude*std::sin((start+i)*6.283185307179586*hz/fs));engine.process(data,2,n);for(int i=0;i<n;++i){check(std::isfinite(b[0][i]),"Non-finite audio");if(start>seconds*fs*.75){sum+=b[0][i]*b[0][i];++count;}out.peak=std::max(out.peak,(double)std::abs(b[0][i]));}out.gr=engine.getMeters().gr;out.ds=engine.getMeters().ds;}
 out.rms=std::sqrt(sum/std::max(1,count));return out;
}
int main(){try{
 voca::Settings neutral;neutral.input=5;neutral.gain=40;neutral.output=0;neutral.compress=false;neutral.deess=false;neutral.xfmr=false;
 auto baseline=run(neutral,1000,.1);
 auto strong=neutral;strong.compress=true;strong.threshold=10;strong.attack=0;strong.release=4;
 auto reduced=run(strong,1000,.1);check(reduced.rms<baseline.rms*.55,"Compressor did not reduce audio");check(reduced.gr>8,"Strong compression not achieved");check(reduced.gr<=16.01,"Compressor maximum exceeded");
 auto mild=strong;mild.threshold=3;check(run(mild,1000,.1).rms>reduced.rms,"Threshold direction reversed");
 auto bypass=strong;bypass.bypass=true;check(std::abs(run(bypass,1000,.1).rms-.1/std::sqrt(2.0))<.001,"Bypass not transparent");
 auto off=neutral;off.power=false;check(run(off,1000,.1).rms<1e-7,"Power off did not mute");
 auto hp=neutral;hp.hpf=2;check(run(hp,30,.1).rms<run(neutral,30,.1).rms*.3,"High pass not working");
 auto eq=neutral;eq.lowPeak=10;eq.lowFreq=4;check(run(eq,100,.01).rms>run(neutral,100,.01).rms*2.5,"Low peak EQ not working");eq.eq=false;check(std::abs(run(eq,100,.01).rms-run(neutral,100,.01).rms)<.0001,"EQ bypass not flat");
 auto dip=neutral;dip.midDip=10;dip.midFreq=4;check(run(dip,1000,.01).rms<run(neutral,1000,.01).rms*.4,"Mid dip not working");
 auto ds=neutral;ds.deess=true;ds.deThreshold=10;ds.deFreq=0;check(run(ds,3000,.1).rms<baseline.rms*.5,"De-esser not reducing selected band");check(run(ds,200,.1).rms>baseline.rms*.8,"De-esser incorrectly suppresses low frequencies");
 for(double fs:{44100.,48000.,96000.}){auto r=run(strong,1000,.2,fs,2);check(r.gr>8,"Sample rate dependent compression");}
 // Stereo link: right-channel transient must reduce a simultaneous quiet left channel.
 voca::Engine linked,separate;linked.prepare(48000);separate.prepare(48000);strong.link=true;linked.configure(strong);strong.link=false;separate.configure(strong);
 std::vector<float> l(256),r(256),l2(256),r2(256);float* a[]={l.data(),r.data()};float* b[]={l2.data(),r2.data()};
 double el=0,es=0;for(int block=0;block<500;++block){for(int i=0;i<256;++i){auto v=std::sin((block*256+i)*6.283185307179586*1000/48000);l[i]=l2[i]=(float)(.001*v);r[i]=r2[i]=(float)(.5*v);}linked.process(a,2,256);separate.process(b,2,256);if(block>450)for(int i=0;i<256;++i){el+=l[i]*l[i];es+=l2[i]*l2[i];}}check(el<es*.2,"Stereo link not working");
 // Silence and extreme settings must stay finite, including all EQ frequencies at Nyquist-limited rates.
 auto extreme=strong;extreme.lowPeak=extreme.hiPeak=10;extreme.midDip=10;extreme.lowFreq=extreme.midFreq=extreme.hiFreq=10;extreme.deess=true;extreme.deThreshold=10;for(double fs:{22050.,44100.,96000.}){run(extreme,8000,.99,fs,2);check(run(extreme,1000,0,fs,1).rms==0,"Silence generated noise");}
 std::cout<<"PASS: compression "<<reduced.gr<<" dB; threshold, bypass, power, HPF, EQ, de-esser, stereo link, sample rates, silence and extremes.\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
