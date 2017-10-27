#include "SessionModel.h"
#include <assert.h>
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
	// Disconnect the session signals:
	if (m_Session != nullptr)
	{
		disconnect(m_Session, &Session::addedDevice,    this, &SessionModel::addDevice);
		disconnect(m_Session, &Session::removingDevice, this, &SessionModel::removeDevice);
	}

	// Clear the previous session's data:
	m_RootOnline->setRowCount(0);
	m_RootOffline->setRowCount(0);
	m_RootBackups->setRowCount(0);

	// Connect the session's signals:
	m_Session = a_Session;
	if (a_Session == nullptr)
	{
		return;
	}
	connect(m_Session, &Session::addedDevice,    this, &SessionModel::addDevice);
	connect(m_Session, &Session::removingDevice, this, &SessionModel::removeDevice);

	// Add the new session's data:
	for (const auto & dev: a_Session->getDevices())
	{
		addDevice(dev.get());
	}
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





void SessionModel::addDevice(Device * a_Device)
{
	assert(a_Device != nullptr);

	// TODO: Check if the device is already added (async)

	// Create the item for the device:
	auto root = getRootForDevice(*a_Device);
	if (root == nullptr)
	{
		assert(!"Adding a device without a known root");
		return;
	}
	auto item = new QStandardItem(a_Device->displayName());
	item->setData(QVariant(reinterpret_cast<qulonglong>(a_Device)), roleDevice);
	root->appendRow(item);

	// Add sub-items for each contact book currently present in the device:
	for (const auto & cbook: a_Device->contactBooks())
	{
		auto itemCB = new QStandardItem(cbook->displayName());
		itemCB->setData(QVariant(reinterpret_cast<qulonglong>(a_Device)),   roleDevice);
		itemCB->setData(QVariant(reinterpret_cast<qulonglong>(cbook.get())), roleContactBook);
		item->appendRow(itemCB);
	}

	emit deviceItemCreated(item->index());
}





void SessionModel::removeDevice(Device * a_Device)
{
	// TODO
}





