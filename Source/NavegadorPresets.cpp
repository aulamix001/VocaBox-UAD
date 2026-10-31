#include "NavegadorPresets.h"
PresetBrowser::PresetBrowser(VocaProcessor& p):processador(p){
 setLookAndFeel(&aparencia);for(auto* b:{&excluir,&renomear,&salvar})b->getProperties().set("acao",true);for(auto* b:{&fechar,&tags,&limpar,&favoritos,&tipo,&artista,&todos,&estrela})b->getProperties().set("semFundo",true);estrela.getProperties().set("estrela",true);fechar.getProperties().set("estrela",true);
 titulo.setText("MANLEY VOXBOX CS PRESETS",juce::dontSendNotification);titulo.setFont(fonteInterface());titulo.setJustificationType(juce::Justification::centred);
 for(auto* c:std::initializer_list<juce::Component*>{&titulo,&fechar,&busca,&tags,&limpar,&favoritos,&tipo,&artista,&todos,&rolagem,&nome,&estrela,&excluir,&renomear,&salvar,&lista,&aviso})addAndMakeVisible(c);
 busca.getProperties().set("campoBusca",true);busca.setIndents(30,0);busca.setJustification(juce::Justification::centredLeft);busca.setFont(fonteInterface());busca.setColour(juce::TextEditor::backgroundColourId,juce::Colour(tema::cores::browserPoco));busca.setColour(juce::TextEditor::outlineColourId,juce::Colour(tema::cores::browserBorda));busca.onTextChange=[this]{atualizar();};busca.setTitle("Search presets");
 nome.setColour(juce::TextEditor::backgroundColourId,juce::Colours::transparentBlack);nome.setColour(juce::TextEditor::outlineColourId,juce::Colours::transparentBlack);nome.setJustification(juce::Justification::centred);nome.setFont(fonteInterface());nome.setTitle("Preset name");
 aviso.setFont(fonteInterface(tema::fontes::nota));aviso.setColour(juce::Label::textColourId,juce::Colour(tema::cores::browserSecundario));
 rolagem.setViewedComponent(&grade,false);rolagem.setScrollBarsShown(true,false);rolagem.setScrollBarThickness(5);
 lista.setModel(this);lista.setRowHeight(tema::presets::linhaPreset);lista.setColour(juce::ListBox::backgroundColourId,juce::Colour(tema::cores::lista));lista.setColour(juce::ListBox::outlineColourId,juce::Colours::transparentBlack);
 fechar.onClick=[this]{if(auto* janela=findParentComponentOfClass<juce::DocumentWindow>())janela->setVisible(false);};
 limpar.onClick=[this]{filtros.clear();atualizar();};tags.onClick=[this]{mostrarTags=!mostrarTags;atualizar();};favoritos.onClick=[this]{somenteFavoritos=!somenteFavoritos;atualizar();};
 tipo.onClick=[this]{aba=0;atualizar();};artista.onClick=[this]{aba=1;atualizar();};todos.onClick=[this]{aba=2;atualizar();};
 estrela.onClick=[this]{processador.alternarFavorito(processador.getCurrentProgram());atualizar();};
 salvar.onClick=[this]{auto n=nome.getText();const auto& atual=processador.bank[(size_t)processador.getCurrentProgram()];if(!atual.isUser&&n==atual.name)n+=" (User)";auto categorias=atual.tags;for(auto& f:filtros)if(!f.containsChar(':'))categorias.addIfNotAlreadyThere(f);if(processador.salvarPreset(n,categorias)<0)aviso.setText(juce::String::fromUTF8("Nome vazio ou já existente."),juce::dontSendNotification);else{busca.clear();somenteFavoritos=false;atualizar();}};
 renomear.onClick=[this]{if(!processador.renomearPreset(processador.getCurrentProgram(),nome.getText()))aviso.setText(juce::String::fromUTF8("Nome vazio ou já existente."),juce::dontSendNotification);atualizar();};
 excluir.onClick=[this]{const int indice=processador.getCurrentProgram();const auto id=processador.bank[(size_t)indice].id;auto seguro=juce::Component::SafePointer<PresetBrowser>(this);juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::QuestionIcon,"Excluir preset",processador.bank[(size_t)indice].name,"Excluir","Cancelar",this,juce::ModalCallbackFunction::create([seguro,id](int resposta){if(seguro&&resposta){for(size_t i=0;i<seguro->processador.bank.size();++i)if(seguro->processador.bank[i].id==id){seguro->processador.excluirPreset((int)i);break;}seguro->atualizar();}}));};
 setSize(tema::presets::largura,tema::presets::altura);atualizar();
}
void PresetBrowser::paint(juce::Graphics& g){g.setGradientFill(juce::ColourGradient(juce::Colour(tema::cores::browserTopo),0,0,juce::Colour(tema::cores::browserBase),0,(float)getHeight(),false));g.fillAll();g.setColour(juce::Colour(tema::cores::presetSelecao));g.fillRect(8,nome.getY(),getWidth()-16,excluir.getBottom()-nome.getY()+5);g.setColour(juce::Colour(tema::cores::browserBorda));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f),3,1);auto* ativo=aba==0?&tipo:aba==1?&artista:&todos;if(mostrarTags){g.setColour(juce::Colour(tema::cores::browserTexto));auto r=ativo->getBounds();g.fillRect(r.getX(),r.getBottom()-2,r.getWidth(),2);}}
bool PresetBrowser::corresponde(const Preset& p)const{if(somenteFavoritos&&!p.favorite)return false;for(auto& f:filtros){if(f.startsWith("artist:")){if(p.artist!=f.substring(7))return false;}else if(f.startsWith("group:")){if(p.group!=f.substring(6))return false;}else if(!p.tags.contains(f))return false;}return (p.name+" "+p.group+" "+p.artist+" "+p.tags.joinIntoString(" ")).containsIgnoreCase(busca.getText());}
void PresetBrowser::desenharTags(){
 botoesTags.clear();chips.clear();juce::StringArray valores;
 if(aba!=1)valores={"Present","Light","Heavy","Saturated","Best Of","Vocal","Drums","Bright","Limiter","De-Ess","Dark","Bass"};
 for(auto& p:processador.bank){if(aba!=1)for(auto& t:p.tags)if(t!=p.group)valores.addIfNotAlreadyThere(t);if(aba!=0&&p.artist.isNotEmpty())valores.addIfNotAlreadyThere("artist:"+p.artist);if(aba==2&&p.group.isNotEmpty())valores.addIfNotAlreadyThere("group:"+p.group);}
 if(aba!=0)std::sort(valores.begin(),valores.end(),[](const juce::String& a,const juce::String& b){const auto nome=[](const juce::String& v){return v.containsChar(':')?v.fromFirstOccurrenceOf(":",false,false):v;};return nome(a).compareIgnoreCase(nome(b))<0;});
 auto criar=[this](const juce::String& valor,bool chip){auto b=std::make_unique<juce::TextButton>((valor.containsChar(':')?valor.fromFirstOccurrenceOf(":",false,false):valor)+(chip?juce::String::fromUTF8(" ×"):""));b->getProperties().set("chip",chip);b->onClick=[this,valor,chip]{if(chip)filtros.removeString(valor);else filtros.addIfNotAlreadyThere(valor);atualizar();};b->setColour(juce::TextButton::buttonColourId,juce::Colour(chip?tema::cores::tagSelecionada:tema::cores::tag));if(chip)addAndMakeVisible(*b);else grade.addAndMakeVisible(*b);return b;};
 for(auto& f:filtros)chips.push_back(criar(f,true));for(auto& v:valores)if(!filtros.contains(v))botoesTags.push_back(criar(v,false));
}
void PresetBrowser::atualizar(){atualizando=true;linhas.clear();for(size_t i=0;i<processador.bank.size();++i)if(corresponde(processador.bank[i]))linhas.push_back((int)i);lista.updateContent();lista.deselectAllRows();const int atual=processador.getCurrentProgram();for(size_t i=0;i<linhas.size();++i)if(linhas[i]==atual)lista.selectRow((int)i);const auto& p=processador.bank[(size_t)atual];nome.setText(p.name,false);estrela.setButtonText(p.favorite?juce::String::fromUTF8("★"):juce::String::fromUTF8("☆"));favoritos.setButtonText(somenteFavoritos?juce::String::fromUTF8("FAVORITES ★"):juce::String::fromUTF8("FAVORITES ☆"));excluir.setEnabled(p.isUser);renomear.setEnabled(p.isUser);limpar.setVisible(!filtros.isEmpty());tipo.getProperties().set("ativo",aba==0);artista.getProperties().set("ativo",aba==1);todos.getProperties().set("ativo",aba==2);desenharTags();atualizando=false;resized();lista.repaint();repaint();}
void PresetBrowser::paintListBoxItem(int linha,juce::Graphics& g,int w,int h,bool selecionada){if(!juce::isPositiveAndBelow(linha,linhas.size()))return;const auto& p=processador.bank[(size_t)linhas[(size_t)linha]];if(selecionada){g.setColour(juce::Colour(tema::cores::tagSelecionada));g.fillAll();}g.setColour(juce::Colour(tema::cores::browserTexto));g.setFont(fonteInterface());g.drawText(p.name,12,0,w-40,h,juce::Justification::centredLeft);if(p.favorite)pintarEstrela(g,{w-17.f,h*.5f},7,true);}
void PresetBrowser::selectedRowsChanged(int linha){if(atualizando||!juce::isPositiveAndBelow(linha,linhas.size()))return;processador.setCurrentProgram(linhas[(size_t)linha]);atualizar();}
void PresetBrowser::resized(){
 using namespace tema::presets;
 const int w=getWidth(),m=margem,interior=w-2*m;
 titulo.setBounds(m,0,interior,cabecalho);fechar.setBounds(w-34,4,28,28);
 busca.setBounds(m,cabecalho,interior,tema::presets::busca);
 int y=cabecalho+tema::presets::busca+8;
 tags.setBounds(14,y,70,barraTags);limpar.setBounds(86,y,26,barraTags);favoritos.setBounds(w-158,y,144,barraTags);y+=barraTags;
 for(auto* b:{&tipo,&artista,&todos})b->setVisible(mostrarTags);rolagem.setVisible(mostrarTags);
 if(mostrarTags){
  const int coluna=(w-28)/3;
  tipo.setBounds(14,y,coluna,abas);artista.setBounds(14+coluna,y,coluna,abas);todos.setBounds(14+2*coluna,y,w-28-2*coluna,abas);y+=abas+8;
  int x=14;
  for(auto& b:chips){const int larguraChip=juce::jmin(w-28,juce::roundToInt(fonteInterface().getStringWidthFloat(b->getButtonText()))+24);if(x+larguraChip>w-14){x=14;y+=chipAltura+gap;}b->setBounds(x,y,larguraChip,chipAltura);b->setVisible(true);x+=larguraChip+gap;}
  if(!chips.empty())y+=chipAltura+8;
  const int alturaDesejada=linhasTags*(tagAltura+gap)-gap;
  const int alturaDisponivel=getHeight()-y-(nomeAltura+acaoAltura+2*linhaPreset+notaAltura+32);
  const int alturaGrade=juce::jlimit(tagAltura,alturaDesejada,alturaDisponivel);
  rolagem.setBounds(14,y,w-28,alturaGrade);
  const int larguraGrade=w-34,larguraBotao=(larguraGrade-2*gap)/3;
  grade.setSize(larguraGrade,juce::jmax(alturaGrade,((int)botoesTags.size()+2)/3*(tagAltura+gap)-gap));
  for(size_t i=0;i<botoesTags.size();++i)botoesTags[i]->setBounds((int)(i%3)*(larguraBotao+gap),(int)(i/3)*(tagAltura+gap),larguraBotao,tagAltura);
  y+=alturaGrade+8;
 }else for(auto& b:chips)b->setVisible(false);
 y=juce::jmin(y,getHeight()-nomeAltura-acaoAltura-2*linhaPreset-notaAltura-18);
 nome.setBounds(m,y,interior-34,nomeAltura);estrela.setBounds(w-m-32,y,32,nomeAltura);y+=nomeAltura+6;
 const int larguraAcao=(interior-2*gap)/3;
 excluir.setBounds(m,y,larguraAcao,acaoAltura);renomear.setBounds(m+larguraAcao+gap,y,larguraAcao,acaoAltura);salvar.setBounds(m+2*(larguraAcao+gap),y,interior-2*(larguraAcao+gap),acaoAltura);y+=acaoAltura+8;
 lista.setBounds(m,y,interior,juce::jmax(linhaPreset,getHeight()-y-notaAltura-m));aviso.setBounds(m,getHeight()-notaAltura-m,interior,notaAltura);
}
