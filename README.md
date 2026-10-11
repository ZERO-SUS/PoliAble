# ✦ PoliAble

> **Local-First Native Visual Web Design & Development Studio**  
> Built with modern **C++20** and **Qt 6**. High-performance, reactive, and professional.

---

## 🌟 Overview

**PoliAble** is a high-performance visual design application built natively from the ground up in modern C++20. Unlike Electron-based or web-wrapped design tools, PoliAble runs natively on the desktop with zero browser overhead, instantaneous startup, and minimal memory usage.

### Key Capabilities

- **📐 Vector Canvas & Infinite Workspace**: High-DPI antialiased vector canvas with infinite pan/zoom, subtle alignment dot-grid, and artboard drop shadows.
- **⚡ Reactive Layout Systems**: Built-in layout engine supporting real **Flexbox** (Auto Layout with Row/Col, Gap, Padding, Justify, Align) and **CSS Grid** (column/row templates, spanning, gaps).
- **📱 Responsive Breakpoint Engine**: 1-click viewport switching between **Desktop (1440px)**, **Laptop (1024px)**, **Tablet (768px)**, and **Mobile (375px)**.
- **🎨 Design Tokens & UI Component Kits**: Pre-curated token color swatches and 1-click insertable UI components (Buttons, Feature Cards, Navbars, Stat Badges).
- **↩ Command Pattern Undo/Redo**: Full undo and redo history stack tracking transforms, layout updates, style changes, and layer additions.
- **💾 Local-First File Format**: Human-readable, versioned `.poliable` document serialization ensuring zero lock-in and complete data ownership.
- **🌑 Studio Dark Theme**: Clean, refined user interface with custom tabbed sidebars, layers filter search, inline renaming, floating zoom overlay, and inspector panel.

---

## 🏗 Architecture

```
PoliAble
├── src/
│   ├── main.cpp                  # Application entry point
│   ├── MainWindow.{h,cpp}        # Studio main window, toolbars, docks, menus
│   ├── canvas/
│   │   ├── CanvasView.{h,cpp}    # QGraphicsView with zoom, pan, dot-grid & artboard
│   │   └── CanvasItem.{h,cpp}    # Interactive QGraphicsItem with 8-point resize handles
│   ├── layout/
│   │   └── LayoutEngine.{h,cpp}  # Flexbox, CSS Grid, and constraint calculation engine
│   ├── model/
│   │   ├── Node.{h,cpp}          # Scene graph nodes (Rectangle, Ellipse, Text, Flex, Grid, Frame)
│   │   ├── Document.{h,cpp}      # Document tree and node management
│   │   ├── Command.{h,cpp}       # Command pattern (AddNode, SetTransform, SetStyle, etc.)
│   │   ├── UndoStack.{h,cpp}     # Undo / redo history stack
│   │   ├── Serializer.{h,cpp}    # Round-trip .poliable text serialization
│   │   └── Styling.{h,cpp}       # Design tokens, gradients, typography, and effects
│   ├── panels/
│   │   ├── LayersPanel.{h,cpp}   # Layer tree with search filtering & context actions
│   │   ├── AssetsPanel.{h,cpp}   # Color tokens and 1-click component templates
│   │   ├── PropertiesPanel.{h,cpp}# Form inspector for transform, flex, grid, and styles
│   │   ├── ConsolePanel.{h,cpp}  # Output console with timestamped logs
│   │   └── TimelinePanel.{h,cpp} # Keyframe animation sequencer mockup
│   └── theme/
│       └── Theme.{h,cpp}         # Modern Studio Dark palette & comprehensive stylesheet
└── test/
    └── model/Node_test.cpp       # 26 automated unit tests
```

---

## 🚀 Building & Running

### Prerequisites

- **C++ Compiler**: MSVC 2022 (v143+) with C++20 support
- **Build System**: CMake 3.21+ and Ninja
- **Qt Framework**: Qt 6.7+ (`Qt6::Core`, `Qt6::Gui`, `Qt6::Widgets`)

### Quick Build (Windows)

```bat
# Configure CMake
cmake -B out/build/x64-debug -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Debug

# Compile
build.bat

# Launch PoliAble
launch.bat
```

### Running Unit Tests

```bat
out\build\x64-debug\poliable_tests.exe
```

---

## 📄 License

MIT License. See [LICENSE](LICENSE) for details.
