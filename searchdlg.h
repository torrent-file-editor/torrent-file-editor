#pragma once

#include <QDialog>
#include <QModelIndex>

class BencodeModel;

#ifdef Q_OS_MAC
class QSizeGrip;
#endif

namespace Ui { class SearchDlg; }

class SearchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDlg(BencodeModel *model, QWidget *parent = 0);
    ~SearchDlg();
    void setReplaceModeEnabled(bool b);

public slots:
    void searchNext();
    void updateSearchNext();
    void resetSearchList();
    void replace();
    void replaceAll();

signals:
    void foundItem(const QModelIndex &index);

#ifdef Q_OS_MAC
protected:
    void resizeEvent(QResizeEvent *event);
#endif

private:
#ifdef Q_OS_MAC
    void updateSizeGripPos();
#endif
    Ui::SearchDlg *ui;
    QModelIndexList _searchList;
    int _searchIndex;
    BencodeModel *_model;
#ifdef Q_OS_MAC
    QSizeGrip *_sizeGrip;
#endif
};
