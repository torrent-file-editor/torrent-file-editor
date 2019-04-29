/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 *
 * Copyright (C) 2019  Ivan Romanov <drizt72@zoho.eu>
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

#include <QAbstractItemDelegate>
#include <QAction>
#include <QApplication>
#include <QCalendarWidget>
#include <QClipboard>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLibraryInfo>
#include <QLineEdit>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QProxyStyle>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QShortcut>
#include <QSizeGrip>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBlock>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <QTreeView>
#include <QUrl>
#include <QVariant>
#include <QWidget>
#include <qmath.h>

#ifdef HAVE_QT5
# include <QJsonDocument>
#else
# include <qjson/serializer.h>
# include <qjson/parser.h>
#endif

#ifdef Q_OS_WIN
# include <io.h>
# include <windows.h>
# include <tchar.h>
# include <windows.h>
# include <wininet.h>
#endif
