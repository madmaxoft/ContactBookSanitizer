#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Session.h"
#include "SessionModel.h"
#include "ContactBookModel.h"





MainWindow::MainWindow(std::unique_ptr<Session> && a_Session):
	Super(nullptr),
	m_UI(new Ui::MainWindow),
	m_Session(std::move(a_Session))
{
	m_UI->setupUi(this);

	// Add a decoration to the splitter handle to make it more visible:
	{
		auto handle = m_UI->splitHorz->handle(1);
		auto layout = new QVBoxLayout(handle);
		layout->setSpacing(0);
		layout->setMargin(0);
		auto line = new QFrame(handle);
		line->setFrameShape(QFrame::StyledPanel);
		line->setFrameShadow(QFrame::Sunken);
		layout->addWidget(line);
	}

	connectSignals();

	// Set up the session display:
	m_SessionModel.reset(new SessionModel(m_Session.get()));
	m_UI->tvSession->setModel(m_SessionModel.get());

	// Set up the ContactBook display:
	m_ContactBookModel.reset(new ContactBookModel(nullptr));
	m_UI->tvContactBook->setModel(m_ContactBookModel.get());
}





MainWindow::~MainWindow()
{
	// No explicit code needed
	// Still, this needs to be present in the CPP file, so that unique_ptr<UI>'s destructor is generated here.
}





void MainWindow::connectSignals()
{
	connect(m_UI->tvSession, &QTreeView::activated, this, &MainWindow::sessionItemActivated);
	connect(m_UI->tvSession, &QTreeView::clicked,   this, &MainWindow::sessionItemActivated);
}





void MainWindow::sessionItemActivated(const QModelIndex & a_Index)
{
	auto cbook = m_SessionModel->getContactBook(a_Index);
	m_ContactBookModel->setContactBook(m_Session->getContactBook(cbook));
}





