#ifndef MAINWINDOW_H
#define MAINWINDOW_H





#include <memory>
#include <QMainWindow>





// fwd:
class Session;
class SessionModel;
class ContactBookModel;
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
};





#endif // MAINWINDOW_H
