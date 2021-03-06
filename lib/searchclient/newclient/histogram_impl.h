/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Carsten Niehaus <cniehaus@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef HISTOGRAM_IMPL_H
#define HISTOGRAM_IMPL_H


#include <QWidget>
#include <QStringList>
#include "ui_histogramwidget.h"

/**
 * @author Carsten Niehaus
 */
class histogramWidget_Impl : public QWidget
{
    Q_OBJECT

    public:
        histogramWidget_Impl( QWidget * parent = 0 );

        void setItems( const QStringList& items );

    public Q_SLOTS:
        void refresh();
        void setQuery( QString );
    
    private:
        Ui_histogramWidget ui;

        QString m_query;
};

#endif // HISTOGRAM_IMPL_H
