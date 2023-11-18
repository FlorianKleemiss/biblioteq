#ifndef _BIBLIOTEQ_PHOTOGRAPH_COMPARE_H_
#define _BIBLIOTEQ_PHOTOGRAPH_COMPARE_H_

#include <QGraphicsView>

class biblioteq_photograph_compare: public QGraphicsView
{
  Q_OBJECT

 public:
  biblioteq_photograph_compare(QWidget *parent);
  void setBestFit(const bool bestFit);
  void setImage1(const QImage &image, const QString &format, const qint64 oid);
  void setImage2(const QImage &image, const QString &format, const qint64 oid);

 private:
  QImage m_image1;
  QString m_format1;
  qint64 m_oid1;
  QImage m_image2;
  QString m_format2;
  qint64 m_oid2;
  bool m_bestFit;
  void resizeEvent(QResizeEvent *event);

};

#endif
