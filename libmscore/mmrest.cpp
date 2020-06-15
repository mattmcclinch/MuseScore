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
#include "sym.h"
#include "undo.h"
#include "utils.h"

namespace Ms {

static const ElementStyle mmRestStyle {
    { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
};

const qreal SPACING = 0.75;

//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Score* s)
    : Rest(s)
{
    m_width = 0;
    m_symsWidth = 0;
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
    m_width = r.m_width;
    m_symsWidth = r.m_symsWidth;
}

//---------------------------------------------------------
//   MMRest::draw
//---------------------------------------------------------

void MMRest::draw(QPainter* painter) const
{
    if (shouldNotBeDrawn() || (track() % VOICES)) {     //only on voice 1
        return;
    }

    qreal _spatium = spatium();

    // draw number
    painter->setPen(curColor());
    std::vector<SymId>&& numberSym = toTimeSigString(QString("%1").arg(m_number));
    QRectF numberBox = symBbox(numberSym);
    qreal x = (m_width - numberBox.width()) * .5;
    qreal y = m_numberPos * spatium() - staff()->height() * .5;
    drawSymbols(numberSym, painter, QPointF(x, y));

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)
      && m_number <= score()->styleI(Sid::mmRestOldStyleMaxMeasures)) {
        x = (m_width - m_symsWidth) * 0.5;
        drawSymbols(m_restSyms, painter, QPointF(x, 0), 1.0, SPACING);
        if (m_isOddNumber) {
            // add the whole rest separately, because otherwise it would draw too low
            SymId sym = SymId::restWhole;
            x += m_symsWidth;
            x -= symBbox(sym).width();
            drawSymbol(sym, painter, QPointF(x, -_spatium));
        }
    } else {
        // draw horizontal line
        qreal hBarThickness = score()->styleP(Sid::mmRestHBarThickness);
        qreal halfHBarThickness = hBarThickness * .5;
        QPen pen(painter->pen());
        pen.setCapStyle(Qt::FlatCap);
        pen.setWidthF(hBarThickness);
        painter->setPen(pen);
        if (score()->styleB(Sid::mmRestNumberMaskHBar)     // avoid painting line through number
          && (y + (numberBox.height() * .5)) > -halfHBarThickness
          && (y - (numberBox.height() * .5)) < halfHBarThickness) {
            qreal gapDistance = (numberBox.width() + _spatium) * .5;
            qreal midpoint = m_width * .5;
            painter->drawLine(QLineF(0.0, 0.0, midpoint - gapDistance, 0.0));
            painter->drawLine(QLineF(midpoint + gapDistance, 0.0, m_width, 0.0));
        } else {
            painter->drawLine(QLineF(0.0, 0.0, m_width, 0.0));
        }

        // draw vertical lines
        qreal vStrokeThickness = score()->styleP(Sid::mmRestHBarVStrokeThickness);
        if (vStrokeThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(vStrokeThickness);
            painter->setPen(pen);
            qreal halfVStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight) * .5;
            painter->drawLine(QLineF(0.0, -halfVStrokeHeight, 0.0, halfVStrokeHeight));
            painter->drawLine(QLineF(m_width, -halfVStrokeHeight, m_width,  halfVStrokeHeight));
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MMRest::layout()
{
    m_number = measure()->mmRestCount();

    for (Element* e : el()) {
        e->layout();
    }

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)) {
        m_restSyms.clear();
        m_symsWidth = 0;

        int counter = m_number;
        SymId sym;

        while (counter >= 4) {
            sym = SymId::restLonga;
            m_restSyms.push_back(sym);
            counter -= 4;
            m_symsWidth += symBbox(sym).width();
        }
        while (counter >= 2) {
            sym = SymId::restDoubleWhole;
            m_restSyms.push_back(sym);
            counter -= 2;
            m_symsWidth += symBbox(sym).width();
        }
        if (counter == 1) {
            m_isOddNumber = true;
            // whole rest not added to m_restSyms, drawn separately due to need for vertical offset
            m_symsWidth += symBbox(SymId::restWhole).width();
            m_symsWidth += spatium() * SPACING;
        } else {
            m_isOddNumber = false;
        }
        m_symsWidth += spatium() * SPACING * (m_restSyms.size() - 1);
    }

    // set clickable area
    if (score()->styleB(Sid::oldStyleMultiMeasureRests)) {
        setbbox(QRectF((m_width - m_symsWidth) * .5, -1.0, m_symsWidth, 2.0));
    } else { // H-bar
        qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight);
        setbbox(QRectF(0.0, -(vStrokeHeight * .5), m_width, vStrokeHeight));
    }
    addbbox(numberRect());
}

//---------------------------------------------------------
//   numberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

QRectF MMRest::numberRect() const
{
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(m_number));

    QRectF r = symBbox(s);
    qreal x = (m_width - r.width()) * .5;
    qreal y = m_numberPos * spatium() - staff()->height() * .5;

    r.translate(QPointF(x, y));
    return r;
}

//---------------------------------------------------------
//   MMRest::write
//---------------------------------------------------------

void MMRest::write(XmlWriter& xml) const
{
    xml.stag("Rest"); // for compatibility, see also Measure::readVoice()
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
//   getProperty
//---------------------------------------------------------

QVariant MMRest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            return m_numberPos;
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
            m_numberPos = v.toDouble();
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
    qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight);
    shape.add(QRectF(0.0, -(vStrokeHeight * .5), m_width, vStrokeHeight));
    shape.add(numberRect());
    return shape;
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
}
