#include "biblioteq_z3950results.h"
#include "qevent.h"

biblioteq_z3950results::biblioteq_z3950results(QWidget *parent,
                                               QStringList &list,
											   const QFont &font,
											   const QString &recordSyntax) : QDialog(parent)
{
	int i = 0;
	int row = -1;
	m_recordSyntax = recordSyntax;
	setWindowModality(Qt::ApplicationModal);
	m_ui.setupUi(this);

	for (i = 0; i < list.size(); i++)
	{
		QString issn("");
		auto parsedList(list.at(i).split("\n"));

		for (int j = 0; j < parsedList.size(); j++)
			if (parsedList.at(j).startsWith("022 "))
			{
				/*
				** $a - International Standard Serial Number (NR)
				*/

				issn = parsedList.at(j);

				if (issn.indexOf("$a") > -1)
					issn = issn.mid(issn.indexOf("$a") + 2).trimmed();

				if (issn.contains(" "))
					issn = issn.mid(0, issn.indexOf(" ")).trimmed();

				if (issn.length() >= 9)
					issn = issn.mid(0, 9).trimmed();

				break;
			}

		if (!issn.isEmpty())
			m_ui.list->addItem(issn);
		else
			m_ui.list->addItem(QString(tr("Record #")) + QString::number(i + 1));

		m_records.append(list[i]);
	}

	list.clear();
    connect(m_ui.list, SIGNAL(currentRowChanged(int)), this, SLOT(slotUpdateQueryText()));
    connect(m_ui.okButton, SIGNAL(clicked()), this,	SLOT(slotSelectRecord()));
    connect(m_ui.cancelButton, SIGNAL(clicked()), this,	SLOT(slotClose()));

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

biblioteq_z3950results::~biblioteq_z3950results()
{
	m_records.clear();
}

void biblioteq_z3950results::changeEvent(QEvent *event)
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

void biblioteq_z3950results::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	slotClose();
}

void biblioteq_z3950results::keyPressEvent(QKeyEvent *event)
{
	if (event && event->key() == Qt::Key_Escape)
		close();

	QDialog::keyPressEvent(event);
}

void biblioteq_z3950results::slotClose(void)
{
	deleteLater();
}

void biblioteq_z3950results::setGlobalFonts(const QFont &font)
{
	setFont(font);

	foreach (auto widget, findChildren<QWidget *>())
	{
		widget->setFont(font);
		widget->update();
	}

	update();
}

void biblioteq_z3950results::slotSelectRecord(void)
{
	QStringList list;

	list = m_ui.textarea->toPlainText().split("\n");

	list.clear();
	close();
}

void biblioteq_z3950results::slotUpdateQueryText(void)
{
	QString title("");
	auto list(m_records.value(m_ui.list->currentRow()).split("\n"));

	for (int i = 0; i < list.size(); i++)
		if (list.at(i).startsWith("245 "))
		{
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

			title = list.at(i);

			if (title.indexOf("$a") > -1)
				title = title.mid(title.indexOf("$a") + 2).trimmed();

			title = title.remove(" $b").trimmed();

			QStringList subfields;

			subfields << "$c"
					  << "$f"
					  << "$g"
					  << "$h"
					  << "$k"
					  << "$n"
					  << "$p"
					  << "$s"
					  << "$6"
					  << "$8";

			for (int i = 0; i < subfields.size(); i++)
				if (title.contains(subfields.at(i)))
				{
					title = title.mid(0, title.indexOf(subfields.at(i))).trimmed();
					break;
				}

			if (title.lastIndexOf('/') > -1)
				title = title.mid(0, title.lastIndexOf('/')).trimmed();

			break;
		}

	m_ui.title->setText(title);
	m_ui.title->setCursorPosition(0);
	m_ui.textarea->setPlainText(m_records.value(m_ui.list->currentRow()));
}
