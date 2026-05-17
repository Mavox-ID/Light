# Contributing to LightOS

Thank you for your interest in contributing to LightOS. To maintain code quality, architectural consistency, and legal compliance, all contributors must follow these guidelines.

## Pull Request Process

We accept contributions exclusively through Pull Requests (PRs). Direct pushes to the main branch are restricted. 

1. Fork the repository and create your branch from `main`.
2. Ensure your code compiles without warnings using the project's default toolchain and Makefiles.
3. If your changes introduce new behavior or modify existing functionality, you must update the changelog in the `info/UPDATES` file.
4. Submit your Pull Request using the provided PR template.

## Licensing and Code Ownership

By submitting a Pull Request to this repository, you agree that your contributions will be licensed under the **LightOS NCSA License - Non-Commercial Source-Available License**.

### Core System Modifications
Any modifications to the kernel, architecture support, drivers, bootloader, desktop environment, or shared libraries (`kernel/`, `arch/`, `drivers/`, `boot/`, `desktop/`, `shared/`, `util/`) fall directly under the primary LightOS license.

### Built-in Applications (`apps/`)
If you modify an existing application or add a new application directly to the `apps/` directory of this repository:
* You retain authorship credit for your work.
* The application becomes part of the LightOS ecosystem and must comply with the LightOS NCSA License terms.
* **You cannot hide the source code.** All additions and updates submitted to the main repository must remain completely open and source-available.

### External Applications (Freelance Work)
As per Section 8 of the LightOS License, if you develop an independent program outside of this repository that merely interacts with the Light API (distributed separately as an `.lpp` program), you retain the right to make it closed source. However, this exception **does not apply** to code submitted directly to the official LightOS repository via Pull Requests.

## Code Quality Standards

* **Consistency:** Match the existing code style in the directory you are modifying (tabs/spaces, naming conventions).
* **Portability:** Ensure that modifications in `kernel/` or `shared/` do not break cross-compilation for either `x86_32` or `x86_64` architectures.
* **No Bloat:** Keep dependencies minimal. Utilize the existing implementations in `shared/` where possible instead of introducing external headers.
