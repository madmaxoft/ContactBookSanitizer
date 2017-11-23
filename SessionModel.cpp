#include "SessionModel.h"
#include <assert.h>
#include <functional>
#include <QDebug>
#include "Session.h"
#include "Device.h"
#include "ContactBook.h"





/** The QStandardItem data roles used for storing our pointers and types. */
enum
{
	roleRole = Qt::UserRole + 2,  // The role for storing the item "type" (device, contactbook etc.) as a role.
	roleDevice,
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





Device * SessionModel::deviceFromIndex(const QModelIndex & a_Index) const
{
	if (!a_Index.isValid())
	{
		return nullptr;
	}
	return reinterpret_cast<Device *>(data(a_Index, roleDevice).toULongLong());
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





QStandardItem * SessionModel::findDeviceItem(const Device * a_Device)
{
	// Recursive lambda for searching subitems:
	std::function<QStandardItem *(QStandardItem *)> find;
	find = [a_Device,&find](QStandardItem * a_Item) -> QStandardItem *
	{
		for (int r = a_Item->rowCount() - 1; r >= 0; --r)
		{
			auto ch = a_Item->child(r);
			if (ch == nullptr)
			{
				assert(!"Unexpected nullptr child item!");
				continue;
			}
			if (ch->data(roleRole) == roleDevice)
			{
				if (reinterpret_cast<const Device *>(ch->data(roleDevice).toULongLong()) == a_Device)
				{
					return ch;
				}
			}
			auto sub = find(ch);
			if (sub != nullptr)
			{
				return sub;
			}
		}  // for r - a_Item.rows[]
		return nullptr;
	};

	// Search all root items:
	for (int r = rowCount() - 1; r >= 0; --r)
	{
		auto sub = find(item(r));
		if (sub != nullptr)
		{
			return sub;
		}
	}

	// Nothing found:
	return nullptr;
}





void SessionModel::addContactBook(QStandardItem & a_DeviceItem, const ContactBook & a_ContactBook)
{
	// Check if the CB is already added (async):
	if (findContactBookItemInDevice(a_DeviceItem, &a_ContactBook) != nullptr)
	{
		qDebug() << "ContactBook " << a_ContactBook.displayName() << " is already present, skipping.";
		return;
	}

	// Add the item:
	auto itemCB = new QStandardItem(a_ContactBook.displayName());
	itemCB->setData(a_DeviceItem.data(roleDevice),                          roleDevice);
	itemCB->setData(QVariant(reinterpret_cast<qulonglong>(&a_ContactBook)), roleContactBook);
	itemCB->setData(QVariant(roleContactBook),                              roleRole);
	a_DeviceItem.appendRow(itemCB);

	// TODO: Emit a signal?
}





QStandardItem * SessionModel::findContactBookItemInDevice(
	const QStandardItem & a_DeviceItem,
	const ContactBook * a_ContactBook
)
{
	auto cbULL = reinterpret_cast<qulonglong>(a_ContactBook);
	int numChildren = a_DeviceItem.rowCount();
	for (int i = 0; i < numChildren; ++i)
	{
		auto cbItem = a_DeviceItem.child(i, 0);
		if (
			(cbItem != nullptr) &&
			(cbItem->data(roleRole).toInt() == roleContactBook) &&
			(cbItem->data(roleContactBook).toULongLong() == cbULL)
		)
		{
			return cbItem;
		}
	}
	return nullptr;
}





void SessionModel::addDevice(Device * a_Device)
{
	assert(a_Device != nullptr);

	// Check if the device is already added (async):
	auto devItem = findDeviceItem(a_Device);
	if (devItem != nullptr)
	{
		qDebug() << "Device " << a_Device->displayName() << " is already added into the model, skipping.";
		return;
	}

	// Create the item for the device:
	auto root = getRootForDevice(*a_Device);
	if (root == nullptr)
	{
		assert(!"Adding a device without a known root");
		return;
	}
	devItem = new QStandardItem(a_Device->displayName());
	devItem->setData(QVariant(reinterpret_cast<qulonglong>(a_Device)), roleDevice);
	devItem->setData(QVariant(roleDevice), roleRole);
	root->appendRow(devItem);

	// Connect the Device's signals:
	connect(a_Device, &Device::addContactBook, this, &SessionModel::addDeviceContactBook);
	connect(a_Device, &Device::delContactBook, this, &SessionModel::delDeviceContactBook);
	connect(a_Device, &Device::online,         this, &SessionModel::deviceOnline);

	// Add sub-items for each contact book currently present in the device:
	for (const auto & cbook: a_Device->contactBooks())
	{
		addContactBook(*devItem, *cbook);
	}

	emit deviceItemCreated(a_Device, devItem->index());
}





void SessionModel::removeDevice(const Device * a_Device)
{
	auto devItem = findDeviceItem(a_Device);
	if (devItem == nullptr)
	{
		qWarning() << "Trying to remove device " << a_Device->displayName() << " that has no item attached to it. Ignoring.";
		return;
	}
	devItem->parent()->removeRow(devItem->row());
}





void SessionModel::addDeviceContactBook(Device * a_Device, ContactBookPtr a_ContactBook)
{
	// Check that the device is valid and has an item:
	if (a_Device == nullptr)
	{
		qDebug() << "Attempting to add a contact book not from a device, ignoring.";
		assert(!"No device specified");
		return;
	}
	auto devItem = findDeviceItem(a_Device);
	if (devItem == nullptr)
	{
		qDebug() << "Attempting to add a contact book for a device that is not in the list, ignoring.";
		assert(!"Device not in list");
		return;
	}

	// If the ContactBook is already added (async), bail out:
	if (findContactBookItemInDevice(*devItem, a_ContactBook.get()) != nullptr)
	{
		qDebug() << "Attempting to add a contact book already present to the model, ignoring.";
		return;
	}

	// Add the item:
	auto itemCB = new QStandardItem(a_ContactBook->displayName());
	itemCB->setData(QVariant(reinterpret_cast<qulonglong>(a_Device)),            roleDevice);
	itemCB->setData(QVariant(reinterpret_cast<qulonglong>(a_ContactBook.get())), roleContactBook);
	itemCB->setData(QVariant(roleContactBook),                                   roleRole);
	devItem->appendRow(itemCB);
}





void SessionModel::delDeviceContactBook(Device * a_Device, const ContactBook * a_ContactBook)
{
	// Check that the device is valid and has an item:
	if (a_Device == nullptr)
	{
		qDebug() << "Attempting to del a contact book not from a device, ignoring.";
		assert(!"No device specified");
		return;
	}
	auto devItem = findDeviceItem(a_Device);
	if (devItem == nullptr)
	{
		qDebug() << "Attempting to del a contact book from a device that is not in the list, ignoring.";
		assert(!"Device not in list");
		return;
	}

	// Remove the CB item:
	auto cbItem = findContactBookItemInDevice(*devItem, a_ContactBook);
	if (cbItem == nullptr)
	{
		qDebug() << "Attempting to del a contact book not present to the model, ignoring.";
		return;
	}
	// TODO: Remove
}





void SessionModel::deviceOnline(Device * a_Device, bool a_IsOnline)
{
	Q_UNUSED(a_IsOnline);

	// Check if the device's item exists:
	auto devItem = findDeviceItem(a_Device);
	if (devItem == nullptr)
	{
		qDebug() << "Device " << a_Device->displayName() << " is going online, but has no item. Adding now.";
		addDevice(a_Device);
		return;
	}

	// Re-root the item:
	auto newRoot = getRootForDevice(*a_Device);
	if (newRoot == devItem->parent())
	{
		// Already under the correct root
		return;
	}
	auto row = devItem->parent()->takeRow(devItem->row());
	newRoot->appendRow(row);
}





