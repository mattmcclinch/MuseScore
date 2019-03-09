//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "fret.h"
#include "measure.h"
#include "system.h"
#include "score.h"
#include "stringdata.h"
#include "chord.h"
#include "note.h"
#include "segment.h"
#include "mscore.h"
#include "harmony.h"

namespace Ms {

//    parent() is Segment or Box
//

//---------------------------------------------------------
//   fretStyle
//---------------------------------------------------------

static const ElementStyle fretStyle {
      { Sid::fretNumPos,                         Pid::FRET_NUM_POS            },
      { Sid::fretMag,                            Pid::MAG                     },
      { Sid::fretPlacement,                      Pid::PLACEMENT               },
      { Sid::fretStrings,                        Pid::FRET_STRINGS            },
      { Sid::fretFrets,                          Pid::FRET_FRETS              },
      { Sid::fretOffset,                         Pid::FRET_OFFSET             },
      /*{ Sid::fretBarre,                          Pid::FRET_BARRE              },*/
      };

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      font.setFamily("FreeSans");
      font.setPointSize(4.0 * mag());
      initElementStyle(&fretStyle);
      }

FretDiagram::FretDiagram(const FretDiagram& f)
   : Element(f)
      {
      _strings    = f._strings;
      _frets      = f._frets;
      _fretOffset = f._fretOffset;
      _maxFrets   = f._maxFrets;
      font        = f.font;
      _userMag    = f._userMag;
      _numPos     = f._numPos;
      _dots       = f._dots;
      _markers    = f._markers;
      _barres     = f._barres;
      _fingering  = f._fingering;

      if (f._harmony)
            _harmony = new Harmony(*f._harmony);
      }

//---------------------------------------------------------
//   fromString
//    Create diagram from string like "XO-123"
//    Always assume barre on the first visible fret
//---------------------------------------------------------

FretDiagram* FretDiagram::fromString(Score* score, const QString &s)
      {
      FretDiagram* fd = new FretDiagram(score);
      int strings = s.size();
      qDebug("%d strings", strings);

      fd->setStrings(strings);
      fd->setFrets(4);
      int offset = 0;
      int barreString = -1;
      std::vector<std::pair<int, int>> dotsToAdd;

      for (int i = 0; i < strings; i++) {
            QChar c = s.at(i);
            if (c == 'X' || c == 'O') {
                  FretMarkerType mt = (c == 'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
                  fd->setMarker(i, mt);
                  }
            else if (c == '-' && barreString == -1) {
                  barreString = i;
                  }
            else {
                  int fret = c.digitValue();
                  if (fret != -1) {
                        dotsToAdd.push_back(make_pair(i, fret));
                        if (fret - 3 > 0 && offset < fret - 3)
                            offset = fret - 3;
                        }
                  }
            }

      if (offset > 0)
            fd->setFretOffset(offset);

      for (std::pair<int, int> d : dotsToAdd) {
            fd->setDot(d.first, d.second - offset, true);
            }

      // This assumes that any barre goes to the end of the fret
      if (barreString >= 0)
            fd->setBarre(barreString, -1, 1);

      return fd;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF FretDiagram::pagePos() const
      {
      if (parent() == 0)
            return pos();
      if (parent()->isSegment()) {
            Measure* m = toSegment(parent())->measure();
            System* system = m->system();
            qreal yp = y();
            if (system)
                  yp += system->staffYpage(staffIdx());
            return QPointF(pageX(), yp);
            }
      else
            return Element::pagePos();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF FretDiagram::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp;
      if (parent()->isSegment()) {
            System* system = toSegment(parent())->measure()->system();
            yp = system->staffCanvasYpage(staffIdx());
            }
      else
            yp = parent()->canvasPos().y();
      QPointF p1(xp, yp);
      return QLineF(p1, canvasPos());
#if 0 // TODOxx
      if (parent()->isSegment()) {
            Segment* s     = toSegment(parent());
            Measure* m     = s->measure();
            System* system = m->system();
            qreal yp      = system->staff(staffIdx())->y() + system->y();
            qreal xp      = m->tick2pos(s->tick()) + m->pagePos().x();
            QPointF p1(xp, yp);

            qreal x  = 0.0;
            qreal y  = 0.0;
            qreal tw = width();
            qreal th = height();
            if (_align & Align::BOTTOM)
                  y = th;
            else if (_align & Align::VCENTER)
                  y = (th * .5);
            else if (_align & Align::BASELINE)
                  y = baseLine();
            if (_align & Align::RIGHT)
                  x = tw;
            else if (_align & Align::HCENTER)
                  x = (tw * .5);
            return QLineF(p1, abbox().topLeft() + QPointF(x, y));
            }
      return QLineF(parent()->pagePos(), abbox().topLeft());
#endif
      }

//---------------------------------------------------------
//   setStrings
//---------------------------------------------------------

void FretDiagram::setStrings(int n)
      {
      int difference = n - _strings;
      if (difference == 0 || n <= 0)
            return;

      // Move all dots, makers, barres to the RIGHT, so we add strings to the left
      // This is more useful - few instruments need strings added to the right.
      DotMap tempDots;
      MarkerMap tempMarkers;

      for (int string = 0; string < _strings; ++string) {
            if (string + difference < 0)
                  continue;

            for (auto const& d : dot(string)) {
                  if (d.exists())
                        tempDots[string + difference].push_back(FretItem::Dot(d));
                  }

            if (marker(string).exists())
                  tempMarkers[string + difference] = marker(string);
            }

      _dots = tempDots;
      _markers = tempMarkers;

      for (int fret = 1; fret <= _frets; ++fret) {
            if (barre(fret).exists()) {
                  if (_barres[fret].startString + difference <= 0) {
                        removeBarre(fret);
                        continue;
                        }

                  _barres[fret].startString = qMax(0, _barres[fret].startString + difference);
                  _barres[fret].endString   = _barres[fret].endString == -1 ? -1 : _barres[fret].endString + difference;
                  }

            }

      _strings = n;
      }

void FretDiagram::setFretOffset(int val)
      {
      _fretOffset = val;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(StringData* stringData, Chord* chord)
      {
      if (!stringData)
            setStrings(6);
      else
            setStrings(stringData->strings());
      if (stringData) {
            for (int string = 0; string < _strings; ++string)
                  setMarker(string, FretMarkerType::CROSS);
            for (const Note* note : chord->notes()) {
                  int string;
                  int fret;
                  if (stringData->convertPitch(note->pitch(), chord->staff(), chord->segment()->tick(), &string, &fret))
                        setDot(string, fret);
                  }
            _maxFrets = stringData->frets();
            }
      else
            _maxFrets = 6;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(QPainter* painter) const
      {
      qreal _spatium = spatium() * _userMag * score()->styleD(Sid::fretMag);
      QPen pen(curColor());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      painter->setPen(pen);
      painter->setBrush(QBrush(QColor(painter->pen().color())));
      qreal x2 = (_strings-1) * stringDist;
      painter->drawLine(QLineF(-lw1 * .5, 0.0, x2 + lw1 * .5, 0.0));

      // Draw strings and frets
      pen.setWidthF(lw1);
      painter->setPen(pen);
      qreal y2 = (_frets+1) * fretDist - fretDist * .5;
      for (int i = 0; i < _strings; ++i) {
            qreal x = stringDist * i;
            painter->drawLine(QLineF(x, _fretOffset ? -_spatium * .2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            qreal y = fretDist * i;
            painter->drawLine(QLineF(0.0, y, x2, y));
            }
      QFont scaledFont(font);
      scaledFont.setPointSizeF(font.pointSize() * _userMag * score()->styleD(Sid::fretMag) * (spatium() / SPATIUM20));
      QFontMetricsF fm(scaledFont, MScore::paintDevice());
      scaledFont.setPointSizeF(scaledFont.pointSizeF() * MScore::pixelRatio);

      painter->setFont(scaledFont);
      qreal dotd = stringDist * .6;

      // Draw dots and markers
      for (auto const& i : _dots) {
            for (auto const& d : i.second) {
                  if (!d.exists())
                        continue;

                  int string = i.first;
                  int fret = d.fret - 1;

                  qreal x = stringDist * string - dotd * .5;
                  qreal y = fretDist * fret + fretDist * .5 - dotd * .5;

                  // TODO: draw different symbols
                  painter->drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            }

      for (auto const& i : _markers) {
            int string = i.first;
            FretItem::Marker marker = i.second;

            QChar markerChar = FretItem::markerToChar(marker.mtype);

            qreal x = stringDist * string;
            qreal y = -fretDist * .3 - fm.ascent();
            painter->drawText(QRectF(x, y, .0, .0),
               Qt::AlignHCenter|Qt::TextDontClip, markerChar);
            }

      // Draw barres
      for (auto const& i : _barres) {
            int fret        = i.first;
            int startString = i.second.startString;
            int endString   = i.second.endString;

            qreal x1    = stringDist * startString;
            qreal newX2 = endString == -1 ? x2 : stringDist * endString;
            qreal y     = fretDist * (fret - 1) + fretDist * .5;
            pen.setWidthF(dotd * .8 * score()->styleD(Sid::barreLineWidth));
            pen.setCapStyle(Qt::RoundCap);
            painter->setPen(pen);
            painter->drawLine(QLineF(x1, y, newX2, y));
            }

      // Draw fret offset number
      if (_fretOffset > 0) {
            qreal fretNumMag = score()->styleD(Sid::fretNumMag);
            scaledFont.setPointSizeF(scaledFont.pointSizeF() * fretNumMag);
            painter->setFont(scaledFont);
            if (_numPos == 0) {
                  painter->drawText(QRectF(-stringDist *.4, .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
                  }
            else {
                  painter->drawText(QRectF(x2 + (stringDist * 0.4), .0, .0, fretDist),
                     Qt::AlignVCenter|Qt::AlignLeft|Qt::TextDontClip,
                     QString("%1").arg(_fretOffset+1));
                  }
            painter->setFont(font);
            }

      // NOTE:JT possible future todo - draw fingerings
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      qreal _spatium  = spatium() * _userMag * score()->styleD(Sid::fretMag);
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      qreal w    = stringDist * (_strings - 1);
      qreal h    = _frets * fretDist + fretDist * .5;
      qreal y    = 0.0;
      qreal dotd = stringDist * .6;
      qreal x    = -((dotd+lw1) * .5);
      w         += dotd + lw1;

      // Always allocate space for markers
      QFont scaledFont(font);
      scaledFont.setPointSize(font.pointSize() * _userMag);
      QFontMetricsF fm(scaledFont, MScore::paintDevice());
      y  = -(fretDist * .1 + fm.height());
      h -= y;

      bbox().setRect(x, y, w, h);

      setPos(-_spatium, -h - styleP(Sid::fretY) + _spatium );

      if (!parent() || !parent()->isSegment()) {
            setPos(QPointF());
            return;
            }
      qreal minDistance = styleP(Sid::fretMinDistance);
      autoplaceSegmentElement(minDistance);
      if (_harmony)
            _harmony->layout();
      if (_harmony && _harmony->visible() && _harmony->autoplace() && _harmony->parent()) {
            Segment* s = toSegment(parent());
            Measure* m = s->measure();
            int si     = staffIdx();

            SysStaff* ss = m->system()->staff(si);
            QRectF r     = _harmony->bbox().translated(m->pos() + s->pos() + pos() + _harmony->pos());

            SkylineLine sk(false);
            sk.add(r.x(), r.bottom(), r.width());
            qreal d = sk.minDistance(ss->skyline().north());
            if (d > -minDistance) {
                  qreal yd = d + minDistance;
                  yd *= -1.0;
                  _harmony->rypos() += yd;
                  r.translate(QPointF(0.0, yd));
                  }
            ss->skyline().add(r);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this, "version=\"2\"");
      Element::writeProperties(xml);

      writeProperty(xml, Pid::FRET_STRINGS);
      writeProperty(xml, Pid::FRET_FRETS);
      writeProperty(xml, Pid::FRET_OFFSET);

      for (int i = 0; i < _strings; ++i) {
            FretItem::Marker m = marker(i);
            std::vector<FretItem::Dot> allDots = dot(i);

            bool dotExists = false;
            for (auto const& d : allDots) {
                  if (d.exists()) {
                        dotExists = true;
                        break;
                        }
                  }

            // Only write a string if we have anything to write
            if (!dotExists && !m.exists())
                  continue;

            // Start the string writing
            xml.stag(QString("string no=\"%1\"").arg(i));

            // Write marker
            if (m.exists())
                  xml.tag("marker", FretItem::markerTypeToName(m.mtype));

            // Write any dots
            for (auto const& d : allDots) {
                  if (d.exists()) {
                        // TODO: write fingering
                        xml.tag(QString("dot fret=\"%1\"").arg(d.fret), FretItem::dotTypeToName(d.dtype));
                        }
                  }

            xml.etag();
            }

      for (int f = 1; f <= _frets; ++f) {
            FretItem::Barre b = barre(f);
            if (!b.exists())
                  continue;

            xml.tag(QString("barre start=\"%1\" end=\"%2\"").arg(b.startString).arg(b.endString), f);
            }

      writeProperty(xml, Pid::MAG);
      if (_harmony)
            _harmony->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(XmlReader& e)
      {
      int version = e.intAttribute("version", 1);
      if (version == 1) {
            read1(e);
            return;
            }

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "strings")
                  readProperty(e, Pid::FRET_STRINGS);
            else if (tag == "frets")
                  readProperty(e, Pid::FRET_FRETS);
            else if (tag == "fretOffset")
                  readProperty(e, Pid::FRET_OFFSET);
            else if (tag == "string") {
                  int no = e.intAttribute("no");
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "dot") {
                              int fret = e.intAttribute("fret", 0);
                              FretDotType dtype = FretItem::nameToDotType(e.readElementText());
                              qDebug("read dot, %d:%d", no, fret);
                              setDot(no, fret, true, dtype);
                              }
                        else if (t == "marker") {
                              FretMarkerType mtype = FretItem::nameToMarkerType(e.readElementText());
                              setMarker(no, mtype);
                              }
                        else if (t == "fingering") {
                              e.readElementText();
                              /*setFingering(no, e.readInt()); NOTE:JT todo */
                              }
                        else
                              e.unknown();
                        }
                  }
            else if (tag == "barre") {
                  int start = e.intAttribute("start", -1);
                  int end = e.intAttribute("end", -1);
                  int fret = e.readInt();

                  setBarre(start, end, fret);
                  }
            else if (tag == "mag")
                  readProperty(e, Pid::MAG);
            else if (tag == "Harmony") {
                  Harmony* h = new Harmony(score());
                  h->read(e);
                  add(h);
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }


//---------------------------------------------------------
//   read1
//    This reads the old 301 fret diagram format, which is really incompatible
//    with the current internal structure.
//---------------------------------------------------------

void FretDiagram::read1(XmlReader& e)
      {
      bool hasBarre = false;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "strings")
                  readProperty(e, Pid::FRET_STRINGS);
            else if (tag == "frets")
                  readProperty(e, Pid::FRET_FRETS);
            else if (tag == "fretOffset")
                  readProperty(e, Pid::FRET_OFFSET);
            else if (tag == "string") {
                  int no = e.intAttribute("no");
                  while (e.readNextStartElement()) {
                        const QStringRef& t(e.name());
                        if (t == "dot")
                              setDot(no, e.readInt());
                        else if (t == "marker")
                              setMarker(no, QChar(e.readInt()) == 'X' ? FretMarkerType::CROSS : FretMarkerType::CIRCLE);
                        /*else if (t == "fingering")
                              setFingering(no, e.readInt());*/
                        else
                              e.unknown();
                        }
                  }
            else if (tag == "barre")
                  hasBarre = e.readBool();
            else if (tag == "mag")
                  readProperty(e, Pid::MAG);
            else if (tag == "Harmony") {
                  Harmony* h = new Harmony(score());
                  h->read(e);
                  add(h);
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }


      if (hasBarre) {
            for (int s = 0; s < _strings; ++s) {
                  for (auto& d : dot(s)) {
                        if (d.exists()) {
                              setBarre(s, d.fret);
                              return;     // We can end here
                              }
                        }
                  }
            }
      }
//---------------------------------------------------------
//   setDot
//    take a fret value of 0 to mean remove the dot
//---------------------------------------------------------

void FretDiagram::setDot(int string, int fret, bool add /*= false*/, FretDotType dtype /*= FretDotType::NORMAL*/)
      {
      if (fret == 0)
            removeDot(string, fret);
      else if (string >= 0 && string < _strings) {
            if (!add)
                  _dots[string].clear();

            _dots[string].push_back(FretItem::Dot(fret));

            setMarker(string, FretMarkerType::NONE);
            }
      }

//---------------------------------------------------------
//   setMarker
//    Remove any dots and barres if the marker is being set to anything other than none.
//---------------------------------------------------------

void FretDiagram::setMarker(int string, FretMarkerType mtype)
      {
      if (string >= 0 && string < _strings) {
            _markers[string] = FretItem::Marker(mtype);
            if (mtype != FretMarkerType::NONE) {
                  for (auto const& d : dot(string)) {
                        if (d.exists())
                              removeDot(string);
                        }

                  removeBarres(string);
                  }
            }
      }

//---------------------------------------------------------
//   setFingering
//    NOTE:JT: todo investigate if we need this
//---------------------------------------------------------

void FretDiagram::setFingering(int string, int finger)
      {
#if 0
      if (_dots.find(string) != _dots.end()) {
            _dots[string].fingering = finger;
            qDebug("set finger: s %d finger %d", string, finger);
            }
#endif
      }

//---------------------------------------------------------
//   setBarre
//    We'll accept a value of -1 for the end string, to denote
//    that the barre goes as far right as possible.
//    Take a start string value of -1 to mean 'remove this barre'
//---------------------------------------------------------

void FretDiagram::setBarre(int startString, int endString, int fret)
      {
      if (startString == -1)
            removeBarre(fret);
      else if (startString >= 0 && endString >= -1 && startString < _strings && endString < _strings)
            _barres[fret] = FretItem::Barre(startString, endString);
      }

//---------------------------------------------------------
//    This version is for clicks on a dot with shift.
//    If there is no barre at fret, then add one with the string as the start.
//    If there is a barre with a -1 end string, set the end string to string.
//    If there is a barre with a set start and end, remove it.
//---------------------------------------------------------

void FretDiagram::setBarre(int string, int fret)
      {
      FretItem::Barre b = barre(fret);

      if (!b.exists()) {
            if (string < _strings - 1) {
                  _barres[fret] = FretItem::Barre(string, -1);
                  removeDotsMarkers(string, -1, fret);
                  setDot(string, fret);
                  }
            } 
      else if (b.endString == -1 && b.startString < string) {
            _barres[fret].endString = string;
            setDot(string, fret);
            }
      else {
            removeDotsMarkers(b.startString, b.endString, fret);
            removeBarre(fret);
            }
      }

//---------------------------------------------------------
//   removeBarre
//    Remove a barre on a given fret.
//---------------------------------------------------------

void FretDiagram::removeBarre(int f)
      {
      auto it = _barres.find(f);
      _barres.erase(it);
      }

//---------------------------------------------------------
//   removeBarres
//    Remove barres crossing a certain point. Fret of 0 means any point along
//    the string.
//---------------------------------------------------------

void FretDiagram::removeBarres(int string, int fret /*= 0*/)
      {
      auto iter = _barres.begin();
      while (iter != _barres.end()) {
            int bfret = iter->first;
            FretItem::Barre b = iter->second;

            if (b.exists() && b.startString <= string && (b.endString >= string || b.endString == -1)) {
                  if (fret > 0 && fret != bfret)
                        ++iter;
                  else
                        iter = _barres.erase(iter);
            }
            else
                  ++iter;
            }
      }   

//---------------------------------------------------------
//   removeMarker
//---------------------------------------------------------

void FretDiagram::removeMarker(int s)
      {
      auto it = _markers.find(s);
      _markers.erase(it);
      }

//---------------------------------------------------------
//   removeDot
//    take a fret value of 0 to mean remove all dots
//---------------------------------------------------------

void FretDiagram::removeDot(int s, int f /*= 0*/)
      {
      if (f > 0) {
            std::vector<FretItem::Dot> tempDots;
            for (auto const& d : dot(s)) {
                  if (d.fret != f)
                        tempDots.push_back(FretItem::Dot(d));
                  }

            _dots[s] = tempDots;
            }
      else
            _dots[s].clear();
      }

//---------------------------------------------------------
//   removeDotsMarkers
//    removes all markers between [ss, es] and dots between [ss, es],
//    where the dots have a fret of fret.
//---------------------------------------------------------

void FretDiagram::removeDotsMarkers(int ss, int es, int fret)
      {
      if (ss == -1)
            return;

      int end = es == -1 ? _strings : es;
      for (int string = ss; string <= end; ++string) {
            removeDot(string, fret);

            if (marker(string).exists())
                  removeMarker(string);
            }
      }

//---------------------------------------------------------
//   dot
//    take fret value of zero to mean all dots 
//---------------------------------------------------------

std::vector<FretItem::Dot> FretDiagram::dot(int s, int f /*= 0*/) const
      {
      if (_dots.find(s) != _dots.end()) {
            if (f != 0) {
                  for (auto const& d : _dots.at(s)) {
                        if (d.fret == f)
                              return std::vector<FretItem::Dot> {FretItem::Dot(d)};
                        }
                  }
            else
                  return _dots.at(s);
            }
      return std::vector<FretItem::Dot> {FretItem::Dot(0)};
      }

//---------------------------------------------------------
//   marker
//---------------------------------------------------------

FretItem::Marker FretDiagram::marker(int s) const
      {
      if (_markers.find(s) != _markers.end())
            return _markers.at(s);
      return FretItem::Marker(FretMarkerType::NONE);
      }

//---------------------------------------------------------
//   barre
//---------------------------------------------------------

FretItem::Barre FretDiagram::barre(int f) const
      {
      if (_barres.find(f) != _barres.end())
            return _barres.at(f);
      return FretItem::Barre(-1, -1);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void FretDiagram::add(Element* e)
      {
      e->setParent(this);
      if (e->isHarmony()) {
            _harmony = toHarmony(e);
            _harmony->setTrack(track());
            }
      else
            qWarning("FretDiagram: cannot add <%s>\n", e->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void FretDiagram::remove(Element* e)
      {
      if (e == _harmony)
            _harmony = 0;
      else
            qWarning("FretDiagram: cannot remove <%s>\n", e->name());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool FretDiagram::acceptDrop(EditData& data) const
      {
      return data.dropElement->type() == ElementType::HARMONY;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* FretDiagram::drop(EditData& data)
      {
      Element* e = data.dropElement;
      if (e->isHarmony()) {
            Harmony* h = toHarmony(e);
            h->setParent(parent());
            h->setTrack(track());
            score()->undoAddElement(h);
            }
      else {
            qWarning("FretDiagram: cannot drop <%s>\n", e->name());
            delete e;
            e = 0;
            }
      return e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void FretDiagram::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      Q_UNUSED(all);
      func(data, this);
      if (_harmony)
            func(data, _harmony);
      }

//---------------------------------------------------------
//   Write MusicXML
//---------------------------------------------------------

void FretDiagram::writeMusicXML(XmlWriter& xml) const
      {
#if 0       // NOTE:JT todo
      qDebug("FretDiagram::writeMusicXML() this %p harmony %p", this, _harmony);
      int strings_ = strings();
      xml.stag("frame");
      xml.tag("frame-strings", strings_);
      xml.tag("frame-frets", frets());
      QString strDots = "'";
      QString strMarker = "'";
      QString strFingering = "'";
      for (int i = 0; i < strings_; ++i) {
            // TODO print frame note
            if (_dots)
                  strDots += QString("%1'").arg(static_cast<int>(_dots[i]));
            if (_marker)
                  strMarker += QString("%1'").arg(static_cast<int>(_marker[i]));
            if (_fingering)
                  strFingering += QString("%1'").arg(static_cast<int>(_fingering[i]));
            if (_marker[i] != 88) {
                  xml.stag("frame-note");
                  xml.tag("string", strings_ - i);
                  if (_dots)
                        xml.tag("fret", _dots[i]);
                  else
                        xml.tag("fret", "0");
                  xml.etag();
                  }
            }
      qDebug("FretDiagram::writeMusicXML() this %p dots %s marker %s fingering %s",
             this, qPrintable(strDots), qPrintable(strMarker), qPrintable(strFingering));
      /*
      xml.tag("root-step", tpc2stepName(rootTpc));
      int alter = tpc2alter(rootTpc);
      if (alter)
            xml.tag("root-alter", alter);
      */
      xml.etag();
#endif
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant FretDiagram::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::MAG:
                  return userMag();
            case Pid::FRET_STRINGS:
                  return strings();
            case Pid::FRET_FRETS:
                  return frets();
            /*case Pid::FRET_BARRE:
                  return barre();*/
            case Pid::FRET_OFFSET:
                  return fretOffset();
            case Pid::FRET_NUM_POS:
                  return _numPos;
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool FretDiagram::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::MAG:
                  setUserMag(v.toDouble());
                  break;
            case Pid::FRET_STRINGS:
                  setStrings(v.toInt());
                  break;
            case Pid::FRET_FRETS:
                  setFrets(v.toInt());
                  break;
            /*case Pid::FRET_BARRE:
                  setBarre(v.toInt());
                  break;*/
            case Pid::FRET_OFFSET:
                  setFretOffset(v.toInt());
                  break;
            case Pid::FRET_NUM_POS:
                  _numPos = v.toInt();
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant FretDiagram::propertyDefault(Pid pid) const
      {
      for (const StyledProperty& p : *styledProperties()) {
            if (p.pid == pid) {
                  if (propertyType(pid) == P_TYPE::SP_REAL)
                        return score()->styleP(p.sid);
                  return score()->styleV(p.sid);
                  }
            }
      return Element::propertyDefault(pid);
      }


//---------------------------------------------------------
//   markerToChar
//---------------------------------------------------------

QChar FretItem::markerToChar(FretMarkerType t)
      {
      switch (t) {
            case FretMarkerType::CIRCLE:
                  return QChar('O');
            case FretMarkerType::CROSS:
                  return QChar('X');
            case FretMarkerType::NONE:
            default:
                  return QChar();
            }
      }

//---------------------------------------------------------
//   markerTypeToName
//---------------------------------------------------------

const std::vector<FretItem::MarkerTypeNameItem> FretItem::markerTypeNameMap = {
      { FretMarkerType::CIRCLE,     "circle"    },
      { FretMarkerType::CROSS,      "cross"     },
      { FretMarkerType::NONE,       "none"      }
      };

QString FretItem::markerTypeToName(FretMarkerType t)
      {
      for (auto i : FretItem::markerTypeNameMap) {
            if (i.mtype == t)
                  return i.name;
            }
      qFatal("Unrecognised FretMarkerType!");
      return QString();       // prevent compiler warnings
      }

//---------------------------------------------------------
//   nameToMarkerType
//---------------------------------------------------------

FretMarkerType FretItem::nameToMarkerType(QString n)
      {
      for (auto i : FretItem::markerTypeNameMap) {
            if (i.name == n)
                  return i.mtype;
            }
      qWarning("Unrecognised marker name!");
      return FretMarkerType::NONE;       // default
      }

//---------------------------------------------------------
//   dotTypeToName
//---------------------------------------------------------

const std::vector<FretItem::DotTypeNameItem> FretItem::dotTypeNameMap = {
      { FretDotType::NORMAL,        "normal"    }
      };

QString FretItem::dotTypeToName(FretDotType t)
      {
      for (auto i : FretItem::dotTypeNameMap) {
            if (i.dtype == t)
                  return i.name;
            }
      qFatal("Unrecognised FretDotType!");
      return QString();       // prevent compiler warnings
      }

//---------------------------------------------------------
//   nameToDotType
//---------------------------------------------------------

FretDotType FretItem::nameToDotType(QString n)
      {
      for (auto i : FretItem::dotTypeNameMap) {
            if (i.name == n)
                  return i.dtype;
            }
      qWarning("Unrecognised dot name!");
      return FretDotType::NORMAL;       // default
      }

}

