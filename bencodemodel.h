/*
 * Copyright (C) 2015  Ivan Romanov <drizt@land.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "abstracttreemodel.h"
#include <QString>

class BencodeModel : public AbstractTreeModel
{
    Q_OBJECT

public:
    enum class Columns
    {
        Name,
        Type,
        Value,
        Count // Trick to count elements in enum
    };

    explicit BencodeModel(QObject *parent = 0);
    ~BencodeModel();

    void add(const QString &name, const QModelIndex &index);
    void remove(const QModelIndex &index);
    void up(const QModelIndex &index);
    void down(const QModelIndex &index);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};
