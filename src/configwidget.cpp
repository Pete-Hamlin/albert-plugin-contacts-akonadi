#include "configwidget.h"
#include "plugin.h"
#include <QAbstractListModel>
#include <QMessageBox>

using namespace std;
using namespace albert::util;

ConfigWidget::ConfigWidget() {
  ui.setupUi(this);

  ui.list_view->setModel(&model);
  connect(ui.update_button, &QPushButton::pressed, this,
          [] { Plugin::instance()->updateCollectionList(); });
}

CollectionsModel::CollectionsModel() {
  connect(Plugin::instance(), &Plugin::collectionsChanged, this, [this] {
    beginResetModel();
    endResetModel();
  });
}

int CollectionsModel::rowCount(const QModelIndex &) const {
  return Plugin::instance()->collections().size();
}

QVariant CollectionsModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};

  switch (const auto &cl = Plugin::instance()->collections().at(index.row());
          role) {
  case Qt::CheckStateRole:
    if (cl.isChecked())
      return Qt::Checked;
    else
      return Qt::Unchecked;

  case Qt::DecorationRole:
    return cl.name;

  case Qt::DisplayRole:
    return cl.name;

  case Qt::ToolTipRole:
    return cl.url;
  }

  return {};
}

bool CollectionsModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (index.isValid() && role == Qt::CheckStateRole) {
    const auto &cl = Plugin::instance()->collections().at(index.row());

    if (!cl.isChecked() && value == Qt::Checked)
      Plugin::instance()->addCollection(cl.id);

    else if (cl.isChecked() && value == Qt::Unchecked)
      Plugin::instance()->removeCollection(cl.id);
    {

      return true;
    }
  }
  return false;
}

Qt::ItemFlags CollectionsModel::flags(const QModelIndex &index) const {
  return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
}
