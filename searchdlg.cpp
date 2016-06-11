#include "searchdlg.h"
#include "ui_searchdlg.h"
#include "bencodemodel.h"

SearchDlg::SearchDlg(BencodeModel *model, QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
#else
    : QDialog(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
#endif
    , ui(new Ui::SearchDlg)
    , _searchList()
    , _searchIndex(0)
    , _model(model)
{
    ui->setupUi(this);

    adjustSize();
    setMinimumHeight(height());
    setMaximumHeight(height());

    updateSearchNext();
}

SearchDlg::~SearchDlg()
{
    delete ui;
}

void SearchDlg::searchNext()
{
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

        if (_searchList.isEmpty())
            ui->lblItemsFound->setText(tr("No matches found"));
        else
            ui->lblItemsFound->setText(tr("Found %n match(es)", 0, _searchList.size()));

        _searchIndex = 0;
    }
    else {
        if (ui->rdDown->isChecked()) {
            _searchIndex = (_searchIndex + 1) == _searchList.size() ? 0 : _searchIndex + 1;
        }
        else {
            _searchIndex = (_searchIndex - 1) == 0 ? _searchList.size() - 1 : _searchIndex - 1;
        }
    }


    if (_searchList.isEmpty())
        return;

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
}

void SearchDlg::resetSearchList()
{
    _searchList.clear();
}
