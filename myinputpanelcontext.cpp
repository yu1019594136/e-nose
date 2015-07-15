/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore>

#include "myinputpanelcontext.h"

//! [0]

MyInputPanelContext::MyInputPanelContext()
{
    inputPanel = new MyInputPanel;
    //connect(inputPanel, SIGNAL(characterGenerated(QChar)), SLOT(sendCharacter(QChar)));
    connect(inputPanel, SIGNAL(characterGenerated(QString)), SLOT(sendCharacter(QString)));

    /* cap键按下标记 1表示小写，-1表示大写*/
    cap_flag = 1;

    /* 更新位置辅助变量 */
    QDesktopWidget w;
    deskWidth = w.availableGeometry().width();
    deskHeight = w.availableGeometry().height();
    inputpanelWidth = inputPanel->width();
    inputpanelHeight = inputPanel->height();
}

//! [0]

MyInputPanelContext::~MyInputPanelContext()
{
    delete inputPanel;
}

//! [1]

bool MyInputPanelContext::filterEvent(const QEvent* event)
{
    if (event->type() == QEvent::RequestSoftwareInputPanel) {
        updatePosition();
        inputPanel->show();
        return true;
    } else if (event->type() == QEvent::CloseSoftwareInputPanel) {
        inputPanel->hide();
        return true;
    }
    return false;
}

//! [1]

QString MyInputPanelContext::identifierName()
{
    return "MyInputPanelContext";
}

void MyInputPanelContext::reset()
{
}

bool MyInputPanelContext::isComposing() const
{
    return false;
}

QString MyInputPanelContext::language()
{
    return "en_US";
}

//! [2]

void MyInputPanelContext::sendCharacter(QString character)
{
    QPointer<QWidget> w = focusWidget();

    if (!w)
        return;

    if(character == "backspace")
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, QString());
        QApplication::sendEvent(w, &keyPress);
    }
    else if(character == "left arrow")
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier, QString());
        QApplication::sendEvent(w, &keyPress);
    }
    else if(character == "right arrow")
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, QString());
        QApplication::sendEvent(w, &keyPress);
    }
    else if(character == "enter")
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, QString());
        QApplication::sendEvent(w, &keyPress);
    }
    else if(character == "cap")
    {
        cap_flag = cap_flag * -1;

        /* 如果标志为-1，则将cap变成CAP以表示当前是大写 */
        inputPanel->set_cap_text(cap_flag);
    }
    else if(character == "space")
    {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(" "));
        QApplication::sendEvent(w, &keyPress);
    }
    else
    {
        if(character[0] >= 'a' && character[0] <= 'z' && cap_flag == -1)
        {
            QKeyEvent keyPress(QEvent::KeyPress, character[0].unicode() - 32, Qt::NoModifier, QString(QChar(character[0].unicode() - 32)));
            QApplication::sendEvent(w, &keyPress);
        }
        else
        {
            QKeyEvent keyPress(QEvent::KeyPress, character[0].unicode(), Qt::NoModifier, character);
            QApplication::sendEvent(w, &keyPress);
        }
    }


    if (!w)
        return;

    if(character == "backspace")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyRelease);
    }
    else if(character == "left arrow")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyRelease);
    }
    else if(character == "right arrow")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyRelease);
    }
    else if(character == "enter")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyRelease);
    }
    else if(character == "cap")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_CapsLock, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyRelease);
    }
    else if(character == "space")
    {
//        QKeyEvent keyRelease(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString());
//        QApplication::sendEvent(w, &keyPress);
    }
    else
    {        
        if(character[0] >= 'a' && character[0] <= 'z' && cap_flag == -1)
        {
            QKeyEvent keyRelease(QEvent::KeyPress, character[0].unicode() - 32, Qt::NoModifier, QString());
            QApplication::sendEvent(w, &keyRelease);
        }
        else
        {
            QKeyEvent keyPress(QEvent::KeyPress, character[0].unicode(), Qt::NoModifier, QString());
            QApplication::sendEvent(w, &keyPress);
        }
    }

}

//! [2]

//! [3]

void MyInputPanelContext::updatePosition()
{
    QWidget *widget = focusWidget();
    if (!widget)
        return;

    QPoint panelPos;//存放最终软键盘摆放的位置

    /*如果获得焦点的部件的左上角坐标在屏幕的上半部分，那么软键盘居中下半屏显示
     *如果获得焦点的部件的左上角坐标在屏幕的下半部分，那么软键盘居中上半屏显示
     *
     * 先计算出获得焦点部件的坐标（左上角坐标widget->pos()），在将其换算成全屏幕的坐标(mapToGlobal)，再取其纵坐标值y()
     */
    if(widget->mapToGlobal(widget->pos()).y() < deskHeight / 2)
        panelPos= QPoint(deskWidth / 2 - inputpanelWidth / 2, deskHeight - inputpanelHeight);//横向居中，软键盘底部贴屏幕底部
    else
        panelPos= QPoint(deskWidth / 2 - inputpanelWidth / 2, 10);//横向居中，软键盘顶部贴屏幕顶部

    inputPanel->move(panelPos);

//    QRect widgetRect = widget->rect();
//    QPoint panelPos = QPoint(widgetRect.left(), widgetRect.bottom() + 2);
//    panelPos = widget->mapToGlobal(panelPos);
//    inputPanel->move(panelPos);
}

//! [3]
