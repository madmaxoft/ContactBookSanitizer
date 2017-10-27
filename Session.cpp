#include "Session.h"
#include "Device.h"





Session::Session():
	Super(nullptr)
{

}





void Session::addDevice(std::unique_ptr<Device> a_Device)
{
	auto dev = a_Device.get();
	m_Devices.push_back(std::move(a_Device));
	emit addedDevice(dev);
}





void Session::startDevices()
{
	for (auto & dev: m_Devices)
	{
		dev->start();
	}
}





void Session::stopDevices()
{
	for (auto & dev: m_Devices)
	{
		dev->stop();
	}
}




