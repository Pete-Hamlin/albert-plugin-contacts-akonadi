#include "CollectionItem.h"
#include "plugin.h"
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <KContacts/Addressee>
#include <QString>
#include <albert/albert.h>
#include <albert/iconutil.h>
#include <albert/logging.h>
#include <albert/standarditem.h>
#include <albert/systemutil.h>
#include <qeventloop.h>

using namespace Qt::StringLiterals;
using namespace std;
using namespace albert::util;
using namespace albert;

CollectionItem::CollectionItem(QString n, QString ur, QString id)
    : name(n), url(ur), id(id) {}

bool CollectionItem::isChecked() const {
  return Plugin::instance()->checked().contains(this->id);
}

void CollectionItem::createIndexItems(vector<IndexItem> &results) const {

  INFO << "Indexing items for collection: " << this->name;
  auto col_id = this->id.toInt();

  Akonadi::CollectionFetchJob *collectionJob = new Akonadi::CollectionFetchJob(
      Akonadi::Collection(col_id), Akonadi::CollectionFetchJob::Base, nullptr);

  QEventLoop loop;
  QObject::connect(
      collectionJob, &Akonadi::CollectionFetchJob::result,
      [=, &loop, &results](KJob *job) {
        auto fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
        if (fetchJob->collections().isEmpty()) {
          WARN << "Unable to find collection: " << col_id;
          return;
        }
        Akonadi::ItemFetchJob *itemFetchJob =
            new Akonadi::ItemFetchJob(fetchJob->collections().first());
        itemFetchJob->fetchScope().fetchFullPayload();

        QObject::connect(
            itemFetchJob, &Akonadi::ItemFetchJob::result, Plugin::instance(),
            [=, &loop, &results](KJob *job) {
              if (job->error()) {
                WARN << "Error fetching items:" << job->errorString();
                return;
              }
              Akonadi::ItemFetchJob *fetchJob =
                  qobject_cast<Akonadi::ItemFetchJob *>(job);
              const Akonadi::Item::List items = fetchJob->items();
              for (const Akonadi::Item &item : items) {
                if (item.hasPayload<KContacts::Addressee>()) {
                  KContacts::Addressee contact =
                      item.payload<KContacts::Addressee>();

                  auto contact_name = contact.formattedName();

                  for (auto phoneNumber : contact.phoneNumbers()) {
                    auto number = phoneNumber.number();
                    QString id = u"phone-%1"_s % number;

                    auto phone_item = StandardItem::make(
                        id, contact_name, number, makeThemeIcon{u"phone"_s},
                        std::vector<Action>{
                            {u"copy"_s, u"Copy"_s,
                             [number]() { setClipboardText(number); }},
                            {u"call"_s, u"Call"_s,
                             [number]() { openUrl(u"tel:%1"_s % number); }},
                            {u"message"_s, u"Message"_s,
                             [number]() { openUrl(u"sms:%1"_s % number); }},
                        });

                    results.emplace_back(phone_item, phone_item->text());
                  }

                  for (auto email : contact.emails()) {

                    QString id = u"email-%1"_s % email;
                    auto email_item = StandardItem::make(
                        id, contact_name, email,
                        makeThemeIcon{u"mail-client"_s},
                        {
                            {u"copy"_s, u"Copy"_s,
                             [email]() { setClipboardText(email); }},
                            {u"mail"_s, u"Compose"_s,
                             [email]() { openUrl(u"mailto:%1"_s % email); }},
                        });

                    results.emplace_back(email_item, email_item->text());
                  }
                }
              }
              loop.quit();
            });
      });
  collectionJob->start();
  loop.exec();
}
