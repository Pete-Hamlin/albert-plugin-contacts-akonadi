
#pragma once
#include <QString>
#include <albert/indexitem.h>
#include <vector>

class CollectionItem {
public:
  CollectionItem(QString name, QString url, QString id);

  void createIndexItems(std::vector<albert::IndexItem> &results) const;

  bool isChecked() const;

  const QString name;
  const QString url;
  const QString id;
};
