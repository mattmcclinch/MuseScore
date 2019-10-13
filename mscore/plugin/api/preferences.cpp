//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "preferences.h"
#include "mscore/preferences.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   sessionStart
//---------------------------------------------------------

SessionStart Preferences::sessionStart() const
      {
      return preferences.sessionStart();
      }

//---------------------------------------------------------
//   musicxmlExportBreaks
//---------------------------------------------------------

MusicxmlExportBreaks Preferences::musicxmlExportBreaks() const
      {
      return preferences.musicxmlExportBreaks();
      }

//---------------------------------------------------------
//   globalStyle
//---------------------------------------------------------

MuseScoreStyleType Preferences::globalStyle() const
      {
      return preferences.globalStyle();
      }

//---------------------------------------------------------
//   isThemeDark
//---------------------------------------------------------

bool Preferences::isThemeDark() const
      {
      return preferences.isThemeDark();
      }

//---------------------------------------------------------
//   get
//---------------------------------------------------------

QVariant Preferences::get(const QString& key)
      {
      return preferences.getVariant(key);
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void Preferences::set(const QString& key, QVariant value)
      {
      preferences.setPreference(key, value);
      }

//---------------------------------------------------------
//   setToDefaultValue
//---------------------------------------------------------

void Preferences::setToDefaultValue(const QString& key)
      {
      preferences.setToDefaultValue(key);
      }

} // namespace PluginAPI
} // namespace Ms
