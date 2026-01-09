#pragma once
#include "CollectionItem.h"
#include <albert/extensionplugin.h>
#include <albert/indexqueryhandler.h>
#include <qcontainerfwd.h>

using namespace albert;

class Plugin : public ExtensionPlugin, public IndexQueryHandler {
  ALBERT_PLUGIN
public:
  Plugin();
  ~Plugin() override;

  QString defaultTrigger() const override { return QStringLiteral("cn "); }

  void updateIndexItems() override;
  QWidget *buildConfigWidget() override;

  void updateCollectionList();
  const std::vector<CollectionItem> &collections() const;
  const QStringList &checked() const;

  static Plugin *instance();

  void addCollection(QString id);
  void removeCollection(QString id);

private:
  void debug(const QString &);
  void error(const QString &, QWidget *modal_parent = nullptr);

  std::vector<CollectionItem> collections_;
  QStringList checked_;
  static Plugin *instance_;

signals:
  void collectionsChanged();
};
