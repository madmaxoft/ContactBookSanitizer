#ifndef DEVICECARDDAV_H
#define DEVICECARDDAV_H




#include "Device.h"





class DeviceCardDav:
	public Device
{
	Q_OBJECT
	using Super = Device;


public:

	/** Returns the display name that should be used for this device. */
	virtual QString displayName() const override { return m_DisplayName; }

	/** Starts the device.
	The device can start its background threads here, and report its Online state only after this call. */
	virtual void start() override;

	/** Stops the device.
	The device should stop its threads here asynchronously. */
	virtual void stop() override;

	/** Returns true if the device is currently online. */
	virtual bool isOnline() const override;

	/** Returns all the contact books currently available in the device. */
	virtual const std::vector<ContactBookPtr> contactBooks() override { return m_ContactBooks; }


protected:

	/** The server's base URL. */
	QString m_ServerUrl;

	/** The user name to use when connecting to the server. */
	QString m_UserName;

	/** The password to use when connecting to the server. */
	QString m_Password;

	/** The contact books on the server. */
	std::vector<ContactBookPtr> m_ContactBooks;


	/** The device name, as displayed to the user. */
	QString m_DisplayName;


	/** Loads the Device-specific data from the configuration.
	a_Config is a config returned by save() in a previous app run, through which a Device descendant is
	expected to persist its logical state - connection settings, login etc. */
	virtual bool load(const QJsonObject & a_Config) override;

	/** Saves the Device-specific config data, so that the Device can be re-created next time
	the app is started. The descendants are expected to save their logical state - connection settings,
	login etc. */
	virtual QJsonObject save() const override;

};





#endif // DEVICECARDDAV_H
