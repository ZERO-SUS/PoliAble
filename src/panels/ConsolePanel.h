#pragma once
#include <QWidget>
#include <QString>

class QPlainTextEdit;

class ConsolePanel : public QWidget {
    Q_OBJECT
public:
    explicit ConsolePanel(QWidget* parent = nullptr);

    // Log a plain message (QString overload used by MainWindow)
    void log(const QString& message);

    // Convenience overload for std::string callers
    void log(const std::string& message);

    void clear();

private:
    QPlainTextEdit* m_output = nullptr;
};
