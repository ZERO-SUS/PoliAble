#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("PoliAble");
    window.resize(800, 600);

    // Create central widget with layout
    auto *centralWidget = new QWidget(&window);
    auto *layout = new QVBoxLayout(centralWidget);

    auto *label = new QLabel("Welcome to PoliAble", centralWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold; padding: 20px;");

    layout->addWidget(label);
    window.setCentralWidget(centralWidget);

    window.show();

    return app.exec();
}
