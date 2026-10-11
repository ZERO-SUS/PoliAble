#pragma once
#include <QMainWindow>
#include <memory>
#include "model/UndoStack.h"

namespace poliable { class Document; }

class LayersPanel;
class AssetsPanel;
class PropertiesPanel;
class ConsolePanel;
class TimelinePanel;
class CodePanel;
class CanvasView;
class CanvasItem;
class QComboBox;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void newDocument();
    void openDocument();
    void saveDocument();
    void saveDocumentAs();

    void addRectangle();
    void addEllipse();
    void addText();
    void addFrame();
    void addFlexContainer();
    void addGridContainer();
    void duplicateSelected();
    void bringToFront();
    void sendToBack();
    void setBreakpoint(int index);
    void onLayoutModified();
    void deleteSelected();
    void undo();
    void redo();

    // Assets integration
    void onAssetColorSelected(const QString& hexColor);
    void onAssetComponentInsert(const QString& componentType);

    // Called when canvas selection changes
    void onCanvasSelectionChanged(CanvasItem* item);
    // Called when layers panel selects a node
    void onLayerNodeSelected(uint64_t nodeId);
    // Called by CanvasView when user deletes via Delete key
    void onDeletePressed();

private:
    void buildMenus();
    void buildToolbar();
    void buildDocks();
    void buildStatusBar();
    void connectSignals();

    void loadFile(const QString& path);
    bool saveFile(const QString& path);

    void updateTitle();
    void logToConsole(const QString& msg);

    std::unique_ptr<poliable::Document> m_doc;
    poliable::UndoStack m_undoStack;

    CanvasView*      m_canvas          = nullptr;
    LayersPanel*     m_layersPanel     = nullptr;
    AssetsPanel*     m_assetsPanel     = nullptr;
    PropertiesPanel* m_propsPanel      = nullptr;
    ConsolePanel*    m_consolePanel    = nullptr;
    TimelinePanel*   m_timelinePanel   = nullptr;
    CodePanel*       m_codePanel       = nullptr;
    QComboBox*       m_breakpointCombo = nullptr;
    QLabel*          m_statusSelectionLabel = nullptr;
    QLabel*          m_statusDocLabel       = nullptr;

    QString m_currentFilePath;   // empty = unsaved
    bool    m_modified = false;
};
