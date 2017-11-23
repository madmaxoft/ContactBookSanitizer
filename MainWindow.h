#ifndef MAINWINDOW_H
#define MAINWINDOW_H





#include <memory>
#include <QMainWindow>





// fwd:
class Session;
class SessionModel;
class ContactBookModel;
class Device;
namespace Ui
{
	class MainWindow;
}





class MainWindow:
	public QMainWindow
{
	Q_OBJECT

	typedef QMainWindow Super;


public:

	explicit MainWindow(std::unique_ptr<Session> && a_Session);
	~MainWindow();

	/** Returns the device that is currently selected in the tree view.
	If a device's subitem is selected, still returns the device for that subitem.
	Returns nullptr if no device selected. */
	Device * selectedDevice();


private:

	std::unique_ptr<Ui::MainWindow> m_UI;

	/** The session containing all the data currently displayed. */
	std::unique_ptr<Session> m_Session;

	/** The model for displaying the session data in tvSession. */
	std::unique_ptr<SessionModel> m_SessionModel;

	/** The model for displaying contacts in the selected ContactBook. */
	std::unique_ptr<ContactBookModel> m_ContactBookModel;


	/** Connects the UI signals and slots. */
	void connectSignals();


private slots:

	/** Triggered when the selection in tvSession changes. */
	void sessionItemActivated(const QModelIndex & a_Index);

	/** Shows the "Add new device" dialog. */
	void addNewDevice(void);

	/** Deletes the currently selected device, after confirmation. */
	void delDevice(void);

	/** Expands the device item represented by the model.
	Triggered by m_SessionModel after a new device is added. */
	void expandDeviceItem(Device * a_Device, const QModelIndex & a_Index);
};





#endif // MAINWINDOW_H
