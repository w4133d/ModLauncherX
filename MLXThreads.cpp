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
#include "MLXThreads.h"

#include <Windows.h>

mlBuildThread::mlBuildThread( const QList<QPair<QString, QStringList>> &Commands, bool IgnoreErrors )
	: mCommands( Commands ), mSuccess( false ), mCancel( false ), mIgnoreErrors( IgnoreErrors )
{
}

void mlBuildThread::run()
{
	bool Success = true;

	for( const QPair<QString, QStringList> &Command : mCommands )
	{
		QProcess *Process = new QProcess();
		connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );
		Process->setWorkingDirectory( QFileInfo( Command.first ).absolutePath() );
		Process->setProcessChannelMode( QProcess::MergedChannels );

		emit OutputReady( Command.first + ' ' + Command.second.join( ' ' ) + "\n" );

		Process->start( Command.first, Command.second );
		for( ;;)
		{
			Sleep( 100 );

			if( Process->waitForReadyRead( 0 ) )
				emit OutputReady( Process->readAll() );

			QProcess::ProcessState State = Process->state();
			if( State == QProcess::NotRunning )
				break;

			if( mCancel )
				Process->kill();
		}

		if( Process->exitStatus() != QProcess::NormalExit )
			return;

		if( Process->exitCode() != 0 )
		{
			Success = false;
			if( !mIgnoreErrors )
				return;
		}
	}

	mSuccess = Success;
}


mlConvertThread::mlConvertThread( QStringList &Files, QString &OutputDir, bool IgnoreErrors, bool OverwriteFiles )
	: mFiles( Files ), mOutputDir( OutputDir ), mSuccess( false ), mCancel( false ), mIgnoreErrors( IgnoreErrors ), mOverwrite( OverwriteFiles )
{
}

void mlConvertThread::run()
{
	bool Success = true;

	unsigned int convCountSuccess = 0;
	unsigned int convCountSkipped = 0;
	unsigned int convCountFailed = 0;

	for( QString file : mFiles )
	{
		QFileInfo file_info( file );
		QString working_directory = file_info.absolutePath();

		QProcess *Process = new QProcess();
		connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );
		Process->setWorkingDirectory( working_directory );
		Process->setProcessChannelMode( QProcess::MergedChannels );

		file = file_info.baseName();

		QString ToolsPath = QDir::fromNativeSeparators( getenv( "TA_TOOLS_PATH" ) );
		QString ExecutablePath = QString( "%1bin/export2bin.exe" ).arg( ToolsPath );

		QStringList args;
		//args.append("/v"); // Verbose
		args.append( "/piped" );

		QString filepath = file_info.absoluteFilePath();

		QString ext = file_info.suffix().toUpper();
		if( ext == "XANIM_EXPORT" )
			ext = ".XANIM_BIN";
		else if( ext == "XMODEL_EXPORT" )
			ext = ".XMODEL_BIN";
		else
		{
			emit OutputReady( "Export2Bin: Skipping file '" + filepath + "' (file has invalid extension)\n" );
			convCountSkipped++;
			continue;
		}

		QString target_filepath = QDir::cleanPath( mOutputDir ) + QDir::separator() + file + ext;

		QFile infile( filepath );
		QFile outfile( target_filepath );

		if( !mOverwrite && outfile.exists() )
		{
			emit OutputReady( "Export2Bin: Skipping file '" + filepath + "' (file already exists)\n" );
			convCountSkipped++;
			continue;
		}

		infile.open( QIODevice::OpenMode::enum_type::ReadOnly );
		if( !infile.isOpen() )
		{
			emit OutputReady( "Export2Bin: Could not open '" + filepath + "' for reading\n" );
			convCountFailed++;
			continue;
		}

		emit OutputReady( "Export2Bin: Converting '" + file + "'" );

		QByteArray buf = infile.readAll();
		infile.close();

		Process->start( ExecutablePath, args );
		Process->write( buf );
		Process->closeWriteChannel();

		QByteArray standardOutputPipeData;
		QByteArray standardErrorPipeData;

		for( ;;)
		{
			Sleep( 20 );

			if( Process->waitForReadyRead( 0 ) )
			{
				standardOutputPipeData.append( Process->readAllStandardOutput() );
				standardErrorPipeData.append( Process->readAllStandardError() );
			}

			QProcess::ProcessState State = Process->state();
			if( State == QProcess::NotRunning )
				break;

			if( mCancel )
				Process->kill();
		}

		if( Process->exitStatus() != QProcess::NormalExit )
		{
			emit OutputReady( "ERROR: Process exited abnormally" );
			Success = false;
			break;
		}

		if( Process->exitCode() != 0 )
		{
			emit OutputReady( standardOutputPipeData );
			emit OutputReady( standardErrorPipeData );

			convCountFailed++;

			if( !mIgnoreErrors )
			{
				Success = false;
				break;
			}

			continue;
		}

		outfile.open( QIODevice::OpenMode::enum_type::WriteOnly );
		if( !outfile.isOpen() )
		{
			emit OutputReady( "Export2Bin: Could not open '" + target_filepath + "' for writing\n" );
			continue;
		}

		outfile.write( standardOutputPipeData );
		outfile.close();

		convCountSuccess++;
	}

	mSuccess = Success;
	if( mSuccess )
	{
		QString msg = QString( "Export2Bin: Finished!\n\n"
							   "Files Processed: %1\n"
							   "Successes: %2\n"
							   "Skipped: %3\n"
							   "Failures: %4\n" ).arg( mFiles.count() ).arg( convCountSuccess ).arg( convCountSkipped ).arg( convCountFailed );
		emit OutputReady( msg );
	}
}
