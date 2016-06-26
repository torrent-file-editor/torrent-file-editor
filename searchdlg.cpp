#include "searchdlg.h"
#include "ui_searchdlg.h"
#include "bencodemodel.h"

#ifdef Q_OS_MAC
# include <QSizeGrip>
#endif

SearchDlg::SearchDlg(BencodeModel *model, QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
#else
    : QDialog(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
    , ui(new Ui::SearchDlg)
    , _searchList()
    , _searchIndex(-1)
    , _model(model)
#ifdef Q_OS_MAC
    , _sizeGrip(new QSizeGrip(this))
#endif
{
    ui->setupUi(this);
    connect(_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(resetSearchList()));
    connect(_model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(resetSearchList()));

#ifdef Q_OS_MAC
    // Workaround. Qt has no size grip on Mac OS X. Bug?
    setSizeGripEnabled(false);
    _sizeGrip->setFixedSize(20, 20);
    updateSizeGripPos();
#endif
}

SearchDlg::~SearchDlg()
{
    delete ui;
}

void SearchDlg::setReplaceModeEnabled(bool b)
{
    setWindowTitle(b ? tr("Replace") : tr("Find"));
    ui->wdgReplace->setVisible(b);
    ui->btnReplace->setVisible(b);
    ui->btnReplaceAll->setVisible(b);

    setMinimumHeight(0);
    setMaximumHeight(16777215);
    adjustSize();
    setFixedHeight(height());

    updateSearchNext();
}

void SearchDlg::searchNext()
{
    _searchIndex += ui->rdDown->isChecked() ? +1 : -1;

    if (ui->rdDown->isChecked() && _searchIndex == _searchList.size()
        || ui->rdUp->isChecked() && _searchIndex == -1) {

        resetSearchList();
    }

    if (_searchList.isEmpty()) {
        QModelIndexList keys;
        QModelIndexList values;

        if (ui->grpKey->isChecked()) {
            QString key = ui->lneKey->text();
            if (key.isEmpty())
                return;

            Qt::MatchFlags matchFlags(Qt::MatchFlag::MatchRecursive);
            if (ui->chkKeyCase->isChecked())
                matchFlags |= Qt::MatchCaseSensitive;

            if (ui->rdKeyExactMatch->isChecked())
                matchFlags |= Qt::MatchFlag::MatchFixedString;
            else if (ui->rdKeyWildcards->isChecked())
                matchFlags |= Qt::MatchFlag::MatchWildcard;
            else if (ui->rdKeyRegexp->isChecked())
                matchFlags |= Qt::MatchFlag::MatchRegExp;

            keys = _model->match(_model->index(0, 0), Qt::DisplayRole, key, -1, matchFlags);
        }

        if (ui->grpValue->isChecked()) {
            QString value = ui->lneValue->text();
            if (value.isEmpty())
                return;

            Qt::ItemDataRole role = Qt::UserRole;
            Qt::MatchFlags matchFlags(Qt::MatchFlag::MatchRecursive);
            if (ui->chkValueCase->isChecked())
                matchFlags |= Qt::MatchCaseSensitive;

            if (ui->rdValueExactMatch->isChecked()) {
                matchFlags |= Qt::MatchFlag::MatchFixedString;
            }
            else if (ui->rdValueWildcards->isChecked()) {
                matchFlags |= Qt::MatchFlag::MatchWildcard;
            }
            else if (ui->rdValueRegexp->isChecked()) {
                matchFlags |= Qt::MatchFlag::MatchRegExp;
            }
            else if (ui->rdValueHex->isChecked()) {
                matchFlags |= Qt::MatchFlag::MatchContains;
                matchFlags &= ~Qt::MatchFlag::MatchCaseSensitive;
                role = static_cast<Qt::ItemDataRole>(Qt::UserRole + 1);
            }

            values = _model->match(_model->index(0, 0), role, value, -1, matchFlags);
        }

        if (ui->grpKey->isChecked() && ui->grpValue->isChecked()) {
            for (const auto &key: keys) {
                for (const auto &value: values) {
                    if (value == key) {
                        _searchList << key;
                    }
                }
            }
        }
        else if (ui->grpKey->isChecked()) {
            _searchList = keys;
        }
        else if (ui->grpValue->isChecked()) {
            _searchList = values;
        }
        _searchIndex = ui->rdDown->isChecked() ? 0 : _searchList.size() - 1;
    }

    if (_searchList.isEmpty()) {
        ui->lblItemsFound->setText(tr("No matches found"));
        _searchIndex = -1;
        return;
    }

    ui->lblItemsFound->setText(tr("%1 of %n match(es)", 0, _searchList.size()).arg(_searchIndex + 1));
    emit foundItem(_searchList.at(_searchIndex));
}

void SearchDlg::updateSearchNext()
{
    bool enabled = false;

    if (ui->grpKey->isChecked() && !ui->lneKey->text().isEmpty())
        enabled = true;
    else if (ui->grpValue->isChecked() && !ui->lneValue->text().isEmpty())
        enabled = true;

    if (ui->grpKey->isChecked() && ui->lneKey->text().isEmpty())
        enabled = false;
    else if (ui->grpValue->isChecked() && ui->lneValue->text().isEmpty())
        enabled = false;

    ui->btnSearchNext->setEnabled(enabled);
    ui->btnReplace->setEnabled(enabled);
    ui->btnReplaceAll->setEnabled(enabled);
}

void SearchDlg::resetSearchList()
{
    _searchList.clear();
    _searchIndex = -1;
    ui->lblItemsFound->setText("");
}

void SearchDlg::replace()
{
    QString replaceStr = ui->lneReplace->text();
    if (_searchIndex >= 0)
        _model->setData(_searchList.at(_searchIndex), replaceStr, Qt::UserRole + (ui->chkHex->isChecked() ? 1 : 0));

    searchNext();
}

void SearchDlg::replaceAll()
{
    if (_searchList.isEmpty())
        searchNext();

    if (_searchList.isEmpty())
        return;

    QString replaceStr = ui->lneReplace->text();
    for (const auto &item: _searchList) {
        _model->setData(item, replaceStr, Qt::UserRole + (ui->chkHex->isChecked() ? 1 : 0));
    }

    QString resStr = tr("%n value(s) was(were) replaced", 0, _searchList.size());
    resetSearchList();
    ui->lblItemsFound->setText(resStr);
}

#ifdef Q_OS_MAC
void SearchDlg::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    updateSizeGripPos();
}

void SearchDlg::updateSizeGripPos()
{
    int x = width() - _sizeGrip->width();
    int y = height() - _sizeGrip->height();
    _sizeGrip->move(x, y);
}
#endif
