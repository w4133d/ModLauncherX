/*
*
* Copyright 2025 prov3ntus
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#pragma once

#include "stdafx.h"

class mlBuildThread: public QThread
{
	Q_OBJECT

	public:
	mlBuildThread( const QList<QPair<QString, QStringList>> &Commands, bool IgnoreErrors );
	void run();
	bool Succeeded() const
	{
		return mSuccess;
	}

	void Cancel()
	{
		mCancel = true;
	}

	signals:
	void OutputReady( const QString &Output );

	protected:
	QList<QPair<QString, QStringList>> mCommands;
	bool mSuccess;
	bool mCancel;
	bool mIgnoreErrors;
};

class mlConvertThread: public QThread
{
	Q_OBJECT

	public:
	mlConvertThread( QStringList &Files, QString &OutputDir, bool IgnoreErrors, bool mOverwrite );
	void run();
	bool Succeeded() const
	{
		return mSuccess;
	}

	void Cancel()
	{
		mCancel = true;
	}

	signals:
	void OutputReady( const QString &Output );

	protected:
	QStringList mFiles;
	QString mOutputDir;
	bool mOverwrite;

	bool mSuccess;
	bool mCancel;
	bool mIgnoreErrors;
};
