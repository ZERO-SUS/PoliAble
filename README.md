# PoliAble

> **Native, Local-First Visual Web Design & Development Environment**

PoliAble is a high-performance native desktop application engineered for visual web design and development, translating visual designs into real, clean, editable production source code.

---

## 🛠️ Tech Stack & Architecture

- **Language:** C++23
- **GUI Framework:** Qt 6 (Core, Gui, Widgets)
- **Build System:** CMake 3.28+ with Ninja
- **Compiler:** MSVC 2022 (x64)

---

## 🚀 Getting Started

### Prerequisites

- **Visual Studio 2022** with MSVC C++ toolchain (x64)
- **CMake** (3.28 or newer)
- **Ninja** build tool
- **Qt 6** (Core, Gui, Widgets) installed

### Configuration & Build

1. **Configure CMake:**
   Run CMake specifying your Qt6 prefix path and Ninja generator:
   ```cmd
   configure.bat
   ```
   Or via PowerShell:
   ```powershell
   ./configure.ps1
   ```

2. **Build and Run:**
   ```cmd
   build.bat
   ```
   Or launch with automatic execution:
   ```powershell
   ./build_and_run.ps1
   ```

---

## 📂 Project Structure

```text
PoliAble/
├── CMakeLists.txt        # Master CMake build configuration
├── build.bat             # Batch build script
├── build_and_run.ps1     # PowerShell configure, compile & run automation
├── configure.bat         # CMake project configuration script
├── configure.ps1         # PowerShell configuration script
├── run_build.bat         # Ninja build and launch runner
├── assets/               # Project graphic assets & media
│   └── 4.png
└── src/                  # Application source code
    └── main.cpp          # Application entry point
```

---

## 📄 License

This project is licensed under the MIT License.
