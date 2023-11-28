#include "biblioteq_photograph_compare.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QResizeEvent>
#include <QScrollBar>

biblioteq_photograph_compare::biblioteq_photograph_compare(QWidget *parent) : QGraphicsView(parent)
{
  m_bestFit = true; // Agrees with default UI setting.
  m_image1 = QImage();
  m_oid1 = -1;
  m_image2 = QImage();
  m_oid2 = -1;
}

void biblioteq_photograph_compare::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);

  if (event && m_bestFit)
  {
    QGraphicsPixmapItem *item = nullptr;

    if (scene() && !scene()->items().isEmpty())
      item = qgraphicsitem_cast<QGraphicsPixmapItem *>(scene()->items().at(0));

    if (item)
    {
      auto image(m_image1);

      if (!image.isNull())
        image = image.scaled(event->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

      item->setPixmap(QPixmap::fromImage(image));
      scene()->setSceneRect(scene()->itemsBoundingRect());
      horizontalScrollBar()->setValue(0);
      verticalScrollBar()->setValue(0);
    }
  }
}

void biblioteq_photograph_compare::setBestFit(const bool bestFit)
{
  m_bestFit = bestFit;
}

void biblioteq_photograph_compare::setImage1(const QImage &image,
                                             const QString &format,
                                             const qint64 oid)
{
  m_format1 = format;
  m_image1 = image;
  m_oid1 = oid;
}

void biblioteq_photograph_compare::setImage2(const QImage &image,
                                             const QString &format,
                                             const qint64 oid)
{
  m_format2 = format;
  m_image2 = image;
  m_oid2 = oid;
}
