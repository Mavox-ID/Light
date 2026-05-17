# **Light OS** — Try hard and you'll succeed.

![Screenshot showing the file manager, text editor, and bitmap image editor.](https://github.com/Mavox-ID/Light/blob/main/res/Example.png)

# WE DON'T GIVE UP

LightOS was completely destroyed - every usable copy was gone, source trees shattered, builds broken, files reduced to fragments. Most people would have declared it dead and moved on.

But we didn’t.

For two relentless months, the system was rebuilt piece by piece through raw HEX editing, binary recovery, manual reconstruction, and countless hours of reverse restoration. Damaged structures were repaired byte by byte. Lost components were traced, rebuilt, and brought back online.

More than $200 was spent purely on HEX edits, recovery work, and bringing the project back from total collapse.

And it worked.

LightOS lives again.

The repository is being restored right now. Files are returning. Components are coming back. The foundation is standing again.

This project was destroyed.
Now it is rebuilding stronger than before.

WE DON'T GIVE UP.

## Supporters

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/S6S71XB1IK)

## Testing

LightOS is still being tested, and many files are not yet open source, but they will be added by 2026.

You can then download the completed .iso or .bin file from Release, or build it yourself by downloading (or cloning) this repo. Afterwards, type 'make utils' and after installation, type 'make build'. Read the license and more information and type yes. Make sure you have internet access and disk space. Close all other programs during the build. The build can take about 30 minutes. Sudo privileges are NOT REQUIRED.

## Building

To get started:

```bash
make utils
```

After this command, check whether you have all the utility versions installed or whether you have the correct version installed for the LightOS build.

### Need:

#### Versioned tools:

- Util: **gcc**        — req: 12.x–13.x
- Util: **g++**        — req: 12.x–13.x
- Util: **ld**         — req: 2.36–2.46.0
- Util: **as**         — req: 2.36–2.46.0
- Util: **nasm**       — req: >= 2.14
- Util: **make**       — req: >= 4.0
- Util: **bison**      — req: >= 3.0
- Util: **flex**       — req: >= 2.6
- Util: **meson**      — req: >= 0.60
- Util: **ninja**      — req: >= 1.10

#### Plain tools:

- Util: **awk**        — required
- Util: **ctags**      — required
- Util: **curl**       — required
- Util: **grep**       — required
- Util: **gzip**       — required
- Util: **sed**        — required
- Util: **tar**        — required
- Util: **xz**         — required
- Util: **python3**    — required
- Util: **git**        — required

After that, make sure that you have a Linux system, also that Linux is updated to the latest version and there are no problems with `make utils`

To build LightOS, you will need to build all utilities and GCC 11 specifically for LightOS (gcc-x86_64-light and g++-x86_64-light)

### Once you've verified that everything is fine and there are no problems with the build, you can confidently write:

```bash
make build
```

After reading the license, type yes. Then read the GCC compiler build information:

- You don't need an internet connection to build LightOS. All archives, configs, etc. are located in res/barchive and other LightOS folders.

- You need approximately 3-4 GB of free space at a minimum, and 5 GB of free space at most to avoid space issues.

- Make sure you're running Linux or Mac and not other operating systems, and that your system is fully updated and there are no issues with `make utils`.

- You need approximately 8 GB of available RAM. It's recommended to terminate all open windows or running processes that consume a lot of memory before building.

- This doesn't require root privileges like sudo, etc.; running it will automatically get everything you need. DO NOT RUN THE BUILD AS `sudo make build`!!!!!

The build itself will definitely have errors like Mesa, but the script will automatically fix them due to build-fix, so don't interrupt the build, otherwise you'll have to start over.

After a successful build, you'll find yourself in the LightOS terminal where you can launch LightOS with the test (t) or qemu-with-kvm (k) command.

You can get the launch command by running LightOS command and then opening the file 'bin/Logs/qemu-start.log'.

You can view other LightOS features in the build management terminal after the project is built and typing 'help'.

I hope this build information helps you. If you encounter any issues, please send me the last 200 lines of the log from the terminal where the build script stopped, and I'll help you resolve the build issue. Alternatively, you can download the LightOS release from the official repo from bin/drive, a ready-made raw file, and an ISO file for loading onto a USB drive.

## Warning!
This OS created in my PC, this repo is uploaded Open Source file from my PC

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
* 2048
* Build Core
* Designer
* Emulator
* File Manager
* Font Book
* Image Editor
* Installer
* IRC Client
* LPlayer
* Markdown Viewer
* OBJ Viewer
* POSIX Launcher
* Script Console
* System Monitor
* Text Editor

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

## In development

Drivers
* Read-write filesystems: LFS, Ext2, Ext4, FAT16/32/64, NTFS.
* Read-only filesystems: ISO9660.

Applications
* Full worked LP (Light Player) with .mp3, .waw, .mp4 etc.
* A working Designer2 with support for changing the LightOS theme directly in the running system
* Working communication channel IRC Client
* And additional settings and information about the PC and system in System Monitor

File Manager
* LPP application icon in the file manager
* When you right-click on a file, you will be given the option 'Open file as' and the choice of which .lpp application to use to open the file or open it as an executable
