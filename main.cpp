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
#include "MLXUtils.h"

#include <iosfwd>
#include <sstream>

void msg_handler( QtMsgType type, const QMessageLogContext &context, const QString &msg );
void __init__();
MLXMainWindow *MLXMainWindow::_instance = nullptr;

int main( int argc, char *argv[] )
{
	__init__();

	QGuiApplication::setDesktopSettingsAware( false );
	//qputenv( "QT_QUICK_CONTROLS_STYLE", "Basic" );
	//qputenv( "QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Light" );

	QApplication App( argc, argv );

	QCoreApplication::setOrganizationDomain( "github.com/w4133d" );
	QCoreApplication::setOrganizationName( "prov3ntus" );
	QCoreApplication::setApplicationName( "ModLauncherX" );
	// Without this, themes will break when swapping b/w 
	// stock styles when the user has Windows light mode on

	//QCoreApplication::setApplicationVersion();

	MLXMainWindow MainWindow;

	// Init
	MainWindow.setup_crash_handler();
	MainWindow.UpdateDB();
	MainWindow.show();
	MainWindow.UpdateTheme();

	return App.exec();
}

static void __init__()
{
	// Clear previous log session
	QFile log_file( MLXUtils::DEBUG_LOG_PATH );
	log_file.open( QIODevice::WriteOnly );
	log_file.close();

	qInstallMessageHandler( &msg_handler );

	qDebug() << "===================== ModLauncherX INIT =====================";

	// Writing a time stamp to the log on INIT
	const auto time = std::time( nullptr );
	auto ss = std::stringstream {};
	ss << std::put_time( std::localtime( &time ), "%F, at %T" );
	qDebug() << "ModLauncherX opened on:" << QString(ss.str().data()) << '\n';
}

// ==========================================================
// =================== CRASHING & LOGGING ===================
// ==========================================================

void msg_handler( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
	QFile log_file( MLXUtils::DEBUG_LOG_PATH );
	if( log_file.open( QIODevice::WriteOnly | QIODevice::Append ) )
	{
		QTextStream stream( &log_file );
		switch( type )
		{
			case QtDebugMsg:    stream << "[ DEBUG    ] "; break;
			case QtWarningMsg:  stream << "[ WARNING  ] "; break;
			case QtCriticalMsg: stream << "[ CRITICAL ] "; break;
			case QtFatalMsg:    stream << "[ FATAL    ] "; break;
		}
		// "[" << context.file << ":" << context.line << " in '" << context.function << "']\t"
		stream << QString( "[%1:%2 in '%3']\t").arg( context.file ).arg( context.line ).arg( context.function ) << msg << "\n";
		//stream << msg << "\n";
	}
}
