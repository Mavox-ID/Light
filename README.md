# **Light OS** — Try hard and you'll succeed.

![Screenshot showing the file manager, text editor, and bitmap image editor.](https://raw.githubusercontent.com/Mavox-ID/Light/main/res/Example.png)

## Supporters

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/S6S71XB1IK)

## Testing

LightOS is still being tested, and many files are not yet open source, but they will be added by 2026.

You can then download the completed .iso or .bin file from Release, or build it yourself by downloading (or cloning) this repo. Afterwards, type 'make utils' and after installation, type 'make build'. Read the license and more information and type yes. Make sure you have internet access and disk space. Close all other programs during the build. The build can take about 30 minutes. Sudo privileges are NOT REQUIRED.

## Building

See `info/BUILD` for a description of how to build and test the system.

## Features

Kernel
* Filesystem independent cache manager.
* Memory manager with shared memory, memory-mapped files and multithreaded paging zeroing and working set balancing.
* Networking stack for TCP/IP.
* Scheduler with multiple priority levels and priority inversion.
* On-demand module loading.
* Virtual filesystem.
* Window manager.
* Audio mixer. (being rewritten)
* POSIX subsystem (Terminal), capable of running GCC (G++) and some Busybox tools.

Applications
* File Manager
* Text Editor
* IRC Client
* System Monitor

Ports
* Bochs
* GCC and Binutils
* FFmpeg
* Mesa (for software-rendered OpenGL)
* Musl

Drivers
* Power management: ACPI with ACPICA.
* Secondary storage: IDE, AHCI and NVMe.
* Graphics: BGA and SVGA.
* Read-write filesystems: Light File System (LFS).
* Read-only filesystems: Ext2, FAT, NTFS, ISO9660.
* Audio: HD Audio.
* NICs: 8254x.
* USB: XHCI, bulk storage devices, human interface devices.

Desktop
* Custom user interface library.
* Software vector renderer with complex animation support.
* Tabbed windows.
* Multi-lingual text rendering and layout with FreeType and Harfbuzz.
