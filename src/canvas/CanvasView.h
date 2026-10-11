#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPointF>
#include <memory>
#include <QList>

#include "model/Document.h"
#include "model/UndoStack.h"

class CanvasItem;
class QLabel;
class QPushButton;

class CanvasView : public QGraphicsView {
    Q_OBJECT
public:
    explicit CanvasView(QWidget *parent = nullptr);

    void setDocument(poliable::Document* doc);
    poliable::Document* document() const { return m_doc; }

    void addCanvasItem(poliable::Node* node);
    void removeItem(CanvasItem* item);
    void rebuildFromDocument();

    CanvasItem* selectedItem() const;
    void setSelectedCanvasItem(CanvasItem* item);

    // Get undo stack for use by CanvasItem
    poliable::UndoStack& undoStack() { return m_undoStack; }

    double zoomScale() const { return m_zoomScale; }

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void zoomToFit();

signals:
    void selectionChanged(CanvasItem* item);
    void nodesChanged();
    void selectedCanvasItemDelete();
    void zoomChanged(double scale);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:
    void onSceneSelectionChanged();

private:
    void setupScene();
    void updateZoomIndicator();
    void positionZoomOverlay();

    QGraphicsScene*      m_scene = nullptr;
    poliable::Document*  m_doc   = nullptr;
    poliable::UndoStack  m_undoStack;

    bool m_panning = false;
    QPoint m_panStart;
    double m_zoomScale = 1.0;

    // Floating Zoom Controls Overlay
    QWidget*     m_zoomOverlay = nullptr;
    QLabel*      m_zoomLabel   = nullptr;
    QPushButton* m_zoomInBtn   = nullptr;
    QPushButton* m_zoomOutBtn  = nullptr;
    QPushButton* m_zoomFitBtn  = nullptr;
};
