//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __TOUCHBAR_H__
#define __TOUCHBAR_H__

class TouchBar {
      QString _identifierPrefix;
      void* _provider;
   public:
      TouchBar(QString identifierPrefix, QString name);
      void addButton(QString name, QAction* a, QString description);
      void setAsTouchBarForWidget(QWidget* widget);
      void setAsTouchBarForApplication();
      };

#endif
