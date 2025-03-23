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

#include "stdafx.h"
#include "MLXUtils.h"

#include <Windows.h>

const QString MLXUtils::BO3_ROOT_PATH = QString( getenv( "TA_TOOLS_PATH" ) ).replace( '\\', '/' );
const QString MLXUtils::DEBUG_LOG_PATH = QString( "%1logs/ModLauncherX.log" ).arg( MLXUtils::BO3_ROOT_PATH );
const QString MLXUtils::MAP_LIST_SAVE_PATH = QString( "%1bin/t7x/mlx_map.tree" ).arg( MLXUtils::BO3_ROOT_PATH );

namespace MLXUtils
{
	void open_folder_select_file( QString file_path )
	{
		QString filePath = file_path.replace( "/", "\\" ); // Ensure Windows-style path
		QString parameters = QString( "/select,\"%1\"" ).arg( filePath );
		qDebug() << "Explorer parameters:" << parameters;

		ShellExecute( NULL, "open", "explorer.exe", parameters.toLatin1().constData(), NULL, SW_SHOWDEFAULT );
	}

	void open_bo3_folder( const QString dir_path )
	{
		ShellExecute(
			NULL, "open",
			QString( "\"%1/%2\"" ).arg( MLXUtils::BO3_ROOT_PATH, dir_path ).toLatin1().constData(),
			"", NULL, SW_SHOWDEFAULT );

	}

	void copy_file_to_clipboard( const QString &filePath )
	{
		QClipboard *clipboard = QApplication::clipboard();
		QMimeData *mime_data = new QMimeData();

		QList<QUrl> urls;
		urls.append( QUrl::fromLocalFile( filePath ) );
		mime_data->setUrls( urls );

		clipboard->setMimeData( mime_data );
	}
}