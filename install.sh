#!/usr/bin/env bash
set -e

check_cmd() {
    if ! command -v "$1" &>/dev/null; then
        echo "ERROR: '$1' not found. Install it and re-run." >&2
        exit 1
    fi
}

install_deps() {
    if command -v apt-get &>/dev/null; then
        echo "Installing dependencies via apt-get..."
        sudo apt-get update -qq
        sudo apt-get install -y gcc libncurses-dev libcapstone-dev
    else
        echo "WARNING: apt-get not found. Please install gcc, libncurses-dev, libcapstone-dev manually."
    fi
}

check_cmd gcc
install_deps

echo "Building rupture..."
make clean
make

echo "Installing to /usr/local/bin/rupture..."
sudo make install

echo ""
echo "rupture installed successfully. Run: rupture <program>"
