# VocaBox — VST3 Windows x64

Plugin de áudio C++/JUCE com painel baseado na referência visual enviada, 25 parâmetros automatizáveis e 50 presets. O HTML original continua disponível como protótipo independente. O VST3 processa áudio de verdade; não depende do HTML, de navegador, de UAD, de hardware DSP ou de conexão à Internet durante o uso.

Implementação independente e aproximada, orientada pelas páginas do manual fornecidas. Não é um produto Manley/Universal Audio, nem uma reprodução sonora certificada ou idêntica do VOXBOX. Nomes dos presets e inscrições do painel são referências do projeto.

## Instalar no Windows 64 bits

1. Feche a DAW e extraia o ZIP Windows.
2. Copie **a pasta inteira `VocaBox.vst3`** para `C:\Program Files\Common Files\VST3\`. A pasta contém `Contents\x86_64-win\VocaBox.vst3`; não mova apenas esse arquivo interno.
3. Abra sua DAW e faça uma nova busca por plugins VST3. Procure **VocaBox**, fabricante **VocaBox Project**.
4. Insira em uma faixa mono ou estéreo. O preset inicial é **Compressão Forte**. O nível resultante depende do áudio de entrada.

A compilação entregue é Windows x86-64 (não ARM, não VST2). Veja `VALIDATION.md` para o que foi efetivamente testado. A confirmação em uma DAW Windows real é uma etapa distinta dos testes automatizados.

## Controles e presets

- Knobs: arraste verticalmente, use a roda do mouse; Shift reduz o passo do arraste. Duplo clique restaura o valor padrão do controle; botão direito ou Ctrl+clique abre a entrada numérica.
- Chaves: clique para percorrer posições. SOURCE, GAIN, ATTACK, RELEASE e seletores de frequência encaixam nas posições disponíveis.
- Campo do preset: abre a janela nativa com busca e filtros. Selecione uma ou mais categorias como Dark, Bass, Vocal, Guitar, Snare ou De-Ess. As entradas compartilhadas apontam para o mesmo preset.
- Compare alterna os parâmetros atuais com o estado de comparação da abertura do editor. Copy/Paste copiam os parâmetros dentro da instância.
- IN/BYPASS é o bypass do plugin. POWER desliga a saída com uma transição suave.
- Os ajustes são salvos na sessão da DAW; os IDs dos parâmetros são estáveis.
- O rodapé mostra a redução **real** de ganho do compressor e do de-esser. O medidor acompanha o seletor, sem sinal aleatório simulado.

`Resources/presets.json` é a fonte de dados dos 50 presets e dos 25 parâmetros; `Source/Parameters.h` é gerado por `python scripts/generate_parameters.py`. Todos os presets fotografados usam **estimativas visuais** para valores contínuos, não exportações oficiais UAD. Os testes garantem aplicação dos valores armazenados aos controles, não exatidão numérica das leituras das fotos. As categorias seguem os grupos informados nas referências; um mesmo preset pode pertencer a várias tags, sem duplicação.

## Áudio implementado

Entrada → filtro passa-altas → compressor óptico aproximado → pré-amplificação/saturação suave → três bandas paramétricas → de-esser ou limiter → saturação suave de saída opcional → OUTPUT.

- Compressor: soft knee, relação nominal 3:1, redução limitada a 16 dB. Threshold cresce no sentido de mais compressão.
- Ataques aproximados: FAST 4 ms, MED FAST 8 ms, MEDIUM 16 ms, MED SLOW 35 ms, SLOW 70 ms. O manual fornece os extremos; valores intermediários são escolhas de implementação.
- Releases: 0,3 / 0,5 / 1 / 2 / 5 s. Link usa o maior sinal dos canais para a detecção estéreo.
- EQ: frequências selecionáveis iguais às escalas. LOW/HI acrescentam ganho e MID corta. Filtros de pico RBJ aproximam as curvas, sem modelar a rede passiva original.
- De-esser: redução dinâmica da banda selecionada em 3/6/9/12 kHz. LIMIT processa a banda inteira com relação 10:1, ataque 2 ms e release 0,5 s; não é um brickwall true-peak.
- Mono/estéreo, processamento float, sample rates de 22,05 a 96 kHz testados. Mudanças de ganho e filtros são suavizadas. Sem alocações, arquivos ou mutexes no loop de processamento.

O mapeamento de áudio usa `INPUT 0…10 → -15…+15 dB`, e `GAIN 40…60 → 0…20 dB` em LINE, com +20 dB adicional em MIC. Isso adapta níveis de uma DAW: não é calibração de dBu para dBFS. O preset forte usa INPUT 7,5, GAIN 45, THRESHOLD 8, FAST/SLOW, OUTPUT -6 dB. Não existe garantia de uma redução fixa em qualquer gravação.

Limitações: não há modelo elétrico de válvulas, transformadores ou memória óptica de múltiplos estágios, oversampling, phantom power, hardware externo ou roteamento dos conectores traseiros. O HPF foi atualizado conforme o manual para FLAT/80/120 Hz, com inclinação de 6 dB/oitava. As escalas de EQ do protótipo limitam os ganhos/corte a 10 dB. O seletor de medição mantém os quatro estados do HTML, com LINE IN/G-R compartilhando o estado de redução. O VST3 usa janela nativa de presets com favoritos e edição da biblioteca do usuário.

## Compilar

Windows: instale Visual Studio 2022 com **Desktop development with C++**, CMake ≥3.22 e Git. No Developer PowerShell:

```powershell
.\scripts\build-windows.ps1
```

CMake baixa o JUCE **7.0.12**, fixado no commit `4f43011b96eb0636104cb3e433894cda98243626`. Nenhuma alteração no framework é necessária. Para um checkout local, passe `-DJUCE_SOURCE_DIR=C:/caminho/JUCE`.

Linux (compilação e testes nativos): instale CMake, Ninja/GCC, libasound2-dev, libfreetype-dev, libx11-dev, libxext-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev e libxrender-dev.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Somente DSP, sem dependências gráficas ou JUCE:

```sh
cmake -S . -B build-dsp -DVOCA_DSP_ONLY=ON
cmake --build build-dsp
ctest --test-dir build-dsp --output-on-failure
```

### Compilação nativa no GitHub Actions

O workflow `.github/workflows/windows-vst3.yml` usa uma máquina **Windows Server 2022 / Visual Studio 2022 / MSVC x64**. Depois de enviar os arquivos do projeto ao GitHub, abra **Actions → Windows x64 VST3 (MSVC) → Run workflow**. Ao concluir, baixe o artefato **VocaBox-Windows-x64-MSVC**. Ele contém o ZIP do plugin, fontes, hashes, identificação da execução e o log dos testes. O workflow só executa por solicitação manual e não publica releases.

Para compilar localmente no Windows, também é possível executar `scripts/build-windows.cmd`. Visual Studio 2022 com o workload C++ e CMake devem estar instalados. Esses caminhos nativos estão preparados, mas ainda não foram executados neste ambiente Linux; o ZIP binário entregue anteriormente continua sendo a compilação cruzada validada em Wine.

Há uma toolchain MinGW opcional em `cmake/mingw-x64.cmake`. A compilação cruzada precisa de um JUCEAide nativo (gerado automaticamente) e das dependências Linux acima. Defina `MINGW_ROOT` para o prefixo de instalação do compilador.

## Dependências e licenças

JUCE 7 é distribuído sob licença comercial ou GPLv3; consulte `THIRD_PARTY/JUCE-LICENSE.md` antes de distribuir uma versão fechada. As licenças e avisos do SDK VST3 e bibliotecas incorporadas acompanham `THIRD_PARTY`. O código completo deste projeto é entregue no ZIP de fontes; o framework exato pode ser obtido pelo CMake no commit fixado. Não foram incorporados DSP ou binários da Universal Audio.

## Navegador de presets e knobs

A janela de 385 × 640 é flutuante, não modal e abre fora do plugin, preferencialmente à esquerda. Se não houver espaço, usa outro lado e mantém a janela acessível na tela. O cabeçalho permite arrastar a janela; fechar e reabrir recalcula a posição a partir do plugin. Ela oferece busca por nome/tags, abas TYPE/ARTIST/ALL, chips removíveis com filtros cumulativos (AND) e favoritos persistentes. DELETE e RENAME atuam apenas em presets do usuário; DELETE pede confirmação. SAVE grava os parâmetros atuais com nome e tags. No Windows, a biblioteca fica em `%APPDATA%\VocaBox\biblioteca-v2.json`; no HTML, fica no armazenamento local do navegador. As bibliotecas dos dois aplicativos são independentes.

O HTML mantém `voxbox.importPresets(arrayOuJSON)` e `voxbox.exportPresets()`. A migração reconhece `[GRUPO] Nome`, `GRUPO - Nome` e `GRUPO_Nome`, preserva tags explícitas e infere tags adicionais pelo nome. Sem prefixo, o grupo padrão é VOXBOX; nomes ambíguos permanecem importados e aparecem no relatório do console e em `voxbox.getImportReport()`. ARTIST usa metadados `artist`/`author` ou o sufixo `by Nome`; sem informação, a aba fica vazia. Os 50 snapshots de fábrica mantêm seus parâmetros originais. As cinco referências finais seguem a sequência Line enviada pelo usuário.

Todos os 16 knobs usam a mesma pele procedural: anel de base, estrias, granulação, sombras de contato/projetada e luz superior esquerda fixa. No HTML, canvas @3x; no VST3, atlas de 128 quadros @3x gerado da mesma função. Os diâmetros nominais são: SOURCE 48 px, INPUT/GAIN 52 px, medidor PREOUT/EQ OUT 44 px, controles médios 48 px, bandas de EQ 42 px e seletores de frequência 36 px. A travessa central elevada, o inserto curto e as estrias discretas acompanham o valor; a iluminação permanece fixa. Mudanças visuais são suavizadas. Botão direito ou Ctrl+clique abre a entrada numérica; arraste vertical, Shift, scroll e duplo clique continuam disponíveis. A pele é uma reconstrução visual, sem certificação de identidade fotográfica com a referência.

Teste do navegador: instale Playwright e execute `node Tests/Browser.test.cjs` (defina `PLAYWRIGHT_MODULE` e `CHROMIUM` se necessário). Verifica filtros AND, migração, abas, favoritos, edição, exclusão confirmada, persistência e interação dos knobs.

Para usar somente o HTML, abra `voxbox.html` no navegador e clique no nome do preset no topo. Para adicionar uma tag ou importar novos presets, execute no console do navegador, por exemplo:

```js
voxbox.importPresets([{name:'[VOXBOX] Meu Vocal',tags:['Vocal','Minha Tag'],params:voxbox.getState()}]);
```

A tag aparece automaticamente na grade TYPE e em ALL. Para regenerar os recursos nativos a partir do HTML, execute `node scripts/export-ui.cjs` com Playwright disponível, depois `python scripts/generate_parameters.py` e recompile.

O VU usa moldura em camadas, vidro âmbar, escala curva não linear com calibração secundária e agulha fina. O painel é exportado @3x (3576 × 990), com a agulha vetorial sobreposta no VST3; a medição de áudio permanece ligada ao DSP. Os detalhes visuais são reconstruídos a partir das referências, sem garantia de identidade pixel a pixel.

## Tema e revisão visual

Edite `Resources/tema-ui.json` para ajustar as cores principais, os diâmetros e raios de escala, a geometria do VU e a tipografia da biblioteca. `Source/TemaVisual.h` e o bloco de tema do HTML são gerados; não os edite manualmente. A biblioteca usa Inter SemiBold incorporada, 14 px em, com normalização das métricas do JUCE, alinhamento inteiro e contraste mínimo validado de 7,66:1.

Depois de editar o tema, execute `node scripts/export-ui.cjs` (com Playwright/Chromium) e `python scripts/generate_parameters.py`, e recompile. O primeiro comando já executa `scripts/sincronizar-tema.py`. Os arquivos gerados e recursos estão versionados, portanto a compilação comum não exige esses passos nem a instalação da fonte.

O painel usa `#4A4478 → #3A3560`, metal escovado sutil, VU com moldura escura fina e faixa positiva simples. As quatro posições do medidor foram preservadas; LINE IN/G-R continuam compartilhados, com textos visualmente separados. Veja [UI_CHANGES.md](UI_CHANGES.md) para a lista completa de arquivos, valores antigos → novos, decisões e comparações antes/depois.
