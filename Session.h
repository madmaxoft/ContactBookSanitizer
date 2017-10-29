#ifndef SESSION_H
#define SESSION_H





#include <memory>

#include <QObject>

#include "Device.h"





class Session:
	public QObject
{
	Q_OBJECT

	typedef QObject Super;

public:

	/** Creates a new instance, with no ContactBooks contained within. */
	explicit Session();

	/** Adds the specified device to the session. */
	void addDevice(std::unique_ptr<Device> a_Device);

	/** Returns all the sources currently stored within. */
	const std::vector<std::unique_ptr<Device>> & getDevices() const { return m_Devices; }

	/** Removes the specified device (and its corresponding contact books) from the session. */
	void delDevice(const Device * a_Device);

	/** Starts all devices in the session. */
	void startDevices();

	/** Stops all devices in the session. */
	void stopDevices();

	/** Sets the filename where the session should be saved. */
	void setFileName(const QString & a_FileName);

	/** Returns the filename where the session should be saved. */
	const QString & fileName() const { return m_FileName; }

	/** Loads a session from the specified filename.
	Doesn't start the devices.
	Returns the loaded session, or nullptr if the loading fails. */
	static std::unique_ptr<Session> loadFromFile(const QString & a_FileName);

	/** Saves the session data into the file specified by m_FileName.
	Throws if the file cannot be written (empty filename, inaccessible file). */
	void saveToFile();

	/** Saves the session data into the specified file.
	Also sets the internal filename to the specified file.
	Throws if the file cannot be written (empty filename, inaccessible file). */
	void saveToFile(const QString & a_FileName);

protected:

	/** The devices in this session. */
	std::vector<std::unique_ptr<Device>> m_Devices;

	/** The filename where the session should be saved. */
	QString m_FileName;


signals:

	/** Emitted after a device is added into the session. */
	void addedDevice(Device * a_Device);

	/** Emitted before a device is removed from the session. */
	void removingDevice(Device * a_Device);

public slots:
};

#endif // SESSION_H
