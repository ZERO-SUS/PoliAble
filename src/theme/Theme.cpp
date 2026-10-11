#include "Theme.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

void Theme::apply() {
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette dark;
    // Window & background surfaces
    dark.setColor(QPalette::Window,          QColor(22, 23, 27));
    dark.setColor(QPalette::WindowText,      QColor(240, 242, 248));
    dark.setColor(QPalette::Base,            QColor(18, 19, 22));
    dark.setColor(QPalette::AlternateBase,   QColor(26, 28, 34));
    dark.setColor(QPalette::ToolTipBase,     QColor(32, 34, 42));
    dark.setColor(QPalette::ToolTipText,     QColor(240, 242, 248));

    // Text colors
    dark.setColor(QPalette::Text,            QColor(230, 233, 242));
    dark.setColor(QPalette::BrightText,      Qt::white);

    // Buttons
    dark.setColor(QPalette::Button,          QColor(34, 36, 45));
    dark.setColor(QPalette::ButtonText,      QColor(230, 233, 242));

    // Highlights & links (Modern Indigo)
    dark.setColor(QPalette::Highlight,       QColor(99, 102, 241));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    dark.setColor(QPalette::Link,            QColor(129, 140, 248));

    // Disabled states
    dark.setColor(QPalette::Disabled, QPalette::Text,       QColor(95, 99, 115));
    dark.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(95, 99, 115));
    dark.setColor(QPalette::Disabled, QPalette::WindowText, QColor(95, 99, 115));

    qApp->setPalette(dark);

    // Comprehensive modern Studio stylesheet
    qApp->setStyleSheet(
        // Base Font and Window
        "QWidget { "
        "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; "
        "  font-size: 12px; "
        "} "
        "QMainWindow { background: #141518; } "

        // Menu Bar & Menus
        "QMenuBar { background: #16171b; color: #c4c7d4; border-bottom: 1px solid #252730; padding: 2px 6px; } "
        "QMenuBar::item { background: transparent; padding: 5px 10px; border-radius: 4px; color: #c4c7d4; } "
        "QMenuBar::item:selected { background: #252833; color: #ffffff; } "
        "QMenu { background: #1d1f26; color: #c4c7d4; border: 1px solid #2c2f3b; border-radius: 6px; padding: 5px; } "
        "QMenu::item { padding: 6px 22px 6px 12px; border-radius: 4px; } "
        "QMenu::item:selected { background: #6366f1; color: #ffffff; } "
        "QMenu::separator { height: 1px; background: #2a2d38; margin: 4px 6px; } "

        // Toolbars
        "QToolBar { background: #16171b; border-bottom: 1px solid #252730; spacing: 5px; padding: 5px 10px; } "
        "QToolButton { color: #c4c7d4; padding: 5px 9px; border: 1px solid transparent; border-radius: 5px; font-size: 11px; font-weight: 500; } "
        "QToolButton:hover { background: #262934; border-color: #343847; color: #ffffff; } "
        "QToolButton:pressed { background: #4f46e5; border-color: #6366f1; color: #ffffff; } "
        "QToolButton:checked { background: #2b2f42; border-color: #6366f1; color: #a5b4fc; } "

        // Dock Widgets
        "QDockWidget { color: #d1d4e0; font-size: 11px; font-weight: 600; } "
        "QDockWidget::title { "
        "  background: #1a1b20; color: #9da1b2; padding: 7px 10px; "
        "  border-bottom: 1px solid #262832; font-size: 11px; font-weight: 600; "
        "  text-transform: uppercase; letter-spacing: 0.5px; "
        "} "
        "QDockWidget::close-button, QDockWidget::float-button { "
        "  background: transparent; border-radius: 3px; padding: 2px; "
        "} "
        "QDockWidget::close-button:hover, QDockWidget::float-button:hover { background: #2e3240; } "

        // Buttons
        "QPushButton { "
        "  background: #252732; color: #e1e4ee; border: 1px solid #333644; "
        "  padding: 5px 12px; border-radius: 5px; font-size: 11px; font-weight: 500; "
        "} "
        "QPushButton:hover { background: #2e313f; border-color: #45495c; color: #ffffff; } "
        "QPushButton:pressed { background: #6366f1; border-color: #6366f1; color: #ffffff; } "
        "QPushButton:disabled { background: #1a1b22; border-color: #262832; color: #626574; } "

        // Inputs & Spinboxes
        "QLineEdit, QSpinBox, QDoubleSpinBox { "
        "  background: #1e2027; color: #f0f2f8; border: 1px solid #2e313d; "
        "  padding: 4px 7px; border-radius: 4px; font-size: 11px; selection-background-color: #6366f1; "
        "} "
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus { "
        "  border: 1px solid #6366f1; background: #22252e; "
        "} "
        "QSpinBox::up-button, QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 14px; border: none; } "
        "QSpinBox::down-button, QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 14px; border: none; } "

        // Comboboxes
        "QComboBox { "
        "  background: #1e2027; color: #f0f2f8; border: 1px solid #2e313d; "
        "  padding: 4px 8px; border-radius: 4px; font-size: 11px; "
        "} "
        "QComboBox:hover, QComboBox:focus { border: 1px solid #6366f1; } "
        "QComboBox::drop-down { border: none; width: 18px; } "
        "QComboBox QAbstractItemView { "
        "  background: #1d1f26; color: #e1e4ee; border: 1px solid #2e313d; "
        "  border-radius: 5px; selection-background-color: #6366f1; selection-color: #ffffff; "
        "} "

        // Trees & Lists
        "QTreeWidget, QListWidget { "
        "  background: #17181c; color: #c8cbd6; border: none; font-size: 12px; outline: none; "
        "} "
        "QTreeWidget::item, QListWidget::item { height: 26px; padding-left: 4px; border-radius: 4px; } "
        "QTreeWidget::item:hover, QListWidget::item:hover { background: #22242d; color: #ffffff; } "
        "QTreeWidget::item:selected, QListWidget::item:selected { background: #2c3042; color: #ffffff; border-left: 2px solid #6366f1; } "

        // Tab Widgets
        "QTabWidget::pane { border: 1px solid #252731; background: #17181c; border-radius: 4px; } "
        "QTabBar::tab { "
        "  background: #17181c; color: #8e92a4; padding: 7px 14px; "
        "  border: none; font-size: 11px; font-weight: 600; "
        "} "
        "QTabBar::tab:selected { background: #1f2129; color: #ffffff; border-bottom: 2px solid #6366f1; } "
        "QTabBar::tab:hover { color: #ffffff; background: #1d1f25; } "

        // Scrollbars
        "QScrollBar:vertical { background: #141518; width: 8px; margin: 0; } "
        "QScrollBar::handle:vertical { background: #2e313d; min-height: 24px; border-radius: 4px; } "
        "QScrollBar::handle:vertical:hover { background: #404454; } "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; } "
        "QScrollBar:horizontal { background: #141518; height: 8px; margin: 0; } "
        "QScrollBar::handle:horizontal { background: #2e313d; min-width: 24px; border-radius: 4px; } "
        "QScrollBar::handle:horizontal:hover { background: #404454; } "
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; } "

        // Splitter
        "QSplitter::handle { background: #21232c; } "
        "QSplitter::handle:hover { background: #6366f1; } "

        // Status Bar
        "QStatusBar { background: #16171b; color: #858999; font-size: 11px; border-top: 1px solid #23252d; } "
        "QStatusBar QLabel { color: #9da1b2; } "

        // Tooltips
        "QToolTip { background: #22242e; color: #ffffff; border: 1px solid #333644; border-radius: 4px; padding: 4px 8px; font-size: 11px; } "
    );
}

