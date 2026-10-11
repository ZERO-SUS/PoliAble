#include "MainWindow.h"
#include "panels/LayersPanel.h"
#include "panels/AssetsPanel.h"
#include "panels/PropertiesPanel.h"
#include "panels/TimelinePanel.h"
#include "panels/ConsolePanel.h"
#include "canvas/CanvasView.h"
#include "canvas/CanvasItem.h"
#include "theme/Theme.h"
#include "model/Document.h"
#include "model/Node.h"
#include "model/Command.h"
#include "model/Serializer.h"
#include "model/UndoStack.h"
#include "layout/LayoutEngine.h"

#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QKeySequence>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QString>
#include <QSettings>
#include <memory>

// ── Construction ─────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_doc(std::make_unique<poliable::Document>("Untitled"))
{
    Theme::apply();
    setWindowTitle("PoliAble Studio");
    resize(1440, 900);
    setMinimumSize(960, 640);
    setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks);

    buildDocks();
    buildMenus();
    buildToolbar();
    buildStatusBar();
    connectSignals();

    updateTitle();
    logToConsole("PoliAble Studio initialized. Ready for designing.");
}

// ── Build UI ──────────────────────────────────────────────────────────────────

void MainWindow::buildDocks() {
    // Central canvas
    m_canvas = new CanvasView(this);
    m_canvas->setDocument(m_doc.get());
    setCentralWidget(m_canvas);

    // ── Left dock: Layers & Assets (Tabbed) ──
    auto* leftTabs = new QTabWidget(this);
    leftTabs->setObjectName("LeftTabs");
    leftTabs->setTabPosition(QTabWidget::North);

    m_layersPanel = new LayersPanel(this);
    m_layersPanel->setDocument(m_doc.get());
    m_assetsPanel = new AssetsPanel(this);

    leftTabs->addTab(m_layersPanel, "  Layers  ");
    leftTabs->addTab(m_assetsPanel, "  Assets & Tokens  ");

    auto* leftDock = new QDockWidget("Project Outline", this);
    leftDock->setWidget(leftTabs);
    leftDock->setObjectName("LeftDock");
    leftDock->setMinimumWidth(240);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // ── Right dock: Properties Inspector ──
    m_propsPanel = new PropertiesPanel(this);
    m_propsPanel->setDocument(m_doc.get());
    m_propsPanel->setUndoStack(&m_undoStack);

    auto* rightDock = new QDockWidget("Inspector", this);
    rightDock->setWidget(m_propsPanel);
    rightDock->setObjectName("RightDock");
    rightDock->setMinimumWidth(260);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // ── Bottom dock: Console & Timeline (Tabbed) ──
    auto* bottomTabs = new QTabWidget(this);
    bottomTabs->setObjectName("BottomTabs");
    bottomTabs->setTabPosition(QTabWidget::North);

    m_consolePanel = new ConsolePanel(this);
    m_timelinePanel = new TimelinePanel(this);

    bottomTabs->addTab(m_consolePanel, "  Console  ");
    bottomTabs->addTab(m_timelinePanel, "  Timeline  ");

    auto* bottomDock = new QDockWidget("Output & Sequencer", this);
    bottomDock->setWidget(bottomTabs);
    bottomDock->setObjectName("BottomDock");
    bottomDock->setMaximumHeight(200);
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock);
}

void MainWindow::buildMenus() {
    // ── File ──
    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Project",     this, &MainWindow::newDocument,   QKeySequence::New);
    fileMenu->addAction("Open Project…",   this, &MainWindow::openDocument,  QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("Save",            this, &MainWindow::saveDocument,  QKeySequence::Save);
    fileMenu->addAction("Save As…",        this, &MainWindow::saveDocumentAs,QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit",            qApp, &QApplication::quit,        QKeySequence::Quit);

    // ── Edit ──
    auto* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("Undo",             this, &MainWindow::undo,              QKeySequence::Undo);
    editMenu->addAction("Redo",             this, &MainWindow::redo,              QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("Duplicate Layer",  this, &MainWindow::duplicateSelected, QKeySequence(Qt::CTRL | Qt::Key_D));
    editMenu->addAction("Delete Selected",  this, &MainWindow::deleteSelected,    QKeySequence::Delete);

    // ── Insert ──
    auto* insertMenu = menuBar()->addMenu("&Insert");
    insertMenu->addAction("Rectangle",       this, &MainWindow::addRectangle,     QKeySequence(Qt::Key_R));
    insertMenu->addAction("Ellipse",         this, &MainWindow::addEllipse,       QKeySequence(Qt::Key_O));
    insertMenu->addAction("Text Element",    this, &MainWindow::addText,          QKeySequence(Qt::Key_T));
    insertMenu->addAction("Frame Artboard",  this, &MainWindow::addFrame,         QKeySequence(Qt::Key_F));
    insertMenu->addSeparator();
    insertMenu->addAction("Flex Container (Row)", this, &MainWindow::addFlexContainer);
    insertMenu->addAction("CSS Grid Container",   this, &MainWindow::addGridContainer);

    // ── Responsive Layout ──
    auto* layoutMenu = menuBar()->addMenu("&Responsive");
    layoutMenu->addAction("Desktop (1440×900)", this, [this]() { setBreakpoint(0); });
    layoutMenu->addAction("Laptop (1024×768)",  this, [this]() { setBreakpoint(1); });
    layoutMenu->addAction("Tablet (768×1024)",   this, [this]() { setBreakpoint(2); });
    layoutMenu->addAction("Mobile (375×812)",   this, [this]() { setBreakpoint(3); });

    // ── Arrange ──
    auto* arrangeMenu = menuBar()->addMenu("&Arrange");
    arrangeMenu->addAction("Bring to Front", this, &MainWindow::bringToFront, QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    arrangeMenu->addAction("Send to Back",   this, &MainWindow::sendToBack,   QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));

    // ── View ──
    auto* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction("Zoom In",           m_canvas, &CanvasView::zoomIn,   QKeySequence::ZoomIn);
    viewMenu->addAction("Zoom Out",          m_canvas, &CanvasView::zoomOut,  QKeySequence::ZoomOut);
    viewMenu->addAction("Reset Zoom (100%)", m_canvas, &CanvasView::resetZoom, QKeySequence(Qt::CTRL | Qt::Key_0));
    viewMenu->addAction("Zoom to Fit",       m_canvas, &CanvasView::zoomToFit, QKeySequence(Qt::SHIFT | Qt::Key_1));
    viewMenu->addSeparator();
    for (auto* dock : findChildren<QDockWidget*>()) {
        viewMenu->addAction(dock->toggleViewAction());
    }

    // ── Help ──
    auto* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("About PoliAble", this, [this]() {
        QMessageBox::about(this, "About PoliAble Studio",
            "<b>PoliAble Studio</b><br>"
            "Version 0.1.0 — Clean Vector Design & Reactive Layout Engine<br><br>"
            "• Local-first vector graphics<br>"
            "• Real Flexbox & CSS Grid containers<br>"
            "• Responsive breakpoint layouts<br>"
            "• Design Tokens & reusable UI kits<br>"
            "• Undo / Redo command history<br><br>"
            "<i>Native C++20 · Qt6 · Professional Performance.</i>");
    });
}

void MainWindow::buildToolbar() {
    auto* tb = addToolBar("Studio Toolbar");
    tb->setObjectName("MainToolbar");
    tb->setMovable(false);
    tb->setIconSize(QSize(16, 16));

    // ── Brand Logo Badge ──
    auto* logoBadge = new QLabel("✦ PoliAble", tb);
    logoBadge->setStyleSheet(
        "QLabel { background: #222533; color: #818cf8; font-weight: bold; font-size: 12px; "
        "padding: 4px 10px; border-radius: 4px; border: 1px solid #363b52; margin-right: 8px; }"
    );
    tb->addWidget(logoBadge);

    // ── Creation Tools ──
    tb->addAction("▭ Rect",     this, &MainWindow::addRectangle);
    tb->addAction("○ Ellipse",  this, &MainWindow::addEllipse);
    tb->addAction("T Text",     this, &MainWindow::addText);
    tb->addAction("▢ Frame",    this, &MainWindow::addFrame);
    tb->addAction("☰ Flex",     this, &MainWindow::addFlexContainer);
    tb->addAction("⊞ Grid",     this, &MainWindow::addGridContainer);

    tb->addSeparator();

    // ── Viewport Selector ──
    auto* vpLabel = new QLabel(" Viewport: ", tb);
    vpLabel->setStyleSheet("QLabel { color: #8a8d9e; font-size: 11px; font-weight: 600; }");
    tb->addWidget(vpLabel);

    m_breakpointCombo = new QComboBox(tb);
    m_breakpointCombo->addItems({
        "Desktop (1440×900)",
        "Laptop (1024×768)",
        "Tablet (768×1024)",
        "Mobile (375×812)"
    });
    connect(m_breakpointCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::setBreakpoint);
    tb->addWidget(m_breakpointCombo);

    tb->addSeparator();

    // ── History & Layer Actions ──
    tb->addAction("↩ Undo",      this, &MainWindow::undo);
    tb->addAction("↪ Redo",      this, &MainWindow::redo);
    tb->addAction("⧉ Duplicate", this, &MainWindow::duplicateSelected);
    tb->addAction("🗑 Delete",   this, &MainWindow::deleteSelected);

    tb->addSeparator();

    // ── Zoom Actions ──
    tb->addAction("⊖",   m_canvas, &CanvasView::zoomOut);
    tb->addAction("⊕",   m_canvas, &CanvasView::zoomIn);
    tb->addAction("Fit", m_canvas, &CanvasView::zoomToFit);

    // Spacer to push Save button to right
    auto* spacer = new QWidget(tb);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tb->addWidget(spacer);

    // ── Quick Save Button ──
    auto* saveBtn = new QPushButton("💾 Save", tb);
    saveBtn->setStyleSheet(
        "QPushButton { background: #4f46e5; color: #ffffff; border: 1px solid #6366f1; "
        "font-weight: 600; padding: 4px 14px; border-radius: 4px; } "
        "QPushButton:hover { background: #4338ca; }"
    );
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveDocument);
    tb->addWidget(saveBtn);
}

void MainWindow::buildStatusBar() {
    statusBar()->showMessage("Ready");

    m_statusSelectionLabel = new QLabel("No selection", this);
    m_statusSelectionLabel->setStyleSheet("color: #9496a8; padding: 0 8px;");
    statusBar()->addPermanentWidget(m_statusSelectionLabel);

    m_statusDocLabel = new QLabel("1440 × 900 px", this);
    m_statusDocLabel->setStyleSheet("color: #6366f1; font-weight: bold; padding: 0 8px;");
    statusBar()->addPermanentWidget(m_statusDocLabel);

    auto* brandVer = new QLabel("PoliAble Studio v0.1", this);
    brandVer->setStyleSheet("color: #717488; padding: 0 8px;");
    statusBar()->addPermanentWidget(brandVer);
}

void MainWindow::connectSignals() {
    // Canvas selection → properties panel
    connect(m_canvas, &CanvasView::selectionChanged,
            this,     &MainWindow::onCanvasSelectionChanged);

    // Canvas node changes → layers refresh
    connect(m_canvas, &CanvasView::nodesChanged,
            m_layersPanel, &LayersPanel::refresh);

    // CanvasView signals for operations
    connect(m_canvas, &CanvasView::selectedCanvasItemDelete,
            this, &MainWindow::onDeletePressed);

    // Layers node click → show in properties
    connect(m_layersPanel, &LayersPanel::nodeSelected,
            this, &MainWindow::onLayerNodeSelected);

    // Layers panel context menu requests
    connect(m_layersPanel, &LayersPanel::requestDelete,
            this, &MainWindow::deleteSelected);
    connect(m_layersPanel, &LayersPanel::requestDuplicate,
            this, &MainWindow::duplicateSelected);
    connect(m_layersPanel, &LayersPanel::requestBringToFront,
            this, &MainWindow::bringToFront);
    connect(m_layersPanel, &LayersPanel::requestSendToBack,
            this, &MainWindow::sendToBack);

    // Assets panel signals
    connect(m_assetsPanel, &AssetsPanel::colorSelected,
            this, &MainWindow::onAssetColorSelected);
    connect(m_assetsPanel, &AssetsPanel::insertComponentRequested,
            this, &MainWindow::onAssetComponentInsert);

    // Layout changed in properties panel -> update canvas & layers
    connect(m_propsPanel, &PropertiesPanel::layoutModified,
            this, &MainWindow::onLayoutModified);
}

// ── File operations ───────────────────────────────────────────────────────────

void MainWindow::newDocument() {
    if (m_modified) {
        auto btn = QMessageBox::question(this, "New Document",
            "You have unsaved changes. Discard them?",
            QMessageBox::Discard | QMessageBox::Cancel);
        if (btn != QMessageBox::Discard) return;
    }
    m_doc = std::make_unique<poliable::Document>("Untitled");
    m_undoStack.clear();
    m_currentFilePath.clear();
    m_modified = false;
    m_canvas->setDocument(m_doc.get());
    m_layersPanel->setDocument(m_doc.get());
    m_propsPanel->setDocument(m_doc.get());
    updateTitle();
    logToConsole("New document created.");
}

void MainWindow::openDocument() {
    QString path = QFileDialog::getOpenFileName(
        this, "Open PoliAble Project", "",
        "PoliAble Project (*.poliable);;All Files (*)");
    if (path.isEmpty()) return;
    loadFile(path);
}

void MainWindow::saveDocument() {
    if (m_currentFilePath.isEmpty()) {
        saveDocumentAs();
        return;
    }
    if (saveFile(m_currentFilePath)) {
        m_modified = false;
        updateTitle();
        statusBar()->showMessage("Saved: " + m_currentFilePath, 3000);
    }
}

void MainWindow::saveDocumentAs() {
    QString path = QFileDialog::getSaveFileName(
        this, "Save PoliAble Project", "",
        "PoliAble Project (*.poliable);;All Files (*)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".poliable")) path += ".poliable";
    if (saveFile(path)) {
        m_currentFilePath = path;
        m_modified = false;
        updateTitle();
        statusBar()->showMessage("Saved: " + path, 3000);
        logToConsole("Saved: " + path);
    }
}

void MainWindow::loadFile(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open file: " + path);
        return;
    }
    QTextStream in(&f);
    std::string text = in.readAll().toStdString();
    f.close();

    auto loaded = std::make_unique<poliable::Document>();
    std::string err;
    if (!poliable::Serializer::fromString(text, *loaded, err)) {
        QMessageBox::critical(this, "Error", "Failed to parse .poliable file:\n" + QString::fromStdString(err));
        return;
    }
    m_doc = std::move(loaded);
    m_currentFilePath = path;
    m_modified = false;
    m_undoStack.clear();
    m_canvas->setDocument(m_doc.get());
    m_layersPanel->setDocument(m_doc.get());
    m_propsPanel->setDocument(m_doc.get());
    updateTitle();
    statusBar()->showMessage("Loaded: " + path, 3000);
    logToConsole("Loaded: " + path);
}

bool MainWindow::saveFile(const QString& path) {
    if (!m_doc) return false;
    std::string text = poliable::Serializer::toString(*m_doc);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot write file: " + path);
        return false;
    }
    QTextStream out(&f);
    out << QString::fromStdString(text);
    f.close();
    return true;
}

// ── Node creation ─────────────────────────────────────────────────────────────

void MainWindow::addRectangle() {
    if (!m_doc) return;
    auto child = std::make_unique<poliable::Node>(
        poliable::NodeType::Rectangle, m_doc->nextId(), "Rectangle");
    child->transform().x = 60;
    child->transform().y = 60;
    child->transform().width  = 160;
    child->transform().height = 100;
    child->style().fillColor  = "#3b82f6";
    child->style().strokeColor = "#60a5fa";
    child->style().strokeWidth = 1.0;
    child->style().cornerRadius = 4.0;

    auto* raw = child.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(child), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->addCanvasItem(raw);
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Rectangle added", 2000);
}

void MainWindow::addEllipse() {
    if (!m_doc) return;
    auto child = std::make_unique<poliable::Node>(
        poliable::NodeType::Ellipse, m_doc->nextId(), "Ellipse");
    child->transform().x = 100;
    child->transform().y = 100;
    child->transform().width  = 120;
    child->transform().height = 120;
    child->style().fillColor  = "#8b5cf6";
    child->style().strokeColor = "#a78bfa";
    child->style().strokeWidth = 1.0;

    auto* raw = child.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(child), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->addCanvasItem(raw);
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Ellipse added", 2000);
}

void MainWindow::addText() {
    if (!m_doc) return;
    auto child = std::make_unique<poliable::Node>(
        poliable::NodeType::Text, m_doc->nextId(), "Text");
    child->transform().x = 240;
    child->transform().y = 60;
    child->transform().width  = 180;
    child->transform().height = 40;
    child->style().fillColor  = "#f1f3f9";
    child->style().textContent = "Heading Text";
    child->style().fontSize   = 18;

    auto* raw = child.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(child), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->addCanvasItem(raw);
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Text node added", 2000);
}

void MainWindow::addFrame() {
    if (!m_doc) return;
    auto child = std::make_unique<poliable::Node>(
        poliable::NodeType::Frame, m_doc->nextId(), "Frame");
    child->transform().x = 80;
    child->transform().y = 80;
    child->transform().width  = 340;
    child->transform().height = 220;
    child->style().strokeColor = "#6366f1";
    child->style().strokeWidth = 1.5;
    child->style().cornerRadius = 8.0;

    auto* raw = child.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(child), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->addCanvasItem(raw);
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Frame artboard added", 2000);
}

void MainWindow::addFlexContainer() {
    if (!m_doc) return;
    auto flex = std::make_unique<poliable::Node>(
        poliable::NodeType::Flex, m_doc->nextId(), "FlexContainer");
    flex->transform().x = 100;
    flex->transform().y = 100;
    flex->transform().width = 440;
    flex->transform().height = 180;
    flex->layout().mode = poliable::LayoutMode::Flex;
    flex->layout().flexDirection = poliable::FlexDirection::Row;
    flex->layout().paddingLeft = 16;
    flex->layout().paddingRight = 16;
    flex->layout().paddingTop = 16;
    flex->layout().paddingBottom = 16;
    flex->layout().gap = 12;

    auto c1 = std::make_unique<poliable::Node>(poliable::NodeType::Rectangle, m_doc->nextId(), "Card 1");
    c1->transform().width = 130;
    c1->transform().height = 90;
    c1->style().fillColor = "#3b82f6";
    c1->style().cornerRadius = 6.0;
    flex->addChild(std::move(c1));

    auto c2 = std::make_unique<poliable::Node>(poliable::NodeType::Rectangle, m_doc->nextId(), "Card 2");
    c2->transform().width = 130;
    c2->transform().height = 90;
    c2->style().fillColor = "#6366f1";
    c2->style().cornerRadius = 6.0;
    flex->addChild(std::move(c2));

    poliable::LayoutEngine::computeLayout(flex.get());

    auto* raw = flex.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(flex), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    logToConsole("Added Flex (Auto Layout) container with 2 cards.");
    statusBar()->showMessage("Flex container added", 2000);
}

void MainWindow::addGridContainer() {
    if (!m_doc) return;
    auto grid = std::make_unique<poliable::Node>(
        poliable::NodeType::Grid, m_doc->nextId(), "GridContainer");
    grid->transform().x = 100;
    grid->transform().y = 100;
    grid->transform().width = 460;
    grid->transform().height = 240;
    grid->layout().mode = poliable::LayoutMode::Grid;
    grid->layout().gridColumns = 2;
    grid->layout().gridRows = 2;
    grid->layout().columnGap = 12;
    grid->layout().rowGap = 12;
    grid->layout().paddingLeft = 16;
    grid->layout().paddingRight = 16;
    grid->layout().paddingTop = 16;
    grid->layout().paddingBottom = 16;

    for (int i = 0; i < 4; ++i) {
        auto cell = std::make_unique<poliable::Node>(
            poliable::NodeType::Rectangle, m_doc->nextId(), QString("Cell %1").arg(i + 1).toStdString());
        cell->layout().widthMode = poliable::SizingMode::Fill;
        cell->layout().heightMode = poliable::SizingMode::Fill;
        cell->style().fillColor = (i % 2 == 0) ? "#1e293b" : "#3b82f6";
        cell->style().cornerRadius = 6.0;
        grid->addChild(std::move(cell));
    }

    poliable::LayoutEngine::computeLayout(grid.get());

    auto* raw = grid.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(grid), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    logToConsole("Added 2x2 CSS Grid container with 4 cells.");
    statusBar()->showMessage("Grid container added", 2000);
}

void MainWindow::duplicateSelected() {
    CanvasItem* item = m_canvas->selectedItem();
    if (!item || !item->node() || !m_doc) return;

    poliable::Node* src = item->node();
    auto clone = std::make_unique<poliable::Node>(
        src->type(), m_doc->nextId(), src->name() + " Copy");
    clone->transform() = src->transform();
    clone->transform().x += 20;
    clone->transform().y += 20;
    clone->style() = src->style();
    clone->setLayout(src->layout());

    auto* raw = clone.get();
    auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(clone), raw->id());
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->addCanvasItem(raw);
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Layer duplicated", 2000);
}

void MainWindow::bringToFront() {
    CanvasItem* item = m_canvas->selectedItem();
    if (!item) return;
    item->setZValue(item->zValue() + 1);
    statusBar()->showMessage("Brought to front", 1500);
}

void MainWindow::sendToBack() {
    CanvasItem* item = m_canvas->selectedItem();
    if (!item) return;
    item->setZValue(std::max(0.0, item->zValue() - 1));
    statusBar()->showMessage("Sent to back", 1500);
}

void MainWindow::onAssetColorSelected(const QString& hexColor) {
    CanvasItem* item = m_canvas->selectedItem();
    if (!item || !item->node() || !m_doc) return;

    auto s = item->node()->style();
    s.fillColor = hexColor.toStdString();
    auto cmd = std::make_unique<poliable::SetStyleCommand>(item->node()->id(), item->node()->style(), s);
    m_undoStack.execute(std::move(cmd), *m_doc);
    item->updateFromNode();
    m_propsPanel->updateFromCanvas(item);
    m_modified = true;
    updateTitle();
    statusBar()->showMessage(QString("Fill color updated to %1").arg(hexColor), 2000);
}

void MainWindow::onAssetComponentInsert(const QString& componentType) {
    if (!m_doc) return;

    if (componentType == "button") {
        auto flex = std::make_unique<poliable::Node>(poliable::NodeType::Flex, m_doc->nextId(), "Button Primary");
        flex->transform().x = 100;
        flex->transform().y = 100;
        flex->transform().width = 160;
        flex->transform().height = 42;
        flex->style().fillColor = "#6366f1";
        flex->style().cornerRadius = 6.0;
        flex->layout().mode = poliable::LayoutMode::Flex;
        flex->layout().flexDirection = poliable::FlexDirection::Row;
        flex->layout().justifyContent = poliable::JustifyContent::Center;
        flex->layout().alignItems = poliable::AlignItems::Center;
        flex->layout().paddingLeft = 16;
        flex->layout().paddingRight = 16;

        auto text = std::make_unique<poliable::Node>(poliable::NodeType::Text, m_doc->nextId(), "Button Label");
        text->transform().width = 100;
        text->transform().height = 24;
        text->style().fillColor = "#ffffff";
        text->style().textContent = "Get Started";
        text->style().fontSize = 14;
        flex->addChild(std::move(text));

        poliable::LayoutEngine::computeLayout(flex.get());
        auto* raw = flex.get();
        auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(flex), raw->id());
        m_undoStack.execute(std::move(cmd), *m_doc);
        m_canvas->rebuildFromDocument();
        m_layersPanel->refresh();
        m_modified = true;
        updateTitle();
        statusBar()->showMessage("Primary Button inserted", 2000);
    } else if (componentType == "card") {
        auto flex = std::make_unique<poliable::Node>(poliable::NodeType::Flex, m_doc->nextId(), "Feature Card");
        flex->transform().x = 120;
        flex->transform().y = 120;
        flex->transform().width = 280;
        flex->transform().height = 180;
        flex->style().fillColor = "#1e2028";
        flex->style().strokeColor = "#333644";
        flex->style().strokeWidth = 1.0;
        flex->style().cornerRadius = 10.0;
        flex->layout().mode = poliable::LayoutMode::Flex;
        flex->layout().flexDirection = poliable::FlexDirection::Column;
        flex->layout().paddingLeft = 16;
        flex->layout().paddingRight = 16;
        flex->layout().paddingTop = 16;
        flex->layout().paddingBottom = 16;
        flex->layout().gap = 10;

        auto title = std::make_unique<poliable::Node>(poliable::NodeType::Text, m_doc->nextId(), "Card Title");
        title->transform().width = 240;
        title->transform().height = 24;
        title->style().fillColor = "#ffffff";
        title->style().textContent = "Analytics Engine";
        title->style().fontSize = 16;
        flex->addChild(std::move(title));

        auto desc = std::make_unique<poliable::Node>(poliable::NodeType::Text, m_doc->nextId(), "Card Description");
        desc->transform().width = 240;
        desc->transform().height = 50;
        desc->style().fillColor = "#9da1b2";
        desc->style().textContent = "High-performance vector canvas with reactive layout engine.";
        desc->style().fontSize = 12;
        flex->addChild(std::move(desc));

        poliable::LayoutEngine::computeLayout(flex.get());
        auto* raw = flex.get();
        auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(flex), raw->id());
        m_undoStack.execute(std::move(cmd), *m_doc);
        m_canvas->rebuildFromDocument();
        m_layersPanel->refresh();
        m_modified = true;
        updateTitle();
        statusBar()->showMessage("Feature Card inserted", 2000);
    } else if (componentType == "navbar") {
        auto nav = std::make_unique<poliable::Node>(poliable::NodeType::Flex, m_doc->nextId(), "Navbar Container");
        nav->transform().x = 40;
        nav->transform().y = 40;
        nav->transform().width = 800;
        nav->transform().height = 64;
        nav->style().fillColor = "#1a1b22";
        nav->style().strokeColor = "#2c2f3b";
        nav->style().strokeWidth = 1.0;
        nav->style().cornerRadius = 8.0;
        nav->layout().mode = poliable::LayoutMode::Flex;
        nav->layout().flexDirection = poliable::FlexDirection::Row;
        nav->layout().justifyContent = poliable::JustifyContent::SpaceBetween;
        nav->layout().alignItems = poliable::AlignItems::Center;
        nav->layout().paddingLeft = 24;
        nav->layout().paddingRight = 24;

        auto logo = std::make_unique<poliable::Node>(poliable::NodeType::Text, m_doc->nextId(), "Logo Text");
        logo->transform().width = 120;
        logo->transform().height = 28;
        logo->style().fillColor = "#6366f1";
        logo->style().textContent = "✦ PoliAble";
        logo->style().fontSize = 16;
        nav->addChild(std::move(logo));

        auto links = std::make_unique<poliable::Node>(poliable::NodeType::Text, m_doc->nextId(), "Nav Links");
        links->transform().width = 240;
        links->transform().height = 24;
        links->style().fillColor = "#cbd0e0";
        links->style().textContent = "Overview   Features   Pricing   Docs";
        links->style().fontSize = 12;
        nav->addChild(std::move(links));

        poliable::LayoutEngine::computeLayout(nav.get());
        auto* raw = nav.get();
        auto cmd = std::make_unique<poliable::AddNodeCommand>(0, std::move(nav), raw->id());
        m_undoStack.execute(std::move(cmd), *m_doc);
        m_canvas->rebuildFromDocument();
        m_layersPanel->refresh();
        m_modified = true;
        updateTitle();
        statusBar()->showMessage("Navbar inserted", 2000);
    } else {
        addFlexContainer();
    }
}

void MainWindow::setBreakpoint(int index) {
    if (!m_doc) return;
    poliable::Breakpoint bp = poliable::Breakpoint::Desktop;
    QString bpName = "Desktop (1440×900)";
    switch (index) {
        case 1: bp = poliable::Breakpoint::Laptop; bpName = "Laptop (1024×768)"; break;
        case 2: bp = poliable::Breakpoint::Tablet; bpName = "Tablet (768×1024)"; break;
        case 3: bp = poliable::Breakpoint::Mobile; bpName = "Mobile (375×812)"; break;
        default: bp = poliable::Breakpoint::Desktop; break;
    }
    poliable::LayoutEngine::applyBreakpoint(*m_doc, bp);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_propsPanel->updateFromCanvas(m_canvas->selectedItem());
    logToConsole(QString("Responsive Viewport switched to %1").arg(bpName));
    if (m_statusDocLabel) {
        m_statusDocLabel->setText(bpName);
    }
    statusBar()->showMessage(QString("Viewport: %1").arg(bpName), 2000);
}

void MainWindow::onLayoutModified() {
    if (!m_doc) return;
    poliable::LayoutEngine::computeDocumentLayout(*m_doc);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_modified = true;
    updateTitle();
}

void MainWindow::deleteSelected() {
    CanvasItem* item = m_canvas->selectedItem();
    if (!item || !item->node()) return;

    poliable::Node::Id id = item->node()->id();
    auto cmd = std::make_unique<poliable::RemoveNodeCommand>(id);
    m_undoStack.execute(std::move(cmd), *m_doc);
    m_canvas->removeItem(item);
    m_layersPanel->refresh();
    m_propsPanel->showNode(nullptr);
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Node deleted", 2000);
}

void MainWindow::undo() {
    if (!m_doc || !m_undoStack.canUndo()) return;
    m_undoStack.undo(*m_doc);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_propsPanel->showNode(nullptr);
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Undo", 1500);
}

void MainWindow::redo() {
    if (!m_doc || !m_undoStack.canRedo()) return;
    m_undoStack.redo(*m_doc);
    m_canvas->rebuildFromDocument();
    m_layersPanel->refresh();
    m_propsPanel->showNode(nullptr);
    m_modified = true;
    updateTitle();
    statusBar()->showMessage("Redo", 1500);
}

// ── Selection bridging ────────────────────────────────────────────────────────

void MainWindow::onCanvasSelectionChanged(CanvasItem* item) {
    m_propsPanel->updateFromCanvas(item);
    if (item && item->node()) {
        m_layersPanel->updateFromNode(item->node());
        if (m_statusSelectionLabel) {
            m_statusSelectionLabel->setText(QString("Selected: %1 (%2×%3)")
                .arg(QString::fromStdString(item->node()->name()))
                .arg(static_cast<int>(item->node()->transform().width))
                .arg(static_cast<int>(item->node()->transform().height)));
        }
    } else {
        if (m_statusSelectionLabel) {
            m_statusSelectionLabel->setText("No selection");
        }
    }
}

void MainWindow::onLayerNodeSelected(uint64_t nodeId) {
    if (!m_doc) return;
    poliable::Node* node = m_doc->find(nodeId);
    if (node) m_propsPanel->showNode(node);
}

// ── Misc ──────────────────────────────────────────────────────────────────────

void MainWindow::updateTitle() {
    QString name = m_currentFilePath.isEmpty()
        ? "Untitled"
        : QFileInfo(m_currentFilePath).fileName();
    setWindowTitle(QString("PoliAble Studio — %1%2")
                   .arg(name)
                   .arg(m_modified ? " •" : ""));
}

void MainWindow::logToConsole(const QString& msg) {
    if (m_consolePanel) m_consolePanel->log(msg);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_modified) {
        auto btn = QMessageBox::question(this, "Quit",
            "You have unsaved changes. Quit anyway?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (btn == QMessageBox::Cancel) { event->ignore(); return; }
        if (btn == QMessageBox::Save)   { saveDocument(); }
    }
    event->accept();
}

void MainWindow::onDeletePressed() {
    deleteSelected();
}
