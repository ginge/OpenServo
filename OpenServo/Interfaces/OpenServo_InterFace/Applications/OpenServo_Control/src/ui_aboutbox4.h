/********************************************************************************
** Form generated from reading UI file 'aboutbox4.ui'
**
** Created: Sun Nov 14 13:44:07 2010
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTBOX4_H
#define UI_ABOUTBOX4_H

#include <Qt3Support/Q3Frame>
#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_aboutBoxWidget
{
public:
    QGridLayout *gridLayout;
    QLabel *textLabel2;
    QLabel *textLabel3;
    QPushButton *closeBtn;
    Q3Frame *frame4;
    QLabel *textLabel5;
    QLabel *textLabel6;
    QLabel *textLabel4;
    QLabel *textLabel8;
    QLabel *textLabel7;

    void setupUi(QDialog *aboutBoxWidget)
    {
        if (aboutBoxWidget->objectName().isEmpty())
            aboutBoxWidget->setObjectName(QString::fromUtf8("aboutBoxWidget"));
        aboutBoxWidget->resize(250, 300);
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(5));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(aboutBoxWidget->sizePolicy().hasHeightForWidth());
        aboutBoxWidget->setSizePolicy(sizePolicy);
        aboutBoxWidget->setMinimumSize(QSize(250, 300));
        aboutBoxWidget->setMaximumSize(QSize(250, 300));
        gridLayout = new QGridLayout(aboutBoxWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textLabel2 = new QLabel(aboutBoxWidget);
        textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
        QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(textLabel2->sizePolicy().hasHeightForWidth());
        textLabel2->setSizePolicy(sizePolicy1);
        textLabel2->setMaximumSize(QSize(32767, 20));
        textLabel2->setWordWrap(false);

        gridLayout->addWidget(textLabel2, 0, 0, 1, 1);

        textLabel3 = new QLabel(aboutBoxWidget);
        textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
        sizePolicy1.setHeightForWidth(textLabel3->sizePolicy().hasHeightForWidth());
        textLabel3->setSizePolicy(sizePolicy1);
        textLabel3->setMaximumSize(QSize(32767, 20));
        textLabel3->setWordWrap(false);

        gridLayout->addWidget(textLabel3, 1, 0, 1, 1);

        closeBtn = new QPushButton(aboutBoxWidget);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));

        gridLayout->addWidget(closeBtn, 5, 0, 1, 1);

        frame4 = new Q3Frame(aboutBoxWidget);
        frame4->setObjectName(QString::fromUtf8("frame4"));
        QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(0));
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(frame4->sizePolicy().hasHeightForWidth());
        frame4->setSizePolicy(sizePolicy2);
        frame4->setMinimumSize(QSize(0, 80));
        frame4->setFrameShape(QFrame::StyledPanel);
        frame4->setFrameShadow(QFrame::Raised);
        textLabel5 = new QLabel(frame4);
        textLabel5->setObjectName(QString::fromUtf8("textLabel5"));
        textLabel5->setGeometry(QRect(10, 30, 210, 18));
        textLabel5->setWordWrap(false);
        textLabel6 = new QLabel(frame4);
        textLabel6->setObjectName(QString::fromUtf8("textLabel6"));
        textLabel6->setGeometry(QRect(10, 50, 200, 18));
        textLabel6->setWordWrap(false);
        textLabel4 = new QLabel(frame4);
        textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
        textLabel4->setGeometry(QRect(10, 10, 190, 16));
        textLabel4->setWordWrap(false);

        gridLayout->addWidget(frame4, 4, 0, 1, 1);

        textLabel8 = new QLabel(aboutBoxWidget);
        textLabel8->setObjectName(QString::fromUtf8("textLabel8"));
        textLabel8->setWordWrap(false);

        gridLayout->addWidget(textLabel8, 3, 0, 1, 1);

        textLabel7 = new QLabel(aboutBoxWidget);
        textLabel7->setObjectName(QString::fromUtf8("textLabel7"));
        sizePolicy1.setHeightForWidth(textLabel7->sizePolicy().hasHeightForWidth());
        textLabel7->setSizePolicy(sizePolicy1);
        textLabel7->setMaximumSize(QSize(32767, 20));
        textLabel7->setWordWrap(false);

        gridLayout->addWidget(textLabel7, 2, 0, 1, 1);


        retranslateUi(aboutBoxWidget);
        QObject::connect(closeBtn, SIGNAL(clicked()), aboutBoxWidget, SLOT(accept()));

        QMetaObject::connectSlotsByName(aboutBoxWidget);
    } // setupUi

    void retranslateUi(QDialog *aboutBoxWidget)
    {
        aboutBoxWidget->setWindowTitle(QApplication::translate("aboutBoxWidget", "About", 0, QApplication::UnicodeUTF8));
        textLabel2->setText(QApplication::translate("aboutBoxWidget", "<h3>OpenServo Test application</h3>", 0, QApplication::UnicodeUTF8));
        textLabel3->setText(QApplication::translate("aboutBoxWidget", "Visit and contrubute to <a href=\"http://www.openservo.com/forums/\">OpenServo Project</a>", 0, QApplication::UnicodeUTF8));
        closeBtn->setText(QApplication::translate("aboutBoxWidget", "C&lose", 0, QApplication::UnicodeUTF8));
        closeBtn->setShortcut(QApplication::translate("aboutBoxWidget", "Alt+L", 0, QApplication::UnicodeUTF8));
        textLabel5->setText(QApplication::translate("aboutBoxWidget", "<a href=\"mailto:Barry.Carter@headfuzz.co.uk\">Barry.Carter@headfuzz.co.uk</a>", 0, QApplication::UnicodeUTF8));
        textLabel6->setText(QApplication::translate("aboutBoxWidget", "<a href=\"http://www.openservo.com/\">www.headfuzz.co.uk</a>", 0, QApplication::UnicodeUTF8));
        textLabel4->setText(QApplication::translate("aboutBoxWidget", "v0.6.2 Copyright 2007 Barry Carter", 0, QApplication::UnicodeUTF8));
        textLabel8->setText(QApplication::translate("aboutBoxWidget", "<p align=\"center\">This may require root access under *NIX systems. Be sure to install the underlying OSIF_xxx library for your OS in a directory in the Path. Windows user put dll in same dir as application.</p>", 0, QApplication::UnicodeUTF8));
        textLabel7->setText(QApplication::translate("aboutBoxWidget", "<a href=\"http://www.openservo.com/\">www.OpenServo.com</a>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class aboutBoxWidget: public Ui_aboutBoxWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTBOX4_H
