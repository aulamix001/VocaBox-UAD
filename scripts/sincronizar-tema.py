#!/usr/bin/env python3
"""Gera o tema C++ e incorpora os mesmos tokens no HTML independente."""
from pathlib import Path
import json,re,base64
from arte_vu import desenhar_vu
raiz=Path(__file__).resolve().parents[1]
tema=json.loads((raiz/'Resources/tema-ui.json').read_text())
cores=tema['cores'];fontes=tema['fontes'];p=tema['presets'];v=tema['vu']
assert fontes['presetsTamanho']>=14

def luminosidade(cor):
 canais=[int(cor[i:i+2],16)/255 for i in (1,3,5)]
 canais=[c/12.92 if c<=.04045 else ((c+.055)/1.055)**2.4 for c in canais]
 return sum(a*b for a,b in zip(canais,[.2126,.7152,.0722]))
def contraste(a,b):
 l=sorted([luminosidade(a),luminosidade(b)])
 return (l[1]+.05)/(l[0]+.05)
for fundo in ['browserTopo','browserBase','tag','tagHover','tagSelecionada']:
 assert contraste(cores['browserTexto'],cores[fundo])>=7,(fundo,'Contraste insuficiente')
 assert contraste(cores['browserSecundario'],cores[fundo])>=7,(fundo,'Contraste secundário insuficiente')

def salvar(caminho,conteudo):
 if not caminho.exists() or caminho.read_text()!=conteudo:caminho.write_text(conteudo)
cpp=['// Gerado de Resources/tema-ui.json por scripts/sincronizar-tema.py.','#pragma once','#include <cstdint>','namespace tema {','namespace cores {']
for nome,cor in cores.items():cpp.append(f'inline constexpr std::uint32_t {nome}=0xff{cor[1:]};')
cpp+=['}','namespace fontes {',f'inline constexpr const char* familia="{fontes["presetsFamilia"]}";',f'inline constexpr float tamanho={fontes["presetsTamanho"]}.f;',f'inline constexpr float nota={fontes["presetsNota"]}.f;',f'inline constexpr const char* amostraMetrica="{fontes["amostraMetrica"]}";',f'inline constexpr float larguraAmostraEm={fontes["larguraAmostraEm"]:.9f}f;','}','namespace presets {']
for nome,valor in p.items():cpp.append(f'inline constexpr int {nome}={valor};')
cpp+=['}','namespace knobs {']
for nome,valor in tema['knobs']['diametros'].items():cpp.append(f'inline constexpr float {nome}={valor}.f;')
cpp+=['}','namespace vu {']
for nome in ['x','y','largura','agulhaComprimento','agulhaEspessura']:
 cpp.append(f'inline constexpr float {nome}={float(v[nome]):.6f}f;')
cpp+=[f'inline constexpr float larguraBase={v["viewBox"][0]}.f;',f'inline constexpr float alturaBase={v["viewBox"][1]}.f;',f'inline constexpr float pivoX={v["pivo"][0]}.f;',f'inline constexpr float pivoY={v["pivo"][1]}.f;',f'inline constexpr const char* aberturaPath={json.dumps(v["aberturaPath"])};']
for nome in ['agulhaCor','agulhaPonta','agulhaSombraCor']:cpp.append(f'inline constexpr std::uint32_t {nome}=0xff{v[nome][1:]};')
cpp+=['}','}']
salvar(raiz/'Source/TemaVisual.h','\n'.join(cpp)+'\n')
cssnomes={'painelTopo':'panel-top','painelBase':'panel-bottom','tinta':'ink','preto':'black','cromado':'chrome','pagina':'page','barra':'toolbar','vermelho':'red','ambar':'amber','muted':'muted','bordaPainel':'panel-border','browserTopo':'browser-top','browserBase':'browser-bg','browserPoco':'browser-well','browserBorda':'browser-border','browserTexto':'browser-text','browserSecundario':'browser-muted','tag':'tag-bg','tagHover':'tag-hover','tagSelecionada':'tag-selected','tagBorda':'browser-line','presetSelecao':'preset-selection','lista':'list-bg','scroll':'scroll-thumb','scrollFundo':'scroll-track','graduacoes':'scale-color'}
variaveis=[f'--{cssnomes[n]}:{c}' for n,c in cores.items() if n in cssnomes]
variaveis += [f'--preset-font:{fontes["presetsTamanho"]}px',f'--preset-weight:{fontes["presetsPeso"]}',f'--browser-width:{p["largura"]}px',f'--browser-height:{p["altura"]}px',f'--tag-height:{p["tagAltura"]}px',f'--chip-height:{p["chipAltura"]}px',f'--tag-gap:{p["gap"]}px',f'--preset-row:{p["linhaPreset"]}px',f'--tag-grid-height:{p["linhasTags"]*(p["tagAltura"]+p["gap"])-p["gap"]}px',f'--vu-left:{v["x"]}px',f'--vu-top:{v["y"]}px',f'--vu-width:{v["largura"]}px',f'--vu-height:{v["largura"]*v["viewBox"][1]/v["viewBox"][0]:.6f}px']
variaveis += [f'--preset-note-font:{fontes["presetsNota"]}px',f'--preset-font-family:{fontes["presetsFamilia"]},Arial,sans-serif',f'--panel-font-size:{fontes["painelTamanho"]}px',f'--panel-font-family:{fontes["painelFamilia"]}']
for chave,nome in [('cabecalho','header'),('busca','search'),('barraTags','tags-bar'),('abas','tabs'),('nomeAltura','name'),('acaoAltura','action'),('notaAltura','note')]:variaveis.append(f'--browser-{nome}-height:{p[chave]}px')
fonte=base64.b64encode((raiz/'Resources'/fontes['presetsArquivo']).read_bytes()).decode()
bloco='/* INICIO TEMA GERADO */\n:root{'+ ';'.join(variaveis)+'}\n@font-face{font-family:"Inter";font-style:normal;font-weight:600;font-display:block;src:url(data:font/ttf;base64,'+fonte+') format("truetype")}\n/* FIM TEMA GERADO */'
html=raiz/'voxbox.html';s=html.read_text()
if '/* INICIO TEMA GERADO */' in s:s=re.sub(r'/\* INICIO TEMA GERADO \*/.*?/\* FIM TEMA GERADO \*/',lambda m:bloco,s,flags=re.S)
else:s=re.sub(r':root\{--panel-top:[^}]+\}',lambda m:bloco,s,count=1)
dados='<script id="temaVisual" type="application/json">'+json.dumps(tema,ensure_ascii=False,separators=(',',':'))+'</script>'
if '<script id="temaVisual"' in s:s=re.sub(r'<script id="temaVisual".*?</script>',lambda m:dados,s,count=1,flags=re.S)
else:s=s.replace('<style>',dados+'\n<style>',1)
s=re.sub(r'<svg class="meter".*?</svg>',lambda m:desenhar_vu(v,fontes),s,count=1,flags=re.S)
salvar(html,s)
print('Tema sincronizado; contraste mínimo:',round(min(contraste(cores['browserSecundario'],cores[f]) for f in ['browserTopo','browserBase','tag','tagHover','tagSelecionada']),2),':1')
