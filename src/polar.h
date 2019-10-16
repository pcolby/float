/*
    Copyright 2019 Paul Colby <git@colby.id.au>

    This file is part of Float.

    Float is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Float is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Float.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QObject>
#include <QUrl>
#include <QWebEnginePage>

#define USE_WEB_ENGINE_VIEW

#ifdef USE_WEB_ENGINE_VIEW
#include <QWebEngineView>
#endif

class NonInteractiveWebPage;

class Polar : public QObject
{
    Q_OBJECT

public:
    explicit Polar(const QString &username, const QString &password, QObject * parent = Q_NULLPTR);
    virtual ~Polar();

public slots:
    void setWeight(const double mass);

protected:
    static QString javaScriptLiteral(QString string, QChar quote = QChar());

protected slots:
    void onLoadFinshed(const bool ok);

private:
    NonInteractiveWebPage * page;
#ifdef USE_WEB_ENGINE_VIEW
    QWebEngineView * view;
#endif
    QString username;
    QString password;
    double mass;

};
