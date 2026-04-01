set -euo pipefail

echo "╔══════════════════════════════════════╗"
echo "║   MiniBrowser — Setup & Build        ║"
echo "╚══════════════════════════════════════╝"
echo ""

echo "▸ [1/3] Atualizando lista de pacotes…"
sudo apt-get update -qq

echo "▸ [2/3] Instalando dependências…"
sudo apt-get install -y \
    build-essential \
    libgtk-3-dev \
    libwebkit2gtk-4.1-dev \
    pkg-config

echo "▸ [3/3] Compilando…"
make clean
make

echo ""
echo "✔  Tudo pronto!"
echo "   Execute com:  ./minibrowser"
echo "   Ou com URL:   ./minibrowser https://exemplo.com"
