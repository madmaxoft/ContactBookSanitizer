#ifndef HORIZONTALCONTACTVIEW_H
#define HORIZONTALCONTACTVIEW_H






#include <QAbstractScrollArea>
#include <QHeaderView>
#include <QStandardItemModel>
#include "ContactBook.h"
#include "DisplayContact.h"





/** Displays contacts from a single ContactBook in a horizontally-scrolling area.
Contacts are draw top-to-bottom, then overflow to the next columnn. All columns have the same width,
adjustable by the user. Each column is represented by two actual QHeaderView columns, one for label,
another for value. */
class HorizontalContactView:
	public QAbstractScrollArea
{
	using Super = QAbstractScrollArea;


public:

	HorizontalContactView(QWidget * a_Parent = nullptr);

	/** Sets the contacts to display. */
	void setContactBook(ContactBookPtr a_ContactBook);

	/** Sets the new font for labels, and recalculates the layout. */
	void setLabelFont(const QFont & a_LabelFont);

	/** Sets the new font for Values, and recalculates the layout. */
	void setValueFont(const QFont & a_ValueFont);

	/** Sets the new font for titles, and recalculates the layout. */
	void setTitleFont(const QFont & a_TitleFont);


protected:

	/** Represents the position of a single contact in the current layout. */
	struct ContactLayout
	{
		int m_Column;
		int m_Top;
		DisplayContactPtr m_Contact;

		ContactLayout(int a_Column, int a_Top, DisplayContactPtr a_Contact):
			m_Column(a_Column),
			m_Top(a_Top),
			m_Contact(a_Contact)
		{
		}
	};


	/** The source of the contacts currently displayed. */
	ContactBookPtr m_ContactBook;

	/** Contacts from m_ContactBook, parsed into their display forms and sorted. */
	std::vector<DisplayContactPtr> m_DisplayContacts;

	/** Stores all contacts in their layout positions.
	Updated by recalculateLayout(). */
	std::vector<ContactLayout> m_CurrentLayout;

	/** The top header that indicactes the row widths*/
	QHeaderView m_Header;

	/** Model for m_Header.
	There's no Qt widget that provides only the header without any model, so we need to provide a dummy
	model ourselves. */
	QStandardItemModel m_HeaderModel;

	/** The font used for rendering the labels for individual rows in the contact. */
	QFont m_LabelFont;

	/** The font used for rendering the values for individual rows in the contact. */
	QFont m_ValueFont;

	/** The font used for rendering the contact's FullName at the top of the contact. */
	QFont m_TitleFont;

	/** Height of each individual row in the contact's display. */
	int m_RowHeight;

	/** Height of the contact's FullName row at the top of the contact. */
	int m_TitleRowHeight;

	/** Width of the half-column for row labels. */
	int m_LabelWidth;

	/** Width of the half-column for row values. */
	int m_ValueWidth;

	/** Set to true while the widget is updating its geometry, to avoid endless loops. */
	bool m_GeometryRecursionBlock;


	/** Parses contacts from m_ContactBook into m_DisplayContacts.
	Then calls recalculateLayout() to update the layout, too. */
	void parseContacts();

	/** Recalculates m_CurrentLayout for the current contents of m_DisplayContacts.
	Also updates the scrollbars as needed. */
	void recalculateLayout();

	/** Returns the height of the specified contact, in pixels. */
	int getContactHeight(const DisplayContactPtr & a_Contact);

	/** Paints a single contact card on the specified painter.
	a_HorzOffset and a_VertOffset are the positions of the horz / vert scrollbars. */
	void paintCard(QPainter & a_Painter, const ContactLayout & a_Card, int a_HorzOffset, int a_VertOffset);

	// QAbstractScrollArea overrides:
	virtual void paintEvent(QPaintEvent * a_Event) override;
	virtual void resizeEvent(QResizeEvent * a_Event) override;

	/** Updates the positions of the header, scrollbars etc. */
	void updateInternalGeometry();


private slots:

	/** A section in m_Header was resized by the user, update the card widths. */
	void headerSectionResized(int a_ColumnIndex, int a_OldSize, int a_NewSize);

	/** The horizontal scrollbar has been changed. */
	void horizontalScrollBarValueChanged(int a_NewValue);
};





#endif // HORIZONTALCONTACTVIEW_H
