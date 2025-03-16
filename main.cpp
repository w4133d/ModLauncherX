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

//#include <QtPlugin>
//Q_IMPORT_PLUGIN( QWindowsIntegrationPlugin )

#include "stdafx.h"
#include "MLXMainWindow.h"

int main( int argc, char *argv[] )
{
	QApplication App( argc, argv );

	QCoreApplication::setOrganizationDomain( "github.com/w4133d" );
	QCoreApplication::setOrganizationName( "prov3ntus" );
	QCoreApplication::setApplicationName( "ModLauncherX" );
	//QCoreApplication::setApplicationVersion();

	MLXMainWindow MainWindow;

	MainWindow.UpdateDB();
	MainWindow.show();

	return App.exec();
}
