#ifndef DEVICE_H
#define DEVICE_H




#include <memory>

#include <QObject>
#include <QJsonObject>





// fwd:
class ContactBook;
using ContactBookPtr = std::shared_ptr<ContactBook>;





/** Interface through which descendants can provide multiple ContactBooks.
Also provides a factory for creating the various Device types, based on their saved settings. */
class Device:
	public QObject
{
	Q_OBJECT
	using Super = QObject;


public:

	// Force a virtual destructor in descendants
	virtual ~Device() {};

	/** Creates a new descendant instance and loads its config, all based on a_Config.
	The descendant is choosen based on the "type" value in a_Config.
	Returns nullptr if descendant type is unknown / invalid, or the descendant fails to load. */
	static std::unique_ptr<Device> createFromConfig(const QJsonObject & a_Config);

	/** Returns the display name that should be used for this device. */
	virtual QString displayName() const = 0;

	/** Starts the device.
	The device can start its background threads here, and report its Online state only after this call. */
	virtual void start() = 0;

	/** Stops the device.
	The device should stop its threads here asynchronously. */
	virtual void stop() = 0;

	/** Returns true if the device is currently online. */
	virtual bool isOnline() const = 0;

	/** Returns true if the device represents a backup.
	The default implementation is sufficient for all descendants except for the actual backup. */
	virtual bool isBackup() const { return false; }

	/** Returns all the contact books currently available in the device. */
	virtual const std::vector<ContactBookPtr> contactBooks() = 0;

	/** Converts the const naked pointer to ContactBook to the mutable shared_ptr version, if available.
	Returns nullptr if the contact book is not known.
	The default implementation searches the results of getContactBooks(), the first match is returned. */
	virtual ContactBookPtr getSharedContactBook(const ContactBook * a_ContactBook);

	/** Loads the Device-specific data from the configuration.
	a_Config is a config returned by save() in a previous app run, through which a Device descendant is
	expected to persist its logical state - connection settings, login etc.
	Returns true on success, false on failure. */
	virtual bool load(const QJsonObject & a_Config) = 0;

	/** Saves the Device-specific config data, so that the Device can be re-created next time
	the app is started. The descendants are expected to save their logical state - connection settings,
	login etc. */
	virtual QJsonObject save() const = 0;


signals:

	/** Emitted when the device becomes online or offline.
	When the device becomes offline, new data cannot be read or written to it.
	The device should already report isOnline() == a_IsOnline when this is called. */
	void online(Device * a_Device, bool a_IsOnline = true);

	/** A new contact book has been detected in the device, it should be added to the underlying session. */
	void addContactBook(Device * a_Device, ContactBookPtr a_ContactBook);

	/** The specified contact book is no longer available in the device, it should be removed from
	the underlying session.
	Note that the contact book data is still present in the form it was last read, so a backup can be made,
	but it is already disconnected, so new data cannot be read nor written to it. */
	void delContactBook(Device * a_Device, const ContactBook * a_ContactBook);


protected:

	/** Creates a new Device subclass instance based on the specified type. */
	static std::unique_ptr<Device> createFromType(const QString & a_Type);
};

#endif // DEVICE_H
