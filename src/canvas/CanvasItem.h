#pragma once
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QStyleOption>

namespace poliable { class Node; class UndoStack; class Document; }

// ── Handle directions ─────────────────────────────────────────────────────────
enum class HandlePos {
    TopLeft, Top, TopRight,
    Right,
    BottomRight, Bottom, BottomLeft,
    Left
};

// ── Resize handle (child item) ────────────────────────────────────────────────
class ResizeHandle : public QGraphicsItem {
public:
    static constexpr double SIZE = 8.0;

    explicit ResizeHandle(HandlePos pos, QGraphicsItem* parent);

    HandlePos handlePos() const { return m_pos; }

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    HandlePos m_pos;
    QPointF   m_dragStartPos;  // scene pos at drag start
    QPointF   m_handleStartPos; // handle pos at drag start
    double    m_handleStartScale = 0.0;  // scale factor at start
};

// ── Canvas item ───────────────────────────────────────────────────────────────
class CanvasItem : public QGraphicsItem {
public:
    explicit CanvasItem(poliable::Node* node, QGraphicsItem* parent = nullptr);

    poliable::Node* node() const { return m_node; }

    // Set the undo stack for creating undoable commands
    void setUndoStack(poliable::UndoStack* stack);

    // Sync visual state from node (call after external changes)
    void updateFromNode();

    // Called by CanvasView when a resize drag ends
    void commitResize(QRectF newLocalRect);

    // Get the selection rect (for drawing selection box)
    QRectF selectionRect() const;

    // Get node for operations (same as node())
    poliable::Node* getNode() const { return m_node; }

    // Snapping: return snapped position
    QPointF snapPosition(const QPointF& pos) const;

    // Get the resize handles (for accessing parent item's handles)
    const QList<ResizeHandle*>& handles() const { return m_handles; }

    // QGraphicsItem interface
    QRectF    boundingRect() const override;
    void      paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                    QWidget* widget = nullptr) override;
    QVariant  itemChange(GraphicsItemChange change, const QVariant& value) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override;

private:
    void buildHandles();
    void updateHandlePositions();
    void showHandles(bool v);

    poliable::Node*           m_node;
    poliable::UndoStack*      m_stack = nullptr;
    QList<ResizeHandle*>      m_handles;

    // For drag operations
    QPointF m_dragStartScenePos;
    QPointF m_itemStartPos;
    bool m_resizing = false;
    ResizeHandle* m_activeHandle = nullptr;
};
