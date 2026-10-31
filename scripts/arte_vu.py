"""Arte vetorial do VU, gerada exclusivamente pelos valores do tema."""
import math

def desenhar_vu(v,fontes):
 cx,cy=v['pivo'];raio=v['faixaRaio']
 a,b=[math.radians(v['angulosSuperiores'][i]) for i in (7,10)]
 faixa=f'M{cx+math.sin(a)*raio:.3f} {cy-math.cos(a)*raio:.3f} A{raio} {raio} 0 0 1 {cx+math.sin(b)*raio:.3f} {cy-math.cos(b)*raio:.3f}'
 return f'''<svg class="meter" viewBox="0 0 {v['viewBox'][0]} {v['viewBox'][1]}" aria-label="Animated VU meter">
<defs>
 <linearGradient id="aroVu" x2=".2" y2="1"><stop stop-color="{v['molduraTopo']}"/><stop offset="1" stop-color="{v['molduraBase']}"/></linearGradient>
 <radialGradient id="ambarVu" cx=".47" cy=".57" r=".75"><stop stop-color="{v['dialCentro']}"/><stop offset=".52" stop-color="{v['dialMeio']}"/><stop offset="1" stop-color="{v['dialBorda']}"/></radialGradient>
 <linearGradient id="vidroVu" x2="0" y2="1"><stop stop-color="{v['reflexoCor']}" stop-opacity=".12"/><stop offset=".35" stop-color="{v['reflexoCor']}" stop-opacity=".015"/><stop offset="1" stop-color="{v['reflexoCor']}" stop-opacity=".04"/></linearGradient>
 <filter id="sombraVu" x="-15%" y="-20%" width="135%" height="150%"><feDropShadow dx="{v['sombraX']}" dy="{v['sombraY']}" stdDeviation="{v['sombraDesvio']}" flood-color="{v['sombraCor']}" flood-opacity="{v['sombraOpacidade']}"/></filter>
 <filter id="recessoVu" x="-10%" y="-10%" width="120%" height="120%"><feGaussianBlur in="SourceAlpha" stdDeviation="{v['sombraInternaDesvio']}" result="borrado"/><feOffset in="borrado" dx=".3" dy=".8" result="deslocado"/><feComposite in="SourceAlpha" in2="deslocado" operator="out" result="rebaixo"/><feFlood flood-color="{v['bordaInternaCor']}" flood-opacity="{v['sombraInternaOpacidade']}" result="corSombra"/><feComposite in="corSombra" in2="rebaixo" operator="in" result="sombraInterna"/><feComposite in="sombraInterna" in2="SourceGraphic" operator="over"/></filter>
 <filter id="papelVu"><feTurbulence type="fractalNoise" baseFrequency=".7" numOctaves="2" seed="12"/><feColorMatrix type="saturate" values="0"/><feComponentTransfer><feFuncA type="linear" slope=".025"/></feComponentTransfer><feBlend in="SourceGraphic" mode="soft-light"/></filter>
 <path id="aberturaVu" d="{v['aberturaPath']}"/>
 <clipPath id="meterClip"><use href="#aberturaVu"/></clipPath>
</defs>
<g filter="url(#sombraVu)">
 <path id="molduraVu" d="{v['molduraPath']}" fill="url(#aroVu)" stroke="{v['bordaCor']}" stroke-width=".8"/>
 <use href="#aberturaVu" fill="{v['molduraInternaCor']}" stroke="{v['bordaInternaCor']}" stroke-width="2.4"/>
 <g class="lamp"><use href="#aberturaVu" fill="url(#ambarVu)" filter="url(#recessoVu)"/>
 <g clip-path="url(#meterClip)">
 <rect x="0" y="0" width="220" height="106" fill="{v['dialCentro']}" opacity=".08" filter="url(#papelVu)"/>
 <g id="vuScale" fill="{v['escalaCor']}" font-family="{fontes['vuFamilia']}" font-size="{v['escalaFonte']}"></g>
 <path id="vuPositiveBand" d="{faixa}" fill="none" stroke="{v['positivoCor']}" stroke-width="{v['faixaEspessura']}" stroke-linecap="round"/>
 <g id="needle" style="transform-origin:{cx}px {cy}px"><path d="M{cx+.5} {cy+2} L{cx+.5} {cy-v['agulhaComprimento']}" stroke="{v['agulhaSombraCor']}" stroke-opacity=".22" stroke-width="{v['agulhaEspessura']}"/><path d="M{cx} {cy+2} L{cx} {cy-v['agulhaComprimento']}" stroke="{v['agulhaCor']}" stroke-width="{v['agulhaEspessura']}"/><path d="M{cx} {cy-v['agulhaComprimento']+6} L{cx} {cy-v['agulhaComprimento']}" stroke="{v['agulhaPonta']}" stroke-width="{v['agulhaEspessura']}"/></g>
 <use href="#aberturaVu" fill="url(#vidroVu)"/>
 </g></g>
 <path d="M15 42 Q111 5 205 35" stroke="{v['brilhoBordaCor']}" stroke-opacity=".24" stroke-width=".6" fill="none"/>
</g></svg>'''
