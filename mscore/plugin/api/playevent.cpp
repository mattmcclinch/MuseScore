//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/undo.h"
#include "elements.h"
#include "playevent.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   PlayEvent::setPitch
//---------------------------------------------------------

void PlayEvent::setPitch(int v)
      {
      Note* parNote = static_cast<Note*>(parentNote);
      Ms::Score* score = parNote->note()->score();
      int parentPitch = parNote->note()->pitch();
      if (!score) {
            qWarning("PluginAPI::PlayEvent::setOntime: A score is required.");
            return;
            }
      // A NoteEvent's pitch is actually summed with the parent pitch. This
      // check ensures that it doesn't result with an illegal pitch.
      if (v + parentPitch <= 0 || v + parentPitch > 127) {
            qWarning("PluginAPI::PlayEvent::setPitch: Invalid relative pitch value.");
            return;
            }
      if (!ne || ne->pitch() == v)
            return;                             // Value hasn't changed so no need for undo
      Ms::NoteEvent e = *ne;                    // Make copy of NoteEvent value
      e.setPitch(v);                           // Set new ontTime value
      score->undo(new ChangeNoteEvent(parNote->note(), ne, e));
      }

//---------------------------------------------------------
//   PlayEvent::setOntime
//---------------------------------------------------------

void PlayEvent::setOntime(int v)
      {
      Note* parNote = static_cast<Note*>(parentNote);
      Ms::Score* score = parNote->note()->score();
      if (!score) {
            qWarning("PluginAPI::PlayEvent::setOntime: A score is required.");
            return;
            }
      // Note that onTime can be negative so a note can play earlier.
      // See: https://musescore.org/en/node/74651
      if (v < -2 * Ms::NoteEvent::NOTE_LENGTH || v > 2 * Ms::NoteEvent::NOTE_LENGTH) {
            qWarning("PluginAPI::PlayEvent::setOntime: Invalid value.");
            return;
            }
      if (!ne || ne->ontime() == v)
            return;                             // Value hasn't changed so no need for undo
      Ms::NoteEvent e = *ne;                    // Make copy of NoteEvent value
      e.setOntime(v);                           // Set new ontTime value
      score->undo(new ChangeNoteEvent(parNote->note(), ne, e));
      }

//---------------------------------------------------------
//   PlayEvent::setLen
//---------------------------------------------------------

void PlayEvent::setLen(int v)
      {
      Note* parNote = static_cast<Note*>(parentNote);
      Ms::Score* score = parNote->note()->score();
      if (!score) {
            qWarning("PluginAPI::PlayEvent::setLen: A score is required.");
            return;
            }
      if (v <= 0 || v > 2 * Ms::NoteEvent::NOTE_LENGTH) {
            qWarning("PluginAPI::PlayEvent::setLen: Invalid value.");
            return;
            }
      if (!ne || ne->len() == v)
            return;                             // Value hasn't changed so no need for undo
      Ms::NoteEvent e = *ne;                    // Make copy of NoteEvent value
      e.setLen(v);                              // Set new length value
      score->undo(new ChangeNoteEvent(parNote->note(), ne, e));
      }

}
}
