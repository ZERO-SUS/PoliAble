#include "AssetsPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

AssetsPanel::AssetsPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: #17181c; border: none; }");

    auto* content = new QWidget(scroll);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    // ── Section 1: Color Tokens ──
    auto* colorHeader = new QLabel("Design Tokens · Colors", content);
    colorHeader->setStyleSheet("QLabel { color: #8e92a4; font-size: 10px; font-weight: bold; text-transform: uppercase; }");
    layout->addWidget(colorHeader);

    auto* colorGrid = new QGridLayout();
    colorGrid->setSpacing(6);

    struct Swatch { const char* name; const char* hex; };
    const Swatch swatches[] = {
        {"Obsidian", "#0f172a"},
        {"Slate",    "#334155"},
        {"Indigo",   "#6366f1"},
        {"Sky",      "#38bdf8"},
        {"Cyan",     "#06b6d4"},
        {"Emerald",  "#10b981"},
        {"Amber",    "#f59e0b"},
        {"Rose",     "#f43f5e"},
        {"Violet",   "#8b5cf6"},
        {"Pure White", "#ffffff"}
    };

    int col = 0, row = 0;
    for (const auto& sw : swatches) {
        auto* btn = new QPushButton(content);
        btn->setFixedSize(26, 26);
        btn->setToolTip(QString("%1 (%2)").arg(sw.name, sw.hex));
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; border: 1px solid #363946; border-radius: 4px; } "
            "QPushButton:hover { border: 2px solid #ffffff; }"
        ).arg(sw.hex));
        QString hexStr = sw.hex;
        connect(btn, &QPushButton::clicked, this, [this, hexStr]() {
            emit colorSelected(hexStr);
        });
        colorGrid->addWidget(btn, row, col);
        if (++col >= 5) {
            col = 0;
            ++row;
        }
    }
    layout->addLayout(colorGrid);

    // Divider
    auto* line = new QFrame(content);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background: #23252e; border: none; height: 1px;");
    layout->addWidget(line);

    // ── Section 2: Component Presets ──
    auto* compHeader = new QLabel("UI Components · 1-Click Insert", content);
    compHeader->setStyleSheet("QLabel { color: #8e92a4; font-size: 10px; font-weight: bold; text-transform: uppercase; }");
    layout->addWidget(compHeader);

    auto addPresetBtn = [&](const QString& label, const QString& type, const QString& icon) {
        auto* btn = new QPushButton(icon + "  " + label, content);
        btn->setStyleSheet(
            "QPushButton { background: #20222a; color: #d0d3de; border: 1px solid #2d303c; "
            "border-radius: 5px; padding: 7px 10px; text-align: left; font-size: 11px; font-weight: 500; } "
            "QPushButton:hover { background: #282a36; border-color: #6366f1; color: #ffffff; }"
        );
        connect(btn, &QPushButton::clicked, this, [this, type]() {
            emit insertComponentRequested(type);
        });
        layout->addWidget(btn);
    };

    addPresetBtn("Primary Button", "button", "🔘");
    addPresetBtn("Feature Card", "card", "🗂️");
    addPresetBtn("Navigation Bar", "navbar", "🧭");
    addPresetBtn("Metric / Stat Card", "stat", "📊");
    addPresetBtn("Hero Section", "hero", "✨");

    layout->addStretch();
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}

void AssetsPanel::refresh() {
    // Dynamic refresh if design tokens change
}
