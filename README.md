Brick_Breaker

Bare-Metal Breakout Game for x86 (i686)

📌 Project Overview

Brick_Breaker is a feature-rich Breakout-style game written entirely in C that runs directly on bare-metal x86 hardware, without relying on any operating system or standard C libraries.

This project was developed as part of an Operating Systems / Low-Level Systems coursework, demonstrating:

Cross-compilation for a non-host target (i686-elf)

Kernel-level C programming

Direct hardware access (VGA, keyboard, PIT timer)

Real-time input handling via interrupts

Game logic running without an OS

From a cybersecurity and systems perspective, this project highlights controlled execution environments, attack surface reduction, and explicit trust boundaries by eliminating OS-level abstractions.

🧠 Technical Highlights

Pure bare-metal (no Linux, no libc, no glibc)

VGA Mode 13h graphics

PS/2 keyboard interrupts

PIT-based frame timing

Custom memory management

Cross-compiled kernel using GCC + Binutils

Build automation via build.sh

🛠️ System Requirements
Host System

Ubuntu / Debian-based Linux (recommended)

qemu-system-i386 (for emulation)

nasm

Build dependencies listed below

⚠️ Windows users should build inside WSL or a Linux VM

🔧 Step 1: Install Required Packages

Open a terminal and run:

sudo apt update
sudo apt install build-essential
sudo apt install bison
sudo apt install flex
sudo apt install libgmp3-dev
sudo apt install libmpc-dev
sudo apt install libmpfr-dev
sudo apt install texinfo
sudo apt install libcloog-isl-dev
sudo apt install libisl-dev


If some packages fail to install, continue — the build will usually still succeed as long as build-essential, bison, and flex are installed.

🧩 Step 2: Create the Cross Compiler (i686-elf)
Why a Cross Compiler?

Using the system’s default GCC is not suitable for kernel development because it:

Assumes an existing OS

Links against system libraries

Emits system calls your kernel does not support

A cross compiler generates code for a bare-metal target, ensuring:

No unintended dependencies

Predictable binary output

Full control over the build process

Download Sources
mkdir -p $HOME/src
cd $HOME/src


Download Binutils 2.35:

wget https://ftp.gnu.org/gnu/binutils/binutils-2.35.tar.xz
tar -xf binutils-2.35.tar.xz


Download GCC 10.2.0:

wget https://ftp.libip6.fr/pub/linux/distributions/gentoo/distfiles/gcc-10.2.0.tar.gz
tar -xf gcc-10.2.0.tar.gz

Build and Install Binutils
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

cd $HOME/src
mkdir build-binutils
cd build-binutils

../binutils-2.35/configure \
  --target=$TARGET \
  --prefix=$PREFIX \
  --with-sysroot \
  --disable-nls \
  --disable-werror

make
make install

Build and Install GCC
cd $HOME/src
mkdir build-gcc
cd build-gcc

../gcc-10.2.0/configure \
  --target=$TARGET \
  --prefix=$PREFIX \
  --disable-nls \
  --enable-languages=c,c++ \
  --without-headers

make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

Verify the Cross Compiler
$HOME/opt/cross/bin/i686-elf-gcc --version


You should see GCC version output.
If not, ensure $PREFIX/bin is in your PATH.

⚙️ Step 3: Make the Build Script Executable

From the project root:

chmod +x build.sh


This is required because Git does not always preserve executable permissions across platforms.

🧱 Step 4: Build the Project
./build.sh


This will:

Assemble boot code

Compile kernel and game sources

Produce the bootable binary image

▶️ Step 5: Run in QEMU
qemu-system-i386 -hda ./bin/os.bin -m 64M -cpu max

🎮 Controls

Arrow keys / A & D – Move paddle

Space – Start / restart

P – Pause

ESC – Exit to kernel

Ctrl – Laser (when enabled)

📂 Repository Structure
Brick_Breaker/
├── src/        # Kernel and game source code
├── build/      # Build output (generated)
├── bin/        # Bootable binaries
├── build.sh    # Automated build script
└── makefile    # Build configuration


build/ and bin/ are intentionally included to preserve project structure.

🎓 Educational Value

This project demonstrates:

Cross-compiler toolchain creation

Bare-metal C programming

Hardware-level graphics and input

Interrupt-driven design

Deterministic execution environments

Low-level system security concepts
