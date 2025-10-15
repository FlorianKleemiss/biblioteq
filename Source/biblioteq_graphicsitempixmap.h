#ifndef _BIBLIOTEQ_GRAPHICSITEMPIXMAP_H_
#define _BIBLIOTEQ_GRAPHICSITEMPIXMAP_H_

#include <qgraphicsitem.h>
#include <qpainter.h>
#include <qstyleoption.h>
static void qt_graphicsItem_highlightSelected(QGraphicsItem *item,
                                              QPainter *painter,
                                              const QStyleOptionGraphicsItem *option)
{
  if (!item || !option || !painter)
    return;

  const auto &murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));

  if (qFuzzyIsNull(qMax(murect.width(), murect.height())))
    return;

  const auto &mbrect = painter->transform().mapRect(item->boundingRect());

  if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
    return;

  const QColor bgcolor(70, 130, 180);
  const qreal pad = 0.0;
  const qreal penWidth = 2.5;

  painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
  painter->drawRect(item->boundingRect().adjusted(pad, pad, -pad, -pad));
}

class biblioteq_graphicsitempixmap : public QGraphicsPixmapItem
{
public:
  biblioteq_graphicsitempixmap(const QPixmap &pixmap, QGraphicsItem *parent) : QGraphicsPixmapItem(pixmap, parent)
  {
    m_hasOriginalImage = false;
  }

  ~biblioteq_graphicsitempixmap()
  {
  }

  void setHasOriginalImage(bool hasOriginal)
  {
    m_hasOriginalImage = hasOriginal;
  }

  bool hasOriginalImage() const
  {
    return m_hasOriginalImage;
  }

  void paint(QPainter *painter,
             const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr)
  {
    Q_UNUSED(widget);

    if (!option || !painter)
      return;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    auto exposed_rect(option->exposedRect.adjusted(-1, -1, 1, 1));

    exposed_rect &= QRectF(offset().x(),
                           offset().y(),
                           pixmap().width(),
                           pixmap().height());
    painter->drawPixmap(exposed_rect, pixmap(), exposed_rect.translated(-offset()));

    if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus))
      qt_graphicsItem_highlightSelected(this, painter, option);

    // Draw green tickmark if original image exists
    if (m_hasOriginalImage)
    {
      QRectF rect = boundingRect();
      qreal size = qMin(rect.width(), rect.height()) * 0.25; // 25% of smaller dimension
      QRectF tickRect(rect.right() - size - 2, rect.bottom() - size - 2, size, size);

      // Draw background circle
      painter->setBrush(QColor(34, 139, 34)); // Forest green
      painter->setPen(QPen(QColor(255, 255, 255), 1.5)); // White border
      painter->drawEllipse(tickRect);

      // Draw checkmark
      painter->setPen(QPen(QColor(255, 255, 255), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      
      QPointF center = tickRect.center();
      qreal checkSize = size * 0.5;
      
      // Draw checkmark path
      QPointF p1(center.x() - checkSize * 0.3, center.y());
      QPointF p2(center.x() - checkSize * 0.05, center.y() + checkSize * 0.3);
      QPointF p3(center.x() + checkSize * 0.35, center.y() - checkSize * 0.35);
      
      painter->drawLine(p1, p2);
      painter->drawLine(p2, p3);
    }
  }

private:
  bool m_hasOriginalImage;
};

#endif
