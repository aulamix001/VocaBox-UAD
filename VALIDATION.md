# Validation — VocaBox 0.1.0

Target: Windows x86-64 VST3. C++17; JUCE 7.0.12, commit 4f43011b96eb0636104cb3e433894cda98243626. Cross-compiled with MinGW GCC 14; native checks with GCC 14. No patches to JUCE required. The MinGW build uses the GDI font backend; JUCE's DirectWrite backend is unavailable with this compiler.

## Automated checks

- DSP: strong-compression test measured approximately 10.63 dB of gain reduction on the controlled input; threshold direction; transparent bypass after settling; power mute; 80/120 Hz HPF; low-peak boost, mid cut and EQ bypass; frequency-selective de-essing; stereo linking; 44.1/48/96 kHz compression; silence and extreme settings at 22.05/44.1/96 kHz.
- Processor: all **50 presets × 25 parameter snapshots** applied to native parameters within tolerance; state saved and restored; valid audio output; native editor construction and offscreen rendering at **1192 × 420**, including its **1192 × 330** face.
- VST3 ABI smoke host, native Linux build: module factory discovery, component creation, stereo bus arrangement, processing lifecycle, 192,000 samples of stereo float audio, finite/nonzero modified output, state save and restore, clean release.
- Windows PE x64 module: the same VST3 ABI smoke host was compiled as a Windows executable and run in Wine 10. It loaded the actual Windows VST3 module and passed the lifecycle, audio and state tests.
- Windows imports inspected: only Windows system DLLs; GCC/libstdc++/winpthread runtimes are statically linked.

CTest runs three checks (`DSP`, `Processor`, `VST3Host`) on the native build. Tests assert behavior rather than merely successful compilation. Source for each check is in `Tests/`.

## Boundaries

A real Windows DAW session, host automation gestures, high-DPI behavior and listening comparisons against physical Manley hardware or the UAD commercial plugin have **not** been validated here. Wine's headless environment was used for Windows audio/ABI testing, not a substitute for a Windows DAW or audio driver. No claim of bit accuracy, analog calibration or sonic equivalence is made.

The reference photos do not disclose exact continuous values. Their stored numerical values are visual estimates, and manual timing ranges/curves are approximated in DSP. `Resources/PRESET_MAP.csv` is the complete mapping for review. The native and HTML browsers offer AND tag filtering, TYPE/ARTIST/ALL tabs, favorites and persistent user preset editing. HTML import migration, favorites across reload, user CRUD/confirmation, search and knob numeric/reset/wheel behavior are covered by Tests/Browser.test.cjs. Native processor tests also verify the initial Line category, AND/clear filters, factory protection, duplicate rejection, user snapshot/favorite persistence, rename/delete and offscreen browser rendering. Browser placement checks cover left preference, right/bottom fallback, negative monitor coordinates and small-screen visibility. The baked panel is rendered at 3× resolution (3576 × 990) and scaled to its unchanged logical size. Visual fidelity has been inspected in screenshots; indistinguishability from the photographic reference is not certified.

## Revisão do tema, VU e legibilidade

O tema canônico está em `Resources/tema-ui.json`; o sincronizador valida contraste de pelo menos 7:1 (mínimo observado: 7,66:1). O teste HTML cobre Inter incorporada, peso 600, tamanho de 14 px, bounds inteiros, faixa curva sem contorno retangular e escala inferior 30/40/50/60/80/100/150. Os testes nativos verificam que a fonte é Inter e que suas métricas correspondem a 14 px em CSS. O executável de testes Windows também foi compilado e executado em Wine, incluindo captura da GUI.

Comparação com `e56e628`: DSP e processor idênticos byte a byte; mesmos 50 presets, IDs, intervalos, passos, padrões, tipos e ângulos. Mudam metadados de dimensão/posição e rótulos visuais, sem uma quinta posição funcional de medição. Consulte `UI_CHANGES.md` e `Review/`. Identidade pixel a pixel com o bitmap de referência não foi demonstrada; as cores do painel usam o intervalo fornecido pelo usuário.
