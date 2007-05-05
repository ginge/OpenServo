/****************************************************************************
** Form interface generated from reading ui file 'bezierwidgetbase.ui'
**
** Created: Thu May 3 23:08:12 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef BEZIERWIDGETBASE_H
#define BEZIERWIDGETBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class Qbezier;
class QServo;
class QLabel;
class QScrollBar;
class QGroupBox;
class QCheckBox;
class QLineEdit;
class QSplitter;
class QListView;
class QListViewItem;
class QPushButton;

class BezierWidgetBase : public QWidget
{
    Q_OBJECT

public:
    BezierWidgetBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BezierWidgetBase();

    QLabel* textLabel3;
    Qbezier* qbezier;
    QScrollBar* bezierScroll;
    QGroupBox* groupBox1;
    QLabel* textLabel1_2;
    QCheckBox* ChkEnableScroll;
    QLineEdit* GridScale;
    QLabel* textLabel1_2_3;
    QLineEdit* FormGridWidth;
    QGroupBox* groupBox1_2;
    QLabel* textLabel1_3;
    QLabel* textLabel1_2_2;
    QCheckBox* ReverseSeek;
    QLineEdit* FormMinPos;
    QLineEdit* FormMaxPos;
    QGroupBox* groupBox7;
    QSplitter* splitter1;
    QListView* listRegOut;
    QListView* listRegOutHex;
    QPushButton* btnOSSend;
    QListView* servoList;
    QServo* ServoSim;
    QLabel* textLabel2_2;
    QLabel* label;
    QLabel* textLabel2;
    QLabel* FormTimePos;
    QLabel* textLabel2_3;
    QLabel* FormMotorPos;
    QLabel* textLabel2_3_2;
    QLabel* FormMotorSpeed;
    QLabel* textLabel2_4;
    QLabel* textLabel2_4_2;
    QLabel* textLabel1_4;
    QLabel* textLabel1;
    QGroupBox* groupBox4;
    QPushButton* FormGridDel;
    QPushButton* FormGridAdd;
    QGroupBox* groupBox5;
    QPushButton* FormBtnSimStart;
    QPushButton* FormBtnSimPause;
    QCheckBox* ServoSimLoop;
    QPushButton* FormBtnSimStop;
    QGroupBox* groupBox6;
    QLabel* lblDebug;

public slots:
    virtual void setLabel(int x, int y);
    virtual void button_clicked();
    virtual void doresize(int w, int h);
    virtual void mouseMove(int x, int y);
    virtual void mousePress(QMouseEvent *e);
    virtual void mouseRelease(QMouseEvent *e);
    virtual void gridTimeChange(const QString &String);
    virtual void servoMaxPosChange(const QString &String);
    virtual void servoMinPosChange(const QString &string);
    virtual void BtnSimStart();
    virtual void BtnSimStop();
    virtual void BtnSimPause();
    virtual void setLabelXY(int x, int y);
    virtual void BtnGridDel();
    virtual void FormBezierScroll(int val);
    virtual void repaintbezier(QPaintEvent *e);
    virtual void BtnGridAdd();
    virtual void UpdateRegisters();
    virtual void enableScroll(bool val);
    virtual void infoUpdate(QString&);
    virtual void BtnOSsendCurve();
    virtual void servoChanged(QListViewItem*);

protected:
    QGridLayout* bezierwidgetbaseLayout;
    QHBoxLayout* layout15;
    QSpacerItem* spacer5_2;
    QSpacerItem* spacer5;
    QVBoxLayout* layout13;
    QHBoxLayout* layout24;
    QSpacerItem* spacer3_2;
    QGridLayout* groupBox7Layout;
    QHBoxLayout* layout11;
    QSpacerItem* spacer8;
    QHBoxLayout* layout19;
    QVBoxLayout* layout18;
    QVBoxLayout* layout17;
    QVBoxLayout* layout16;
    QVBoxLayout* layout15_2;
    QGridLayout* layout10;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // BEZIERWIDGETBASE_H
