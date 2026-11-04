#include "../Source/Dsp.h"
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static double harmonic(const std::vector<double>& samples,double frequency,double rate){double re=0,im=0;for(size_t i=0;i<samples.size();++i){const auto phase=6.283185307179586*frequency*i/rate;re+=samples[i]*std::cos(phase);im+=samples[i]*std::sin(phase);}return 2*std::hypot(re,im)/samples.size();}
static std::vector<double> render(double amplitude,double frequency,double rate=48000){voca::TubeStage stage;stage.prepare(rate);std::vector<double> result;for(int i=0;i<int(rate*2);++i){const auto y=stage.tick(amplitude*std::sin(6.283185307179586*frequency*i/rate));check(std::isfinite(y),"Non-finite tube output");if(i>=int(rate))result.push_back(y);}return result;}
int main(){try{
 const auto quiet=render(.001,1000),hot=render(.8,1000);
 const auto fundamental=harmonic(hot,1000,48000),second=harmonic(hot,2000,48000),third=harmonic(hot,3000,48000);
 check(std::abs(harmonic(quiet,1000,48000)/.001-1)<.01,"Tube small-signal gain differs from unity");
 check(second/fundamental>.015,"Tube has no measurable even harmonics");check(third/fundamental>.01,"Tube has no measurable odd harmonics");
 check(fundamental/.8<.95,"Hot signal was not softly compressed");
 double mean=0;for(auto sample:hot)mean+=sample;check(std::abs(mean/hot.size())<1e-6,"Tube leaves a DC offset");
 const auto high=render(2,9000);std::vector<double> naive(high.size());const double bias=std::tanh(.22);
 for(size_t i=0;i<naive.size();++i)naive[i]=(std::tanh(.95*2*std::sin(6.283185307179586*9000*i/48000)+.22)-bias)/(.95*(1-bias*bias));
 const auto alias=harmonic(high,21000,48000),rawAlias=harmonic(naive,21000,48000);
 check(alias<rawAlias*.8,"Antiderivative processing did not reduce the measured aliased third harmonic");
 for(double rate:{22050.,44100.,48000.,96000.}){voca::TubeStage stage;stage.prepare(rate);for(int i=0;i<1000;++i)check(stage.tick(0)==0,"Silence generates output");for(double x:{1e-12,1.,-1.,1000.,-1000.,1e30,-1e30})check(std::isfinite(stage.tick(x)),"Extreme input is not finite");}
 std::cout<<"PASS: tube unity low-level gain, soft compression, H2 "<<voca::gainToDb(second/fundamental)<<" dBc, H3 "<<voca::gainToDb(third/fundamental)<<" dBc, DC removal; aliased H3 reduced "<<voca::gainToDb(rawAlias/alias)<<" dB at 9 kHz/48 kHz; silence/rates/extremes.\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
