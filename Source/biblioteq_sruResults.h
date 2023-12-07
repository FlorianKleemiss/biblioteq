#ifndef _BIBLIOTEQ_SRURESULTS_H_
#define _BIBLIOTEQ_SRURESULTS_H_

#include "ui_biblioteq_sruResults.h"

class biblioteq_sruresults : public QDialog
{
  Q_OBJECT

public:
  biblioteq_sruresults(QWidget *parent,
                       const QList<QByteArray> &list,
                       const QFont &font);
  ~biblioteq_sruresults();

private:
  QList<QByteArray> m_records;
  Ui_sruResultsDialog m_ui;
  void changeEvent(QEvent *event);
  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void setGlobalFonts(const QFont &font);

private slots:
  void slotClose(void);
  void slotSelectRecord(void);
  void slotUpdateQueryText(void);
};

#endif
