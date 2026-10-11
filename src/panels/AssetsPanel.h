#pragma once
#include <QWidget>
#include <QString>

class AssetsPanel : public QWidget {
    Q_OBJECT
public:
    explicit AssetsPanel(QWidget *parent = nullptr);
    void refresh();

signals:
    void colorSelected(const QString& hexColor);
    void insertComponentRequested(const QString& componentType);
};
