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

#ifndef __PLUGIN_API_PREFERENCES_H__
#define __PLUGIN_API_PREFERENCES_H__

namespace Ms {

enum class SessionStart : char;
enum class MusicxmlExportBreaks : char;
enum class MuseScoreStyleType : char;

namespace PluginAPI {

class Preferences : public QObject {
      Q_OBJECT

      Q_PROPERTY(SessionStart sessionStart READ sessionStart)
      Q_PROPERTY(MusicxmlExportBreaks musicxmlExportBreaks READ musicxmlExportBreaks)
      Q_PROPERTY(MuseScoreStyleType globalStyle READ globalStyle)
      Q_PROPERTY(bool isThemeDark READ isThemeDark)

   public:
      SessionStart sessionStart() const;
      MusicxmlExportBreaks musicxmlExportBreaks() const;
      MuseScoreStyleType globalStyle() const;
      bool isThemeDark() const;

      Q_INVOKABLE QVariant get(const QString& key);
      Q_INVOKABLE void set(const QString& key, QVariant value);
      Q_INVOKABLE void setToDefaultValue(const QString& key);
      };

} // namespace PluginAPI
} // namespace Ms
#endif
