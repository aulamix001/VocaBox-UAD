# VocaBox — download Windows x64

[Baixar VocaBox-Windows-x64.zip](https://github.com/aulamix001/VocaBox-UAD/raw/refs/heads/main/Downloads/VocaBox-Windows-x64.zip)

Extraia e copie a pasta inteira `VocaBox.vst3` para `C:\Program Files\Common Files\VST3\`, com a DAW fechada. Depois abra a DAW e faça uma nova busca por plugins.

Esta revisão inclui o VU com moldura fina escura, faixa positiva sem retângulo e escala inferior de sete valores; knobs proporcionais; painel `#4A4478 → #3A3560`; fonte Inter SemiBold incorporada, 14 px em, contraste mínimo 7,66:1. A biblioteca permanece flutuante. A lógica de áudio/DSP e os 50 presets foram preservados. Veja [UI_CHANGES.md](../UI_CHANGES.md) e [Review](../Review) para os arquivos, valores antigos → novos, decisões e imagens.

O binário foi compilado para Windows x64 com MinGW no Linux. O executável Windows de testes passou no Wine, incluindo fonte, captura da GUI, snapshots, persistência e áudio; o host VST3 Windows também passou. Isso não substitui a verificação em uma DAW Windows real. O workflow MSVC/Visual Studio 2022 e o código-fonte completo permanecem na raiz do repositório.

`SHA256SUMS.txt` contém o hash do ZIP.
