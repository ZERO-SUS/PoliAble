#include "CanvasView.h"
#include "CanvasItem.h"
#include "model/Node.h"
#include "model/Command.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsDropShadowEffect>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QPainter>
#include <cmath>

// ── Construction ─────────────────────────────────────────────────────────────

CanvasView::CanvasView(QWidget* parent)
    : QGraphicsView(parent)
{
    setupScene();
}

void CanvasView::setupScene() {
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-10000, -10000, 20000, 20000);

    // High rendering quality
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, false);

    // Modern studio canvas background
    setBackgroundBrush(QColor(18, 19, 23));

    // Pan / Zoom view anchors
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setDragMode(QGraphicsView::NoDrag);

    // ── Floating Zoom Overlay ──
    m_zoomOverlay = new QWidget(this);
    m_zoomOverlay->setObjectName("ZoomOverlay");
    m_zoomOverlay->setStyleSheet(
        "#ZoomOverlay { "
        "  background: rgba(24, 26, 33, 220); "
        "  border: 1px solid #2e3240; "
        "  border-radius: 6px; "
        "} "
        "QPushButton { "
        "  background: transparent; color: #c4c7d4; border: none; "
        "  padding: 3px 7px; font-weight: bold; font-size: 11px; border-radius: 4px; "
        "} "
        "QPushButton:hover { background: #323646; color: #ffffff; } "
        "QLabel { color: #e2e5f0; font-size: 11px; font-weight: 600; padding: 2px 4px; } "
    );

    auto* overlayLayout = new QHBoxLayout(m_zoomOverlay);
    overlayLayout->setContentsMargins(4, 3, 4, 3);
    overlayLayout->setSpacing(2);

    m_zoomOutBtn = new QPushButton("－", m_zoomOverlay);
    m_zoomOutBtn->setToolTip("Zoom Out");
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &CanvasView::zoomOut);

    m_zoomLabel = new QLabel("100%", m_zoomOverlay);
    m_zoomLabel->setCursor(Qt::PointingHandCursor);
    m_zoomLabel->setToolTip("Click to reset zoom to 100%");

    m_zoomInBtn = new QPushButton("＋", m_zoomOverlay);
    m_zoomInBtn->setToolTip("Zoom In");
    connect(m_zoomInBtn, &QPushButton::clicked, this, &CanvasView::zoomIn);

    m_zoomFitBtn = new QPushButton("Fit", m_zoomOverlay);
    m_zoomFitBtn->setToolTip("Fit Artboard in View");
    connect(m_zoomFitBtn, &QPushButton::clicked, this, &CanvasView::zoomToFit);

    overlayLayout->addWidget(m_zoomOutBtn);
    overlayLayout->addWidget(m_zoomLabel);
    overlayLayout->addWidget(m_zoomInBtn);
    overlayLayout->addWidget(m_zoomFitBtn);

    m_zoomOverlay->adjustSize();
    positionZoomOverlay();

    viewport()->installEventFilter(this);

    connect(m_scene, &QGraphicsScene::selectionChanged,
            this, &CanvasView::onSceneSelectionChanged);
}

void CanvasView::positionZoomOverlay() {
    if (!m_zoomOverlay) return;
    int x = viewport()->width() - m_zoomOverlay->width() - 16;
    int y = viewport()->height() - m_zoomOverlay->height() - 16;
    m_zoomOverlay->move(std::max(10, x), std::max(10, y));
}

// ── Draw subtle studio dot-grid in canvas background ────────────────────────
void CanvasView::drawBackground(QPainter* painter, const QRectF& rect) {
    // Fill workspace canvas background
    painter->fillRect(rect, QColor(18, 19, 23));

    // Draw subtle dot grid for alignment
    constexpr double GRID_SPACING = 24.0;
    const double left = std::floor(rect.left() / GRID_SPACING) * GRID_SPACING;
    const double top  = std::floor(rect.top() / GRID_SPACING) * GRID_SPACING;

    // Soft subtle dots
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 16));

    for (double x = left; x < rect.right(); x += GRID_SPACING) {
        for (double y = top; y < rect.bottom(); y += GRID_SPACING) {
            painter->drawRect(QRectF(x, y, 1.0, 1.0));
        }
    }
}

// ── Scene page background (Modern artboard with title chip) ─────────────────
static void addPageBackground(QGraphicsScene* scene, double w = 1440, double h = 900) {
    // Remove existing background items
    for (auto* item : scene->items()) {
        if (item->zValue() < -100) {
            scene->removeItem(item);
            delete item;
        }
    }

    // 1. Ambient Drop Shadow underneath artboard
    auto* shadow = new QGraphicsRectItem(2, 4, w, h);
    shadow->setZValue(-1002);
    shadow->setPen(Qt::NoPen);
    shadow->setBrush(QColor(0, 0, 0, 70));
    scene->addItem(shadow);

    // 2. Main Artboard Canvas (clean white)
    auto* page = new QGraphicsRectItem(0, 0, w, h);
    page->setZValue(-1000);
    page->setPen(QPen(QColor(46, 50, 62), 1));
    page->setBrush(QColor(255, 255, 255));
    scene->addItem(page);

    // 3. Modern Frame Label Chip above artboard
    QString frameLabel = QString("Desktop — %1 × %2").arg(static_cast<int>(w)).arg(static_cast<int>(h));
    auto* text = new QGraphicsSimpleTextItem(frameLabel);
    text->setZValue(-999);
    text->setPos(0, -22);
    QFont f;
    f.setFamily("Segoe UI");
    f.setPixelSize(11);
    f.setWeight(QFont::DemiBold);
    text->setFont(f);
    text->setBrush(QColor(140, 145, 162));
    scene->addItem(text);
}

// ── Document binding ─────────────────────────────────────────────────────────

void CanvasView::setDocument(poliable::Document* doc) {
    m_doc = doc;
    rebuildFromDocument();
}

void CanvasView::rebuildFromDocument() {
    if (!m_scene) return;

    // Clear user items (keep page background)
    QList<QGraphicsItem*> all = m_scene->items();
    for (auto* item : all) {
        if (item->zValue() >= 0) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    double pageW = (m_doc && m_doc->root()) ? m_doc->root()->transform().width : 1440.0;
    double pageH = (m_doc && m_doc->root()) ? m_doc->root()->transform().height : 900.0;

    // Add page background
    addPageBackground(m_scene, pageW, pageH);

    if (!m_doc) return;

    // Walk the document tree and create CanvasItems
    std::function<void(poliable::Node*)> walk = [&](poliable::Node* node) {
        if (!node) return;
        if (node != m_doc->root()) {
            auto* item = new CanvasItem(node);
            item->setUndoStack(&m_undoStack);
            m_scene->addItem(item);
        }
        for (const auto& child : node->children()) {
            walk(child.get());
        }
    };
    walk(m_doc->root());

    emit nodesChanged();
}

// ── Item management ──────────────────────────────────────────────────────────

void CanvasView::addCanvasItem(poliable::Node* node) {
    if (!m_scene || !node) return;
    auto* item = new CanvasItem(node);
    m_scene->addItem(item);
    // Select the new item
    m_scene->clearSelection();
    item->setSelected(true);
    emit nodesChanged();
}

void CanvasView::removeItem(CanvasItem* item) {
    if (!item || !m_scene) return;
    m_scene->removeItem(item);
    delete item;
    emit nodesChanged();
}

CanvasItem* CanvasView::selectedItem() const {
    if (!m_scene) return nullptr;
    const auto sel = m_scene->selectedItems();
    if (sel.isEmpty()) return nullptr;
    return dynamic_cast<CanvasItem*>(sel.first());
}

void CanvasView::setSelectedCanvasItem(CanvasItem* item) {
    if (!m_scene) return;
    m_scene->clearSelection();
    if (item) item->setSelected(true);
}

// ── Selection notifications ──────────────────────────────────────────────────

void CanvasView::onSceneSelectionChanged() {
    emit selectionChanged(selectedItem());
}

// ── Zoom Methods ─────────────────────────────────────────────────────────────

void CanvasView::zoomIn() {
    double newScale = m_zoomScale * 1.2;
    newScale = std::min(20.0, newScale);
    m_zoomScale = newScale;
    resetTransform();
    scale(m_zoomScale, m_zoomScale);
    updateZoomIndicator();
    emit zoomChanged(m_zoomScale);
}

void CanvasView::zoomOut() {
    double newScale = m_zoomScale / 1.2;
    newScale = std::max(0.05, newScale);
    m_zoomScale = newScale;
    resetTransform();
    scale(m_zoomScale, m_zoomScale);
    updateZoomIndicator();
    emit zoomChanged(m_zoomScale);
}

void CanvasView::resetZoom() {
    m_zoomScale = 1.0;
    resetTransform();
    scale(1.0, 1.0);
    updateZoomIndicator();
    emit zoomChanged(m_zoomScale);
}

void CanvasView::zoomToFit() {
    double pageW = (m_doc && m_doc->root()) ? m_doc->root()->transform().width : 1440.0;
    double pageH = (m_doc && m_doc->root()) ? m_doc->root()->transform().height : 900.0;
    QRectF artboardRect(0, 0, pageW, pageH);

    fitInView(artboardRect.adjusted(-60, -60, 60, 60), Qt::KeepAspectRatio);
    m_zoomScale = transform().m11();
    updateZoomIndicator();
    emit zoomChanged(m_zoomScale);
}

void CanvasView::wheelEvent(QWheelEvent* event) {
    const double delta = event->angleDelta().y();
    const double scaleFactor = (delta > 0) ? 1.15 : (1.0 / 1.15);

    double newScale = m_zoomScale * scaleFactor;
    newScale = std::max(0.05, std::min(20.0, newScale));
    m_zoomScale = newScale;

    resetTransform();
    scale(m_zoomScale, m_zoomScale);
    updateZoomIndicator();
    emit zoomChanged(m_zoomScale);

    event->accept();
}

void CanvasView::updateZoomIndicator() {
    if (!m_zoomLabel) return;
    int pct = static_cast<int>(std::round(m_zoomScale * 100));
    m_zoomLabel->setText(QString("%1%").arg(pct));
}

// ── Pan (Middle-mouse or Space + drag) ──────────────────────────────────────

void CanvasView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        QPoint delta = event->pos() - m_panStart;
        m_panStart = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// ── Keyboard ─────────────────────────────────────────────────────────────────

void CanvasView::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace: {
            if (!qApp->focusWidget() ||
                qApp->focusWidget()->inherits("QGraphicsView")) {
                emit selectedCanvasItemDelete();
            }
            break;
        }
        case Qt::Key_0:
            resetZoom();
            break;
        case Qt::Key_Space:
            setDragMode(QGraphicsView::ScrollHandDrag);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
            break;
    }
}

void CanvasView::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        setDragMode(QGraphicsView::NoDrag);
    }
    QGraphicsView::keyReleaseEvent(event);
}

void CanvasView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    positionZoomOverlay();
}

bool CanvasView::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == viewport()) {
        if (ev->type() == QEvent::Resize) {
            positionZoomOverlay();
        } else if (ev->type() == QEvent::MouseButtonPress) {
            // Check if user clicked on zoom label to reset zoom
            if (m_zoomLabel && m_zoomLabel->geometry().contains(
                    m_zoomOverlay->mapFromParent(viewport()->mapFromGlobal(QCursor::pos())))) {
                resetZoom();
            }
        }
    }
    return QGraphicsView::eventFilter(obj, ev);
}
