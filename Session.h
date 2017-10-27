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


protected:
	std::vector<std::unique_ptr<Device>> m_Devices;


signals:

	/** Emitted after a device is added into the session. */
	void addedDevice(Device * a_Device);

	/** Emitted before a device is removed from the session. */
	void removingDevice(Device * a_Device);

public slots:
};

#endif // SESSION_H
