#pragma once

#include <QDialog>
#include <QModelIndex>

class BencodeModel;

namespace Ui { class SearchDlg; }

class SearchDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDlg(BencodeModel *model, QWidget *parent = 0);
    ~SearchDlg();

public slots:
    void searchNext();
    void updateSearchNext();
    void resetSearchList();

signals:
    void foundItem(const QModelIndex &index);

private:
    Ui::SearchDlg *ui;
    QModelIndexList _searchList;
    int _searchIndex;
    BencodeModel *_model;
};
