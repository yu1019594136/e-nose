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

#include "myinputpanel.h"

//! [0]

MyInputPanel::MyInputPanel()
    : QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint),
      lastFocusedWidget(0)
{
    form.setupUi(this);

    //绑定全局改变焦点信号槽
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(saveFocusWidget(QWidget*,QWidget*)));

    signalMapper.setMapping(form.panelButton_1, form.panelButton_1);
    signalMapper.setMapping(form.panelButton_2, form.panelButton_2);
    signalMapper.setMapping(form.panelButton_3, form.panelButton_3);
    signalMapper.setMapping(form.panelButton_4, form.panelButton_4);
    signalMapper.setMapping(form.panelButton_5, form.panelButton_5);
    signalMapper.setMapping(form.panelButton_6, form.panelButton_6);
    signalMapper.setMapping(form.panelButton_7, form.panelButton_7);
    signalMapper.setMapping(form.panelButton_8, form.panelButton_8);
    signalMapper.setMapping(form.panelButton_9, form.panelButton_9);
    signalMapper.setMapping(form.panelButton_0, form.panelButton_0);

    signalMapper.setMapping(form.panelButton_sign0, form.panelButton_sign0);
    signalMapper.setMapping(form.panelButton_sign1, form.panelButton_sign1);
    signalMapper.setMapping(form.panelButton_sign2, form.panelButton_sign2);
    signalMapper.setMapping(form.panelButton_sign3, form.panelButton_sign3);
    signalMapper.setMapping(form.panelButton_sign4, form.panelButton_sign4);
    signalMapper.setMapping(form.panelButton_sign5, form.panelButton_sign5);
    signalMapper.setMapping(form.panelButton_sign6, form.panelButton_sign6);
    signalMapper.setMapping(form.panelButton_sign7, form.panelButton_sign7);
    signalMapper.setMapping(form.panelButton_sign8, form.panelButton_sign8);
    signalMapper.setMapping(form.panelButton_sign9, form.panelButton_sign9);
    signalMapper.setMapping(form.pushButton_sign10, form.pushButton_sign10);
    signalMapper.setMapping(form.pushButton_sign11, form.pushButton_sign11);
    signalMapper.setMapping(form.pushButton_semicolon, form.pushButton_semicolon);
    signalMapper.setMapping(form.pushButton_slash_left, form.pushButton_slash_left);
    signalMapper.setMapping(form.pushButton_slash_right, form.pushButton_slash_right);
    signalMapper.setMapping(form.pushButton_space, form.pushButton_space);
    signalMapper.setMapping(form.pushButton_question_mark, form.pushButton_question_mark);
    signalMapper.setMapping(form.pushButton_period, form.pushButton_period);
    signalMapper.setMapping(form.pushButton_comma, form.pushButton_comma);
    signalMapper.setMapping(form.pushButton_cap, form.pushButton_cap);

    signalMapper.setMapping(form.pushButton_Backspace, form.pushButton_Backspace);
    signalMapper.setMapping(form.panelButton_left_arrow, form.panelButton_left_arrow);
    signalMapper.setMapping(form.panelButton_right_arrow, form.panelButton_right_arrow);
    signalMapper.setMapping(form.pushButton_enter, form.pushButton_enter);

    signalMapper.setMapping(form.pushButton_a, form.pushButton_a);
    signalMapper.setMapping(form.pushButton_b, form.pushButton_b);
    signalMapper.setMapping(form.pushButton_c, form.pushButton_c);
    signalMapper.setMapping(form.pushButton_d, form.pushButton_d);
    signalMapper.setMapping(form.pushButton_e, form.pushButton_e);
    signalMapper.setMapping(form.pushButton_f, form.pushButton_f);
    signalMapper.setMapping(form.pushButton_g, form.pushButton_g);
    signalMapper.setMapping(form.pushButton_h, form.pushButton_h);
    signalMapper.setMapping(form.pushButton_i, form.pushButton_i);
    signalMapper.setMapping(form.pushButton_j, form.pushButton_j);
    signalMapper.setMapping(form.pushButton_k, form.pushButton_k);
    signalMapper.setMapping(form.pushButton_l, form.pushButton_l);
    signalMapper.setMapping(form.pushButton_m, form.pushButton_m);
    signalMapper.setMapping(form.pushButton_n, form.pushButton_n);
    signalMapper.setMapping(form.pushButton_o, form.pushButton_o);
    signalMapper.setMapping(form.pushButton_p, form.pushButton_p);
    signalMapper.setMapping(form.pushButton_q, form.pushButton_q);
    signalMapper.setMapping(form.pushButton_r, form.pushButton_r);
    signalMapper.setMapping(form.pushButton_s, form.pushButton_s);
    signalMapper.setMapping(form.pushButton_t, form.pushButton_t);
    signalMapper.setMapping(form.pushButton_u, form.pushButton_u);
    signalMapper.setMapping(form.pushButton_v, form.pushButton_v);
    signalMapper.setMapping(form.pushButton_w, form.pushButton_w);
    signalMapper.setMapping(form.pushButton_x, form.pushButton_x);
    signalMapper.setMapping(form.pushButton_y, form.pushButton_y);
    signalMapper.setMapping(form.pushButton_z, form.pushButton_z);

    connect(form.panelButton_1, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_2, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_3, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_4, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_5, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_6, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_7, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_8, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_9, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_0, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));

    connect(form.panelButton_sign0, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign1, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign2, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign3, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign4, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign5, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign6, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign7, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign8, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_sign9, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_sign10, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_sign11, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));

    connect(form.pushButton_Backspace, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_enter, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_left_arrow, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.panelButton_right_arrow, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_space, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_slash_right, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_slash_left, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_semicolon, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_question_mark, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_period, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_comma, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_cap, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));


    connect(form.pushButton_a, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_b, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_c, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_d, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_e, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_f, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_g, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_h, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_i, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_j, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_k, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_l, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_m, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_n, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_o, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_p, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_q, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_r, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_s, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_t, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_u, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_v, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_w, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_x, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_y, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));
    connect(form.pushButton_z, SIGNAL(clicked()),
            &signalMapper, SLOT(map()));

    connect(&signalMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(buttonClicked(QWidget*)));
}

//! [0]

bool MyInputPanel::event(QEvent *e)
{
    switch (e->type()) {
//! [1]
    case QEvent::WindowActivate:
        if (lastFocusedWidget)
            lastFocusedWidget->activateWindow();
        break;
//! [1]
    default:
        break;
    }

    return QWidget::event(e);
}

//! [2]

void MyInputPanel::saveFocusWidget(QWidget * /*oldFocus*/, QWidget *newFocus)
{
    if (newFocus != 0 && !this->isAncestorOf(newFocus)) {
        lastFocusedWidget = newFocus;
    }
}

//! [2]

//! [3]

void MyInputPanel::buttonClicked(QWidget *w)
{
    //QChar chr = qvariant_cast<QChar>(w->property("buttonValue"));
    QString chr = qvariant_cast<QString>(w->property("buttonValue"));
    emit characterGenerated(chr);
}

//! [3]

void MyInputPanel::set_cap_text(int flag)
{
    if(flag == -1)
        form.pushButton_cap->setText("CAP");
    else
        form.pushButton_cap->setText("cap");
}
