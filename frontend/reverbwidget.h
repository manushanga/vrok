#ifndef REVERBWIDGET_H
#define REVERBWIDGET_H

#include <QDockWidget>

namespace Ui {
class ReverbWidget;
}

class ReverbWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ReverbWidget(QWidget *parent = 0);
    ~ReverbWidget();

private:
    Ui::ReverbWidget *ui;
};

#endif // REVERBWIDGET_H
