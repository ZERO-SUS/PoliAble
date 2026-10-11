#pragma once
// Qt stub headers for headless build
// Replace with real Qt headers when Qt6 is properly installed

// Minimal stubs for Qt classes we use
namespace Qt {
    enum class AlignmentFlag { AlignLeft = 0x0001, AlignCenter = 0x0004 };
    enum class Orientation { Horizontal = 0, Vertical = 1 };
}

namespace poliable {
    // Stub classes - placeholder for real Qt types
    class StubWidget {};
    class StubMainWindow : public StubWidget {};
    class StubQLabel : public StubWidget {};
    class StubQPushButton : public StubWidget {};
    class StubQLineEdit : public StubWidget {};
    class StubQDoubleSpinBox : public StubWidget {};
    class StubQTreeWidget : public StubWidget {};
    class StubQVBoxLayout : public StubWidget {};
    class StubQFormLayout : public StubWidget {};
    class StubQSplitter : public StubWidget {};
    class StubQDockWidget : public StubWidget {};
    class StubQMenuBar : public StubWidget {};
    class StubQMenu : public StubWidget {};
    class StubQAction {};
    class StubQStatusBar : public StubWidget {};
    class StubQApplication {
    public:
        static void exec() {}
        static void quit() {}
        static void setStyle(const char*) {}
        static void setStyleSheet(const char*) {}
    };
}
