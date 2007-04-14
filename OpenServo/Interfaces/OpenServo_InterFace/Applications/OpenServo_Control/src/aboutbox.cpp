/****************************************************************************
** Form implementation generated from reading ui file 'aboutbox.ui'
**
** Created: Sun Apr 1 02:28:41 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "aboutbox.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a aboutBoxWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
aboutBoxWidget::aboutBoxWidget( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "aboutBoxWidget" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 250, 300 ) );
    setMaximumSize( QSize( 250, 300 ) );
    aboutBoxWidgetLayout = new QGridLayout( this, 1, 1, 11, 6, "aboutBoxWidgetLayout"); 

    textLabel2 = new QLabel( this, "textLabel2" );
    textLabel2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, textLabel2->sizePolicy().hasHeightForWidth() ) );
    textLabel2->setMaximumSize( QSize( 32767, 20 ) );

    aboutBoxWidgetLayout->addWidget( textLabel2, 0, 0 );

    textLabel3 = new QLabel( this, "textLabel3" );
    textLabel3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, textLabel3->sizePolicy().hasHeightForWidth() ) );
    textLabel3->setMaximumSize( QSize( 32767, 20 ) );

    aboutBoxWidgetLayout->addWidget( textLabel3, 1, 0 );

    closeBtn = new QPushButton( this, "closeBtn" );

    aboutBoxWidgetLayout->addWidget( closeBtn, 5, 0 );

    frame4 = new QFrame( this, "frame4" );
    frame4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, frame4->sizePolicy().hasHeightForWidth() ) );
    frame4->setMinimumSize( QSize( 0, 80 ) );
    frame4->setFrameShape( QFrame::StyledPanel );
    frame4->setFrameShadow( QFrame::Raised );

    textLabel5 = new QLabel( frame4, "textLabel5" );
    textLabel5->setGeometry( QRect( 10, 30, 210, 18 ) );

    textLabel6 = new QLabel( frame4, "textLabel6" );
    textLabel6->setGeometry( QRect( 10, 50, 200, 18 ) );

    textLabel4 = new QLabel( frame4, "textLabel4" );
    textLabel4->setGeometry( QRect( 10, 10, 190, 16 ) );

    aboutBoxWidgetLayout->addWidget( frame4, 4, 0 );

    textLabel8 = new QLabel( this, "textLabel8" );

    aboutBoxWidgetLayout->addWidget( textLabel8, 3, 0 );

    textLabel7 = new QLabel( this, "textLabel7" );
    textLabel7->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, textLabel7->sizePolicy().hasHeightForWidth() ) );
    textLabel7->setMaximumSize( QSize( 32767, 20 ) );

    aboutBoxWidgetLayout->addWidget( textLabel7, 2, 0 );
    languageChange();
    resize( QSize(250, 300).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( closeBtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
aboutBoxWidget::~aboutBoxWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void aboutBoxWidget::languageChange()
{
    setCaption( tr( "About" ) );
    textLabel2->setText( tr( "<h3>OpenServo Test application</h3>" ) );
    textLabel3->setText( tr( "Visit and contrubute to <a href=\"http://www.openservo.com/forums/\">OpenServo Project</a>" ) );
    closeBtn->setText( tr( "C&lose" ) );
    closeBtn->setAccel( QKeySequence( tr( "Alt+L" ) ) );
    textLabel5->setText( tr( "<a href=\"mailto:Barry.Carter@headfuzz.co.uk\">Barry.Carter@headfuzz.co.uk</a>" ) );
    textLabel6->setText( tr( "<a href=\"http://www.openservo.com/\">www.headfuzz.co.uk</a>" ) );
    textLabel4->setText( tr( "v0.6.1 Copyright 2007 Barry Carter" ) );
    textLabel8->setText( tr( "<p align=\"center\">This may require root access under *NIX systems. Be sure to install the underlying OSIF_xxx library for your OS</p>" ) );
    textLabel7->setText( tr( "<a href=\"http://www.openservo.com/\">www.OpenServo.com</a>" ) );
}

