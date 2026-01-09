#include "plugin.h"
#include "configwidget.h"
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <albert/albert.h>
#include <albert/logging.h>
#include <albert/notification.h>
#include <albert/standarditem.h>

ALBERT_LOGGING_CATEGORY("contacts_kde")

using namespace Qt::StringLiterals;
using namespace albert;
using namespace std;

const char *CFG_COLLECTIONS = "collections";

Plugin *Plugin::instance_ = nullptr;

Plugin::Plugin() {
  instance_ = this;
  auto s = settings();

  checked_ = s->value(CFG_COLLECTIONS, QStringList()).toStringList();

  connect(this, &Plugin::collectionsChanged, this, &Plugin::updateIndexItems);

  updateCollectionList();
}

Plugin::~Plugin() {
  auto s = settings();
  s->setValue(CFG_COLLECTIONS, checked_);
}
Plugin *Plugin::instance() { return instance_; }

void Plugin::updateIndexItems() {

  vector<IndexItem> items;

  for (const auto &collection : collections_)
    if (collection.isChecked())
      collection.createIndexItems(items);

  INFO << u"Indexed %1 items"_s.arg(items.size());
  setIndexItems(::move(items));
}

QWidget *Plugin::buildConfigWidget() { return new ConfigWidget; }

const vector<CollectionItem> &Plugin::collections() const {
  return collections_;
}

const QStringList &Plugin::checked() const { return checked_; }

void Plugin::addCollection(QString id) {

  checked_ << id;
  emit collectionsChanged();
}

void Plugin::removeCollection(QString id) {
  if (checked_.removeOne(id)) {
    emit collectionsChanged();
  } else {
    WARN << "Unable to find collection " << id;
  }
}

void Plugin::updateCollectionList() {

  INFO << "Fetching contact collections from akonadi";

  // Create a fetch job to list all collections
  Akonadi::CollectionFetchJob *rootFetchJob = new Akonadi::CollectionFetchJob(
      Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);

  QObject::connect(
      rootFetchJob, &Akonadi::CollectionFetchJob::result, this,
      [this](KJob *job) {
        if (job->error()) {
          error(tr("Error fetching collections: %1").arg(job->errorString()));
        }

        collections_.clear();
        Akonadi::CollectionFetchJob *fetchJob =
            qobject_cast<Akonadi::CollectionFetchJob *>(job);
        const Akonadi::Collection::List collections = fetchJob->collections();
        for (const Akonadi::Collection &collection : collections) {
          if (collection.contentMimeTypes().contains(
                  QString::fromUtf8("text/directory"))) {
            collections_.emplace_back(collection.displayName(),
                                      collection.name(),
                                      QString::number(collection.id()));
            DEBG << tr("Got collection: %1").arg(collection.name());
          }
        }
        emit collectionsChanged();
      });
}

void Plugin::error(const QString &msg, QWidget *modal_parent) {
  WARN << msg;
  QMessageBox::warning(modal_parent, qApp->applicationDisplayName(), msg);
}
