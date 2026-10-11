#include "CanvasItem.h"
#include "model/Node.h"
#include "model/Command.h"
#include "model/UndoStack.h"

#include <QPainter>
#include <QColor>
#include <QFont>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

// ── ResizeHandle ──────────────────────────────────────────────────────────────

ResizeHandle::ResizeHandle(HandlePos pos, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_pos(pos)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setZValue(100);  // Above the CanvasItem
}

QRectF ResizeHandle::boundingRect() const {
    return QRectF(-SIZE/2, -SIZE/2, SIZE, SIZE);
}

void ResizeHandle::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing, true);
    QRectF r = boundingRect().adjusted(0.5, 0.5, -0.5, -0.5);
    p->setPen(QPen(QColor("#6366f1"), 1.5));
    p->setBrush(QColor(255, 255, 255));
    p->drawRoundedRect(r, 2.0, 2.0);
}

void ResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    m_handleStartPos = pos();
    m_dragStartPos = e->scenePos();
    e->accept();
}

void ResizeHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    e->accept();  // consume, don't let parent handle
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    e->accept();
}

// ── CanvasItem ────────────────────────────────────────────────────────────────

CanvasItem::CanvasItem(poliable::Node* node, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_node(node), m_stack(nullptr)
{
    Q_ASSERT(m_node);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(0);

    buildHandles();
    updateFromNode();
}

void CanvasItem::setUndoStack(poliable::UndoStack* stack) {
    m_stack = stack;
}

void CanvasItem::buildHandles() {
    for (auto* h : m_handles) delete h;
    m_handles.clear();

    auto* parent = this;
    m_handles << new ResizeHandle(HandlePos::TopLeft,    parent);
    m_handles << new ResizeHandle(HandlePos::Top,        parent);
    m_handles << new ResizeHandle(HandlePos::TopRight,   parent);
    m_handles << new ResizeHandle(HandlePos::Right,      parent);
    m_handles << new ResizeHandle(HandlePos::BottomRight,parent);
    m_handles << new ResizeHandle(HandlePos::Bottom,     parent);
    m_handles << new ResizeHandle(HandlePos::BottomLeft, parent);
    m_handles << new ResizeHandle(HandlePos::Left,       parent);

    updateHandlePositions();
}

void CanvasItem::updateHandlePositions() {
    if (m_handles.isEmpty()) return;
    const auto& t = m_node->transform();
    double w = t.width;
    double h = t.height;
    // Handle offsets from top-left corner (outside the item)
    const double off = 6.0;

    auto setPos = [&](HandlePos hp, double x, double y) {
        for (auto* h : m_handles) {
            if (h->handlePos() == hp) {
                h->setPos(x, y);
                break;
            }
        }
    };

    setPos(HandlePos::TopLeft,    -off,          -off);
    setPos(HandlePos::Top,        w/2 - ResizeHandle::SIZE/2, -off);
    setPos(HandlePos::TopRight,   w - off,       -off);
    setPos(HandlePos::Right,      w - off,       h/2 - ResizeHandle::SIZE/2);
    setPos(HandlePos::BottomRight,w - off,       h - off);
    setPos(HandlePos::Bottom,     w/2 - ResizeHandle::SIZE/2, h - off);
    setPos(HandlePos::BottomLeft, -off,          h - off);
    setPos(HandlePos::Left,       -off,          h/2 - ResizeHandle::SIZE/2);
}

void CanvasItem::showHandles(bool v) {
    for (auto* h : m_handles) h->setVisible(v);
}

void CanvasItem::updateFromNode() {
    if (!m_node) return;
    const auto& t = m_node->transform();
    setPos(t.x, t.y);
    setOpacity(t.opacity);
    updateHandlePositions();
    update();
}

void CanvasItem::commitResize(QRectF newLocalRect) {
    if (!m_node || !m_stack || !m_stack->document()) return;

    auto oldT = m_node->transform();
    auto newT = oldT;
    newT.width  = newLocalRect.width();
    newT.height = newLocalRect.height();

    if (newT.width != oldT.width || newT.height != oldT.height) {
        auto cmd = std::make_unique<poliable::SetTransformCommand>(
            m_node->id(), oldT, newT);
        m_stack->execute(std::move(cmd));
    }
}

QRectF CanvasItem::selectionRect() const {
    if (!m_node) return {};
    const auto& t = m_node->transform();
    return QRectF(0, 0, t.width, t.height);
}

QPointF CanvasItem::snapPosition(const QPointF& pos) const {
    // Snap to 8px grid by default
    constexpr double SNAP_GRID = 8.0;
    double x = std::round(pos.x() / SNAP_GRID) * SNAP_GRID;
    double y = std::round(pos.y() / SNAP_GRID) * SNAP_GRID;
    return QPointF(x, y);
}

QRectF CanvasItem::boundingRect() const {
    if (!m_node) return {};
    const auto& t = m_node->transform();
    return QRectF(0, 0, t.width, t.height);
}

void CanvasItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget*) {
    if (!m_node) return;
    const auto& t = m_node->transform();
    const auto& s = m_node->style();
    QRectF rect(0, 0, t.width, t.height);

    painter->save();

    // Rotate around center if needed
    if (t.rotation != 0.0) {
        painter->translate(t.width/2, t.height/2);
        painter->rotate(t.rotation);
        painter->translate(-t.width/2, -t.height/2);
    }

    switch (m_node->type()) {
        case poliable::NodeType::Rectangle: {
            QColor fill(QString::fromStdString(s.fillColor));
            QColor stroke(QString::fromStdString(s.strokeColor));
            QPen pen(stroke, s.strokeWidth);
            painter->setPen(pen);
            painter->setBrush(fill);
            // Rounded rectangle if cornerRadius > 0
            if (s.cornerRadius > 0.001) {
                painter->drawRoundedRect(rect, s.cornerRadius, s.cornerRadius);
            } else {
                painter->drawRect(rect);
            }
            break;
        }

        case poliable::NodeType::Frame: {
            QColor stroke(QString::fromStdString(s.strokeColor));
            QPen pen(stroke, s.strokeWidth);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            if (s.cornerRadius > 0.001) {
                painter->drawRoundedRect(rect, s.cornerRadius, s.cornerRadius);
            } else {
                painter->drawRect(rect);
            }
            break;
        }

        case poliable::NodeType::Ellipse: {
            QColor fill(QString::fromStdString(s.fillColor));
            QColor stroke(QString::fromStdString(s.strokeColor));
            QPen pen(stroke, s.strokeWidth);
            painter->setPen(pen);
            painter->setBrush(fill);
            painter->drawEllipse(rect);
            break;
        }

        case poliable::NodeType::Line: {
            QColor stroke(QString::fromStdString(s.strokeColor));
            QPen pen(stroke, s.strokeWidth);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(rect.left(), rect.top(), rect.right(), rect.bottom());
            break;
        }

        case poliable::NodeType::Text: {
            QColor fill(QString::fromStdString(s.fillColor));
            painter->setPen(QPen(fill));
            painter->setBrush(Qt::NoBrush);
            QFont font;
            font.setPixelSize(std::max(8, s.fontSize));
            painter->setFont(font);
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                              QString::fromStdString(s.textContent));
            // Dashed border for visibility
            painter->setPen(QPen(QColor("#888888"), 1, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(rect);
            break;
        }

        case poliable::NodeType::Image: {
            // Placeholder for image rendering
            QColor fill(40, 40, 45);
            QColor stroke(100, 100, 110);
            QPen pen(stroke, 1);
            painter->setPen(pen);
            painter->setBrush(fill);
            painter->drawRect(rect);
            // Show placeholder text
            painter->setPen(QPen(QColor("#666666"), 1));
            painter->drawText(rect.adjusted(8, 8, -8, -8), Qt::AlignCenter,
                              QString("Image: %1x%2")
                              .arg(int(t.width)).arg(int(t.height)));
            break;
        }

        case poliable::NodeType::Group: {
            QColor stroke(80, 80, 85);
            QPen pen(stroke, 1, Qt::DotLine);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(rect);
            break;
        }

        case poliable::NodeType::Canvas: {
            QColor fill(35, 35, 40);
            painter->setPen(Qt::NoPen);
            painter->setBrush(fill);
            painter->drawRect(rect);
            break;
        }

        case poliable::NodeType::Flex: {
            QColor fill(32, 35, 42, 180);
            QColor stroke(75, 120, 200, 160);
            painter->setPen(QPen(stroke, 1.5, Qt::DashLine));
            painter->setBrush(fill);
            painter->drawRect(rect);

            // Draw padding boundary guide if padding > 0
            const auto& l = m_node->layout();
            if (l.paddingLeft > 0 || l.paddingTop > 0 || l.paddingRight > 0 || l.paddingBottom > 0) {
                QRectF padRect(rect.left() + l.paddingLeft, rect.top() + l.paddingTop,
                               std::max(0.0, rect.width() - l.paddingLeft - l.paddingRight),
                               std::max(0.0, rect.height() - l.paddingTop - l.paddingBottom));
                painter->setPen(QPen(QColor(100, 160, 255, 60), 1, Qt::DotLine));
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(padRect);
            }

            // Direction badge
            painter->setPen(QPen(QColor("#7cb7ff")));
            QFont f;
            f.setPixelSize(10);
            painter->setFont(f);
            QString dirStr = (l.flexDirection == poliable::FlexDirection::Row) ? "Flex Row →" :
                             (l.flexDirection == poliable::FlexDirection::Column) ? "Flex Col ↓" :
                             (l.flexDirection == poliable::FlexDirection::RowReverse) ? "Flex Row ←" : "Flex Col ↑";
            painter->drawText(rect.adjusted(6, 4, -4, -4), Qt::AlignTop | Qt::AlignLeft, dirStr);
            break;
        }

        case poliable::NodeType::Grid: {
            QColor fill(34, 38, 36, 180);
            QColor stroke(60, 180, 120, 160);
            painter->setPen(QPen(stroke, 1.5, Qt::DashLine));
            painter->setBrush(fill);
            painter->drawRect(rect);

            // Draw grid lines
            const auto& l = m_node->layout();
            int cols = std::max(1, l.gridColumns);
            int rows = std::max(1, l.gridRows);
            double colGap = (l.columnGap > 0.0) ? l.columnGap : l.gap;
            double rowGap = (l.rowGap > 0.0) ? l.rowGap : l.gap;

            double padL = l.paddingLeft;
            double padR = l.paddingRight;
            double padT = l.paddingTop;
            double padB = l.paddingBottom;

            double availW = std::max(0.0, rect.width() - padL - padR - (cols - 1) * colGap);
            double cellW = availW / cols;
            double availH = std::max(0.0, rect.height() - padT - padB - (rows - 1) * rowGap);
            double cellH = availH / rows;

            painter->setPen(QPen(QColor(60, 180, 120, 60), 1, Qt::DotLine));
            painter->setBrush(Qt::NoBrush);
            for (int r = 0; r < rows; ++r) {
                for (int c = 0; c < cols; ++c) {
                    QRectF cellRect(rect.left() + padL + c * (cellW + colGap),
                                   rect.top() + padT + r * (cellH + rowGap),
                                   cellW, cellH);
                    painter->drawRect(cellRect);
                }
            }

            // Grid badge
            painter->setPen(QPen(QColor("#5cdba0")));
            QFont f;
            f.setPixelSize(10);
            painter->setFont(f);
            painter->drawText(rect.adjusted(6, 4, -4, -4), Qt::AlignTop | Qt::AlignLeft,
                              QString("Grid %1×%2").arg(cols).arg(rows));
            break;
        }

        case poliable::NodeType::Scroll: {
            QColor fill(30, 30, 35);
            QColor stroke(100, 100, 120);
            painter->setPen(QPen(stroke, 1));
            painter->setBrush(fill);
            painter->drawRect(rect);

            // Scrollbar track & thumb representation
            QRectF track(rect.right() - 8, rect.top() + 2, 6, rect.height() - 4);
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(50, 50, 60));
            painter->drawRoundedRect(track, 3, 3);
            QRectF thumb(rect.right() - 8, rect.top() + 4, 6, std::max(12.0, rect.height() * 0.3));
            painter->setBrush(QColor(100, 100, 120));
            painter->drawRoundedRect(thumb, 3, 3);
            break;
        }

        default:
            painter->setPen(QPen(QColor("#666666"), 1));
            painter->setBrush(QColor(80, 80, 90));
            painter->drawRect(rect);
            break;
    }

    // Selection highlight with handles
    if (option && (option->state & QStyle::State_Selected)) {
        showHandles(true);
        painter->setPen(QPen(QColor("#6366f1"), 1.5));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect.adjusted(-1, -1, 1, 1));
    } else {
        showHandles(false);
    }

    painter->restore();
}

QVariant CanvasItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged && m_node && !m_resizing) {
        // We'll sync back in mouseReleaseEvent so moves are undoable
        Q_UNUSED(value);
    }
    return QGraphicsItem::itemChange(change, value);
}

void CanvasItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    // Check if clicking on a handle
    for (auto* h : m_handles) {
        if (h->contains(h->mapFromScene(e->scenePos()))) {
            m_resizing = true;
            m_activeHandle = h;
            m_dragStartScenePos = e->scenePos();
            e->accept();
            return;
        }
    }

    // Regular item drag
    if (e->button() == Qt::LeftButton) {
        m_resizing = false;
        m_activeHandle = nullptr;
        m_dragStartScenePos = e->scenePos();
        m_itemStartPos = pos();
        if (m_node) {
            m_node->transform().x = m_itemStartPos.x();
            m_node->transform().y = m_itemStartPos.y();
        }
    }
    QGraphicsItem::mousePressEvent(e);
}

void CanvasItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e) {
    if (m_resizing && m_activeHandle) {
        // Handle resize logic - calculate new size based on handle movement
        // For now, just consume the event
        e->accept();
        return;
    }
    QGraphicsItem::mouseMoveEvent(e);
}

void CanvasItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsItem::mouseReleaseEvent(e);

    if (e->button() != Qt::LeftButton) return;
    if (!m_node) return;

    if (m_resizing) {
        // Resize complete - apply changes
        if (m_activeHandle) {
            QRectF newRect = boundingRect();
            commitResize(newRect);
        }
        m_resizing = false;
        m_activeHandle = nullptr;
        return;
    }

    // Check for significant move (undoable)
    QPointF currentPos = pos();
    QPointF delta = currentPos - m_itemStartPos;

    const double moveThreshold = 0.5;
    if (qAbs(delta.x()) > moveThreshold || qAbs(delta.y()) > moveThreshold) {
        if (m_stack && m_node && m_stack->document()) {
            auto oldT = m_node->transform();
            auto newT = oldT;
            newT.x = currentPos.x();
            newT.y = currentPos.y();

            auto cmd = std::make_unique<poliable::SetTransformCommand>(
                m_node->id(), oldT, newT);
            m_stack->execute(std::move(cmd));
        }
    }
}
