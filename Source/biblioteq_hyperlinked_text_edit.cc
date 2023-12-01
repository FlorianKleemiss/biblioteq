#include "biblioteq.h"
#include "biblioteq_hyperlinked_text_edit.h"

biblioteq_hyperlinked_text_edit::biblioteq_hyperlinked_text_edit(QWidget *parent) : QTextBrowser(parent)
{
  connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(slotAnchorClicked(QUrl)));
  qmain = nullptr;
}

void biblioteq_hyperlinked_text_edit::setMultipleLinks(const QString &searchType,
                                                       const QString &searchField,
                                                       const QString &str)
{
  if (!qmain)
    return;

  QString html("");
  QStringList tmplist;
  int i = 0;

  tmplist = str.trimmed().split("\n");

  for (i = 0; i < tmplist.size(); i++)
  {
    if (qmain->getDB().driverName() == "QSQLITE")
      html += tmplist[i].trimmed();
    else
      html += QString("<a href=\"%1?%2?%3\">" + tmplist[i] + "</a>").arg(searchType).arg(searchField).arg(tmplist[i].trimmed());

    if (i != tmplist.size() - 1)
      html += "<br>";
  }

  setText(html);
}

void biblioteq_hyperlinked_text_edit::setQMain(biblioteq *biblioteq)
{
  qmain = biblioteq;
}

void biblioteq_hyperlinked_text_edit::slotAnchorClicked(const QUrl &url)
{
  if (!qmain)
    return;

  QString searchKey("");
  QString searchType("");
  QString searchValue("");
  QStringList tmplist;
  auto path(url.toString());

  tmplist = path.split("?");

  if (tmplist.size() >= 3)
  {
    searchKey = tmplist[1];
    searchType = tmplist[0];
    searchValue = tmplist[2];

    if (searchType == "book_search")
      qmain->bookSearch(searchKey, searchValue);
  }

  tmplist.clear();
}
