/****************************************************************************
** Form interface generated from reading ui file 'aboutbox.ui'
**
** Created: Sun Mar 18 21:56:56 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ABOUTBOXWIDGET_H
#define ABOUTBOXWIDGET_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QFrame;

class aboutBoxWidget : public QDialog
{
    Q_OBJECT

public:
    aboutBoxWidget( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~aboutBoxWidget();

    QLabel* textLabel2;
    QLabel* textLabel3;
    QPushButton* closeBtn;
    QFrame* frame4;
    QLabel* textLabel5;
    QLabel* textLabel6;
    QLabel* textLabel4;
    QLabel* textLabel8;
    QLabel* textLabel7;

protected:
    QGridLayout* aboutBoxWidgetLayout;

protected slots:
    virtual void languageChange();

};

#endif // ABOUTBOXWIDGET_H
