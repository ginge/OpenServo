/****************************************************************************
** Form interface generated from reading ui file 'mainwin.ui'
**
** Created: Fri Apr 20 01:09:10 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TESTMAINWIN_H
#define TESTMAINWIN_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QSpinBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QTextEdit;
class QCheckBox;
class QListView;
class QListViewItem;

class testMainWin : public QDialog
{
    Q_OBJECT

public:
    testMainWin( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~testMainWin();

    QGroupBox* groupBox5;
    QSpinBox* setPos;
    QGroupBox* groupBox2;
    QPushButton* cmdReboot;
    QPushButton* cmdPWM;
    QPushButton* cmdSave;
    QPushButton* cmdRestore;
    QPushButton* cmdDefault;
    QPushButton* cmdFlash;
    QPushButton* fileLoad;
    QGroupBox* groupBox3;
    QPushButton* voltageRequest;
    QLabel* textLabel2_2;
    QLabel* posLbl;
    QLabel* textLabel2_2_2;
    QLabel* pwmLbl;
    QLabel* textLabel2_3;
    QLabel* spdLbl;
    QLabel* textLabel2_3_2;
    QLabel* timLbl;
    QLabel* textLabel2_4;
    QLabel* curLbl;
    QLabel* textLabel2_4_2;
    QLabel* voltLbl;
    QGroupBox* groupBox1;
    QLabel* textLabel1_4;
    QLabel* textLabel1_4_2;
    QLineEdit* setupD;
    QLabel* textLabel1_4_2_2;
    QLineEdit* setupSMin;
    QLabel* textLabel1_2;
    QLineEdit* setupI;
    QLineEdit* setupP;
    QLabel* textLabel1;
    QLineEdit* setupAddr;
    QLabel* textLabel1_3;
    QLineEdit* setupSMax;
    QGroupBox* groupBox2_2;
    QLabel* textLabel3_2;
    QLineEdit* genericData;
    QLineEdit* genericRegister;
    QLineEdit* genericDevice;
    QLabel* textLabel3;
    QLabel* textLabel3_5;
    QPushButton* genericRead;
    QPushButton* genericWrite;
    QLabel* textLabel3_2_2;
    QLineEdit* genericLen;
    QLabel* textLabel3_4;
    QLabel* genericDataRead;
    QLabel* textLabel3_4_2;
    QTextEdit* logBox;
    QPushButton* writeBtn;
    QPushButton* buttonCancel;
    QCheckBox* liveData;
    QLabel* textLabel2;
    QPushButton* btnAbout;
    QSpinBox* timerIntervalBox;
    QPushButton* readBtn;
    QLabel* textLabel3_6;
    QGroupBox* groupBox4;
    QPushButton* scanBusBtn;
    QListView* otherDevList;
    QListView* servoList;
    QListView* adapterList;

public slots:
    virtual void readServo();
    virtual void writeServo();
    virtual void scanBus();
    virtual void commandReboot();
    virtual void commandPWM();
    virtual void commandSave();
    virtual void commandRestore();
    virtual void commandDefault();
    virtual void commandFlash();
    virtual void flashFileLoad();
    virtual void requestVoltage();
    virtual void liveDataClick();
    virtual void servoSelectChange(QListViewItem*selItem);
    virtual void timerIntervalChange(int);
    virtual void genericWriteData();
    virtual void genericReadData();
    virtual void adapterSelectChange(QListViewItem *selitem);
    virtual void logBoxClear(int,int);
    virtual void aboutClicked();

protected:
    QGridLayout* testMainWinLayout;
    QSpacerItem* Horizontal_Spacing2;
    QHBoxLayout* layout21;
    QSpacerItem* spacer3_2;
    QGridLayout* groupBox5Layout;
    QHBoxLayout* groupBox2Layout;
    QHBoxLayout* layout26;
    QGridLayout* groupBox3Layout;
    QSpacerItem* spacer3;
    QVBoxLayout* layout10;
    QHBoxLayout* layout9;
    QHBoxLayout* layout8;
    QHBoxLayout* layout7;
    QGridLayout* groupBox1Layout;
    QGridLayout* layout9_2;
    QGridLayout* groupBox2_2Layout;
    QGridLayout* layout24;
    QGridLayout* groupBox4Layout;

protected slots:
    virtual void languageChange();

};

#endif // TESTMAINWIN_H
