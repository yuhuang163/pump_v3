#!/usr/bin/env bash
# Install Linux deps for BleFactoryTool (needs sudo once)
set -euo pipefail
cd "$(dirname "$0")"

echo "=== apt: python3-tk / python3-venv / python3-pip ==="
sudo apt-get update
sudo apt-get install -y python3-tk python3.8-venv python3-pip

echo "=== create venv + install bleak ==="
rm -rf .venv
python3 -m venv .venv
# shellcheck disable=SC1091
source .venv/bin/activate
python -m pip install --upgrade pip
pip install -r requirements.txt

echo "=== verify ==="
python -c "import tkinter; import bleak; print('OK tkinter + bleak', bleak.__version__)"

echo
echo "Run:  source .venv/bin/activate && python main.py"
