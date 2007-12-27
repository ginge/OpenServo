/****************************************************************************
** Form implementation generated from reading ui file 'mainwin.ui'
**
** Created: Sun Mar 18 21:16:55 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "mainwin.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a testMainWin as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
testMainWin::testMainWin( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "testMainWin" );
    setSizeGripEnabled( TRUE );
    testMainWinLayout = new QGridLayout( this, 1, 1, 11, 6, "testMainWinLayout"); 

    layout21 = new QHBoxLayout( 0, 0, 6, "layout21"); 

    groupBox5 = new QGroupBox( this, "groupBox5" );
    groupBox5->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, groupBox5->sizePolicy().hasHeightForWidth() ) );
    groupBox5->setMinimumSize( QSize( 80, 0 ) );
    groupBox5->setMaximumSize( QSize( 70, 55 ) );
    groupBox5->setColumnLayout(0, Qt::Vertical );
    groupBox5->layout()->setSpacing( 6 );
    groupBox5->layout()->setMargin( 11 );
    groupBox5Layout = new QGridLayout( groupBox5->layout() );
    groupBox5Layout->setAlignment( Qt::AlignTop );

    setPos = new QSpinBox( groupBox5, "setPos" );
    setPos->setMinimumSize( QSize( 60, 0 ) );
    setPos->setMaximumSize( QSize( 200, 32767 ) );
    setPos->setMaxValue( 1024 );
    setPos->setLineStep( 10 );
    setPos->setValue( 400 );

    groupBox5Layout->addWidget( setPos, 0, 0 );
    layout21->addWidget( groupBox5 );
    spacer3_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout21->addItem( spacer3_2 );

    groupBox2 = new QGroupBox( this, "groupBox2" );
    groupBox2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 1, 0, groupBox2->sizePolicy().hasHeightForWidth() ) );
    groupBox2->setMinimumSize( QSize( 375, 55 ) );
    groupBox2->setMaximumSize( QSize( 375, 55 ) );
    groupBox2->setColumnLayout(0, Qt::Vertical );
    groupBox2->layout()->setSpacing( 6 );
    groupBox2->layout()->setMargin( 11 );
    groupBox2Layout = new QHBoxLayout( groupBox2->layout() );
    groupBox2Layout->setAlignment( Qt::AlignTop );

    cmdReboot = new QPushButton( groupBox2, "cmdReboot" );
    cmdReboot->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdReboot );

    cmdPWM = new QPushButton( groupBox2, "cmdPWM" );
    cmdPWM->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdPWM );

    cmdSave = new QPushButton( groupBox2, "cmdSave" );
    cmdSave->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdSave );

    cmdRestore = new QPushButton( groupBox2, "cmdRestore" );
    cmdRestore->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdRestore );

    cmdDefault = new QPushButton( groupBox2, "cmdDefault" );
    cmdDefault->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdDefault );

    cmdFlash = new QPushButton( groupBox2, "cmdFlash" );
    cmdFlash->setEnabled( FALSE );
    cmdFlash->setMaximumSize( QSize( 50, 32767 ) );
    groupBox2Layout->addWidget( cmdFlash );

    fileLoad = new QPushButton( groupBox2, "fileLoad" );
    fileLoad->setMaximumSize( QSize( 20, 32767 ) );
    groupBox2Layout->addWidget( fileLoad );
    layout21->addWidget( groupBox2 );

    testMainWinLayout->addMultiCellLayout( layout21, 1, 1, 1, 10 );

    layout26 = new QHBoxLayout( 0, 0, 6, "layout26"); 

    groupBox3 = new QGroupBox( this, "groupBox3" );
    groupBox3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, groupBox3->sizePolicy().hasHeightForWidth() ) );
    groupBox3->setMinimumSize( QSize( 290, 0 ) );
    groupBox3->setMaximumSize( QSize( 32767, 90 ) );
    groupBox3->setColumnLayout(0, Qt::Vertical );
    groupBox3->layout()->setSpacing( 6 );
    groupBox3->layout()->setMargin( 11 );
    groupBox3Layout = new QGridLayout( groupBox3->layout() );
    groupBox3Layout->setAlignment( Qt::AlignTop );

    voltageRequest = new QPushButton( groupBox3, "voltageRequest" );
    voltageRequest->setMaximumSize( QSize( 30, 20 ) );

    groupBox3Layout->addWidget( voltageRequest, 1, 1 );
    spacer3 = new QSpacerItem( 20, 37, QSizePolicy::Minimum, QSizePolicy::Expanding );
    groupBox3Layout->addItem( spacer3, 0, 1 );

    layout10 = new QVBoxLayout( 0, 0, 6, "layout10"); 

    layout9 = new QHBoxLayout( 0, 0, 6, "layout9"); 

    textLabel2_2 = new QLabel( groupBox3, "textLabel2_2" );
    textLabel2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_2->sizePolicy().hasHeightForWidth() ) );
    textLabel2_2->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_2->setMaximumSize( QSize( 50, 20 ) );
    layout9->addWidget( textLabel2_2 );

    posLbl = new QLabel( groupBox3, "posLbl" );
    posLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, posLbl->sizePolicy().hasHeightForWidth() ) );
    layout9->addWidget( posLbl );

    textLabel2_2_2 = new QLabel( groupBox3, "textLabel2_2_2" );
    textLabel2_2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_2_2->sizePolicy().hasHeightForWidth() ) );
    textLabel2_2_2->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_2_2->setMaximumSize( QSize( 50, 20 ) );
    layout9->addWidget( textLabel2_2_2 );

    pwmLbl = new QLabel( groupBox3, "pwmLbl" );
    pwmLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, pwmLbl->sizePolicy().hasHeightForWidth() ) );
    layout9->addWidget( pwmLbl );
    layout10->addLayout( layout9 );

    layout8 = new QHBoxLayout( 0, 0, 6, "layout8"); 

    textLabel2_3 = new QLabel( groupBox3, "textLabel2_3" );
    textLabel2_3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_3->sizePolicy().hasHeightForWidth() ) );
    textLabel2_3->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_3->setMaximumSize( QSize( 50, 20 ) );
    layout8->addWidget( textLabel2_3 );

    spdLbl = new QLabel( groupBox3, "spdLbl" );
    spdLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, spdLbl->sizePolicy().hasHeightForWidth() ) );
    layout8->addWidget( spdLbl );

    textLabel2_3_2 = new QLabel( groupBox3, "textLabel2_3_2" );
    textLabel2_3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_3_2->sizePolicy().hasHeightForWidth() ) );
    textLabel2_3_2->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_3_2->setMaximumSize( QSize( 50, 20 ) );
    layout8->addWidget( textLabel2_3_2 );

    timLbl = new QLabel( groupBox3, "timLbl" );
    timLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, timLbl->sizePolicy().hasHeightForWidth() ) );
    layout8->addWidget( timLbl );
    layout10->addLayout( layout8 );

    layout7 = new QHBoxLayout( 0, 0, 6, "layout7"); 

    textLabel2_4 = new QLabel( groupBox3, "textLabel2_4" );
    textLabel2_4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_4->sizePolicy().hasHeightForWidth() ) );
    textLabel2_4->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_4->setMaximumSize( QSize( 50, 20 ) );
    layout7->addWidget( textLabel2_4 );

    curLbl = new QLabel( groupBox3, "curLbl" );
    curLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, curLbl->sizePolicy().hasHeightForWidth() ) );
    layout7->addWidget( curLbl );

    textLabel2_4_2 = new QLabel( groupBox3, "textLabel2_4_2" );
    textLabel2_4_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, textLabel2_4_2->sizePolicy().hasHeightForWidth() ) );
    textLabel2_4_2->setMinimumSize( QSize( 50, 0 ) );
    textLabel2_4_2->setMaximumSize( QSize( 50, 20 ) );
    layout7->addWidget( textLabel2_4_2 );

    voltLbl = new QLabel( groupBox3, "voltLbl" );
    voltLbl->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, voltLbl->sizePolicy().hasHeightForWidth() ) );
    layout7->addWidget( voltLbl );
    layout10->addLayout( layout7 );

    groupBox3Layout->addMultiCellLayout( layout10, 0, 1, 0, 0 );
    layout26->addWidget( groupBox3 );

    groupBox1 = new QGroupBox( this, "groupBox1" );
    groupBox1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, groupBox1->sizePolicy().hasHeightForWidth() ) );
    groupBox1->setMinimumSize( QSize( 170, 90 ) );
    groupBox1->setMaximumSize( QSize( 170, 90 ) );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    groupBox1Layout = new QGridLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    layout2 = new QGridLayout( 0, 1, 1, 0, 0, "layout2"); 

    textLabel1_2 = new QLabel( groupBox1, "textLabel1_2" );

    layout2->addMultiCellWidget( textLabel1_2, 1, 2, 0, 0 );

    setupSMin = new QLineEdit( groupBox1, "setupSMin" );

    layout2->addWidget( setupSMin, 2, 3 );

    textLabel1_4 = new QLabel( groupBox1, "textLabel1_4" );

    layout2->addMultiCellWidget( textLabel1_4, 0, 1, 2, 2 );

    setupSMax = new QLineEdit( groupBox1, "setupSMax" );

    layout2->addMultiCellWidget( setupSMax, 0, 1, 3, 3 );

    textLabel1 = new QLabel( groupBox1, "textLabel1" );

    layout2->addWidget( textLabel1, 0, 0 );

    setupD = new QLineEdit( groupBox1, "setupD" );

    layout2->addWidget( setupD, 3, 1 );

    textLabel1_4_2 = new QLabel( groupBox1, "textLabel1_4_2" );

    layout2->addWidget( textLabel1_4_2, 2, 2 );

    setupP = new QLineEdit( groupBox1, "setupP" );

    layout2->addWidget( setupP, 0, 1 );

    textLabel1_3 = new QLabel( groupBox1, "textLabel1_3" );

    layout2->addWidget( textLabel1_3, 3, 0 );

    setupI = new QLineEdit( groupBox1, "setupI" );

    layout2->addWidget( setupI, 2, 1 );

    groupBox1Layout->addLayout( layout2, 0, 0 );
    layout26->addWidget( groupBox1 );

    testMainWinLayout->addMultiCellLayout( layout26, 0, 0, 1, 10 );

    groupBox2_2 = new QGroupBox( this, "groupBox2_2" );
    groupBox2_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, groupBox2_2->sizePolicy().hasHeightForWidth() ) );
    groupBox2_2->setMinimumSize( QSize( 0, 85 ) );
    groupBox2_2->setMaximumSize( QSize( 32767, 85 ) );
    groupBox2_2->setColumnLayout(0, Qt::Vertical );
    groupBox2_2->layout()->setSpacing( 6 );
    groupBox2_2->layout()->setMargin( 11 );
    groupBox2_2Layout = new QGridLayout( groupBox2_2->layout() );
    groupBox2_2Layout->setAlignment( Qt::AlignTop );

    textLabel3_2 = new QLabel( groupBox2_2, "textLabel3_2" );

    groupBox2_2Layout->addWidget( textLabel3_2, 0, 5 );

    genericData = new QLineEdit( groupBox2_2, "genericData" );
    genericData->setMinimumSize( QSize( 40, 0 ) );

    groupBox2_2Layout->addWidget( genericData, 0, 6 );

    genericRegister = new QLineEdit( groupBox2_2, "genericRegister" );
    genericRegister->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, genericRegister->sizePolicy().hasHeightForWidth() ) );
    genericRegister->setMinimumSize( QSize( 40, 0 ) );
    genericRegister->setMaximumSize( QSize( 40, 32767 ) );

    groupBox2_2Layout->addMultiCellWidget( genericRegister, 0, 0, 3, 4 );

    genericDevice = new QLineEdit( groupBox2_2, "genericDevice" );
    genericDevice->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, genericDevice->sizePolicy().hasHeightForWidth() ) );
    genericDevice->setMinimumSize( QSize( 40, 0 ) );
    genericDevice->setMaximumSize( QSize( 40, 32767 ) );

    groupBox2_2Layout->addWidget( genericDevice, 0, 1 );

    textLabel3 = new QLabel( groupBox2_2, "textLabel3" );

    groupBox2_2Layout->addWidget( textLabel3, 0, 2 );

    textLabel3_5 = new QLabel( groupBox2_2, "textLabel3_5" );

    groupBox2_2Layout->addWidget( textLabel3_5, 0, 0 );

    layout24 = new QGridLayout( 0, 1, 1, 0, 6, "layout24"); 

    genericRead = new QPushButton( groupBox2_2, "genericRead" );
    genericRead->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, genericRead->sizePolicy().hasHeightForWidth() ) );

    layout24->addWidget( genericRead, 1, 1 );

    genericWrite = new QPushButton( groupBox2_2, "genericWrite" );
    genericWrite->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, genericWrite->sizePolicy().hasHeightForWidth() ) );

    layout24->addWidget( genericWrite, 0, 1 );

    groupBox2_2Layout->addMultiCellLayout( layout24, 0, 1, 7, 7 );

    textLabel3_2_2 = new QLabel( groupBox2_2, "textLabel3_2_2" );

    groupBox2_2Layout->addWidget( textLabel3_2_2, 1, 0 );

    genericLen = new QLineEdit( groupBox2_2, "genericLen" );
    genericLen->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, genericLen->sizePolicy().hasHeightForWidth() ) );
    genericLen->setMinimumSize( QSize( 40, 0 ) );
    genericLen->setMaximumSize( QSize( 40, 32767 ) );

    groupBox2_2Layout->addWidget( genericLen, 1, 1 );

    textLabel3_4 = new QLabel( groupBox2_2, "textLabel3_4" );
    textLabel3_4->setMaximumSize( QSize( 60, 32767 ) );

    groupBox2_2Layout->addMultiCellWidget( textLabel3_4, 1, 1, 2, 3 );

    genericDataRead = new QLabel( groupBox2_2, "genericDataRead" );

    groupBox2_2Layout->addMultiCellWidget( genericDataRead, 1, 1, 4, 6 );

    testMainWinLayout->addMultiCellWidget( groupBox2_2, 2, 2, 1, 10 );

    textLabel3_4_2 = new QLabel( this, "textLabel3_4_2" );
    textLabel3_4_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, textLabel3_4_2->sizePolicy().hasHeightForWidth() ) );
    textLabel3_4_2->setMaximumSize( QSize( 20, 32767 ) );

    testMainWinLayout->addWidget( textLabel3_4_2, 3, 1 );

    logBox = new QTextEdit( this, "logBox" );
    logBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, 0, 2, logBox->sizePolicy().hasHeightForWidth() ) );
    logBox->setMinimumSize( QSize( 0, 20 ) );
    logBox->setTextFormat( QTextEdit::LogText );
    logBox->setWordWrap( QTextEdit::WidgetWidth );
    logBox->setReadOnly( TRUE );

    testMainWinLayout->addMultiCellWidget( logBox, 3, 3, 2, 10 );

    writeBtn = new QPushButton( this, "writeBtn" );
    writeBtn->setEnabled( FALSE );
    writeBtn->setMaximumSize( QSize( 55, 32767 ) );

    testMainWinLayout->addWidget( writeBtn, 4, 3 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setMaximumSize( QSize( 55, 32767 ) );
    buttonCancel->setAutoDefault( TRUE );

    testMainWinLayout->addWidget( buttonCancel, 4, 10 );
    Horizontal_Spacing2 = new QSpacerItem( 30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    testMainWinLayout->addItem( Horizontal_Spacing2, 4, 8 );

    liveData = new QCheckBox( this, "liveData" );
    liveData->setEnabled( FALSE );

    testMainWinLayout->addWidget( liveData, 4, 4 );

    textLabel2 = new QLabel( this, "textLabel2" );

    testMainWinLayout->addWidget( textLabel2, 4, 5 );

    btnAbout = new QPushButton( this, "btnAbout" );
    btnAbout->setMaximumSize( QSize( 55, 32767 ) );
    btnAbout->setAutoDefault( TRUE );

    testMainWinLayout->addWidget( btnAbout, 4, 9 );

    timerIntervalBox = new QSpinBox( this, "timerIntervalBox" );
    timerIntervalBox->setEnabled( FALSE );
    timerIntervalBox->setMinValue( 20 );
    timerIntervalBox->setValue( 30 );

    testMainWinLayout->addWidget( timerIntervalBox, 4, 6 );

    readBtn = new QPushButton( this, "readBtn" );
    readBtn->setEnabled( FALSE );
    readBtn->setMaximumSize( QSize( 55, 32767 ) );

    testMainWinLayout->addMultiCellWidget( readBtn, 4, 4, 1, 2 );

    textLabel3_6 = new QLabel( this, "textLabel3_6" );

    testMainWinLayout->addWidget( textLabel3_6, 4, 7 );

    groupBox4 = new QGroupBox( this, "groupBox4" );
    groupBox4->setMinimumSize( QSize( 83, 240 ) );
    groupBox4->setMaximumSize( QSize( 83, 32767 ) );
    groupBox4->setColumnLayout(0, Qt::Vertical );
    groupBox4->layout()->setSpacing( 6 );
    groupBox4->layout()->setMargin( 11 );
    groupBox4Layout = new QGridLayout( groupBox4->layout() );
    groupBox4Layout->setAlignment( Qt::AlignTop );

    adapterList = new QListView( groupBox4, "adapterList" );
    adapterList->addColumn( tr( "Adapter(s)" ) );
    adapterList->setMinimumSize( QSize( 0, 70 ) );
    adapterList->setMaximumSize( QSize( 65, 70 ) );
    adapterList->setVScrollBarMode( QListView::Auto );
    adapterList->setHScrollBarMode( QListView::AlwaysOff );

    groupBox4Layout->addWidget( adapterList, 0, 0 );

    scanBusBtn = new QPushButton( groupBox4, "scanBusBtn" );
    scanBusBtn->setMaximumSize( QSize( 65, 32767 ) );
    scanBusBtn->setAutoDefault( TRUE );

    groupBox4Layout->addWidget( scanBusBtn, 2, 0 );

    servoList = new QListView( groupBox4, "servoList" );
    servoList->addColumn( tr( "Servo(s)" ) );
    servoList->setMinimumSize( QSize( 50, 0 ) );
    servoList->setMaximumSize( QSize( 65, 32767 ) );

    groupBox4Layout->addWidget( servoList, 1, 0 );

    testMainWinLayout->addMultiCellWidget( groupBox4, 0, 4, 0, 0 );
    languageChange();
    resize( QSize(610, 323).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( readBtn, SIGNAL( clicked() ), this, SLOT( readServo() ) );
    connect( writeBtn, SIGNAL( clicked() ), this, SLOT( writeServo() ) );
    connect( scanBusBtn, SIGNAL( clicked() ), this, SLOT( scanBus() ) );
    connect( cmdReboot, SIGNAL( clicked() ), this, SLOT( commandReboot() ) );
    connect( cmdPWM, SIGNAL( clicked() ), this, SLOT( commandPWM() ) );
    connect( cmdDefault, SIGNAL( clicked() ), this, SLOT( commandDefault() ) );
    connect( cmdRestore, SIGNAL( clicked() ), this, SLOT( commandRestore() ) );
    connect( cmdSave, SIGNAL( clicked() ), this, SLOT( commandSave() ) );
    connect( cmdFlash, SIGNAL( clicked() ), this, SLOT( commandFlash() ) );
    connect( fileLoad, SIGNAL( clicked() ), this, SLOT( flashFileLoad() ) );
    connect( voltageRequest, SIGNAL( clicked() ), this, SLOT( requestVoltage() ) );
    connect( liveData, SIGNAL( toggled(bool) ), this, SLOT( liveDataClick() ) );
    connect( servoList, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( servoSelectChange(QListViewItem*) ) );
    connect( timerIntervalBox, SIGNAL( valueChanged(int) ), this, SLOT( timerIntervalChange(int) ) );
    connect( genericRead, SIGNAL( clicked() ), this, SLOT( genericReadData() ) );
    connect( genericWrite, SIGNAL( clicked() ), this, SLOT( genericWriteData() ) );
    connect( adapterList, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( adapterSelectChange(QListViewItem*) ) );
    connect( logBox, SIGNAL( doubleClicked(int,int) ), this, SLOT( logBoxClear(int,int) ) );
    connect( btnAbout, SIGNAL( clicked() ), this, SLOT( aboutClicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
testMainWin::~testMainWin()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void testMainWin::languageChange()
{
    setCaption( tr( "OpenServo Test" ) );
    groupBox5->setTitle( tr( "Position" ) );
    groupBox2->setTitle( tr( "Servo Command" ) );
    cmdReboot->setText( tr( "Re&boot" ) );
    cmdReboot->setAccel( QKeySequence( tr( "Alt+B" ) ) );
    cmdPWM->setText( tr( "PWM &off" ) );
    cmdPWM->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    cmdSave->setText( tr( "S&ave" ) );
    cmdSave->setAccel( QKeySequence( tr( "Alt+A" ) ) );
    cmdRestore->setText( tr( "Restore" ) );
    cmdRestore->setAccel( QKeySequence( QString::null ) );
    cmdDefault->setText( tr( "Default" ) );
    cmdDefault->setAccel( QKeySequence( QString::null ) );
    cmdFlash->setText( tr( "Flash" ) );
    cmdFlash->setAccel( QKeySequence( QString::null ) );
    fileLoad->setText( tr( "..." ) );
    fileLoad->setAccel( QKeySequence( QString::null ) );
    QToolTip::add( fileLoad, tr( "Load the file to flash" ) );
    groupBox3->setTitle( tr( "Servo Output" ) );
    voltageRequest->setText( tr( "V" ) );
    QToolTip::add( voltageRequest, tr( "Request new Voltage sample" ) );
    textLabel2_2->setText( tr( "Position" ) );
    posLbl->setText( tr( "0" ) );
    textLabel2_2_2->setText( tr( "PWM" ) );
    pwmLbl->setText( tr( "0" ) );
    textLabel2_3->setText( tr( "Speed" ) );
    spdLbl->setText( tr( "0" ) );
    textLabel2_3_2->setText( tr( "Timer" ) );
    timLbl->setText( tr( "0" ) );
    textLabel2_4->setText( tr( "Current" ) );
    curLbl->setText( tr( "0" ) );
    textLabel2_4_2->setText( tr( "Voltage" ) );
    voltLbl->setText( tr( "0" ) );
    groupBox1->setTitle( tr( "Servo Setup" ) );
    textLabel1_2->setText( tr( "I" ) );
    setupSMin->setText( tr( "0x0000" ) );
    textLabel1_4->setText( tr( "SMax" ) );
    setupSMax->setText( tr( "0x0000" ) );
    textLabel1->setText( tr( "P" ) );
    setupD->setText( tr( "0x0000" ) );
    textLabel1_4_2->setText( tr( "SMin" ) );
    setupP->setText( tr( "0x0000" ) );
    textLabel1_3->setText( tr( "D" ) );
    setupI->setText( tr( "0x0000" ) );
    groupBox2_2->setTitle( tr( "Generic comms" ) );
    textLabel3_2->setText( tr( "Data" ) );
    genericData->setText( tr( "0x00 0x00 0x00 0x00" ) );
    genericRegister->setText( tr( "0x00" ) );
    genericDevice->setText( tr( "0x00" ) );
    textLabel3->setText( tr( "Register" ) );
    textLabel3_5->setText( tr( "device" ) );
    genericRead->setText( tr( "Read" ) );
    genericRead->setAccel( QKeySequence( QString::null ) );
    genericWrite->setText( tr( "Write" ) );
    genericWrite->setAccel( QKeySequence( QString::null ) );
    textLabel3_2_2->setText( tr( "length" ) );
    genericLen->setText( tr( "0" ) );
    textLabel3_4->setText( tr( "Data Read" ) );
    genericDataRead->setText( QString::null );
    textLabel3_4_2->setText( tr( "Log" ) );
    logBox->setText( QString::null );
    QToolTip::add( logBox, tr( "Log output. Double click to clear" ) );
    writeBtn->setText( tr( "Write" ) );
    writeBtn->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Quit" ) );
    buttonCancel->setAccel( QKeySequence( tr( "Alt+Q" ) ) );
    liveData->setText( tr( "&Live data" ) );
    liveData->setAccel( QKeySequence( tr( "Alt+L" ) ) );
    textLabel2->setText( tr( "Freq" ) );
    btnAbout->setText( tr( "About" ) );
    btnAbout->setAccel( QKeySequence( QString::null ) );
    readBtn->setText( tr( "Read" ) );
    readBtn->setAccel( QKeySequence( QString::null ) );
    textLabel3_6->setText( tr( "ms" ) );
    groupBox4->setTitle( tr( "Servos" ) );
    adapterList->header()->setLabel( 0, tr( "Adapter(s)" ) );
    scanBusBtn->setText( tr( "Sca&n Bus" ) );
    scanBusBtn->setAccel( QKeySequence( tr( "Alt+N" ) ) );
    servoList->header()->setLabel( 0, tr( "Servo(s)" ) );
}

void testMainWin::readServo()
{
    qWarning( "testMainWin::readServo(): Not implemented yet" );
}

void testMainWin::writeServo()
{
    qWarning( "testMainWin::writeServo(): Not implemented yet" );
}

void testMainWin::scanBus()
{
    qWarning( "testMainWin::scanBus(): Not implemented yet" );
}

void testMainWin::commandReboot()
{
    qWarning( "testMainWin::commandReboot(): Not implemented yet" );
}

void testMainWin::commandPWM()
{
    qWarning( "testMainWin::commandPWM(): Not implemented yet" );
}

void testMainWin::commandSave()
{
    qWarning( "testMainWin::commandSave(): Not implemented yet" );
}

void testMainWin::commandRestore()
{
    qWarning( "testMainWin::commandRestore(): Not implemented yet" );
}

void testMainWin::commandDefault()
{
    qWarning( "testMainWin::commandDefault(): Not implemented yet" );
}

void testMainWin::commandFlash()
{
    qWarning( "testMainWin::commandFlash(): Not implemented yet" );
}

void testMainWin::flashFileLoad()
{
    qWarning( "testMainWin::flashFileLoad(): Not implemented yet" );
}

void testMainWin::requestVoltage()
{
    qWarning( "testMainWin::requestVoltage(): Not implemented yet" );
}

void testMainWin::liveDataClick()
{
    qWarning( "testMainWin::liveDataClick(): Not implemented yet" );
}

void testMainWin::servoSelectChange(QListViewItem*)
{
    qWarning( "testMainWin::servoSelectChange(QListViewItem*): Not implemented yet" );
}

void testMainWin::timerIntervalChange(int)
{
    qWarning( "testMainWin::timerIntervalChange(int): Not implemented yet" );
}

void testMainWin::genericWriteData()
{
    qWarning( "testMainWin::genericWriteData(): Not implemented yet" );
}

void testMainWin::genericReadData()
{
    qWarning( "testMainWin::genericReadData(): Not implemented yet" );
}

void testMainWin::adapterSelectChange(QListViewItem*)
{
    qWarning( "testMainWin::adapterSelectChange(QListViewItem*): Not implemented yet" );
}

void testMainWin::logBoxClear(int,int)
{
    qWarning( "testMainWin::logBoxClear(int,int): Not implemented yet" );
}

void testMainWin::aboutClicked()
{
    qWarning( "testMainWin::aboutClicked(): Not implemented yet" );
}

