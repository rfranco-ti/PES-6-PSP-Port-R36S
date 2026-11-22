#!/bin/bash
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" 2>/dev/null && pwd)"
[ -n "$SCRIPT_DIR" ] || SCRIPT_DIR="$(pwd)"

BOOT_LOG="$SCRIPT_DIR/PES6-bootstrap.log"
exec >"$BOOT_LOG" 2>&1
set -x

GAMEDIR="$SCRIPT_DIR/pes6"
if [ ! -d "$GAMEDIR" ]; then
    exit 10
fi

cp "$BOOT_LOG" "$GAMEDIR/log.txt" 2>/dev/null || true
exec >>"$GAMEDIR/log.txt" 2>&1

controlfolder=""
for candidate in \
  "$SCRIPT_DIR/PortMaster" \
  "/opt/system/Tools/PortMaster" \
  "/opt/tools/PortMaster" \
  "/roms/ports/PortMaster" \
  "/roms2/ports/PortMaster" \
  "/storage/roms/ports/PortMaster"
do
  if [ -f "$candidate/control.txt" ]; then
    controlfolder="$candidate"
    break
  fi
done

if [ -n "$controlfolder" ]; then
  source "$controlfolder/control.txt"
  type get_controls >/dev/null 2>&1 && get_controls
  [ -f "$controlfolder/mod_${CFW_NAME}.txt" ] && source "$controlfolder/mod_${CFW_NAME}.txt"
fi

cd "$GAMEDIR" || exit 11
chmod +x ./pes6 2>/dev/null || true

for gov in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor /sys/devices/system/cpu/cpufreq/policy*/scaling_governor /sys/class/devfreq/*/governor; do
  [ -f "$gov" ] && echo performance | ${ESUDO:-sudo} tee "$gov" > /dev/null 2>&1 || true
done

export SDL_GAMECONTROLLERCONFIG_FILE="$GAMEDIR/gamecontrollerdb.txt"
if [ -n "${sdl_controllerconfig:-}" ]; then
  export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
fi

DATADIR="$GAMEDIR/gamedata"
ASSETDIR="$DATADIR/assets"
EXTRACTOR="$GAMEDIR/tools/extractor/pes6-extractor"

iso_file=""
if [ -f "$DATADIR/pes6.iso" ]; then
  iso_file="$DATADIR/pes6.iso"
else
  for candidate in "$DATADIR"/*.iso "$DATADIR"/*.ISO; do
    if [ -f "$candidate" ]; then
      iso_file="$candidate"
      break
    fi
  done
fi

if [ -z "$iso_file" ]; then
  echo "ERRO: ISO do PES 6 nao encontrada no diretorio pes6/gamedata/"
  exit 12
fi

if [ ! -f "$EXTRACTOR" ] || ! chmod +x "$EXTRACTOR"; then
  echo "ERRO: extrator nao encontrado ou sem permissao de execucao: $EXTRACTOR"
  exit 13
fi

if ! mkdir -p "$ASSETDIR"; then
  echo "ERRO: nao foi possivel criar $ASSETDIR"
  exit 14
fi

if ! "$EXTRACTOR" "$iso_file" "$ASSETDIR/EBOOT.elf"; then
  echo "ERRO: nao foi possivel preparar os arquivos do jogo. Consulte as mensagens acima."
  exit 14
fi

export PES6_DISC_ISO="$iso_file"
export PES6_IO_ROOT="$GAMEDIR"
export PES6_VERSION="original"

for dev_node in /dev/dri/card0 /dev/dri/renderD128 /dev/mali0 /dev/uinput /dev/tty1 /dev/console; do
  [ -e "$dev_node" ] && ${ESUDO:-sudo} chmod 666 "$dev_node" 2>/dev/null || true
done

mali_dir="/tmp/pes6_mali"
rm -rf "$mali_dir"
mkdir -p "$mali_dir"

mali_blob=""
for candidate in \
  "/usr/lib/aarch64-linux-gnu/libmali-bifrost-g31-rxp0-gbm.so" \
  "/usr/lib/aarch64-linux-gnu/libmali-midgard-t76x-r14p0-gbm.so" \
  "/usr/lib/libmali.so" \
  "/usr/lib/aarch64-linux-gnu/libmali.so"
do
  if [ -f "$candidate" ]; then
    mali_blob="$candidate"
    break
  fi
done

if [ -z "$mali_blob" ]; then
  mali_blob=$(find /usr/lib -name "libmali-bifrost-g31-*.so" -o -name "libmali*.so" 2>/dev/null | head -n 1)
fi

if [ -n "$mali_blob" ]; then
  ln -sf "$mali_blob" "$mali_dir/libEGL.so.1"
  ln -sf "$mali_blob" "$mali_dir/libEGL.so"
  ln -sf "$mali_blob" "$mali_dir/libGLESv2.so.2"
  ln -sf "$mali_blob" "$mali_dir/libGLESv2.so"
  ln -sf "$mali_blob" "$mali_dir/libgbm.so.1"
  ln -sf "$mali_blob" "$mali_dir/libgbm.so"
  export LD_LIBRARY_PATH="$mali_dir:$GAMEDIR/libs:$LD_LIBRARY_PATH"
else
  export LD_LIBRARY_PATH="$GAMEDIR/libs:$LD_LIBRARY_PATH"
fi

export SDL_VIDEODRIVER="kmsdrm"
export SDL_AUDIODRIVER="alsa"
export AUDIODEV="default"
export SDL_VIDEO_FORCE_EGL=1
export SDL_VIDEO_GL_DRIVER="libGLESv2.so"
export SDL_VIDEO_EGL_DRIVER="libEGL.so"
export PES6_SCALE=2
export PES6_STRETCH=1
export PES6_GE_ASYNC=1
export PES6_FPS_LOG=1

if type pm_platform_helper >/dev/null 2>&1; then
  pm_platform_helper "$GAMEDIR/pes6"
fi

./pes6 "$ASSETDIR/EBOOT.elf" --window 1 --gpu
result=$?
if [ "$result" -gt 128 ]; then
  printf '[pes6] status %s (compativel com sinal %s)\n' "$result" "$((result - 128))"
else
  printf '[pes6] encerrado com status %s\n' "$result"
fi

type pm_finish >/dev/null 2>&1 && pm_finish
exit "$result"
