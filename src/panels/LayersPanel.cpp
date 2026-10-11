#include "LayersPanel.h"
#include "model/Document.h"
#include "model/Node.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QString>

LayersPanel::LayersPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Search & Filter Bar ──
    auto* searchContainer = new QWidget(this);
    searchContainer->setStyleSheet("background: #1a1b20; border-bottom: 1px solid #262832;");
    auto* searchLayout = new QHBoxLayout(searchContainer);
    searchLayout->setContentsMargins(8, 6, 8, 6);
    searchLayout->setSpacing(6);

    m_searchBox = new QLineEdit(searchContainer);
    m_searchBox->setPlaceholderText("🔍 Filter layers...");
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setStyleSheet(
        "QLineEdit { background: #15161a; color: #e0e2ec; border: 1px solid #282a36; "
        "border-radius: 4px; padding: 4px 8px; font-size: 11px; }"
        "QLineEdit:focus { border: 1px solid #6366f1; }"
    );
    connect(m_searchBox, &QLineEdit::textChanged, this, &LayersPanel::onFilterChanged);
    searchLayout->addWidget(m_searchBox);

    auto* expandBtn = new QToolButton(searchContainer);
    expandBtn->setText("⤢");
    expandBtn->setToolTip("Expand All");
    connect(expandBtn, &QToolButton::clicked, this, [this]() { if (m_tree) m_tree->expandAll(); });
    searchLayout->addWidget(expandBtn);

    auto* collapseBtn = new QToolButton(searchContainer);
    collapseBtn->setText("⤡");
    collapseBtn->setToolTip("Collapse All");
    connect(collapseBtn, &QToolButton::clicked, this, [this]() { if (m_tree) m_tree->collapseAll(); });
    searchLayout->addWidget(collapseBtn);

    layout->addWidget(searchContainer);

    // ── Layers Tree ──
    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setIndentation(14);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setStyleSheet(
        "QTreeWidget { background: #17181c; color: #d0d3de; border: none; font-size: 12px; outline: none; } "
        "QTreeWidget::item { height: 26px; padding: 2px 4px; border-radius: 4px; margin: 1px 4px; } "
        "QTreeWidget::item:hover { background: #22242d; color: #ffffff; } "
        "QTreeWidget::item:selected { background: #2c3042; color: #ffffff; border-left: 2px solid #6366f1; } "
    );
    layout->addWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemClicked,
            this, &LayersPanel::onItemClicked);
    connect(m_tree, &QTreeWidget::itemDoubleClicked,
            this, &LayersPanel::onItemDoubleClicked);
    connect(m_tree, &QTreeWidget::itemChanged,
            this, &LayersPanel::onItemChanged);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &LayersPanel::showContextMenu);
}

void LayersPanel::setDocument(poliable::Document* doc) {
    m_doc = doc;
    refresh();
}

void LayersPanel::refresh() {
    m_isUpdating = true;
    m_tree->clear();
    if (!m_doc || !m_doc->root()) {
        m_isUpdating = false;
        return;
    }

    for (const auto& child : m_doc->root()->children()) {
        auto* topItem = new QTreeWidgetItem(m_tree);
        topItem->setFlags(topItem->flags() | Qt::ItemIsEditable);
        topItem->setText(0, nodeTypeLabel(child.get()) + "  " +
                            QString::fromStdString(child->name()));
        topItem->setData(0, Qt::UserRole, static_cast<qulonglong>(child->id()));
        populateTree(child.get(), topItem);
    }

    m_tree->expandAll();
    m_isUpdating = false;

    if (!m_searchBox->text().isEmpty()) {
        onFilterChanged(m_searchBox->text());
    }
}

void LayersPanel::populateTree(poliable::Node* node, QTreeWidgetItem* parentItem) {
    for (const auto& child : node->children()) {
        auto* item = new QTreeWidgetItem(parentItem);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0, nodeTypeLabel(child.get()) + "  " +
                         QString::fromStdString(child->name()));
        item->setData(0, Qt::UserRole, static_cast<qulonglong>(child->id()));
        populateTree(child.get(), item);
    }
}

QString LayersPanel::nodeTypeLabel(poliable::Node* node) {
    if (!node) return "?";
    switch (node->type()) {
        case poliable::NodeType::Rectangle: return "▭";
        case poliable::NodeType::Text:      return "T";
        case poliable::NodeType::Group:     return "⬡";
        case poliable::NodeType::Canvas:    return "⬜";
        case poliable::NodeType::Frame:     return "▢";
        case poliable::NodeType::Image:     return "🖼";
        case poliable::NodeType::Ellipse:   return "○";
        case poliable::NodeType::Line:      return "╱";
        case poliable::NodeType::Flex:      return "☰";
        case poliable::NodeType::Grid:      return "⊞";
        case poliable::NodeType::Scroll:    return "↕";
        default:                            return "·";
    }
}

void LayersPanel::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    if (!item || m_isUpdating) return;
    uint64_t id = item->data(0, Qt::UserRole).toULongLong();
    emit nodeSelected(id);
}

void LayersPanel::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    if (!item) return;
    m_tree->editItem(item, column);
}

void LayersPanel::onItemChanged(QTreeWidgetItem* item, int column) {
    if (!item || m_isUpdating || !m_doc) return;
    uint64_t id = item->data(0, Qt::UserRole).toULongLong();
    auto* node = m_doc->find(id);
    if (!node) return;

    QString text = item->text(column).trimmed();
    // Strip type icon prefix if present
    QString prefix = nodeTypeLabel(node) + "  ";
    if (text.startsWith(prefix)) {
        text = text.mid(prefix.length());
    }
    if (!text.isEmpty() && text.toStdString() != node->name()) {
        node->setName(text.toStdString());
        m_isUpdating = true;
        item->setText(0, prefix + text);
        m_isUpdating = false;
    }
}

void LayersPanel::onFilterChanged(const QString& text) {
    QString query = text.trimmed().toLower();

    std::function<bool(QTreeWidgetItem*)> filterItem = [&](QTreeWidgetItem* item) -> bool {
        bool match = item->text(0).toLower().contains(query);
        bool childMatch = false;
        for (int i = 0; i < item->childCount(); ++i) {
            if (filterItem(item->child(i))) {
                childMatch = true;
            }
        }
        bool visible = match || childMatch;
        item->setHidden(!visible);
        if (visible && !query.isEmpty()) {
            item->setExpanded(true);
        }
        return visible;
    };

    QTreeWidgetItem* root = m_tree->invisibleRootItem();
    for (int i = 0; i < root->childCount(); ++i) {
        filterItem(root->child(i));
    }
}

void LayersPanel::showContextMenu(const QPoint& pos) {
    auto* item = m_tree->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    menu.addAction("Rename", this, [this, item]() {
        m_tree->editItem(item, 0);
    });
    menu.addAction("Duplicate", this, [this]() {
        emit requestDuplicate();
    });
    menu.addSeparator();
    menu.addAction("Bring to Front", this, [this]() {
        emit requestBringToFront();
    });
    menu.addAction("Send to Back", this, [this]() {
        emit requestSendToBack();
    });
    menu.addSeparator();
    menu.addAction("Delete", this, [this]() {
        emit requestDelete();
    });
    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void LayersPanel::updateFromNode(poliable::Node* node) {
    if (!node || !m_tree || m_isUpdating) return;

    std::function<QTreeWidgetItem*(QTreeWidgetItem*, uint64_t)> find =
        [&](QTreeWidgetItem* parent, uint64_t targetId) -> QTreeWidgetItem* {
            for (int i = 0; i < parent->childCount(); ++i) {
                auto* child = parent->child(i);
                if (child->data(0, Qt::UserRole).toULongLong() == targetId)
                    return child;
                auto* found = find(child, targetId);
                if (found) return found;
            }
            return nullptr;
        };

    QTreeWidgetItem* root = m_tree->invisibleRootItem();
    for (int i = 0; i < root->childCount(); ++i) {
        auto* topItem = root->child(i);
        if (topItem->data(0, Qt::UserRole).toULongLong() == node->id()) {
            m_tree->setCurrentItem(topItem);
            return;
        }
        auto* found = find(topItem, node->id());
        if (found) {
            m_tree->setCurrentItem(found);
            return;
        }
    }
}
