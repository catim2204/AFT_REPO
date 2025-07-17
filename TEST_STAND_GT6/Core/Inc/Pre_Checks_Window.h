#ifndef PRECHECK
#define PRECHECK

#include "QWidget"
class PreCheckWindow : public QWidget 
{
    Q_OBJECT

public:
    explicit PreCheckWindow(QWidget *parent = nullptr);
    ~PreCheckWindow();

private slots:

signals:

private:
    PreCheckWindow *dialogBox;
};







#endif //PRECHECK