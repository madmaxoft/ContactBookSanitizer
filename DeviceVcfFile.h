#ifndef DEVICEVCFFILE_H
#define DEVICEVCFFILE_H




#include "Device.h"





class DeviceVcfFile:
	public Device
{
	Q_OBJECT

public:
	DeviceVcfFile();


	/** Returns the display name that should be used for this device. */
	virtual QString displayName() const override;

	/** Starts the device.
	The device can start its background threads here, and report its Online state only after this call. */
	virtual void start() override;

	/** Stops the device.
	The device should stop its threads here asynchronously. */
	virtual void stop() override;

	/** Returns true if the device is currently online. */
	virtual bool isOnline() const override;

	/** Returns all the contact books currently available in the device. */
	virtual const std::vector<ContactBookPtr> contactBooks() override { return {m_ContactBook}; }


protected:

	/** The name of the VCF file that is represented by this device. */
	QString m_VcfFileName;

	/** The base filename of m_VcfFileName (the file name without path nor extension). */
	QString m_VcfFileNameBase;

	/** The contact book containing the parsed contact data. */
	ContactBookPtr m_ContactBook;

	/** The device name, as displayed to the user.
	Normally set to the base file name of m_VcfFileName, but can be other values to indicate problems, for example. */
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




#endif // DEVICEVCFFILE_H
