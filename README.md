# emu-6502

A cycle-accurate Nintendo Entertainment System (NES) emulator written in C. 

## Features

*   **Cycle-Accurate Emulation:** Precise synchronization between the CPU and PPU.
*   **Audio Support:** Full APU emulation featuring dynamic rate control algorithms for seamless audio/video synchronization.
*   **Cross-Platform UI:** Hardware-accelerated rendering and input management powered by [Raylib](https://www.raylib.com/).
*   **Custom Memory Mapping:** Robust bus architecture to cleanly handle CPU/PPU memory access and mapper routing.

## Architectural Decisions

*   **Language Choice (C):** Chosen for maximum control over memory layout and optimal low-level performance, which is critical for cycle-accurate emulation.
*   **Rendering & Input:** Raylib was selected for the frontend to keep the codebase lightweight and focused on emulation logic rather than platform-specific windowing code.
*   **Synchronization Strategy:** The emulator drives the PPU and APU based on CPU cycles. A dynamic rate control algorithm is implemented on the audio buffer to prevent audio popping and maintain a steady display rate without relying on strict V-sync blocking.
*   **Memory Bus Design:** Implemented a unified custom memory mapping system that allows the CPU and PPU to cleanly interface with various cartridge boards (mappers) through function pointers and memory intercepts.

## Supported Mappers

Currently, the emulator supports the following mappers:

*   **[Mapper 000 (NROM)](https://nesdir.github.io/mapper0.html):** The standard mapper used by early NES games (e.g., *Super Mario Bros.*, *Donkey Kong*).
*   **[Mapper 001 (MMC1)](https://nesdir.github.io/mapper1.html):** A versatile mapper that introduced switchable memory banks and selectable screen mirroring, famously used for games with save batteries (e.g., *The Legend of Zelda*, *Metroid*).
*   **[Mapper 002 (UxROM)](https://nesdir.github.io/mapper2.html):** A popular mapper utilizing switchable program banks with fixed character RAM, commonly adopted for games requiring larger codebases (e.g., *Mega Man*, *Castlevania*).
*   **[Mapper 004 (MMC3)](https://nesdir.github.io/mapper4.html):** An advanced mapper featuring fine-grained bank switching and scanline-based IRQs for complex split-screen scrolling effects (e.g., *Super Mario Bros. 3*, *Mega Man 3*).

## Building and Installation

The project uses a standard Makefile for compilation and is designed to be easily built in a Linux environment.

### Prerequisites

You will need a C compiler (`gcc` or `clang`), `make`, and the Raylib development libraries installed on your system.

On Debian/Ubuntu-based systems, you can typically install the required dependencies with:
```bash
sudo apt update
sudo apt install build-essential libraylib-dev
```

### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/your-nes-emulator.git
   cd your-nes-emulator
   ```
2. Build the emulator:
   ```bash
   make
   ```
3. Run a ROM:
   ```bash
   ./nes_emu path/to/your/rom.nes
   ```
