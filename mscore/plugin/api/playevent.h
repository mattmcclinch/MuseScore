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

#ifndef __PLUGIN_API_NOTEEVENT_H__
#define __PLUGIN_API_NOTEEVENT_H__

#include "libmscore/noteevent.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   PlayEvent
//    Wrapper class for internal Ms::NoteEvent
//
//   This is based on the wrapper in excerpt.h. 
//---------------------------------------------------------

class PlayEvent : public QObject {
    Q_OBJECT
    /// The relative pitch of the event.
    /// This is added to the parent note's actual pitch.
    /// \since MuseScore 3.4
    Q_PROPERTY(int pitch READ pitch WRITE setPitch)
    /// Time to turn on the note event.
    /// \since MuseScore 3.4
    Q_PROPERTY(int ontime READ ontime WRITE setOntime)
    /// The length of time for the event.
    /// \since MuseScore 3.4
    Q_PROPERTY(int len READ len WRITE setLen)
    /// Time note will turn off.
    /// This value derived from onTime and len.
    /// \since MuseScore 3.4
    Q_PROPERTY(int offtime READ offtime)

    /// \cond MS_INTERNAL
 protected:
    Ms::NoteEvent* ne;
    QObject* parentNote;

 public:
    PlayEvent(Ms::NoteEvent* _ne, QObject* _parent)
       : QObject(), ne(_ne), parentNote(_parent) {}
    PlayEvent(const PlayEvent&) = delete;
    PlayEvent& operator=(const PlayEvent&) = delete;
    virtual ~PlayEvent() {}

    int pitch() const { return ne->pitch(); }
    int ontime() const { return ne->ontime(); }
    int offtime() const { return ne->offtime(); }
    int len() const { return ne->len(); }
    void setPitch(int v);
    void setOntime(int v);
    void setLen(int v);
    /// \endcond
};

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   \relates Ms::NoteEvent
//---------------------------------------------------------

template <class Wrapper, class T>
Wrapper* playEventWrap(T* t, QObject* parent)
      {
      Wrapper* w = t ? new Wrapper(t, parent) : nullptr;
      // All wrapper objects should belong to JavaScript code.
      QQmlEngine::setObjectOwnership(w, QQmlEngine::JavaScriptOwnership);
      return w;
      }

//---------------------------------------------------------
//   qml access to containers of NoteEvent
//
//   QmlNoteEventsListAccess provides a convenience interface
//   for QQmlListProperty providing access to plugins for NoteEvent
//   Containers.
//
//   based on QmlListAccess in excerpt.h
//---------------------------------------------------------


template <typename T, class Container>
class QmlPlayEventsListAccess : public QQmlListProperty<T> {
public:
      QmlPlayEventsListAccess(QObject* obj, Container& container)
            : QQmlListProperty<T>(obj, &container, &count, &at) {};

      static int count(QQmlListProperty<T>* l)         { return int(static_cast<Container*>(l->data)->size()); }
      static T*     at(QQmlListProperty<T>* l, int i)  { return playEventWrap<T>(&(*(static_cast<Container*>(l->data)))[i], l->object); }
      };

/** \cond PLUGIN_API \private \endcond */
template<typename T, class Container>
QmlPlayEventsListAccess<T, Container> wrapPlayEventsContainerProperty(QObject* obj, Container& c)
      {
      return QmlPlayEventsListAccess<T, Container>(obj, c);
      }


} // namespace PluginAPI
} // namespace Ms
#endif
