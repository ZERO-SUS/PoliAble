#pragma once
#include <QWidget>
#include <QTreeWidget>

namespace poliable { class Document; class Node; }

class QLineEdit;
class QToolButton;

class LayersPanel : public QWidget {
    Q_OBJECT
public:
    explicit LayersPanel(QWidget* parent = nullptr);

    void setDocument(poliable::Document* doc);

    // Rebuild the tree from the current document state
    void refresh();

    // Highlight the item associated with a specific node
    void updateFromNode(poliable::Node* node);

signals:
    // Emitted when the user clicks a layer, carrying the node id
    void nodeSelected(uint64_t nodeId);
    void requestDelete();
    void requestDuplicate();
    void requestBringToFront();
    void requestSendToBack();

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onFilterChanged(const QString& text);
    void showContextMenu(const QPoint& pos);

private:
    void populateTree(poliable::Node* node, QTreeWidgetItem* parentItem);
    static QString nodeTypeLabel(poliable::Node* node);

    QLineEdit*             m_searchBox = nullptr;
    QTreeWidget*           m_tree = nullptr;
    poliable::Document*    m_doc  = nullptr;
    bool                   m_isUpdating = false;
};
