//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================


#ifndef NOTETWEAKERDIALOG_H
#define NOTETWEAKERDIALOG_H

#include "ui_notetweakerdialog.h"

namespace Ui {
class NoteTweakerDialog;
}

namespace Ms {

class Staff;
class Note;
class Chord;

class NoteTweakerDialog : public QDialog
      {
      Q_OBJECT

      Staff* _staff;
      QList<Note*> noteList;

   public:
      explicit NoteTweakerDialog(QWidget *parent = nullptr);
      ~NoteTweakerDialog();

      void setStaff(Staff* s);

   signals:
      void notesChanged();

   public slots:
      void setNoteOffTime();

   private:
      void addChord(Chord* chord, int voice);
      void updateNotes();
      void clearNoteData();


      Ui::NoteTweakerDialog *ui;
      };

}

#endif // NOTETWEAKERDIALOG_H
