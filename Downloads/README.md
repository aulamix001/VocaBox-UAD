# VocaBox — download Windows x64

[Baixar VocaBox-Windows-x64.zip](https://github.com/aulamix001/VocaBox-UAD/raw/refs/heads/main/Downloads/VocaBox-Windows-x64.zip)

Extraia o ZIP e copie a pasta inteira `VocaBox.vst3` para `C:\Program Files\Common Files\VST3\`, com a DAW fechada. Depois abra a DAW e faça uma nova busca por plugins.

Este binário foi compilado para Windows x64 com MinGW no Linux. A interface inclui a janela flutuante de presets, os knobs com relevo, o VU em alta resolução e o painel roxo atualizado. Os testes de DSP, snapshots de presets, estado do host e processamento VST3 passaram; o módulo Windows foi validado em Wine. A execução em uma DAW Windows real não foi verificada neste ambiente.

O código-fonte e o workflow para compilar com Visual Studio 2022/MSVC estão na raiz do repositório. `SHA256SUMS.txt` contém o hash do ZIP.
