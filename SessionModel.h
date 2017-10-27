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


protected slots:

	/** Adds the item for the device. */
	void addDevice(Device * a_Device);

	/** Removes the item representing the device. */
	void removeDevice(Device * a_Device);


signals:

	/** Emitted after an item corresponding to a new device is created. */
	void deviceItemCreated(const QModelIndex & a_Index);
};





#endif // SESSIONMODEL_H
