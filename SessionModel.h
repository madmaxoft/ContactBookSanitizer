#ifndef SESSIONMODEL_H
#define SESSIONMODEL_H





#include <memory>

#include <QStandardItemModel>





// fwd:
class Session;
class ContactBook;
class Device;
using ContactBookPtr = std::shared_ptr<ContactBook>;





/** A model that displays a Session in a tree-like manner, down to its individual address-books. */
class SessionModel:
	public QStandardItemModel
{
	Q_OBJECT
	typedef QStandardItemModel Super;

public:

	/** Creates a new instance that displays the specified session. */
	explicit SessionModel(Session * a_Session);

	/** Resets the model to display the specified session. */
	void setSession(Session * a_Session);

	/** Returns the device corresponding to the specified item.
	Works for device's subitems, too, still returns the corresponding device.
	Returns nullptr if the item doesn't represent a device at all. */
	Device * deviceFromIndex(const QModelIndex & a_Index) const;

	/** Returns the contact book that is represented by the specified model index.
	Returns nullptr if the model index doesn't represent a ContactBook. */
	ContactBookPtr getContactBook(const QModelIndex & a_Index) const;


protected:


	/** The session to be modelled by this instance. */
	Session * m_Session;

	/** The root item under which Online devices are added. */
	QStandardItem * m_RootOnline;

	/** The root item under which the Offline devices are added. */
	QStandardItem * m_RootOffline;

	/** The root item undec which backups are added. */
	QStandardItem * m_RootBackups;


	/** Returns the root item under which the device should be added.
	Returns one of m_RootOnline, m_RootOffline or m_RootBackups. */
	QStandardItem * getRootForDevice(const Device & a_Device);

	/** Returns the item representing the specified device.
	Returns nullptr if no such item exists. */
	QStandardItem * findDeviceItem(const Device * a_Device);

	/** Adds the item representing the specified ContactBook to a_DeviceItem, unless already present. */
	void addContactBook(QStandardItem & a_DeviceItem, const ContactBook & a_ContactBook);

	/** Returns the item representing the specified ContactBook.
	Returns nullptr if no such item found.
	a_DeviceItem is the item for the Device which contains the ContactBook. */
	QStandardItem * findContactBookItemInDevice(
		const QStandardItem & a_DeviceItem,
		const ContactBook * a_ContactBook
	);


protected slots:

	/** Adds the item for the device.
	If the item for this Device already exists, does nothing.
	Connects the device's signals so that we receive the addDeviceContactBook, delDeviceContactBook and
	deviceOnline signals. */
	void addDevice(Device * a_Device);

	/** Removes the item representing the device. */
	void removeDevice(const Device * a_Device);

	/** Adds the item for the specified ContactBook.
	If the item for this ContactBook already exists, does nothing. */
	void addDeviceContactBook(Device * a_Device, ContactBookPtr a_ContactBook);

	/** Removes the item representing the ContactBook. */
	void delDeviceContactBook(Device * a_Device, const ContactBook * a_ContactBook);

	/** Moves the device's item to the right root, based on whether the device is online or not. */
	void deviceOnline(Device * a_Device, bool a_IsOnline);

signals:

	/** Emitted after an item corresponding to a new device is created. */
	void deviceItemCreated(Device * a_Device, const QModelIndex & a_Index);
};





#endif // SESSIONMODEL_H
