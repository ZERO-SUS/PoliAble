#include "CodePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>

CodePanel::CodePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Header / Action Bar ──
    auto* header = new QWidget(this);
    header->setStyleSheet("background: #1a1b20; border-bottom: 1px solid #262832;");
    auto* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(10, 5, 10, 5);
    hLayout->setSpacing(6);

    auto* title = new QLabel("Code Inspector", header);
    title->setStyleSheet("QLabel { color: #8e92a4; font-size: 11px; font-weight: bold; text-transform: uppercase; }");
    hLayout->addWidget(title);

    m_statusLabel = new QLabel("● Live Sync", header);
    m_statusLabel->setStyleSheet("QLabel { color: #10b981; font-size: 10px; font-weight: bold; margin-left: 6px; }");
    hLayout->addWidget(m_statusLabel);

    hLayout->addStretch();

    auto* copyBtn = new QPushButton("📋 Copy", header);
    copyBtn->setToolTip("Copy code to clipboard");
    copyBtn->setStyleSheet(
        "QPushButton { background: #22242c; color: #cbd0e0; border: 1px solid #2f3240; "
        "border-radius: 4px; padding: 3px 9px; font-size: 11px; } "
        "QPushButton:hover { background: #2c2f3d; color: #ffffff; }"
    );
    connect(copyBtn, &QPushButton::clicked, this, &CodePanel::onCopyClicked);
    hLayout->addWidget(copyBtn);

    auto* exportBtn = new QPushButton("🚀 Export Project...", header);
    exportBtn->setToolTip("Export project files to disk");
    exportBtn->setStyleSheet(
        "QPushButton { background: #4f46e5; color: #ffffff; border: 1px solid #6366f1; "
        "border-radius: 4px; padding: 3px 12px; font-size: 11px; font-weight: bold; } "
        "QPushButton:hover { background: #4338ca; }"
    );
    connect(exportBtn, &QPushButton::clicked, this, &CodePanel::onExportClicked);
    hLayout->addWidget(exportBtn);

    mainLayout->addWidget(header);

    // ── Tabs ──
    m_tabs = new QTabWidget(this);
    m_tabs->setStyleSheet(
        "QTabWidget::pane { border: none; background: #141518; } "
        "QTabBar::tab { background: #17181c; color: #8e92a4; padding: 6px 14px; font-size: 11px; font-weight: 600; border: none; } "
        "QTabBar::tab:selected { background: #1e2026; color: #ffffff; border-bottom: 2px solid #6366f1; } "
    );

    auto createEditor = [this]() {
        auto* ed = new QPlainTextEdit(this);
        ed->setReadOnly(true);
        ed->setStyleSheet(
            "QPlainTextEdit { background: #131417; color: #d1d5db; "
            "font-family: 'Consolas', 'Cascadia Code', 'Courier New', monospace; "
            "font-size: 11px; line-height: 1.4; border: none; padding: 8px; selection-background-color: #6366f1; }"
        );
        return ed;
    };

    m_htmlEditor = createEditor();
    m_cssEditor  = createEditor();
    m_reactEditor= createEditor();
    m_twEditor   = createEditor();

    m_tabs->addTab(m_htmlEditor, "HTML5");
    m_tabs->addTab(m_cssEditor,  "CSS");
    m_tabs->addTab(m_reactEditor,"React (TSX)");
    m_tabs->addTab(m_twEditor,   "Tailwind CSS");

    mainLayout->addWidget(m_tabs);
}

void CodePanel::setDocument(poliable::Document* doc) {
    m_doc = doc;
    refresh();
}

void CodePanel::refresh() {
    if (!m_doc) return;
    m_cachedCode = poliable::CodeGenerator::generate(*m_doc);
    updateEditorContent();
}

void CodePanel::updateEditorContent() {
    m_htmlEditor->setPlainText(QString::fromStdString(m_cachedCode.html));
    m_cssEditor->setPlainText(QString::fromStdString(m_cachedCode.css));
    m_reactEditor->setPlainText(QString::fromStdString(m_cachedCode.reactTsx));
    m_twEditor->setPlainText(QString::fromStdString(m_cachedCode.tailwindHtml));
}

void CodePanel::onTabChanged(int /*index*/) {
    // Current tab changed
}

void CodePanel::onCopyClicked() {
    int idx = m_tabs->currentIndex();
    QString text;
    if (idx == 0) text = m_htmlEditor->toPlainText();
    else if (idx == 1) text = m_cssEditor->toPlainText();
    else if (idx == 2) text = m_reactEditor->toPlainText();
    else if (idx == 3) text = m_twEditor->toPlainText();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);

    if (m_statusLabel) {
        m_statusLabel->setText("✓ Copied!");
        m_statusLabel->setStyleSheet("color: #60a5fa; font-size: 10px; font-weight: bold; margin-left: 6px;");
    }
}

void CodePanel::onExportClicked() {
    if (!m_doc) return;

    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Target Export Directory", "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;

    int idx = m_tabs->currentIndex();
    poliable::ExportFormat fmt = poliable::ExportFormat::HtmlCss;
    if (idx == 2) fmt = poliable::ExportFormat::ReactTypeScript;
    else if (idx == 3) fmt = poliable::ExportFormat::TailwindHtml;

    std::string err;
    if (poliable::CodeGenerator::exportProject(*m_doc, dir.toStdString(), fmt, err)) {
        QMessageBox::information(this, "Export Success",
            "Project successfully exported to:\n" + dir);
        emit projectExported(dir);
    } else {
        QMessageBox::critical(this, "Export Error",
            "Failed to export project:\n" + QString::fromStdString(err));
    }
}
