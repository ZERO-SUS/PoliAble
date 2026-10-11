#include "TimelinePanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

TimelinePanel::TimelinePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Transport Toolbar ──
    auto* transport = new QWidget(this);
    transport->setStyleSheet("background: #1a1b20; border-bottom: 1px solid #262832;");
    auto* tLayout = new QHBoxLayout(transport);
    tLayout->setContentsMargins(10, 4, 10, 4);
    tLayout->setSpacing(6);

    auto makeBtn = [&](const QString& text, const QString& tip) {
        auto* b = new QPushButton(text, transport);
        b->setToolTip(tip);
        b->setFixedSize(24, 22);
        b->setStyleSheet(
            "QPushButton { background: #20222a; color: #c0c3d2; border: 1px solid #2d303c; "
            "border-radius: 3px; font-size: 10px; } "
            "QPushButton:hover { background: #2e3240; color: #ffffff; }"
        );
        return b;
    };

    tLayout->addWidget(makeBtn("⏮", "First Frame"));
    tLayout->addWidget(makeBtn("◀", "Previous Frame"));
    tLayout->addWidget(makeBtn("▶", "Play Animation"));
    tLayout->addWidget(makeBtn("⏸", "Pause"));
    tLayout->addWidget(makeBtn("⏭", "Next Frame"));

    auto* timecode = new QLabel("00:00:00.00", transport);
    timecode->setStyleSheet("color: #6366f1; font-family: monospace; font-size: 11px; font-weight: bold; margin-left: 8px;");
    tLayout->addWidget(timecode);

    tLayout->addStretch();

    auto* fpsBadge = new QLabel("60 FPS", transport);
    fpsBadge->setStyleSheet(
        "background: #252834; color: #a5b4fc; border: 1px solid #373b4d; "
        "border-radius: 3px; padding: 2px 6px; font-size: 10px; font-weight: bold;"
    );
    tLayout->addWidget(fpsBadge);

    layout->addWidget(transport);

    // ── Track list & timeline placeholder ──
    auto* tracksWidget = new QWidget(this);
    tracksWidget->setStyleSheet("background: #15161a;");
    auto* tracksLayout = new QVBoxLayout(tracksWidget);
    tracksLayout->setContentsMargins(12, 8, 12, 8);
    tracksLayout->setSpacing(4);

    auto addTrackRow = [&](const QString& name, const QString& value) {
        auto* row = new QWidget(tracksWidget);
        auto* rLayout = new QHBoxLayout(row);
        rLayout->setContentsMargins(4, 2, 4, 2);

        auto* nameLbl = new QLabel(name, row);
        nameLbl->setStyleSheet("color: #9da1b2; font-size: 11px;");
        rLayout->addWidget(nameLbl);

        rLayout->addStretch();

        auto* valLbl = new QLabel(value, row);
        valLbl->setStyleSheet("color: #64748b; font-size: 10px; font-family: monospace;");
        rLayout->addWidget(valLbl);

        tracksLayout->addWidget(row);
    };

    addTrackRow("◆  Transform.x / y", "0.0s ─── ◆ ─── 1.5s");
    addTrackRow("◆  Transform.opacity", "0.0s ─── ◆ ─── 1.0s");
    addTrackRow("◆  Style.fillColor", "0.0s ───────── 2.0s");

    tracksLayout->addStretch();
    layout->addWidget(tracksWidget);
}
