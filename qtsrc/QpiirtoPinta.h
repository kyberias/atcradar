#ifndef QPIIRTOPINTA_H
#define QPIIRTOPINTA_H

#include "piirtopinta.hpp"

class QPainterPiirtoPinta : public IPiirtoPinta {
    QPainter* m_painter;
public:
    void SetPainter(QPainter* painter) {
        m_painter = painter;
    }

    void rectangleColor(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned int color) {
        m_painter->drawRect(x1, y1, x2-x1, y2-y1);
    }

    void lineColor(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned int color) {
        m_painter->drawLine(x1, y1, x2, y2);
    }

    void circleColor(unsigned short x, unsigned short y, unsigned short rad, unsigned int color) {
        m_painter->drawEllipse(QPointF(x, y), rad, rad);
    }

    void trigonColor(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned short x3, unsigned short y3, unsigned int color) {
        QPoint points[3] = {QPoint(x1, y1), QPoint(x2, y2), QPoint(x3, y3)};
        m_painter->drawPolygon(points, 3);
    }

    void piirra_kuva(const char* tiedosto, int x, int y, bool keskikohta = false) { }

    void kirjoita_tekstia(std::string teksti, int x, int y, bool aktiivinen = false) {
        m_painter->drawText(x, y, teksti.c_str());
    }

    int get_fontinkoko() { return 16; }

    void flip() { }
};


#endif // QPIIRTOPINTA_H