//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mmrest.h"
#include "measure.h"
#include "score.h"
#include "undo.h"
#include "utils.h"

namespace Ms {
//---------------------------------------------------------
//   mmRestStyle
//---------------------------------------------------------

static const ElementStyle mmRestStyle {
    { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
};

//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Score* s)
    : Rest(s)
{
    _mmWidth = 0;
    if (score()) {
        initElementStyle(&mmRestStyle);
    }
}

MMRest::MMRest(const MMRest& r, bool link)
    : Rest(r, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<MMRest*>(&r)));
        setAutoplace(true);
    }
    _mmWidth = r._mmWidth;
}

//---------------------------------------------------------
//   MMRest::draw
//---------------------------------------------------------

void MMRest::draw(QPainter* painter) const
{
    if (shouldNotBeDrawn() || (track() % VOICES)) { //only on voice 1
        return;
    }

    qreal _spatium = spatium();

    // draw number
    painter->setPen(curColor());
    int n = measure()->mmRestCount();
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));
    QRectF numberBox = symBbox(s);
    qreal x = (_mmWidth - numberBox.width()) * .5;
    qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
    drawSymbols(s, painter, QPointF(x, y));

    // draw horizontal line
    qreal hBarThickness = score()->styleP(Sid::mmRestHBarThickness);
    qreal halfHBarThickness = hBarThickness * .5;
    QPen pen(painter->pen());
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(hBarThickness);
    painter->setPen(pen);
    if (score()->styleB(Sid::mmRestNumberMaskHBar) // avoid painting line through number
      && (y + (numberBox.height() * .5 )) > -halfHBarThickness
      && (y - (numberBox.height() * .5 )) < halfHBarThickness) {
        qreal gapDistance = (numberBox.width() + _spatium) * .5;
        qreal midpoint = _mmWidth * .5;
        painter->drawLine(QLineF(0.0, 0.0, midpoint - gapDistance, 0.0));
        painter->drawLine(QLineF(midpoint + gapDistance, 0.0, _mmWidth, 0.0));
    } else {
        painter->drawLine(QLineF(0.0, 0.0, _mmWidth, 0.0));
    }

    // draw vertical lines
    qreal vStrokeThickness = score()->styleP(Sid::mmRestHBarVStrokeThickness);
    if (vStrokeThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
        pen.setWidthF(vStrokeThickness);
        painter->setPen(pen);
        qreal halfVStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight) * .5;
        painter->drawLine(QLineF(0.0, -halfVStrokeHeight, 0.0, halfVStrokeHeight));
        painter->drawLine(QLineF(_mmWidth, -halfVStrokeHeight, _mmWidth,  halfVStrokeHeight));
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MMRest::layout()
{
    for (Element* e : el()) {
        e->layout();
    }
    _mmWidth = score()->styleP(Sid::minMMRestWidth) * mag();
    // setbbox(QRectF(0.0, -_spatium, _mmWidth, 2.0 * _spatium));
    return;
}

void MMRest::layout(qreal val)
{
    _mmWidth = val;
    qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight) * .5;
    bbox().setRect(0.0, -(vStrokeHeight * .5), _mmWidth, vStrokeHeight);
    addbbox(mmRestNumberRect());
}

//---------------------------------------------------------
//   mmRestNumberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

QRectF MMRest::mmRestNumberRect() const
{
    int n = measure()->mmRestCount();
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));

    QRectF r = symBbox(s);
    qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
    qreal x = (_mmWidth - r.width()) * .5;

    r.translate(QPointF(x, y));
    return r;
}

//---------------------------------------------------------
//   MMRest::write
//---------------------------------------------------------

void MMRest::write(XmlWriter& xml) const
{
    xml.stag("Rest");
    ChordRest::writeProperties(xml);
    el().write(xml);
    xml.etag();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MMRest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            return score()->styleV(Sid::mmRestNumberPos);
        default:
            return Rest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void MMRest::resetProperty(Pid propertyId)
{
    setProperty(propertyId, propertyDefault(propertyId));
    return;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid MMRest::getPropertyStyle(Pid propertyId) const
{
    if (propertyId == Pid::MMREST_NUMBER_POS) {
        return Sid::mmRestNumberPos;
    }
    return Rest::getPropertyStyle(propertyId);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant MMRest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            return _mmRestNumberPos;
        default:
            return Rest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MMRest::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            _mmRestNumberPos = v.toDouble();
            triggerLayout();
            break;
        default:
            return Rest::setProperty(propertyId, v);
    }
    return true;
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape MMRest::shape() const
{
    Shape shape;
    shape.add(Rest::shape());

    qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight) * .5;
    shape.add(QRectF(0.0, -(vStrokeHeight * .5), _mmWidth, vStrokeHeight));
    shape.add(mmRestNumberRect());
    return shape;
}
}
