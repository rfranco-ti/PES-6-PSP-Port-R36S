# Pro Evolution Soccer 6: Port for R36S

<p align="center"><img src="pes6/cover.png" alt="Pro Evolution Soccer 6" width="320"/></p>

## English

The PSP PES 6 port for AArch64 Linux handhelds, using static recompilation. Improvements and fixes are in progress.

The ISO and original game executable are not included. The port executable contains code recompiled from the game. This version has been used on an R36S; compatibility and stability depend on the firmware and need further testing.

### Requirements

- An AArch64 Linux firmware with glibc 2.36 or newer, SDL2, EGL, GLES2 and ALSA.
- Your own European PES 6 PSP ISO, disc ID `ULES00476`, v1.03.

### Install

1. Download **Code → Download ZIP** and extract it.
2. Copy `PES 6.sh`, `PES 6.png` and the `pes6/` folder into `ports/`.
3. Copy your ISO to `ports/pes6/gamedata/pes6.iso`.
4. Start PES 6 from the ports menu. On first launch, the extractor validates the ISO and creates `gamedata/assets/EBOOT.elf` automatically. The ISO remains in place. The log is saved in `ports/pes6/log.txt`.

The extractor accepts the tested European v1.03 game executable. Other revisions are not verified. When updating an older installation, move its ISO into `gamedata/`. An existing invalid `EBOOT.elf` is preserved; move it elsewhere before trying again.

### Controls

The table uses SDL gamepad button names; physical labels can vary with firmware mapping. In-game assignments can be changed in the game.

| Input | PSP input |
|---|---|
| D-pad / left stick | D-pad / analog stick |
| B | Cross (X) |
| A | Circle |
| Y | Square |
| X | Triangle |
| L1 / R1 | L / R |
| Start / Select | Start / Select |
| Select + Start | Exit |

## Português

Port de PES 6 de PSP para portáteis Linux AArch64, usando recompilação estática. Melhorias e correções estão em andamento.

A ISO e o executável original do jogo não estão incluídos. O executável do port contém código recompilado do jogo. Esta versão foi usada em um R36S; compatibilidade e estabilidade dependem do firmware e precisam de mais testes.

### Requisitos

- Firmware Linux AArch64 com glibc 2.36 ou superior, SDL2, EGL, GLES2 e ALSA.
- Sua ISO europeia de PES 6 para PSP, ID `ULES00476`, v1.03.

### Instalação

1. Baixe o ZIP em **Code → Download ZIP** e extraia.
2. Copie `PES 6.sh`, `PES 6.png` e a pasta `pes6/` para `ports/`.
3. Copie sua ISO para `ports/pes6/gamedata/pes6.iso`.
4. Abra PES 6 no menu de ports. Na primeira execução, o extrator valida a ISO e cria `gamedata/assets/EBOOT.elf` automaticamente. A ISO permanece na pasta. O log fica em `ports/pes6/log.txt`.

O extrator aceita o executável da versão europeia v1.03 testada. Outras revisões não foram verificadas. Ao atualizar uma instalação anterior, mova a ISO para `gamedata/`. Se já existir um `EBOOT.elf` inválido, ele será preservado; mova-o para outro lugar antes de tentar novamente.

### Controles

A tabela usa os nomes dos botões SDL; os nomes físicos podem variar conforme o mapeamento do firmware. As funções podem ser alteradas nas configurações do jogo.

| Controle | Comando PSP |
|---|---|
| Direcional / analógico esquerdo | Direcional / analógico |
| B | X |
| A | Círculo |
| Y | Quadrado |
| X | Triângulo |
| L1 / R1 | L / R |
| Start / Select | Start / Select |
| Select + Start | Sair |

### Créditos

Créditos ao [pspdecrypt](https://github.com/John-K/pspdecrypt), cuja contribuição possibilita a extração dos dados da ISO do próprio jogador.
