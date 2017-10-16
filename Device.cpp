#include "Device.h"
#include <QtDebug>
#include "ExampleDevice.h"
#include "DeviceVcfFile.h"





std::unique_ptr<Device> Device::createFromConfig(const QJsonObject & a_Config)
{
	// Get the type out of the config:
	auto type = a_Config["type"].toString();
	if (type.isNull())
	{
		qWarning() << "Device is missing type";
		return nullptr;
	}

	// Create the device subclass instance, based on type:
	auto dev = createFromType(type);
	if (type == nullptr)
	{
		return nullptr;
	}

	// Load the device config:
	if (!dev->load(a_Config))
	{
		return nullptr;
	}
	return dev;
}





ContactBookPtr Device::getSharedContactBook(const ContactBook * a_ContactBook)
{
	for (auto & cb: contactBooks())
	{
		if (cb.get() == a_ContactBook)
		{
			return cb;
		}
	}
	return nullptr;
}





std::unique_ptr<Device> Device::createFromType(const QString & a_Type)
{
	if (a_Type == "Example")
	{
		return std::unique_ptr<Device>(new ExampleDevice);
	}
	if (a_Type == "VcfFile")
	{
		return std::unique_ptr<Device>(new DeviceVcfFile);
	}

	// TODO: Other device types

	qWarning() << "Device type not recognized: " << a_Type;
	return nullptr;
}




