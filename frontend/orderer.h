#ifndef ORDERER_H
#define ORDERER_H
#include <QDialog>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QListView>
#include <QPushButton>

#define PB_UP 0
#define PB_DOWN 1
#define PB_OK 2
#define PB_CANCEL 3

class Orderer : public QDialog {
    Q_OBJECT
private:
    QListView lv;
    QPushButton pb[4];
public:
    bool okPressed;
    QStandardItemModel *model;
    Orderer( QStandardItemModel *list, QWidget *parent=0) :
        QDialog(parent),
        model(list),
        okPressed(false)
    {
        lv.setEditTriggers(QAbstractItemView::NoEditTriggers);
        lv.setParent(this);
        lv.setModel(list);
        lv.setGeometry(10,10, 200 , 280);
        setFixedHeight(300);
        setFixedWidth(320);
        for (int i=0;i<4;i++)
            pb[i].setParent(this);

        pb[PB_UP].setText("Move up");
        pb[PB_UP].setGeometry(220,10, 90, 25);
        pb[PB_DOWN].setText("Move down");
        pb[PB_DOWN].setGeometry(220,45, 90, 25);
        pb[PB_OK].setText("OK");
        pb[PB_OK].setGeometry(220,85, 90,25);
        pb[PB_CANCEL].setText("Cancel");
        pb[PB_CANCEL].setGeometry(220, 120, 90,25);

        connect(&pb[PB_UP],SIGNAL(clicked()),this,SLOT(up()));
        connect(&pb[PB_DOWN],SIGNAL(clicked()),this,SLOT(down()));
        connect(&pb[PB_OK],SIGNAL(clicked()),this,SLOT(ok()));
        connect(&pb[PB_CANCEL],SIGNAL(clicked()),this,SLOT(cancel()));


    }
public slots:

    void up(){
        if (lv.selectionModel()->selectedRows().size()) {
            int i=lv.selectionModel()->selectedRows().at(0).row();
            if (i>0) {
                QString tmp = model->item(i)->text();
                Qt::CheckState c = model->item(i)->checkState();
                model->item(i)->setText( model->item(i-1)->text() );
                model->item(i)->setCheckState( model->item(i-1)->checkState() );
                model->item(i-1)->setText(tmp);
                model->item(i-1)->setCheckState(c);

                lv.selectionModel()->setCurrentIndex(model->index(i-1,0),QItemSelectionModel::Select);
                lv.selectionModel()->setCurrentIndex(model->index(i,0),QItemSelectionModel::Deselect);

            }
        }
    }
    void down(){
        if (lv.selectionModel()->selectedRows().size()) {
            int i=lv.selectionModel()->selectedRows().at(0).row();
            if (i<model->rowCount() -1) {
                QString tmp = model->item(i+1)->text();
                Qt::CheckState c = model->item(i+1)->checkState();
                model->item(i+1)->setText( model->item(i)->text() );
                model->item(i+1)->setCheckState( model->item(i)->checkState() );
                model->item(i)->setText(tmp);
                model->item(i)->setCheckState(c);

                lv.selectionModel()->setCurrentIndex(model->index(i+1,0),QItemSelectionModel::Select);
                lv.selectionModel()->setCurrentIndex(model->index(i,0),QItemSelectionModel::Deselect);

            }
        }
    }
    void ok(){
        okPressed=true;
        close();
    }
    void cancel(){
        close();
    }
signals:
    void selectionDone();
    void selectionCancelled();
};
#endif // ORDERER_H
