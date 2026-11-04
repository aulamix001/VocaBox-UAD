# VocaBox: presets, válvula e auditoria de encerramento

Base inspecionada: `0fc22c241c399ddd3d7e1ec58263711b61196f04`, com alterações locais posteriores descritas aqui. Não foi criado commit nem publicada release no GitHub. Plataforma de validação: Debian 13 x86-64, GCC 14.2, JUCE 7.0.12 no commit `4f43011b96eb0636104cb3e433894cda98243626`, Chromium 155.0.8059.79 / Playwright 1.62.1.

## Mudanças

- Banco de 100 presets: 50 snapshots anteriores preservados integralmente (IDs, índices e valores), mais 50 ajustes autorais com snapshots completos dos 25 parâmetros. Categorias incluem vozes masculina/feminina, guitarras, baixos, bateria, piano/teclas, cordas, sopros, synth e percussão. Best Of seleciona dez dos novos ajustes. Dark e Light são tags combináveis, não cópias adicionais de presets.
- JSON incorporado ao VST3, HTML independente e CSV sincronizados. Os controles carregam os valores ao selecionar cada preset; não há detecção de instrumento nem MIDI Learn implementado neste projeto.
- Estados novos armazenam o ID estável do programa e a quantidade de presets de fábrica. Estados antigos com índices de presets do usuário são deslocados a partir da posição histórica 50, mantendo seus valores sonoros e a identificação do preset quando a biblioteca está disponível.
- A prioridade posterior do usuário autorizou mudar a saturação DSP. O estágio anterior `tanh` simétrico foi substituído por uma curva assimétrica com bias, normalização de pequeno sinal, ADAA de primeira ordem e bloqueio DC em 8 Hz. INPUT/GAIN excitam a curva; OUTPUT ajusta o nível. Compressor, EQ e de-esser mantêm seus algoritmos. O áudio processado de sessões anteriores muda; não se promete identidade sonora com a versão antiga.

## Encerramento e ownership

A inspeção encontrou diálogos numéricos alocados sem proprietário explícito e teardown dos attachments antes de terminar um arraste. São riscos verificáveis no código; não foram identificados como causa de um congelamento real no Studio One.

`Source/Common/EditorLifetime.h` encerra a interação pelo mouse/Slider do JUCE, incluindo o `DragInProgress` e o popup, enquanto slider, attachment e LookAndFeel existem. O editor para o timer, cancela diálogos, encerra interações e somente depois destrói attachments e controles. Referências seguras protegem operações que podem destruir componentes.

Os knobs possuem seus diálogos com `unique_ptr`. Callbacks verificam `SafePointer` e a identidade do diálogo atual. A confirmação de exclusão da biblioteca também tem proprietário explícito. Entradas numéricas utilizam `Slider::ScopedDragNotification`, equilibrando a notificação aos attachments reais.

Foi reproduzida uma assertion de shutdown relacionada a `GlyphInfo`/`Path` após a renderização da fonte Inter mantida em variável estática. A fonte agora é liberada em `DeletedAtShutdown`; os logs Debug finais são verificados para essas assertions.

Não foram acrescentados mutexes, arquivos, componentes, logs ou alocações ao `processBlock`. As preferências gráficas usam atomic. O processor não deriva de Timer; o timer do editor é interrompido no destrutor. A rotina DSP não consulta a preferência gráfica.

## Gráficos e versão JUCE

O projeto VocaBox usa JUCE 7.0.12; H-Delay/CopperEcho não estão neste repositório. Não foi aplicada a atualização JUCE 9 mencionada no texto de referência, nem atribuída a ela uma correção para esta DAW.

No JUCE 7 fixado, o construtor do peer Windows já seleciona GDI/índice 0 e `NativeImageType::create` já cria `SoftwarePixelData`. Por isso não há patches nem cópias privadas desses módulos: nenhuma fábrica precisava ser alterada. `cmake/CompatibleGraphics.cmake` verifica os pontos esperados e falha claramente se o framework for incompatível; grava `compatible-graphics.txt`. A opção OFF mantém a inicialização upstream, que nesta versão também começa em GDI.

No Windows, Options → Graphics oferece CPU/GDI e a implementação Direct2D experimental do JUCE 7 como escolha explícita. O adaptador RAII aplica a preferência ao peer e remove o listener na destruição. `graphicsMode` é uma propriedade adicional de estado, sem virar parâmetro de áudio. Presets não a modificam. Estados antigos usam o padrão da política.

Não foi desativado HiDPI nem alterado o painel, as texturas, os tamanhos dos controles ou os 25 parâmetros. A nova opção usa espaço livre na barra no Windows. Recursos gráficos existentes permaneceram inalterados. Isso não equivale a uma comparação de capturas Windows nem a um teste em todas as GPUs.

## Comandos e evidências

No ambiente preparado:

```sh
source /workspace/.vocabox-env/activate.sh
bash /workspace/.vocabox-env/start.sh
export DISPLAY=:99
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DJUCE_SOURCE_DIR=/workspace/.vocabox-env/JUCE \
  -DCMAKE_PREFIX_PATH=/workspace/.vocabox-env/sysroot/usr
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
CHROMIUM=/usr/bin/chromium node Tests/Browser.test.cjs
CHROMIUM=/usr/bin/chromium node Tests/PresetBank.test.cjs
```

| Verificação | Resultado |
| --- | --- |
| DSP: compressão, bypass, power, HPF, EQ, de-esser, link, taxas, silêncio e extremos | PASS |
| Válvula: pequeno sinal, compressão suave, harmônicos, DC, aliasing e extremos | PASS |
| Processor: 100 × 25 valores, persistência, estado novo/antigo e renderização | PASS |
| VST3Host: bundle real carregado, processamento e estado | PASS |
| EditorLifetime: 100 instâncias, 300 ciclos de peers nativos, áudio em thread separada | PASS Debug e ASan/UBSan |
| Navegador: knobs, filtros AND, migração, favoritos, CRUD e persistência | PASS |
| PresetBank: 100 seleções reais pela UI × 25 valores, paridade HTML/JSON, categorias | PASS |
| Fábricas Windows: verificações CMake contra os fontes fixados do JUCE | PASS (inspeção de fontes, não execução Windows) |

O harness de ciclo de vida aplica e reabre o diálogo numérico, fecha o editor com o diálogo aberto e durante arraste real com popup e attachment. Ao remover a instância, o host de teste espera a thread concluir o callback antes de liberar o processor. Há watchdog de 120 segundos, checkpoints fora do áudio, restauração de estado, alternância de sample rate/buffer, processamento offline e buffers de tamanho zero. É um harness de processor/editor; o teste separado VST3Host cobre o carregamento do módulo. Não é uma validação de todos os ciclos via SDK VST3.

Na análise de seno de 1 kHz, amplitude 0,8, a válvula apresentou H2 ≈ −23,57 dBc e H3 ≈ −28,59 dBc. Para seno de 9 kHz/48 kHz, o terceiro harmônico rebatido para 21 kHz caiu ≈ 7,99 dB frente à curva sem ADAA. É um caso medido, não garantia de ausência de aliasing.

Sanitizers:

```sh
cmake -S . -B build-asan -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DJUCE_SOURCE_DIR=/workspace/.vocabox-env/JUCE \
  -DCMAKE_PREFIX_PATH=/workspace/.vocabox-env/sysroot/usr \
  -DCMAKE_C_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined' \
  -DCMAKE_SHARED_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build-asan --parallel 2
DISPLAY=:99 ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-asan --output-on-failure
```

Resultado final ASan/UBSan: **5/5 PASS**, com `detect_leaks=1` e `halt_on_error=1`, sem relatórios de erro ou assertions JUCE nos logs. A suíte Debug também completou **5/5 PASS**.

Consulte os logs da execução final em `build/Testing/Temporary/LastTest.log` e `build-asan/Testing/Temporary/LastTest.log`. ASan/UBSan não substituem um detector de data races.

## Artefatos e limites

- Bundle Linux Debug: `build/VocaBox_artefacts/Debug/VST3/VocaBox.vst3`, contendo `Contents/x86_64-linux/VocaBox.so`.
- Comparação WAV: `dist/VocaBox-Tube-Before-After.wav`: 4 s com o DSP anterior, 0,25 s de silêncio, 4 s com o novo DSP. Notas sintetizadas, sem gravação vocal; RMS igualado. SHA256: `1d140926ab3acc8494eb330888b937f9c9853f4fee3723dededc9aad3f1077d0`.
- As instruções de instalação e inicialização foram salvas nos campos `install_script` e `start_skill` do rascunho. A publicação e a restauração em nova tarefa dependem da ação do usuário no produto; não foram executadas pelo agente.
- NÃO EXECUTADO: Windows/MSVC, troca CPU/GPU em peer Windows, escala SDK/DPI e monitores físicos, validator oficial Steinberg, AudioPluginHost oficial, minidumps e Studio One/Fender Studio Pro 8.4. Não foi produzido um novo VST3 Windows nem publicada release.
- O congelamento informado na DAW permanece sem reprodução e sem stacks. Para investigar esse caso: usar PDBs correspondentes, Break All, stacks de todas as threads e dump, anotando ação, versão da DAW/driver, escala, sample rate e buffer. As correções e os testes daqui não comprovam que esse congelamento foi resolvido.
