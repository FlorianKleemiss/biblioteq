#include "biblioteq_sruResults.h"
#include "qevent.h"

#include <QXmlStreamReader>

biblioteq_sruresults::biblioteq_sruresults(QWidget *parent,
                                           const QList<QByteArray> &list,
                                           biblioteq_magazine *magazine_arg,
                                           const QFont &font) : QDialog(parent)
{
  int row = -1;

  m_records = list;
  m_magazine = magazine_arg;
  setWindowModality(Qt::ApplicationModal);
  m_ui.setupUi(this);

  for (int i = 0; i < m_records.size(); i++)
  {
    QString issn("");
    QXmlStreamReader reader(m_records.at(i));

    while (!reader.atEnd())
      if (reader.readNextStartElement())
      {
        if (reader.name().toString().toLower().trimmed() == "datafield")
        {
          auto tag(reader.attributes().value("tag").toString().trimmed());

          if (tag == "022")
          {
            /*
            ** $a - International Standard Serial Number (NR)
            */

            while (reader.readNextStartElement())
              if (reader.name().toString().toLower().trimmed() == "subfield")
              {
                if (reader.attributes().value("code").toString().trimmed() == "a")
                {
                  issn.append(reader.readElementText());
                  break;
                }
                else
                  reader.skipCurrentElement();
              }
              else
                break;
          }
        }
      }

    if (!issn.isEmpty())
      m_ui.list->addItem(issn);
    else
      m_ui.list->addItem(QString(tr("Record #")) + QString::number(i + 1));
  }

  connect(m_ui.list, SIGNAL(currentRowChanged(int)), this, SLOT(slotUpdateQueryText()));
  connect(m_ui.okButton, SIGNAL(clicked()), this, SLOT(slotSelectRecord()));
  connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(slotClose()));

  if (row == -1)
    row = 0;

  m_ui.list->setCurrentRow(row);
  m_ui.splitter->setStretchFactor(0, 0);
  m_ui.splitter->setStretchFactor(1, 1);
  setGlobalFonts(font);

  if (parent)
    resize(qRound(0.85 * parent->size().width()),
           qRound(0.85 * parent->size().height()));

  exec();
  QApplication::processEvents();
}

biblioteq_sruresults::~biblioteq_sruresults()
{
  m_records.clear();
}

void biblioteq_sruresults::changeEvent(QEvent *event)
{
  if (event)
    switch (event->type())
    {
    case QEvent::LanguageChange:
    {
      m_ui.retranslateUi(this);
      break;
    }
    default:
      break;
    }

  QDialog::changeEvent(event);
}

void biblioteq_sruresults::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);
  slotClose();
}

void biblioteq_sruresults::keyPressEvent(QKeyEvent *event)
{
  if (event && event->key() == Qt::Key_Escape)
    close();

  QDialog::keyPressEvent(event);
}

void biblioteq_sruresults::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach (auto widget, findChildren<QWidget *>())
  {
    widget->setFont(font);
    widget->update();
  }

  update();
}

void biblioteq_sruresults::slotClose(void)
{
  deleteLater();
}

void biblioteq_sruresults::slotSelectRecord(void)
{
  close();
}

void biblioteq_sruresults::slotUpdateQueryText(void)
{
  QString title("");
  QXmlStreamReader reader(m_records.value(m_ui.list->currentRow()));

  /*
  ** $a - Title (NR)
  ** $b - Remainder of title (NR)
  ** $c - Statement of responsibility, etc. (NR)
  ** $f - Inclusive dates (NR)
  ** $g - Bulk dates (NR)
  ** $h - Medium (NR)
  ** $k - Form (R)
  ** $n - Number of part/section of a work (R)
  ** $p - Name of part/section of a work (R)
  ** $s - Version (NR)
  ** $6 - Linkage (NR)
  ** $8 - Field link and sequence number (R)
  */

  while (!reader.atEnd())
    if (reader.readNextStartElement())
      if (reader.name().toString().toLower().trimmed() == "datafield")
      {
        auto tag(reader.attributes().value("tag").toString().trimmed());

        if (tag == "245")
        {
          while (reader.readNextStartElement())
            if (reader.name().toString().toLower().trimmed() == "subfield")
            {
              if (reader.attributes().value("code").toString().trimmed() == "a" ||
                  reader.attributes().value("code").toString().trimmed() == "b")
                title.append(reader.readElementText());
              else
                reader.skipCurrentElement();
            }
            else
              break;
        }
      }

  title = title.mid(0, title.lastIndexOf('/')).trimmed();
  m_ui.title->setText(title);
  m_ui.title->setCursorPosition(0);
  m_ui.textarea->setPlainText(m_records.value(m_ui.list->currentRow()));
}
