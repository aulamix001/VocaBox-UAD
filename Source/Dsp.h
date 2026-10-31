#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace voca {
inline double dbToGain(double d) { return std::pow(10.0, d / 20.0); }
inline double gainToDb(double g) { return 20.0 * std::log10(std::max(1.0e-12, g)); }
struct Settings {
    bool power=true, bypass=false, phase=false, mic=false, link=true, compress=true, deess=false, eq=true, xfmr=true;
    int hpf=0, attack=0, release=4, deFreq=2, lowFreq=4, midFreq=4, hiFreq=5;
    double input=5, gain=45, threshold=8, deThreshold=5, output=0, lowPeak=0, midDip=0, hiPeak=0;
};
struct Meters { double input=0, pre=0, output=0, gr=0, ds=0; };
struct Coeff { double b0=1,b1=0,b2=0,a1=0,a2=0; };
inline Coeff filter(double fs, double hz, double q, double gain, int type) {
    hz=std::clamp(hz, 10.0, fs * .45);
    const auto w=6.283185307179586*hz/fs, c=std::cos(w), sn=std::sin(w), alpha=sn/(2*q);
    if(type==0) { const double k=std::tan(w*.5), b=1/(1+k); return {b,-b,0,(k-1)/(k+1),0}; }
    if(type==1) { const double a0=1+alpha; return {alpha/a0,0,-alpha/a0,-2*c/a0,(1-alpha)/a0}; }
    const double A=dbToGain(gain/2), a0=1+alpha/A;
    return {(1+alpha*A)/a0,-2*c/a0,(1-alpha*A)/a0,-2*c/a0,(1-alpha/A)/a0};
}
struct Biquad {
    Coeff c{}, target{}; double z1=0,z2=0;
    double tick(double x, double smooth) {
        c.b0+=smooth*(target.b0-c.b0);c.b1+=smooth*(target.b1-c.b1);c.b2+=smooth*(target.b2-c.b2);
        c.a1+=smooth*(target.a1-c.a1);c.a2+=smooth*(target.a2-c.a2);
        const double y=c.b0*x+z1;z1=c.b1*x-c.a1*y+z2;z2=c.b2*x-c.a2*y;return y;
    }
    void reset(){z1=z2=0;c=target;}
};
// Independent, allocation-free stereo processing core. This is an approximation,
// not a circuit model or a bit-identical emulation of a commercial plugin.
class Engine {
public:
    static constexpr std::array<double,11> lows={20,35,50,70,100,150,200,300,500,700,1000};
    static constexpr std::array<double,11> mids={200,300,500,700,1000,1500,2000,3000,4000,5000,7000};
    static constexpr std::array<double,11> highs={1500,2000,3000,4000,5000,6400,8000,10000,12000,16000,20000};
    static constexpr std::array<double,5> attacks={.004,.008,.016,.035,.070};
    static constexpr std::array<double,5> releases={.3,.5,1,2,5};
    void prepare(double sampleRate){fs=sampleRate; for(auto& c:channels)c=Channel{}; inputSmooth=preSmooth=outputSmooth=wetSmooth=powerSmooth=1; meters={};}
    void configure(const Settings& p){
        settings=p;
        inputTarget=dbToGain((p.input-5)*3); // 0–10 UI → -15…+15 dB input trim.
        preTarget=dbToGain(p.gain-40+(p.mic?20:0));
        outputTarget=dbToGain(p.output);
        attack=std::exp(-1.0/(fs*attacks[std::clamp(p.attack,0,4)]));
        release=std::exp(-1.0/(fs*releases[std::clamp(p.release,0,4)]));
        smooth=1-std::exp(-1.0/(fs*.02));
        threshold=6-p.threshold*4.2; deThreshold=6-p.deThreshold*4.2;
        for(auto& ch:channels){
            ch.hp.target=filter(fs,p.hpf==2?120:80,.707,0,0);
            ch.low.target=filter(fs,lows[std::clamp(p.lowFreq,0,10)],.75,p.eq?p.lowPeak:0,2);
            ch.mid.target=filter(fs,mids[std::clamp(p.midFreq,0,10)],.85,p.eq?-p.midDip:0,2);
            ch.high.target=filter(fs,highs[std::clamp(p.hiFreq,0,10)],.75,p.eq?p.hiPeak:0,2);
            constexpr double freqs[]={3000,6000,9000,12000,6000};
            ch.band.target=filter(fs,freqs[std::clamp(p.deFreq,0,4)],1.0,0,1);
        }
    }
    void process(float* const* data,int count,int samples){
        count=std::clamp(count,1,2); Meters block{};
        const double da=std::exp(-1/(fs*.002)),dr=std::exp(-1/(fs*.5));
        for(int n=0;n<samples;++n){
            inputSmooth+=smooth*(inputTarget-inputSmooth);preSmooth+=smooth*(preTarget-preSmooth);
            outputSmooth+=smooth*(outputTarget-outputSmooth);wetSmooth+=smooth*((settings.bypass?0.0:1.0)-wetSmooth);
            powerSmooth+=smooth*((settings.power?1.0:0.0)-powerSmooth);
            std::array<double,2> x{},dry{},equalized{},bands{};
            for(int c=0;c<count;++c){dry[c]=std::isfinite(data[c][n])?data[c][n]:0;block.input=std::max(block.input,std::abs(dry[c]));x[c]=dry[c]*inputSmooth*(settings.phase&&settings.mic?-1:1);auto h=channels[c].hp.tick(x[c],smooth);if(settings.hpf)x[c]=h;}
            const double linked=std::max(std::abs(x[0]),count>1?std::abs(x[1]):0.0);
            for(int c=0;c<count;++c){auto& ch=channels[c];
                const double peak=settings.link?linked:std::abs(x[c]);
                ch.env=follow(ch.env,peak,attack,release);
                const double reduction=settings.compress?softReduction(gainToDb(ch.env)-threshold,3,6,16):0;
                ch.gr=follow(ch.gr,reduction,attack,settings.compress?release:1-smooth);block.gr=std::max(block.gr,ch.gr);
                double y=x[c]*dbToGain(-ch.gr)*preSmooth;
                // Gentle tube-like saturation; no claim of exact triode or transformer modelling.
                y=std::tanh(y*.65)/.65;
                block.pre=std::max(block.pre,std::abs(y));
                y=ch.high.tick(ch.mid.tick(ch.low.tick(y,smooth),smooth),smooth);
                equalized[c]=y;bands[c]=ch.band.tick(y,smooth);
            }
            const double linkedDS=settings.deFreq==4?std::max(std::abs(equalized[0]),std::abs(equalized[1])):std::max(std::abs(bands[0]),std::abs(bands[1]));
            for(int c=0;c<count;++c){auto& ch=channels[c];double y=equalized[c];const double band=bands[c];
                const double detector=settings.link?linkedDS:std::abs(settings.deFreq==4?y:band);
                ch.dsEnv=follow(ch.dsEnv,detector,da,dr);
                const double ds=settings.deess?softReduction(gainToDb(ch.dsEnv)-deThreshold,10,6,32):0;
                ch.dsGr=follow(ch.dsGr,ds,da,settings.deess?dr:1-smooth);block.ds=std::max(block.ds,ch.dsGr);
                y=settings.deFreq==4?y*dbToGain(-ch.dsGr):y-band*(1-dbToGain(-ch.dsGr));
                if(settings.xfmr)y=std::tanh(y*.2)/.2;
                y*=outputSmooth;block.output=std::max(block.output,std::abs(y));
                y=(dry[c]*(1-wetSmooth)+y*wetSmooth)*powerSmooth;
                data[c][n]=std::isfinite(y)?float(y):0.0f;
            }
        }
        meters=block;
    }
    Meters getMeters()const{return meters;}
    static double softReduction(double over,double ratio,double knee,double maximum){
        if(over<=-knee/2)return 0;
        const double slope=1-1/ratio;
        return std::min(maximum,over<knee/2?slope*(over+knee/2)*(over+knee/2)/(2*knee):slope*over);
    }
private:
    struct Channel {Biquad hp,low,mid,high,band;double env=0,gr=0,dsEnv=0,dsGr=0;};
    static double follow(double old,double target,double a,double r){const auto k=target>old?a:r;return target+k*(old-target);}
    std::array<Channel,2> channels{};Settings settings{};Meters meters{};
    double fs=48000,inputSmooth=1,preSmooth=1,outputSmooth=1,wetSmooth=1,powerSmooth=1;
    double inputTarget=1,preTarget=1,outputTarget=1,smooth=.001,threshold=-20,deThreshold=-20,attack=.99,release=.999;
};
}
