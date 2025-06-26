cmake_minimum_required(VERSION 3.16)
project(contacts_kde VERSION 0.2)

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")
find_package(ECM ${KF_MIN_VERSION} REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
# include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KPim6Akonadi REQUIRED)
find_package(KF6Contacts REQUIRED)
find_package(Albert REQUIRED)

albert_plugin(
	QT Widgets
	LINK PRIVATE
		KPim6::AkonadiCore
		KPim6::AkonadiWidgets
		KF6::Contacts
)
