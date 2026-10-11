#pragma once
#include <QWidget>
#include <QString>
#include "model/Document.h"
#include "codegen/CodeGenerator.h"

class QPlainTextEdit;
class QTabWidget;
class QPushButton;
class QLabel;

class CodePanel : public QWidget {
    Q_OBJECT
public:
    explicit CodePanel(QWidget* parent = nullptr);

    void setDocument(poliable::Document* doc);
    void refresh();

signals:
    void projectExported(const QString& dirPath);

private slots:
    void onTabChanged(int index);
    void onCopyClicked();
    void onExportClicked();

private:
    void updateEditorContent();

    poliable::Document* m_doc = nullptr;
    QTabWidget*     m_tabs       = nullptr;
    QPlainTextEdit* m_htmlEditor = nullptr;
    QPlainTextEdit* m_cssEditor  = nullptr;
    QPlainTextEdit* m_reactEditor= nullptr;
    QPlainTextEdit* m_twEditor   = nullptr;
    QLabel*         m_statusLabel= nullptr;

    poliable::GeneratedCode m_cachedCode;
};
