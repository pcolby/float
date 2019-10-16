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

#include <QWebEnginePage>

class NonInteractiveWebPage : public QWebEnginePage
{

public:
    explicit NonInteractiveWebPage(QObject * parent = Q_NULLPTR);
    NonInteractiveWebPage(QWebEngineProfile * profile, QObject * parent = Q_NULLPTR);
    ~NonInteractiveWebPage() override;

protected:

    // Base class overrides.
    QStringList chooseFiles(FileSelectionMode mode, const QStringList &oldFiles,
                            const QStringList &acceptedMimeTypes) override;
    void javaScriptAlert(const QUrl &securityOrigin, const QString &msg) override;
    bool javaScriptConfirm(const QUrl &securityOrigin, const QString &msg) override;
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message,
                                  int lineNumber, const QString &sourceID) override;
    bool javaScriptPrompt(const QUrl &securityOrigin, const QString &msg,
                          const QString &defaultValue, QString *result) override;

};
