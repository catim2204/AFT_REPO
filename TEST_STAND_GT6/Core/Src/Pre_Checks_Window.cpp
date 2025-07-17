// #include "Pre_Checks_Window.h"
// #include <QLabel>
// #include <QVBoxLayout>
// #include <QPushButton>
// #include "Color_Code_ANSI.h"

// PreCheckWindow::PreCheckWindow(QWidget *parent)
//     : QWidget(parent)
// {

// }

// PreCheckWindow::~PreCheckWindow()
// {
   
// }

#include "Pre_Checks_Window.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include "Color_Code_ANSI.h"

PreCheckWindow::PreCheckWindow(QWidget *parent)
    : QWidget(parent)
{
    dialogBox= new PreCheckWindow(this);
    // Create widgets
    QLabel *infoLabel = new QLabel("Pre-checks in progress...", this);
    QPushButton *closeButton = new QPushButton("Close", this);

    // Connect button to close the window
    connect(closeButton, &QPushButton::clicked, this, &PreCheckWindow::close);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel);
    layout->addWidget(closeButton);

    // Set layout and window properties
    setLayout(layout);
    setWindowTitle("Pre-Check Window");
    setFixedSize(300, 150);  // Optional
}

PreCheckWindow::~PreCheckWindow()
{
    // No manual deletion needed; Qt handles child widgets
}
