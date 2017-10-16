#include "SessionModel.h"
#include "Session.h"
#include "Device.h"
#include "ContactBook.h"





/** The QStandardItem data roles used for storing our pointers. */
enum
{
	roleDevice = Qt::UserRole + 2,
	roleContactBook,
};





SessionModel::SessionModel(Session * a_Session):
	m_Session(nullptr),
	m_RootOnline(new QStandardItem(tr("Online"))),
	m_RootOffline(new QStandardItem(tr("Offline"))),
	m_RootBackups(new QStandardItem(tr("Backups")))
{
	appendRow(m_RootOnline);
	appendRow(m_RootOffline);
	appendRow(m_RootBackups);
	setSession(a_Session);
}





void SessionModel::setSession(Session * a_Session)
{
	// TODO: Disconnect the session signals

	// Clear the previous session's data:
	m_RootOnline->setRowCount(0);
	m_RootOffline->setRowCount(0);
	m_RootBackups->setRowCount(0);

	// Add the new session's data:
	m_Session = a_Session;
	if (a_Session == nullptr)
	{
		return;
	}
	for (const auto & dev: a_Session->getDevices())
	{
		auto root = getRootForDevice(*dev);
		if (root == nullptr)
		{
			continue;
		}
		auto item = new QStandardItem(dev->displayName());
		item->setData(QVariant(reinterpret_cast<qulonglong>(dev.get())), roleDevice);
		root->appendRow(item);
		for (const auto & cbook: dev->contactBooks())
		{
			auto itemCB = new QStandardItem(cbook->displayName());
			itemCB->setData(QVariant(reinterpret_cast<qulonglong>(dev.get())),   roleDevice);
			itemCB->setData(QVariant(reinterpret_cast<qulonglong>(cbook.get())), roleContactBook);
			item->appendRow(itemCB);
		}
	}

	// TODO: Connect the session's signals
}





ContactBookPtr SessionModel::getContactBook(const QModelIndex & a_Index) const
{
	auto item = itemFromIndex(a_Index);
	if (item == nullptr)
	{
		return nullptr;
	}
	auto dev = reinterpret_cast<Device *>(item->data(roleDevice).toULongLong());
	if (dev == nullptr)
	{
		return nullptr;
	}
	auto cb = reinterpret_cast<const ContactBook *>(item->data(roleContactBook).toULongLong());
	return dev->getSharedContactBook(cb);
}





QStandardItem * SessionModel::getRootForDevice(const Device & a_Device)
{
	if (a_Device.isBackup())
	{
		return m_RootBackups;
	}
	if (a_Device.isOnline())
	{
		return m_RootOnline;
	}
	else
	{
		return m_RootOffline;
	}
}





