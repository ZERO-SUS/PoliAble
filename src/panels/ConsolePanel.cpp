#include "ConsolePanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QDateTime>
#include <QString>
#include <string>

ConsolePanel::ConsolePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background: #1a1b20; border-bottom: 1px solid #262832;");
    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(10, 4, 10, 4);

    auto* title = new QLabel("Output Console", headerWidget);
    title->setStyleSheet("QLabel { color: #8e92a4; font-size: 11px; font-weight: 600; text-transform: uppercase; }");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

    auto* clearBtn = new QPushButton("Clear", headerWidget);
    clearBtn->setStyleSheet(
        "QPushButton { background: #20222a; color: #a0a3b2; border: 1px solid #2d303c; "
        "border-radius: 3px; padding: 2px 8px; font-size: 10px; } "
        "QPushButton:hover { background: #2a2d38; color: #ffffff; }"
    );
    connect(clearBtn, &QPushButton::clicked, this, &ConsolePanel::clear);
    headerLayout->addWidget(clearBtn);

    layout->addWidget(headerWidget);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(500);
    m_output->setStyleSheet(
        "QPlainTextEdit { background: #141518; color: #cbd0e0; "
        "font-family: 'Consolas', 'Cascadia Code', 'Courier New', monospace; font-size: 11px; "
        "border: none; padding: 6px; selection-background-color: #6366f1; }"
    );
    layout->addWidget(m_output);
}

void ConsolePanel::log(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_output->appendPlainText(QString("[%1]  %2").arg(timestamp, message));
    auto* bar = m_output->verticalScrollBar();
    if (bar) bar->setValue(bar->maximum());
}

void ConsolePanel::log(const std::string& message) {
    log(QString::fromStdString(message));
}

void ConsolePanel::clear() {
    m_output->clear();
}
