/********************************************************************************
** Form generated from reading UI file 'mainwin4.ui'
**
** Created: Sun Nov 14 13:44:07 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIN4_H
#define UI_MAINWIN4_H

#include <Qt3Support/Q3Frame>
#include <Qt3Support/Q3GroupBox>
#include <Qt3Support/Q3Header>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3MimeSourceFactory>
#include <Qt3Support/Q3TextEdit>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_testMainWin
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    Q3GroupBox *groupBox5;
    QGridLayout *gridLayout1;
    QSpinBox *setPos;
    QSpacerItem *spacer3_2;
    Q3GroupBox *groupBox2;
    QHBoxLayout *hboxLayout1;
    QPushButton *cmdReboot;
    QPushButton *cmdPWM;
    QPushButton *cmdSave;
    QPushButton *cmdRestore;
    QPushButton *cmdDefault;
    QPushButton *cmdFlash;
    QPushButton *fileLoad;
    QHBoxLayout *hboxLayout2;
    Q3GroupBox *groupBox3;
    QGridLayout *gridLayout2;
    QPushButton *voltageRequest;
    QSpacerItem *spacer3;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout3;
    QLabel *textLabel2_2;
    QLabel *posLbl;
    QLabel *textLabel2_2_2;
    QLabel *pwmLbl;
    QHBoxLayout *hboxLayout4;
    QLabel *textLabel2_3;
    QLabel *spdLbl;
    QLabel *textLabel2_3_2;
    QLabel *timLbl;
    QHBoxLayout *hboxLayout5;
    QLabel *textLabel2_4;
    QLabel *curLbl;
    QLabel *textLabel2_4_2;
    QLabel *voltLbl;
    Q3GroupBox *groupBox1;
    QGridLayout *gridLayout3;
    QGridLayout *gridLayout4;
    QLabel *textLabel1_4;
    QLabel *textLabel1_4_2;
    QLineEdit *setupD;
    QLabel *textLabel1_4_2_2;
    QLineEdit *setupSMin;
    QLabel *textLabel1_2;
    QLineEdit *setupI;
    QLabel *textLabel1;
    QLineEdit *setupAddr;
    QLabel *textLabel1_3;
    QLineEdit *setupP;
    QLineEdit *setupSMax;
    Q3GroupBox *groupBox2_2;
    QGridLayout *gridLayout5;
    QLineEdit *genericData;
    QLineEdit *genericRegister;
    QLineEdit *genericDevice;
    QLabel *textLabel3;
    QLabel *textLabel3_5;
    QGridLayout *gridLayout6;
    QPushButton *genericRead;
    QPushButton *genericWrite;
    QLabel *textLabel3_2_2;
    QLineEdit *genericLen;
    QLabel *textLabel3_4;
    QLabel *genericDataRead;
    QLabel *textLabel3_2;
    QLabel *textLabel3_4_2;
    QPushButton *writeBtn;
    QPushButton *buttonCancel;
    QSpacerItem *Horizontal_Spacing2;
    QCheckBox *liveData;
    QLabel *textLabel2;
    QPushButton *btnAbout;
    QSpinBox *timerIntervalBox;
    QPushButton *readBtn;
    QLabel *textLabel3_6;
    Q3GroupBox *groupBox4;
    QGridLayout *gridLayout7;
    QPushButton *scanBusBtn;
    Q3ListView *otherDevList;
    Q3ListView *servoList;
    Q3ListView *adapterList;
    Q3TextEdit *logBox;

    void setupUi(QDialog *testMainWin)
    {
        if (testMainWin->objectName().isEmpty())
            testMainWin->setObjectName(QString::fromUtf8("testMainWin"));
        testMainWin->resize(756, 735);
        testMainWin->setSizeGripEnabled(true);
        gridLayout = new QGridLayout(testMainWin);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        groupBox5 = new Q3GroupBox(testMainWin);
        groupBox5->setObjectName(QString::fromUtf8("groupBox5"));
        groupBox5->setColumnLayout(0, Qt::Vertical);
        groupBox5->layout()->setSpacing(6);
        groupBox5->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout1 = new QGridLayout();
        QBoxLayout *boxlayout = qobject_cast<QBoxLayout *>(groupBox5->layout());
        if (boxlayout)
            boxlayout->addLayout(gridLayout1);
        gridLayout1->setAlignment(Qt::AlignTop);
        gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
        setPos = new QSpinBox(groupBox5);
        setPos->setObjectName(QString::fromUtf8("setPos"));
        setPos->setMinimumSize(QSize(60, 0));
        setPos->setMaximumSize(QSize(200, 32767));
        setPos->setMaximum(1024);
        setPos->setSingleStep(10);
        setPos->setValue(400);

        gridLayout1->addWidget(setPos, 0, 0, 1, 1);


        hboxLayout->addWidget(groupBox5);

        spacer3_2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacer3_2);

        groupBox2 = new Q3GroupBox(testMainWin);
        groupBox2->setObjectName(QString::fromUtf8("groupBox2"));
        groupBox2->setColumnLayout(0, Qt::Vertical);
        groupBox2->layout()->setSpacing(6);
        groupBox2->layout()->setContentsMargins(11, 11, 11, 11);
        hboxLayout1 = new QHBoxLayout();
        QBoxLayout *boxlayout1 = qobject_cast<QBoxLayout *>(groupBox2->layout());
        if (boxlayout1)
            boxlayout1->addLayout(hboxLayout1);
        hboxLayout1->setAlignment(Qt::AlignTop);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        cmdReboot = new QPushButton(groupBox2);
        cmdReboot->setObjectName(QString::fromUtf8("cmdReboot"));

        hboxLayout1->addWidget(cmdReboot);

        cmdPWM = new QPushButton(groupBox2);
        cmdPWM->setObjectName(QString::fromUtf8("cmdPWM"));

        hboxLayout1->addWidget(cmdPWM);

        cmdSave = new QPushButton(groupBox2);
        cmdSave->setObjectName(QString::fromUtf8("cmdSave"));

        hboxLayout1->addWidget(cmdSave);

        cmdRestore = new QPushButton(groupBox2);
        cmdRestore->setObjectName(QString::fromUtf8("cmdRestore"));

        hboxLayout1->addWidget(cmdRestore);

        cmdDefault = new QPushButton(groupBox2);
        cmdDefault->setObjectName(QString::fromUtf8("cmdDefault"));

        hboxLayout1->addWidget(cmdDefault);

        cmdFlash = new QPushButton(groupBox2);
        cmdFlash->setObjectName(QString::fromUtf8("cmdFlash"));
        cmdFlash->setEnabled(false);
        cmdFlash->setMaximumSize(QSize(50, 32767));

        hboxLayout1->addWidget(cmdFlash);

        fileLoad = new QPushButton(groupBox2);
        fileLoad->setObjectName(QString::fromUtf8("fileLoad"));

        hboxLayout1->addWidget(fileLoad);


        hboxLayout->addWidget(groupBox2);


        gridLayout->addLayout(hboxLayout, 1, 1, 1, 10);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        groupBox3 = new Q3GroupBox(testMainWin);
        groupBox3->setObjectName(QString::fromUtf8("groupBox3"));
        groupBox3->setColumnLayout(0, Qt::Vertical);
        groupBox3->layout()->setSpacing(6);
        groupBox3->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout2 = new QGridLayout();
        QBoxLayout *boxlayout2 = qobject_cast<QBoxLayout *>(groupBox3->layout());
        if (boxlayout2)
            boxlayout2->addLayout(gridLayout2);
        gridLayout2->setAlignment(Qt::AlignTop);
        gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
        voltageRequest = new QPushButton(groupBox3);
        voltageRequest->setObjectName(QString::fromUtf8("voltageRequest"));

        gridLayout2->addWidget(voltageRequest, 1, 1, 1, 1);

        spacer3 = new QSpacerItem(20, 37, QSizePolicy::Minimum, QSizePolicy::Preferred);

        gridLayout2->addItem(spacer3, 0, 1, 1, 1);

        vboxLayout = new QVBoxLayout();
        vboxLayout->setSpacing(6);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        textLabel2_2 = new QLabel(groupBox3);
        textLabel2_2->setObjectName(QString::fromUtf8("textLabel2_2"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(textLabel2_2->sizePolicy().hasHeightForWidth());
        textLabel2_2->setSizePolicy(sizePolicy);
        textLabel2_2->setMinimumSize(QSize(60, 0));
        textLabel2_2->setMaximumSize(QSize(60, 16777215));
        textLabel2_2->setWordWrap(false);

        hboxLayout3->addWidget(textLabel2_2);

        posLbl = new QLabel(groupBox3);
        posLbl->setObjectName(QString::fromUtf8("posLbl"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(posLbl->sizePolicy().hasHeightForWidth());
        posLbl->setSizePolicy(sizePolicy1);
        posLbl->setMinimumSize(QSize(80, 0));
        posLbl->setWordWrap(false);

        hboxLayout3->addWidget(posLbl);

        textLabel2_2_2 = new QLabel(groupBox3);
        textLabel2_2_2->setObjectName(QString::fromUtf8("textLabel2_2_2"));
        sizePolicy.setHeightForWidth(textLabel2_2_2->sizePolicy().hasHeightForWidth());
        textLabel2_2_2->setSizePolicy(sizePolicy);
        textLabel2_2_2->setMinimumSize(QSize(60, 0));
        textLabel2_2_2->setMaximumSize(QSize(60, 16777215));
        textLabel2_2_2->setWordWrap(false);

        hboxLayout3->addWidget(textLabel2_2_2);

        pwmLbl = new QLabel(groupBox3);
        pwmLbl->setObjectName(QString::fromUtf8("pwmLbl"));
        sizePolicy1.setHeightForWidth(pwmLbl->sizePolicy().hasHeightForWidth());
        pwmLbl->setSizePolicy(sizePolicy1);
        pwmLbl->setMinimumSize(QSize(80, 0));
        pwmLbl->setWordWrap(false);

        hboxLayout3->addWidget(pwmLbl);


        vboxLayout->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        textLabel2_3 = new QLabel(groupBox3);
        textLabel2_3->setObjectName(QString::fromUtf8("textLabel2_3"));
        sizePolicy.setHeightForWidth(textLabel2_3->sizePolicy().hasHeightForWidth());
        textLabel2_3->setSizePolicy(sizePolicy);
        textLabel2_3->setMinimumSize(QSize(60, 0));
        textLabel2_3->setMaximumSize(QSize(60, 16777215));
        textLabel2_3->setWordWrap(false);

        hboxLayout4->addWidget(textLabel2_3);

        spdLbl = new QLabel(groupBox3);
        spdLbl->setObjectName(QString::fromUtf8("spdLbl"));
        sizePolicy1.setHeightForWidth(spdLbl->sizePolicy().hasHeightForWidth());
        spdLbl->setSizePolicy(sizePolicy1);
        spdLbl->setMinimumSize(QSize(80, 0));
        spdLbl->setWordWrap(false);

        hboxLayout4->addWidget(spdLbl);

        textLabel2_3_2 = new QLabel(groupBox3);
        textLabel2_3_2->setObjectName(QString::fromUtf8("textLabel2_3_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(60);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(textLabel2_3_2->sizePolicy().hasHeightForWidth());
        textLabel2_3_2->setSizePolicy(sizePolicy2);
        textLabel2_3_2->setMinimumSize(QSize(60, 0));
        textLabel2_3_2->setWordWrap(false);

        hboxLayout4->addWidget(textLabel2_3_2);

        timLbl = new QLabel(groupBox3);
        timLbl->setObjectName(QString::fromUtf8("timLbl"));
        sizePolicy1.setHeightForWidth(timLbl->sizePolicy().hasHeightForWidth());
        timLbl->setSizePolicy(sizePolicy1);
        timLbl->setMinimumSize(QSize(80, 0));
        timLbl->setWordWrap(false);

        hboxLayout4->addWidget(timLbl);


        vboxLayout->addLayout(hboxLayout4);

        hboxLayout5 = new QHBoxLayout();
        hboxLayout5->setSpacing(6);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        textLabel2_4 = new QLabel(groupBox3);
        textLabel2_4->setObjectName(QString::fromUtf8("textLabel2_4"));
        sizePolicy.setHeightForWidth(textLabel2_4->sizePolicy().hasHeightForWidth());
        textLabel2_4->setSizePolicy(sizePolicy);
        textLabel2_4->setMinimumSize(QSize(60, 0));
        textLabel2_4->setMaximumSize(QSize(60, 16777215));
        textLabel2_4->setWordWrap(false);

        hboxLayout5->addWidget(textLabel2_4);

        curLbl = new QLabel(groupBox3);
        curLbl->setObjectName(QString::fromUtf8("curLbl"));
        sizePolicy1.setHeightForWidth(curLbl->sizePolicy().hasHeightForWidth());
        curLbl->setSizePolicy(sizePolicy1);
        curLbl->setMinimumSize(QSize(80, 0));
        curLbl->setWordWrap(false);

        hboxLayout5->addWidget(curLbl);

        textLabel2_4_2 = new QLabel(groupBox3);
        textLabel2_4_2->setObjectName(QString::fromUtf8("textLabel2_4_2"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(60);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(textLabel2_4_2->sizePolicy().hasHeightForWidth());
        textLabel2_4_2->setSizePolicy(sizePolicy3);
        textLabel2_4_2->setMinimumSize(QSize(60, 0));
        textLabel2_4_2->setWordWrap(false);

        hboxLayout5->addWidget(textLabel2_4_2);

        voltLbl = new QLabel(groupBox3);
        voltLbl->setObjectName(QString::fromUtf8("voltLbl"));
        sizePolicy1.setHeightForWidth(voltLbl->sizePolicy().hasHeightForWidth());
        voltLbl->setSizePolicy(sizePolicy1);
        voltLbl->setMinimumSize(QSize(80, 0));
        voltLbl->setWordWrap(false);

        hboxLayout5->addWidget(voltLbl);


        vboxLayout->addLayout(hboxLayout5);


        gridLayout2->addLayout(vboxLayout, 0, 0, 2, 1);


        hboxLayout2->addWidget(groupBox3);

        groupBox1 = new Q3GroupBox(testMainWin);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        groupBox1->setColumnLayout(0, Qt::Vertical);
        groupBox1->layout()->setSpacing(6);
        groupBox1->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout3 = new QGridLayout();
        QBoxLayout *boxlayout3 = qobject_cast<QBoxLayout *>(groupBox1->layout());
        if (boxlayout3)
            boxlayout3->addLayout(gridLayout3);
        gridLayout3->setAlignment(Qt::AlignTop);
        gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
        gridLayout4 = new QGridLayout();
        gridLayout4->setSpacing(0);
        gridLayout4->setContentsMargins(0, 0, 0, 0);
        gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
        textLabel1_4 = new QLabel(groupBox1);
        textLabel1_4->setObjectName(QString::fromUtf8("textLabel1_4"));
        textLabel1_4->setMinimumSize(QSize(36, 0));
        textLabel1_4->setMaximumSize(QSize(36, 16777215));
        textLabel1_4->setWordWrap(false);

        gridLayout4->addWidget(textLabel1_4, 0, 3, 1, 2);

        textLabel1_4_2 = new QLabel(groupBox1);
        textLabel1_4_2->setObjectName(QString::fromUtf8("textLabel1_4_2"));
        textLabel1_4_2->setMinimumSize(QSize(36, 0));
        textLabel1_4_2->setMaximumSize(QSize(36, 16777215));
        textLabel1_4_2->setWordWrap(false);

        gridLayout4->addWidget(textLabel1_4_2, 1, 3, 1, 1);

        setupD = new QLineEdit(groupBox1);
        setupD->setObjectName(QString::fromUtf8("setupD"));
        sizePolicy.setHeightForWidth(setupD->sizePolicy().hasHeightForWidth());
        setupD->setSizePolicy(sizePolicy);
        setupD->setMinimumSize(QSize(63, 0));
        setupD->setMaximumSize(QSize(63, 16777215));

        gridLayout4->addWidget(setupD, 2, 2, 1, 1);

        textLabel1_4_2_2 = new QLabel(groupBox1);
        textLabel1_4_2_2->setObjectName(QString::fromUtf8("textLabel1_4_2_2"));
        sizePolicy.setHeightForWidth(textLabel1_4_2_2->sizePolicy().hasHeightForWidth());
        textLabel1_4_2_2->setSizePolicy(sizePolicy);
        textLabel1_4_2_2->setMinimumSize(QSize(36, 0));
        textLabel1_4_2_2->setMaximumSize(QSize(36, 16777215));
        textLabel1_4_2_2->setWordWrap(false);

        gridLayout4->addWidget(textLabel1_4_2_2, 2, 3, 1, 1);

        setupSMin = new QLineEdit(groupBox1);
        setupSMin->setObjectName(QString::fromUtf8("setupSMin"));
        sizePolicy.setHeightForWidth(setupSMin->sizePolicy().hasHeightForWidth());
        setupSMin->setSizePolicy(sizePolicy);
        setupSMin->setMinimumSize(QSize(63, 0));
        setupSMin->setMaximumSize(QSize(63, 16777215));

        gridLayout4->addWidget(setupSMin, 1, 5, 1, 1);

        textLabel1_2 = new QLabel(groupBox1);
        textLabel1_2->setObjectName(QString::fromUtf8("textLabel1_2"));
        sizePolicy.setHeightForWidth(textLabel1_2->sizePolicy().hasHeightForWidth());
        textLabel1_2->setSizePolicy(sizePolicy);
        textLabel1_2->setMinimumSize(QSize(30, 0));
        textLabel1_2->setMaximumSize(QSize(30, 16777215));
        textLabel1_2->setWordWrap(false);

        gridLayout4->addWidget(textLabel1_2, 1, 0, 1, 1);

        setupI = new QLineEdit(groupBox1);
        setupI->setObjectName(QString::fromUtf8("setupI"));
        sizePolicy.setHeightForWidth(setupI->sizePolicy().hasHeightForWidth());
        setupI->setSizePolicy(sizePolicy);
        setupI->setMinimumSize(QSize(63, 0));
        setupI->setMaximumSize(QSize(63, 16777215));

        gridLayout4->addWidget(setupI, 1, 1, 1, 2);

        textLabel1 = new QLabel(groupBox1);
        textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
        sizePolicy.setHeightForWidth(textLabel1->sizePolicy().hasHeightForWidth());
        textLabel1->setSizePolicy(sizePolicy);
        textLabel1->setMinimumSize(QSize(30, 0));
        textLabel1->setMaximumSize(QSize(30, 16777215));
        textLabel1->setWordWrap(false);

        gridLayout4->addWidget(textLabel1, 0, 0, 1, 2);

        setupAddr = new QLineEdit(groupBox1);
        setupAddr->setObjectName(QString::fromUtf8("setupAddr"));
        sizePolicy.setHeightForWidth(setupAddr->sizePolicy().hasHeightForWidth());
        setupAddr->setSizePolicy(sizePolicy);
        setupAddr->setMinimumSize(QSize(45, 0));
        setupAddr->setMaximumSize(QSize(45, 16777215));

        gridLayout4->addWidget(setupAddr, 2, 4, 1, 2);

        textLabel1_3 = new QLabel(groupBox1);
        textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
        textLabel1_3->setMinimumSize(QSize(30, 0));
        textLabel1_3->setMaximumSize(QSize(30, 16777215));
        textLabel1_3->setWordWrap(false);

        gridLayout4->addWidget(textLabel1_3, 2, 0, 1, 2);

        setupP = new QLineEdit(groupBox1);
        setupP->setObjectName(QString::fromUtf8("setupP"));
        sizePolicy.setHeightForWidth(setupP->sizePolicy().hasHeightForWidth());
        setupP->setSizePolicy(sizePolicy);
        setupP->setMinimumSize(QSize(63, 0));
        setupP->setMaximumSize(QSize(63, 16777215));

        gridLayout4->addWidget(setupP, 0, 2, 1, 1);

        setupSMax = new QLineEdit(groupBox1);
        setupSMax->setObjectName(QString::fromUtf8("setupSMax"));
        sizePolicy.setHeightForWidth(setupSMax->sizePolicy().hasHeightForWidth());
        setupSMax->setSizePolicy(sizePolicy);
        setupSMax->setMinimumSize(QSize(63, 0));
        setupSMax->setMaximumSize(QSize(63, 16777215));

        gridLayout4->addWidget(setupSMax, 0, 5, 1, 1);


        gridLayout3->addLayout(gridLayout4, 0, 0, 1, 1);


        hboxLayout2->addWidget(groupBox1);


        gridLayout->addLayout(hboxLayout2, 0, 1, 1, 10);

        groupBox2_2 = new Q3GroupBox(testMainWin);
        groupBox2_2->setObjectName(QString::fromUtf8("groupBox2_2"));
        groupBox2_2->setColumnLayout(0, Qt::Vertical);
        groupBox2_2->layout()->setSpacing(6);
        groupBox2_2->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout5 = new QGridLayout();
        QBoxLayout *boxlayout4 = qobject_cast<QBoxLayout *>(groupBox2_2->layout());
        if (boxlayout4)
            boxlayout4->addLayout(gridLayout5);
        gridLayout5->setAlignment(Qt::AlignTop);
        gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
        genericData = new QLineEdit(groupBox2_2);
        genericData->setObjectName(QString::fromUtf8("genericData"));

        gridLayout5->addWidget(genericData, 0, 7, 1, 1);

        genericRegister = new QLineEdit(groupBox2_2);
        genericRegister->setObjectName(QString::fromUtf8("genericRegister"));

        gridLayout5->addWidget(genericRegister, 0, 3, 1, 2);

        genericDevice = new QLineEdit(groupBox2_2);
        genericDevice->setObjectName(QString::fromUtf8("genericDevice"));

        gridLayout5->addWidget(genericDevice, 0, 1, 1, 1);

        textLabel3 = new QLabel(groupBox2_2);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        textLabel3->setWordWrap(false);

        gridLayout5->addWidget(textLabel3, 0, 2, 1, 1);

        textLabel3_5 = new QLabel(groupBox2_2);
        textLabel3_5->setObjectName(QString::fromUtf8("textLabel3_5"));
        textLabel3_5->setWordWrap(false);

        gridLayout5->addWidget(textLabel3_5, 0, 0, 1, 1);

        gridLayout6 = new QGridLayout();
        gridLayout6->setSpacing(6);
        gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
        genericRead = new QPushButton(groupBox2_2);
        genericRead->setObjectName(QString::fromUtf8("genericRead"));

        gridLayout6->addWidget(genericRead, 1, 1, 1, 1);

        genericWrite = new QPushButton(groupBox2_2);
        genericWrite->setObjectName(QString::fromUtf8("genericWrite"));

        gridLayout6->addWidget(genericWrite, 0, 1, 1, 1);


        gridLayout5->addLayout(gridLayout6, 0, 8, 2, 1);

        textLabel3_2_2 = new QLabel(groupBox2_2);
        textLabel3_2_2->setObjectName(QString::fromUtf8("textLabel3_2_2"));
        textLabel3_2_2->setWordWrap(false);

        gridLayout5->addWidget(textLabel3_2_2, 1, 0, 1, 1);

        genericLen = new QLineEdit(groupBox2_2);
        genericLen->setObjectName(QString::fromUtf8("genericLen"));

        gridLayout5->addWidget(genericLen, 1, 1, 1, 1);

        textLabel3_4 = new QLabel(groupBox2_2);
        textLabel3_4->setObjectName(QString::fromUtf8("textLabel3_4"));
        textLabel3_4->setWordWrap(false);

        gridLayout5->addWidget(textLabel3_4, 1, 2, 1, 2);

        genericDataRead = new QLabel(groupBox2_2);
        genericDataRead->setObjectName(QString::fromUtf8("genericDataRead"));
        genericDataRead->setWordWrap(false);

        gridLayout5->addWidget(genericDataRead, 1, 4, 1, 4);

        textLabel3_2 = new QLabel(groupBox2_2);
        textLabel3_2->setObjectName(QString::fromUtf8("textLabel3_2"));
        textLabel3_2->setWordWrap(false);

        gridLayout5->addWidget(textLabel3_2, 0, 5, 1, 1);


        gridLayout->addWidget(groupBox2_2, 2, 1, 1, 10);

        textLabel3_4_2 = new QLabel(testMainWin);
        textLabel3_4_2->setObjectName(QString::fromUtf8("textLabel3_4_2"));
        textLabel3_4_2->setWordWrap(false);

        gridLayout->addWidget(textLabel3_4_2, 3, 1, 1, 1);

        writeBtn = new QPushButton(testMainWin);
        writeBtn->setObjectName(QString::fromUtf8("writeBtn"));
        writeBtn->setEnabled(false);
        writeBtn->setMaximumSize(QSize(55, 32767));

        gridLayout->addWidget(writeBtn, 4, 3, 1, 1);

        buttonCancel = new QPushButton(testMainWin);
        buttonCancel->setObjectName(QString::fromUtf8("buttonCancel"));
        buttonCancel->setMaximumSize(QSize(55, 32767));
        buttonCancel->setAutoDefault(true);

        gridLayout->addWidget(buttonCancel, 4, 10, 1, 1);

        Horizontal_Spacing2 = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(Horizontal_Spacing2, 4, 8, 1, 1);

        liveData = new QCheckBox(testMainWin);
        liveData->setObjectName(QString::fromUtf8("liveData"));
        liveData->setEnabled(false);

        gridLayout->addWidget(liveData, 4, 4, 1, 1);

        textLabel2 = new QLabel(testMainWin);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        textLabel2->setWordWrap(false);

        gridLayout->addWidget(textLabel2, 4, 5, 1, 1);

        btnAbout = new QPushButton(testMainWin);
        btnAbout->setObjectName(QString::fromUtf8("btnAbout"));
        btnAbout->setMaximumSize(QSize(55, 32767));
        btnAbout->setAutoDefault(true);

        gridLayout->addWidget(btnAbout, 4, 9, 1, 1);

        timerIntervalBox = new QSpinBox(testMainWin);
        timerIntervalBox->setObjectName(QString::fromUtf8("timerIntervalBox"));
        timerIntervalBox->setEnabled(false);
        timerIntervalBox->setMinimum(20);
        timerIntervalBox->setValue(30);

        gridLayout->addWidget(timerIntervalBox, 4, 6, 1, 1);

        readBtn = new QPushButton(testMainWin);
        readBtn->setObjectName(QString::fromUtf8("readBtn"));
        readBtn->setEnabled(false);
        readBtn->setMaximumSize(QSize(55, 32767));

        gridLayout->addWidget(readBtn, 4, 1, 1, 2);

        textLabel3_6 = new QLabel(testMainWin);
        textLabel3_6->setObjectName(QString::fromUtf8("textLabel3_6"));
        textLabel3_6->setWordWrap(false);

        gridLayout->addWidget(textLabel3_6, 4, 7, 1, 1);

        groupBox4 = new Q3GroupBox(testMainWin);
        groupBox4->setObjectName(QString::fromUtf8("groupBox4"));
        groupBox4->setColumnLayout(0, Qt::Vertical);
        groupBox4->layout()->setSpacing(6);
        groupBox4->layout()->setContentsMargins(11, 11, 11, 11);
        gridLayout7 = new QGridLayout();
        QBoxLayout *boxlayout5 = qobject_cast<QBoxLayout *>(groupBox4->layout());
        if (boxlayout5)
            boxlayout5->addLayout(gridLayout7);
        gridLayout7->setAlignment(Qt::AlignTop);
        gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
        scanBusBtn = new QPushButton(groupBox4);
        scanBusBtn->setObjectName(QString::fromUtf8("scanBusBtn"));
        scanBusBtn->setAutoDefault(true);

        gridLayout7->addWidget(scanBusBtn, 3, 0, 1, 1);

        otherDevList = new Q3ListView(groupBox4);
        otherDevList->addColumn(QApplication::translate("testMainWin", "Other", 0, QApplication::UnicodeUTF8));
        otherDevList->header()->setClickEnabled(false, otherDevList->header()->count() - 1);
        otherDevList->header()->setResizeEnabled(false, otherDevList->header()->count() - 1);
        otherDevList->setObjectName(QString::fromUtf8("otherDevList"));
        otherDevList->setVScrollBarMode(Q3ScrollView::Auto);
        otherDevList->setHScrollBarMode(Q3ScrollView::AlwaysOff);

        gridLayout7->addWidget(otherDevList, 2, 0, 1, 1);

        servoList = new Q3ListView(groupBox4);
        servoList->addColumn(QApplication::translate("testMainWin", "Servo(s)", 0, QApplication::UnicodeUTF8));
        servoList->header()->setClickEnabled(true, servoList->header()->count() - 1);
        servoList->header()->setResizeEnabled(false, servoList->header()->count() - 1);
        servoList->setObjectName(QString::fromUtf8("servoList"));

        gridLayout7->addWidget(servoList, 1, 0, 1, 1);

        adapterList = new Q3ListView(groupBox4);
        adapterList->addColumn(QApplication::translate("testMainWin", "Adapter(s)", 0, QApplication::UnicodeUTF8));
        adapterList->header()->setClickEnabled(true, adapterList->header()->count() - 1);
        adapterList->header()->setResizeEnabled(false, adapterList->header()->count() - 1);
        adapterList->setObjectName(QString::fromUtf8("adapterList"));
        adapterList->setVScrollBarMode(Q3ScrollView::Auto);
        adapterList->setHScrollBarMode(Q3ScrollView::AlwaysOff);

        gridLayout7->addWidget(adapterList, 0, 0, 1, 1);


        gridLayout->addWidget(groupBox4, 0, 0, 5, 1);

        logBox = new Q3TextEdit(testMainWin);
        logBox->setObjectName(QString::fromUtf8("logBox"));
        logBox->setDragAutoScroll(true);
        logBox->setTextFormat(Qt::LogText);
        logBox->setWordWrap(Q3TextEdit::WidgetWidth);
        logBox->setReadOnly(true);
        logBox->setAutoFormatting(Q3TextEdit::AutoAll);

        gridLayout->addWidget(logBox, 3, 2, 1, 9);

        QWidget::setTabOrder(setPos, writeBtn);
        QWidget::setTabOrder(writeBtn, readBtn);
        QWidget::setTabOrder(readBtn, cmdReboot);
        QWidget::setTabOrder(cmdReboot, cmdPWM);
        QWidget::setTabOrder(cmdPWM, cmdSave);
        QWidget::setTabOrder(cmdSave, cmdRestore);
        QWidget::setTabOrder(cmdRestore, cmdDefault);
        QWidget::setTabOrder(cmdDefault, cmdFlash);
        QWidget::setTabOrder(cmdFlash, fileLoad);
        QWidget::setTabOrder(fileLoad, voltageRequest);
        QWidget::setTabOrder(voltageRequest, setupP);
        QWidget::setTabOrder(setupP, setupI);
        QWidget::setTabOrder(setupI, setupD);
        QWidget::setTabOrder(setupD, setupSMax);
        QWidget::setTabOrder(setupSMax, setupSMin);
        QWidget::setTabOrder(setupSMin, setupAddr);
        QWidget::setTabOrder(setupAddr, genericDevice);
        QWidget::setTabOrder(genericDevice, genericRegister);
        QWidget::setTabOrder(genericRegister, genericData);
        QWidget::setTabOrder(genericData, genericWrite);
        QWidget::setTabOrder(genericWrite, genericLen);
        QWidget::setTabOrder(genericLen, genericRead);
        QWidget::setTabOrder(genericRead, liveData);
        QWidget::setTabOrder(liveData, timerIntervalBox);
        QWidget::setTabOrder(timerIntervalBox, adapterList);
        QWidget::setTabOrder(adapterList, servoList);
        QWidget::setTabOrder(servoList, scanBusBtn);
        QWidget::setTabOrder(scanBusBtn, logBox);
        QWidget::setTabOrder(logBox, btnAbout);
        QWidget::setTabOrder(btnAbout, buttonCancel);

        retranslateUi(testMainWin);
        QObject::connect(buttonCancel, SIGNAL(clicked()), testMainWin, SLOT(reject()));
        QObject::connect(readBtn, SIGNAL(clicked()), testMainWin, SLOT(readServo()));
        QObject::connect(writeBtn, SIGNAL(clicked()), testMainWin, SLOT(writeServo()));
        QObject::connect(scanBusBtn, SIGNAL(clicked()), testMainWin, SLOT(scanBus()));
        QObject::connect(cmdReboot, SIGNAL(clicked()), testMainWin, SLOT(commandReboot()));
        QObject::connect(cmdPWM, SIGNAL(clicked()), testMainWin, SLOT(commandPWM()));
        QObject::connect(cmdDefault, SIGNAL(clicked()), testMainWin, SLOT(commandDefault()));
        QObject::connect(cmdRestore, SIGNAL(clicked()), testMainWin, SLOT(commandRestore()));
        QObject::connect(cmdSave, SIGNAL(clicked()), testMainWin, SLOT(commandSave()));
        QObject::connect(cmdFlash, SIGNAL(clicked()), testMainWin, SLOT(commandFlash()));
        QObject::connect(fileLoad, SIGNAL(clicked()), testMainWin, SLOT(flashFileLoad()));
        QObject::connect(voltageRequest, SIGNAL(clicked()), testMainWin, SLOT(requestVoltage()));
        QObject::connect(liveData, SIGNAL(toggled(bool)), testMainWin, SLOT(liveDataClick()));
        QObject::connect(servoList, SIGNAL(selectionChanged(Q3ListViewItem*)), testMainWin, SLOT(servoSelectChange(Q3ListViewItem*)));
        QObject::connect(timerIntervalBox, SIGNAL(valueChanged(int)), testMainWin, SLOT(timerIntervalChange(int)));
        QObject::connect(genericRead, SIGNAL(clicked()), testMainWin, SLOT(genericReadData()));
        QObject::connect(genericWrite, SIGNAL(clicked()), testMainWin, SLOT(genericWriteData()));
        QObject::connect(adapterList, SIGNAL(selectionChanged(Q3ListViewItem*)), testMainWin, SLOT(adapterSelectChange(Q3ListViewItem*)));
        QObject::connect(logBox, SIGNAL(doubleClicked(int,int)), testMainWin, SLOT(logBoxClear(int,int)));
        QObject::connect(btnAbout, SIGNAL(clicked()), testMainWin, SLOT(aboutClicked()));

        QMetaObject::connectSlotsByName(testMainWin);
    } // setupUi

    void retranslateUi(QDialog *testMainWin)
    {
        testMainWin->setWindowTitle(QApplication::translate("testMainWin", "OpenServo Test", 0, QApplication::UnicodeUTF8));
        groupBox5->setTitle(QApplication::translate("testMainWin", "Position", 0, QApplication::UnicodeUTF8));
        groupBox2->setTitle(QApplication::translate("testMainWin", "Servo Command", 0, QApplication::UnicodeUTF8));
        cmdReboot->setText(QApplication::translate("testMainWin", "Re&boot", 0, QApplication::UnicodeUTF8));
        cmdReboot->setShortcut(QApplication::translate("testMainWin", "Alt+B", 0, QApplication::UnicodeUTF8));
        cmdPWM->setText(QApplication::translate("testMainWin", "PWM &off", 0, QApplication::UnicodeUTF8));
        cmdPWM->setShortcut(QApplication::translate("testMainWin", "Alt+O", 0, QApplication::UnicodeUTF8));
        cmdSave->setText(QApplication::translate("testMainWin", "S&ave", 0, QApplication::UnicodeUTF8));
        cmdSave->setShortcut(QApplication::translate("testMainWin", "Alt+A", 0, QApplication::UnicodeUTF8));
        cmdRestore->setText(QApplication::translate("testMainWin", "Restore", 0, QApplication::UnicodeUTF8));
        cmdRestore->setShortcut(QString());
        cmdDefault->setText(QApplication::translate("testMainWin", "Default", 0, QApplication::UnicodeUTF8));
        cmdDefault->setShortcut(QString());
        cmdFlash->setText(QApplication::translate("testMainWin", "Flash", 0, QApplication::UnicodeUTF8));
        cmdFlash->setShortcut(QString());
#ifndef QT_NO_TOOLTIP
        fileLoad->setToolTip(QApplication::translate("testMainWin", "Load the file to flash", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        fileLoad->setText(QApplication::translate("testMainWin", "...", 0, QApplication::UnicodeUTF8));
        fileLoad->setShortcut(QString());
        groupBox3->setTitle(QApplication::translate("testMainWin", "Servo Output", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        voltageRequest->setToolTip(QApplication::translate("testMainWin", "Request new Voltage sample", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        voltageRequest->setText(QApplication::translate("testMainWin", "V", 0, QApplication::UnicodeUTF8));
        textLabel2_2->setText(QApplication::translate("testMainWin", "Position", 0, QApplication::UnicodeUTF8));
        posLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel2_2_2->setText(QApplication::translate("testMainWin", "PWM", 0, QApplication::UnicodeUTF8));
        pwmLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel2_3->setText(QApplication::translate("testMainWin", "Speed", 0, QApplication::UnicodeUTF8));
        spdLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel2_3_2->setText(QApplication::translate("testMainWin", "Timer", 0, QApplication::UnicodeUTF8));
        timLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel2_4->setText(QApplication::translate("testMainWin", "Current", 0, QApplication::UnicodeUTF8));
        curLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel2_4_2->setText(QApplication::translate("testMainWin", "Voltage", 0, QApplication::UnicodeUTF8));
        voltLbl->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        groupBox1->setTitle(QApplication::translate("testMainWin", "Servo Setup", 0, QApplication::UnicodeUTF8));
        textLabel1_4->setText(QApplication::translate("testMainWin", "SMax", 0, QApplication::UnicodeUTF8));
        textLabel1_4_2->setText(QApplication::translate("testMainWin", "SMin", 0, QApplication::UnicodeUTF8));
        setupD->setText(QApplication::translate("testMainWin", "0x0000", 0, QApplication::UnicodeUTF8));
        textLabel1_4_2_2->setText(QApplication::translate("testMainWin", "Addr", 0, QApplication::UnicodeUTF8));
        setupSMin->setText(QApplication::translate("testMainWin", "0x0000", 0, QApplication::UnicodeUTF8));
        textLabel1_2->setText(QApplication::translate("testMainWin", "I", 0, QApplication::UnicodeUTF8));
        setupI->setText(QApplication::translate("testMainWin", "0x0000", 0, QApplication::UnicodeUTF8));
        textLabel1->setText(QApplication::translate("testMainWin", "P", 0, QApplication::UnicodeUTF8));
        setupAddr->setText(QApplication::translate("testMainWin", "0x00", 0, QApplication::UnicodeUTF8));
        textLabel1_3->setText(QApplication::translate("testMainWin", "D", 0, QApplication::UnicodeUTF8));
        setupP->setText(QApplication::translate("testMainWin", "0x0000", 0, QApplication::UnicodeUTF8));
        setupSMax->setText(QApplication::translate("testMainWin", "0x0000", 0, QApplication::UnicodeUTF8));
        groupBox2_2->setTitle(QApplication::translate("testMainWin", "Generic comms", 0, QApplication::UnicodeUTF8));
        genericData->setText(QApplication::translate("testMainWin", "0x00 0x00 0x00 0x00", 0, QApplication::UnicodeUTF8));
        genericRegister->setText(QApplication::translate("testMainWin", "0x00", 0, QApplication::UnicodeUTF8));
        genericDevice->setText(QApplication::translate("testMainWin", "0x00", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("testMainWin", "Register", 0, QApplication::UnicodeUTF8));
        textLabel3_5->setText(QApplication::translate("testMainWin", "device", 0, QApplication::UnicodeUTF8));
        genericRead->setText(QApplication::translate("testMainWin", "Read", 0, QApplication::UnicodeUTF8));
        genericRead->setShortcut(QString());
        genericWrite->setText(QApplication::translate("testMainWin", "Write", 0, QApplication::UnicodeUTF8));
        genericWrite->setShortcut(QString());
        textLabel3_2_2->setText(QApplication::translate("testMainWin", "length", 0, QApplication::UnicodeUTF8));
        genericLen->setText(QApplication::translate("testMainWin", "0", 0, QApplication::UnicodeUTF8));
        textLabel3_4->setText(QApplication::translate("testMainWin", "Data Read", 0, QApplication::UnicodeUTF8));
        genericDataRead->setText(QString());
        textLabel3_2->setText(QApplication::translate("testMainWin", "Data", 0, QApplication::UnicodeUTF8));
        textLabel3_4_2->setText(QApplication::translate("testMainWin", "Log", 0, QApplication::UnicodeUTF8));
        writeBtn->setText(QApplication::translate("testMainWin", "Write", 0, QApplication::UnicodeUTF8));
        writeBtn->setShortcut(QString());
        buttonCancel->setText(QApplication::translate("testMainWin", "&Quit", 0, QApplication::UnicodeUTF8));
        buttonCancel->setShortcut(QApplication::translate("testMainWin", "Alt+Q", 0, QApplication::UnicodeUTF8));
        liveData->setText(QApplication::translate("testMainWin", "&Live data", 0, QApplication::UnicodeUTF8));
        liveData->setShortcut(QApplication::translate("testMainWin", "Alt+L", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("testMainWin", "Freq", 0, QApplication::UnicodeUTF8));
        btnAbout->setText(QApplication::translate("testMainWin", "About", 0, QApplication::UnicodeUTF8));
        btnAbout->setShortcut(QString());
        readBtn->setText(QApplication::translate("testMainWin", "Read", 0, QApplication::UnicodeUTF8));
        readBtn->setShortcut(QString());
        textLabel3_6->setText(QApplication::translate("testMainWin", "ms", 0, QApplication::UnicodeUTF8));
        groupBox4->setTitle(QApplication::translate("testMainWin", "Servos", 0, QApplication::UnicodeUTF8));
        scanBusBtn->setText(QApplication::translate("testMainWin", "Sca&n Bus", 0, QApplication::UnicodeUTF8));
        scanBusBtn->setShortcut(QApplication::translate("testMainWin", "Alt+N", 0, QApplication::UnicodeUTF8));
        otherDevList->header()->setLabel(0, QApplication::translate("testMainWin", "Other", 0, QApplication::UnicodeUTF8));
        servoList->header()->setLabel(0, QApplication::translate("testMainWin", "Servo(s)", 0, QApplication::UnicodeUTF8));
        adapterList->header()->setLabel(0, QApplication::translate("testMainWin", "Adapter(s)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        logBox->setToolTip(QApplication::translate("testMainWin", "Log output. Double click to clear", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        logBox->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class testMainWin: public Ui_testMainWin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIN4_H
