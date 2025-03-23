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
#include "MLXMainWindow.h"
#include "MLXUtils.h"
#include "MLXMapList.h"

#include <windows.h>

#pragma comment(lib, "steam_api64.lib")

const int AppId = 311210;

const char *gTags[] = {
	"Animation", "Audio", "Character", "Map", "Mod", "Mode", "Model", "Multiplayer", "Scorestreak",
	"Skin", "Specialist", "Texture", "UI", "Vehicle", "Visual Effect", "Weapon", "WIP", "Zombies"
};

MLXMainWindow::MLXMainWindow()
{
	QSettings Settings;

	mBuildThread = NULL;
	mBuildLanguage = Settings.value( "BuildLanguage", "english" ).toString();
	mTheme = Settings.value( "Theme", "T7x" ).toString();

	// Qt prefers '/' over '\\'
	//MLXUtils::BO3_ROOT_PATH = QString( getenv( "TA_TOOLS_PATH" ) ).replace( '\\', '/' );

	//qApp->setFont( QFont( GetCustomFont( LATO ).family(), 11 ) );

	setWindowIcon( QIcon( ":/resources/ModLauncherX.png" ) );
	setWindowTitle( "ModLauncherX" );

	setMinimumSize( 512, 512 );
	resize( 1536, 1152 );

	RegisterFonts();
	DefineToolTips();

	CreateActions();
	CreateMenu();
	CreateToolBar();

	mExport2BinGUIWidget = NULL;

	QSplitter *CentralWidget = new QSplitter();
	CentralWidget->setOrientation( Qt::Vertical );

	QWidget *TopWidget = new QWidget();
	CentralWidget->addWidget( TopWidget );

	QHBoxLayout *TopLayout = new QHBoxLayout( TopWidget );
	TopWidget->setLayout( TopLayout );

	mFileListWidget = new MLXMapList();
	//mFileListWidget->setHeaderHidden( true );
	//mFileListWidget->setUniformRowHeights( true );
	//mFileListWidget->setDragEnabled( true );
	//mFileListWidget->viewport()->setAcceptDrops( true );
	//mFileListWidget->setDropIndicatorShown( true );
	//mFileListWidget->setDragDropMode( QAbstractItemView::InternalMove );
	//mFileListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
	//mFileListWidget->setRootIsDecorated(false);
	TopLayout->addWidget( mFileListWidget );

	//connect( mFileListWidget, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( ContextMenuRequested() ) );
	connect( mFileListWidget,
			 SIGNAL( customContextMenuRequested( const QPoint & ) ),
			 mFileListWidget, SLOT( on_context_menu_requested() )
	);

	//TopLayout->addStretch( -4 ); // DEFAULT: 1

	QVBoxLayout *SideLayout = new QVBoxLayout();
	TopLayout->addLayout( SideLayout );

	QVBoxLayout *ActionsLayout = new QVBoxLayout();
	SideLayout->addLayout( ActionsLayout );

	QHBoxLayout *CompileLayout = new QHBoxLayout();
	ActionsLayout->addLayout( CompileLayout );

	mCompileEnabledWidget = new QCheckBox( "Compile" );
	mCompileEnabledWidget->setToolTip( mlxToolTips[ TOOLTIP_COMPILE ] );
	CompileLayout->addWidget( mCompileEnabledWidget );

	mCompileModeWidget = new QComboBox();
	mCompileModeWidget->addItems( QStringList() << "Ents" << "Full" );
	mCompileModeWidget->setCurrentIndex( 1 );
	CompileLayout->addWidget( mCompileModeWidget );

	QHBoxLayout *LightLayout = new QHBoxLayout();
	ActionsLayout->addLayout( LightLayout );

	mLightEnabledWidget = new QCheckBox( "Light" );
	mLightEnabledWidget->setToolTip( mlxToolTips[ TOOLTIP_LIGHT ] );
	LightLayout->addWidget( mLightEnabledWidget );

	mLightQualityWidget = new QComboBox();
	mLightQualityWidget->addItems( QStringList() << "Low" << "Medium" << "High" );
	mLightQualityWidget->setCurrentIndex( 1 );
	mLightQualityWidget->setMinimumWidth( 64 ); // Fix for "Medium" being cut off in the dark theme
	//mLightQualityWidget->setFixedWidth( 110 );
	//mLightQualityWidget->setFixedHeight( 20 );
	LightLayout->addWidget( mLightQualityWidget );

	mLinkEnabledWidget = new QCheckBox( "Link" );
	mLinkEnabledWidget->setToolTip( mlxToolTips[ TOOLTIP_LINK ] );
	ActionsLayout->addWidget( mLinkEnabledWidget );

	mRunEnabledWidget = new QCheckBox( "Run" );
	mRunEnabledWidget->setToolTip( mlxToolTips[ TOOLTIP_RUN ] );
	ActionsLayout->addWidget( mRunEnabledWidget );

	mRunOptionsWidget = new QLineEdit();
	mRunOptionsWidget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
	mRunOptionsWidget->setToolTip( mlxToolTips[ TOOLTIP_EXEC_ARGS ] );
	mRunOptionsWidget->setPlaceholderText( "Enter custom run arguments..." );
	ActionsLayout->addWidget( mRunOptionsWidget );

	mBuildButton = new QPushButton( "Build" );
	connect( mBuildButton, SIGNAL( clicked() ), mActionEditBuild, SLOT( trigger() ) );
	mBuildButton->setToolTip( mlxToolTips[ TOOLTIP_BUILD ] );
	ActionsLayout->addWidget( mBuildButton );

	mDvarsButton = new QPushButton( "Dvar Options" );
	connect( mDvarsButton, SIGNAL( clicked() ), this, SLOT( OnEditDvars() ) );
	mDvarsButton->setToolTip( mlxToolTips[ TOOLTIP_DVARS ] );
	ActionsLayout->addWidget( mDvarsButton );

	/*
	mSaveOutputButton = new QPushButton( "Save Output" );
	connect( mSaveOutputButton, SIGNAL( clicked() ), this, SLOT( OnSaveOutputLog() ) );
	mSaveOutputButton->setToolTip( mlxToolTips[ TOOLTIP_SAVE_OUTPUT ] );
	ActionsLayout->addWidget( mSaveOutputButton );
	*/

	mOpenLogButton = new QPushButton( "Open Game Log" );
	connect( mOpenLogButton, SIGNAL( clicked() ), this, SLOT( OnOpenConsoleMPLog() ) );
	mOpenLogButton->setToolTip( mlxToolTips[ TOOLTIP_OPEN_LOG ] );
	ActionsLayout->addWidget( mOpenLogButton );

	mDebugButton = new QPushButton( "Apply Theme [DEBUG]" );
	connect( mDebugButton, SIGNAL( clicked() ), this, SLOT( OnDebugButtonPressed() ) );
	ActionsLayout->addWidget( mDebugButton );

	/*
	mIgnoreErrorsWidget = new QCheckBox( "Ignore Errors" );
	mIgnoreErrorsWidget->setToolTip( mlxToolTips[ TOOLTIP_IGNORE_ERRORS ] );
	ActionsLayout->addWidget( mIgnoreErrorsWidget );
	*/

	mCleanXPAKsWidget = new QCheckBox( "Clean XPAKs" );
	mCleanXPAKsWidget->setToolTip( mlxToolTips[ TOOLTIP_CLEAN_XPAKS ] );
	ActionsLayout->addWidget( mCleanXPAKsWidget );

	/////////////////
	// SEARCH AREA //
	/////////////////

	//SideLayout->addSpacing( 10 );

	QVBoxLayout *SearchLayout = new QVBoxLayout();
	SearchLayout->addStretch( 1 );
	SideLayout->addLayout( SearchLayout );

	QLabel *SearchText = new QLabel( "Search" );
	SearchLayout->addWidget( SearchText );

	QHBoxLayout *FindSublayout = new QHBoxLayout();
	SearchLayout->addLayout( FindSublayout );

	SearchBar = new QLineEdit( this );
	SearchBar->setMaximumWidth( 150 );
	SearchBar->setPlaceholderText( "Enter search..." );
	// Very expensive
	//connect( SearchBar, SIGNAL( textEdited( const QString & ) ), this, SLOT( OnSearchTextChanged( const QString & ) ) );
	connect( SearchBar, SIGNAL( returnPressed() ), this, SLOT( OnReturnPressed() ) );
	FindSublayout->addWidget( SearchBar );

	FindNextButton = new QPushButton( "Find", this );
	FindNextButton->setMaximumWidth( 50 );
	connect( FindNextButton, SIGNAL( clicked() ), this, SLOT( OnFindNext() ) );
	//FindNextButton->setIcon( QIcon( ":/resources/search/FindNext.png" ) );
	FindSublayout->addWidget( FindNextButton );

	CaseSensitiveCheck = new QCheckBox( "Case Sensitive", this );
	//CaseSensitiveCheck->setIcon( QIcon( ":/resources/search/CaseSensitivity.png" ) );
	SearchLayout->addWidget( CaseSensitiveCheck );

	WholeWordCheck = new QCheckBox( "Match Whole Word", this );
	SearchLayout->addWidget( WholeWordCheck );

	///////////////////
	// OUTPUT WIDGET //
	///////////////////

	mOutputWidget = new QPlainTextEdit( this );
	mOutputWidget->setReadOnly( true );

	syntax_highlighter = new MLXOutputHighlighter( Settings.value( "UseColorCodes", true ).toBool() ? mOutputWidget->document() : nullptr );
	// For search highlighting
	LastMatchCursor = mOutputWidget->textCursor();
	CentralWidget->addWidget( mOutputWidget );

	setCentralWidget( CentralWidget );

	mShippedMapList << "mp_aerospace" << "mp_apartments" << "mp_arena" << "mp_banzai" << "mp_biodome"
		<< "mp_chinatown" << "mp_city" << "mp_conduit" << "mp_crucible" << "mp_cryogen" << "mp_ethiopia"
		<< "mp_freerun_01" << "mp_freerun_02" << "mp_freerun_03" << "mp_freerun_04" << "mp_havoc" << "mp_infection"
		<< "mp_kung_fu" << "mp_metro" << "mp_miniature" << "mp_nuketown_x" << "mp_redwood" << "mp_rise" << "mp_rome"
		<< "mp_ruins" << "mp_sector" << "mp_shrine" << "mp_skyjacked" << "mp_spire" << "mp_stronghold" << "mp_veiled"
		<< "mp_waterpark" << "mp_western" << "zm_castle" << "zm_factory" << "zm_genesis" << "zm_island" << "zm_levelcommon"
		<< "zm_stalingrad" << "zm_zod";

	////////////////
	// STATUS BAR //
	////////////////

	StatusBar = new QStatusBar();
	setStatusBar( StatusBar );
	StatusBar->showMessage( "T7x Launcher - pv" );

	////////
	// YE //
	////////

	Settings.beginGroup( "MainWindow" );
	resize( QSize( 1024, 768 ) );
	move( QPoint( 200, 200 ) );
	restoreGeometry( Settings.value( "Geometry" ).toByteArray() );
	restoreState( Settings.value( "State" ).toByteArray() );
	Settings.endGroup();

	SteamAPI_Init();

	connect( &mTimer, SIGNAL( timeout() ), this, SLOT( SteamUpdate() ) );
	mTimer.start( 2000 );

	//qDebug() << "Running 'MLXMapList::populate_file_tree()' on instance...";
	mFileListWidget->populate_file_tree();

	// Initial theme application call is now done after 
	// object instantiation & construction to ensure
	// it applies everything properly (see main.cpp)
	//UpdateTheme();
}

MLXMainWindow::~MLXMainWindow()
{
}

void MLXMainWindow::UpdateTheme()
{
	if( mTheme == "Default" )
	{
		qApp->setStyle( "WindowsVista" );
		qApp->setStyleSheet( "" );

		syntax_highlighter->default_color = QColor( "#242323" );
	}
	else if( mTheme == "Legacy" )
	{
		qApp->setStyle( "Windows" );
		qApp->setStyleSheet( "" );

		syntax_highlighter->default_color = QColor( "#b2b2b2" );
	}
	else if( mTheme == "Windows" )
	{
		qApp->setStyle( "Fusion" );
		qApp->setStyleSheet( "" );

		syntax_highlighter->default_color = QColor( "#b2b2b2" );
	}
	else if( mTheme == "Modern" )
	{
		qApp->setStyle( "Windows11" );
		qApp->setStyleSheet( "" );

		syntax_highlighter->default_color = QColor( "#b2b2b2" );
	}
	else if( mTheme == "Treyarch" )
	{
		qApp->setStyle( "Windows" );
		QFile file( QString( "%1/radiant/stylesheet.qss" ).arg( MLXUtils::BO3_ROOT_PATH ) );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		file.close();
		qApp->setStyleSheet( styleSheet );

		syntax_highlighter->default_color = QColor( "#b2b2b2" );
	}
	else if( mTheme == "T7x" )
	{
		qApp->setStyle( "Windows" );
		QFile file( QString( "%1/bin/t7x/pv_t7x_stylesheet.qss" ).arg( MLXUtils::BO3_ROOT_PATH ) );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		file.close();
		qApp->setStyleSheet( styleSheet );

		syntax_highlighter->default_color = QColor( "#b2b2b2" );
	}

	mOutputWidget->setFont( QSettings().value( "UseMonoOutputFont", true ).toBool() ? GetFont( FIRA_CODE ) : DefaultFont );
	syntax_highlighter->rehighlight();
	ClearHighlights();
}

QFont MLXMainWindow::GetFont( mlx_fonts font_enum )
{
	return QFont( QFontDatabase::applicationFontFamilies( RegisteredFonts[ font_enum ] ).at( 0 ) );
}

void MLXMainWindow::RegisterFonts()
{
	RegisteredFonts[ FIRA_CODE ] = QFontDatabase::addApplicationFont( ":resources/fonts/FiraCode-Regular.ttf" );
	RegisteredFonts[ INTER ] = QFontDatabase::addApplicationFont( ":resources/fonts/Inter_18pt-Regular.ttf" );
	RegisteredFonts[ LATO ] = QFontDatabase::addApplicationFont( ":resources/fonts/Lato-Regular.ttf" );

	//QString font_out = QString( "'%1'\n" ).arg( qApp->font().family() );
	for( int id : RegisteredFonts.values() )
	{
		if( id == -1 )
		{
			QMessageBox::critical( this, QString( "ERROR" ), QString( "One or more fonts failed to register!" ) );
		}

		//font_out.append( QString( "'%1'\n" ).arg( QFontDatabase::applicationFontFamilies( id ).at( 0 ) ) );
	}

	DefaultFont = qApp->font().family();
	//QMessageBox::information( this, QString( "Fonts" ), QString( "Successfully loaded fonts:\n%1" ).arg( font_out ) );
}

void MLXMainWindow::HighlightAllMatches( const QString &text )
{
	if( text.isEmpty() ) return; // Ignore empty searches

	QTextDocument *document = mOutputWidget->document();
	QTextCursor cursor( document );

	QTextCharFormat highlight_format;
	highlight_format.setBackground( SearchHighlightColor );

	int match_count = 0;

	while( !cursor.isNull() && !cursor.atEnd() )
	{
		cursor = document->find( text, cursor, GetSearchFlags() );
		if( !cursor.isNull() )
		{
			QTextCursor highlightCursor = cursor; // Separate cursor to apply format
			highlightCursor.mergeCharFormat( highlight_format );

			// Save the first match cursor for later navigation
			if( match_count == 0 )
			{
				LastMatchCursor = cursor;
				mOutputWidget->setTextCursor( LastMatchCursor );
			}

			match_count++;
		}
	}

	if( match_count == 0 )
	{
		StatusBar->showMessage( "No results found", 5000 );
	}
	else
	{
		StatusBar->showMessage( QString( "Search: Found %1 matches" ).arg( match_count ) );
	}
}

void MLXMainWindow::ClearHighlights()
{
	QTextDocument *document = mOutputWidget->document();
	QTextCursor cursor( document );

	QTextCharFormat default_format;
	default_format.setBackground( Qt::transparent );

	while( !cursor.isNull() && !cursor.atEnd() )
	{
		cursor.select( QTextCursor::WordUnderCursor );
		cursor.setCharFormat( default_format );
		cursor.movePosition( QTextCursor::NextWord );
	}

	// Reset cursor
	LastMatchCursor = mOutputWidget->textCursor();
}

QTextDocument::FindFlags MLXMainWindow::GetSearchFlags()
{
	QTextDocument::FindFlags flags;

	if( CaseSensitiveCheck->isChecked() )
		flags |= QTextDocument::FindCaseSensitively;

	if( WholeWordCheck->isChecked() )
		flags |= QTextDocument::FindWholeWords;

	return flags;
}

void MLXMainWindow::DefineToolTips()
{
	// Main:

	mlxToolTips[ TOOLTIP_COMPILE ] =
		"Build/apply changes made in Radaint since last compile."
		+ QString( "\nIf no changes were made, this doesn't need to be checked." )
		+ QString( "\nMake sure \"Link\" is checked, otherwise changes from Radiant will not update in-game." );

	mlxToolTips[ TOOLTIP_LIGHT ] =
		"Rebuild lighting from scratch. To use lighting built in Radiant instead,"
		+ QString( "\ngo to Radiant --> File --> Export Lighting, and make sure" )
		+ QString( "\n\"Export build lighting\" is checked (leave other checkboxes as default)." )
		+ QString( "\nMake sure \"Link\" is checked, otherwise newly built lighting will not update in-game." );

	mlxToolTips[ TOOLTIP_LINK ] =
		"Link all files (scripts, compiled Radiant map, sounds, lighting info, assets, etc.) together."
		+ QString( "\nThis must always be checked, as only the Linker generates files the game actually reads." );

	mlxToolTips[ TOOLTIP_RUN ] =
		"Runs the game after all build processes have completed."
		+ QString( "\nNote: This option will be ignored if there are any" )
		+ QString( "\nerrors(and \"Ignore Errors\" is off)." );

	mlxToolTips[ TOOLTIP_EXEC_ARGS ] =
		"Extra arguments to run the game with.";

	mlxToolTips[ TOOLTIP_BUILD ] =
		"Executes all of the above checked components.";

	mlxToolTips[ TOOLTIP_DVARS ] =
		"Edit Dvars that the game runs with.";

	mlxToolTips[ TOOLTIP_COPY_OUTPUT ] =
		"ye";

	mlxToolTips[ TOOLTIP_SAVE_OUTPUT ] =
		"Save the current launcher output (the text\nbox at the bottom) to a \"logs\" folder in your BO3 root.";

	mlxToolTips[ TOOLTIP_OPEN_LOG ] =
		"Opens console_mp.log found in the BO3 root. This file is a log"
		+ QString( "\nof everything printed to the console during the game's runtime." )
		+ QString( "\nYou can enable the log in the Dvars menu above (under \"Build\")." );

	mlxToolTips[ TOOLTIP_IGNORE_ERRORS ] =
		"For ADVANCED users only. This allows you to ignore errors that"
		+ QString( "\nwould otherwise halt the build process, and continue" )
		+ QString( "\nbuilding your map. Please don't use this unless you" )
		+ QString( "\nknow what you're doing, as, you should fix errors if" )
		+ QString( "\nthey're not letting you build your project." );



	// Settings:

	mlxToolTips[ TOOLTIP_OUTPUT_USES_MONO_FONT ] =
		"Decide whether the output (the text box at the bottom of the launcher\n"
		+ QString( "uses a monospace font. Unchecked uses the default font.\n" )
		+ QString( "Font used for monospace is \"Fira Code\"" );

	mlxToolTips[ TOOLTIP_OUTPUT_USES_COLOR_CODES ] =
		"Whether to change the color of the text in\nthe output when a color code is detected";

	mlxToolTips[ TOOLTIP_SEARCH_HIGHLIGHT_COLOR ] =
		"Red: Soft red, not too intense."
		+ QString( "\nOrange: Warm and pale, good for light or dark themes." )
		+ QString( "\nYellow: Muted pastel yellow, avoids eye strain." )
		+ QString( "\nGreen: Soft green, natural and readable." )
		+ QString( "\nCyan: Gentle cyan, cool and calm." )
		+ QString( "\nBlue: Light blue, soft but visible." )
		+ QString( "\nViolet: Pale purple, easy on the eyes." );

	mlxToolTips[ TOOLTIP_THEME ] =
		"Choose which theme you would like. 'Adaptive' means the dark/light scheme"
		+ QString( "\nwill be based on your current system setting. The 3arc and T7x" )
		+ QString( "\nthemes are always dark. Feel free to try them all out." )
		+ QString( "\nIf you get any visual bugs, please restart the Launcher." )
		+ QString( "\nDefault: The default theme. Light mode." )
		+ QString( "\nWindows: A slightly modified version of the default theme (adaptive)." )
		+ QString( "\nModern: A modernized theme (adaptive)." )
		+ QString( "\nTreyarch: The classic theme we all know and love." )
		+ QString( "\nT7x: A modernisation of the 3arc theme, made for the T7x Suite." );

	mlxToolTips[ TOOLTIP_CLEAN_XPAKS ] =
		"TODO";
}

void MLXMainWindow::CreateActions()
{
	// File

	mActionFileNew = new QAction( QIcon( ":/resources/FileNew.png" ), "&New...", this );
	mActionFileNew->setShortcut( QKeySequence( "Ctrl+N" ) );
	connect( mActionFileNew, SIGNAL( triggered() ), this, SLOT( OnFileNew() ) );

	mActionFileAssetEditor = new QAction( QIcon( ":/resources/AssetEditor.png" ), "&Asset Editor", this );
	mActionFileAssetEditor->setShortcut( QKeySequence( "Ctrl+A" ) );
	connect( mActionFileAssetEditor, SIGNAL( triggered() ), this, SLOT( OnFileAssetEditor() ) );

	mActionFileLevelEditor = new QAction( QIcon( ":/resources/Radiant.png" ), "Open in &Radiant", this );
	mActionFileLevelEditor->setShortcut( QKeySequence( "Ctrl+R" ) );
	mActionFileLevelEditor->setToolTip( "Level Editor" );
	connect( mActionFileLevelEditor, SIGNAL( triggered() ), this, SLOT( OnFileLevelEditor() ) );

	mActionFileExport2Bin = new QAction( QIcon( ":/resources/Export2Bin.png" ), "&Export2Bin GUI", this );
	mActionFileExport2Bin->setShortcut( QKeySequence( "Ctrl+E" ) );
	connect( mActionFileExport2Bin, SIGNAL( triggered() ), this, SLOT( OnFileExport2Bin() ) );

	mActionFileExit = new QAction( QIcon( ":/resources/Quit.png" ), "E&xit", this );
	connect( mActionFileExit, SIGNAL( triggered() ), this, SLOT( close() ) );


	// Edit

	mActionEditBuild = new QAction( QIcon( ":/resources/Go.png" ), "Build", this );
	mActionEditBuild->setShortcut( QKeySequence( "Ctrl+B" ) );
	connect( mActionEditBuild, SIGNAL( triggered() ), this, SLOT( OnEditBuild() ) );

	mActionEditPublish = new QAction( QIcon( ":/resources/publish.png" ), "Publish", this );
	mActionEditPublish->setShortcut( QKeySequence( "Ctrl+P" ) );
	connect( mActionEditPublish, SIGNAL( triggered() ), this, SLOT( OnEditPublish() ) );

	mActionEditOptions = new QAction( QIcon( ":/resources/zone.png" ), "&Options", this );
	mActionEditOptions->setShortcut( QKeySequence( "Ctrl+O" ) );
	connect( mActionEditOptions, SIGNAL( triggered() ), this, SLOT( OnEditOptions() ) );


	// Help

	mActionHelpAbout = new QAction( QIcon( ":/resources/about.png" ), "&About ModLauncherX", this );
	connect( mActionHelpAbout, SIGNAL( triggered() ), this, SLOT( OnHelpAbout() ) );

	mActionHelpOpenGitHub = new QAction( QIcon( ":/resources/OpenExternal.png" ), "&Open GitHub Repo...", this );
	connect( mActionHelpOpenGitHub, SIGNAL( triggered() ), this, SLOT( OnHelpOpenGitHubRepo() ) );

	mActionHelpReportIssue = new QAction( QIcon( ":/resources/ContactUs.png" ), "&Contact Us", this );
	connect( mActionHelpReportIssue, SIGNAL( triggered() ), this, SLOT( OnHelpReportIssue() ) );


	// Tools

	mActionToolsSaveLauncherOutput = new QAction( "&Save Output", this );
	mActionToolsSaveLauncherOutput->setToolTip( mlxToolTips[ TOOLTIP_SAVE_OUTPUT ] );
	connect( mActionToolsSaveLauncherOutput, &QAction::triggered, this, &MLXMainWindow::OnSaveOutputLog );

	mActionToolsCopyLauncherOutput = new QAction( "&Copy Output", this );
	mActionToolsCopyLauncherOutput->setToolTip( mlxToolTips[ TOOLTIP_COPY_OUTPUT ] );
	connect( mActionToolsCopyLauncherOutput, &QAction::triggered, this,
			 [ this ]()
			 {
				 QApplication::clipboard()->setText( mOutputWidget->toPlainText() );
			 }
	);

	mActionToolsConsoleMPLog = new QAction( "&Open console_mp.log" );
	mActionToolsConsoleMPLog->setToolTip( mlxToolTips[ TOOLTIP_OPEN_LOG ] );
	connect( mActionToolsConsoleMPLog, &QAction::triggered, this, &MLXMainWindow::OnOpenConsoleMPLog );

	mActionToolsMLXDebugLog = new QAction( "Open Debug Log" );
	connect( mActionToolsMLXDebugLog, &QAction::triggered, this,
			 []()
			 {
				 MLXUtils::open_folder_select_file( MLXUtils::DEBUG_LOG_PATH );
			 }
	);

	mActionToolsIgnoreErrors = new QAction( "Ignore Errors" );
	mActionToolsIgnoreErrors->setCheckable( true );

	// Folders

	QIcon folder_icon( ":/resources/folder.png" );

	mActionFolderRoot = new QAction( folder_icon, "&Open Root Folder", this );
	connect( mActionFolderRoot, &QAction::triggered, this, []()
			 {
				 MLXUtils::open_bo3_folder();
			 } );

	mActionFolderUsermaps = new QAction( folder_icon, "&Open Usermaps Folder", this );
	connect( mActionFolderUsermaps, &QAction::triggered, this,
			 []()
			 {
				 MLXUtils::open_bo3_folder( "usermaps" );
			 }
	);

	mActionFolderShare = new QAction( folder_icon, "&Open Share Folder", this );
	connect( mActionFolderShare, &QAction::triggered, this,
			 []()
			 {
				 MLXUtils::open_bo3_folder( "share" );
			 }
	);

	mActionFolderScriptsGlobal = new QAction( folder_icon, "&Open Global Scripts Folder", this );
	connect( mActionFolderScriptsGlobal, &QAction::triggered, this,
			 []()
			 {
				 MLXUtils::open_bo3_folder( "share/raw/scripts" );
			 }
	);

	mActionFolderSndAliases = new QAction( folder_icon, "&Open Sound Aliases Folder", this );
	connect( mActionFolderSndAliases, &QAction::triggered, this,
			 []()
			 {
				 MLXUtils::open_bo3_folder( "share/raw/sound/aliases" );
			 }
	);
}

void MLXMainWindow::CreateMenu()
{
	QMenuBar *MenuBar = new QMenuBar( this );

	QMenu *FileMenu = new QMenu( "&File", MenuBar );
	FileMenu->addAction( mActionFileNew );
	FileMenu->addSeparator();
	FileMenu->addAction( mActionFileAssetEditor );
	FileMenu->addAction( mActionFileLevelEditor );
	FileMenu->addAction( mActionFileExport2Bin );
	FileMenu->addSeparator();
	FileMenu->addAction( mActionFileExit );

	QMenu *EditMenu = new QMenu( "&Edit", MenuBar );
	EditMenu->addAction( mActionEditBuild );
	EditMenu->addAction( mActionEditPublish );
	EditMenu->addSeparator();
	EditMenu->addAction( mActionEditOptions );

	QMenu *ToolsMenu = new QMenu( "&Tools", MenuBar );
	ToolsMenu->addAction( mActionToolsSaveLauncherOutput );
	ToolsMenu->addAction( mActionToolsConsoleMPLog );
	ToolsMenu->addSeparator();
	ToolsMenu->addAction( mActionToolsMLXDebugLog );
	ToolsMenu->addAction( mActionToolsIgnoreErrors );

	QMenu *FoldersMenu = new QMenu( "F&olders", MenuBar );
	QList< QAction *> _actions;
	_actions << mActionFolderRoot << mActionFolderUsermaps
		<< mActionFolderShare << mActionFolderScriptsGlobal
		<< mActionFolderSndAliases;
	FoldersMenu->addActions( _actions );

	QMenu *HelpMenu = new QMenu( "&Help", MenuBar );
	HelpMenu->addAction( mActionHelpReportIssue );
	HelpMenu->addAction( mActionHelpOpenGitHub );
	HelpMenu->addAction( mActionHelpAbout );

	MenuBar->addAction( FileMenu->menuAction() );
	MenuBar->addAction( EditMenu->menuAction() );
	MenuBar->addAction( ToolsMenu->menuAction() );
	MenuBar->addAction( FoldersMenu->menuAction() );
	MenuBar->addAction( HelpMenu->menuAction() );

	setMenuBar( MenuBar );
}

void MLXMainWindow::CreateToolBar()
{
	QToolBar *ToolBar = new QToolBar( "Toolbar", this );
	ToolBar->setObjectName( QStringLiteral( "StandardToolBar" ) );

	ToolBar->addAction( mActionFileNew );
	ToolBar->addAction( mActionEditBuild );
	ToolBar->addAction( mActionEditPublish );
	//ToolBar->addSeparator();
	ToolBar->addAction( mActionFileAssetEditor );
	ToolBar->addAction( mActionFileLevelEditor );
	ToolBar->addAction( mActionFileExport2Bin );

	addToolBar( Qt::LeftToolBarArea, ToolBar );
}

// Seems like this isn't used - pv
/*
void MLXMainWindow::closeEvent( QCloseEvent *Event )
{
	QSettings Settings;
	Settings.beginGroup( "MainWindow" );
	Settings.setValue( "Geometry", saveGeometry() );
	Settings.setValue( "State", saveState() );
	Settings.endGroup();

	Event->accept();
}
*/

void MLXMainWindow::UpdateDB()
{
	if( mBuildThread )
		return;

	QList<QPair<QString, QStringList>> Commands;
	Commands.append( QPair<QString, QStringList>( QString( "%1/gdtdb/gdtdb.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() << "/update" ) );

	StartBuildThread( Commands );
}

void MLXMainWindow::StartBuildThread( const QList<QPair<QString, QStringList>> &Commands )
{
	mBuildButton->setText( "Cancel" );
	mOutputWidget->clear();

	//mBuildThread = new mlBuildThread( Commands, mIgnoreErrorsWidget->isChecked() );
	mBuildThread = new mlBuildThread( Commands, mActionToolsIgnoreErrors->isChecked() );
	connect( mBuildThread, SIGNAL( OutputReady( QString ) ), this, SLOT( BuildOutputReady( QString ) ) );
	connect( mBuildThread, SIGNAL( finished() ), this, SLOT( BuildFinished() ) );
	mBuildThread->start();
}

void MLXMainWindow::StartConvertThread( QStringList &pathList, QString &outputDir, bool allowOverwrite )
{
	mConvertThread = new mlConvertThread( pathList, outputDir, true, allowOverwrite );
	connect( mConvertThread, SIGNAL( OutputReady( QString ) ), this, SLOT( BuildOutputReady( QString ) ) );
	connect( mConvertThread, SIGNAL( finished() ), this, SLOT( BuildFinished() ) );
	mConvertThread->start();
}

void MLXMainWindow::ShowPublishDialog()
{
	QDialog Dialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	Dialog.setWindowTitle( "Publish Mod" );

	QVBoxLayout *Layout = new QVBoxLayout( &Dialog );

	QFormLayout *FormLayout = new QFormLayout();
	Layout->addLayout( FormLayout );

	QLineEdit *TitleWidget = new QLineEdit();
	TitleWidget->setText( mTitle );
	FormLayout->addRow( "Title:", TitleWidget );

	QLineEdit *DescriptionWidget = new QLineEdit();
	DescriptionWidget->setText( mDescription );
	FormLayout->addRow( "Description:", DescriptionWidget );

	QLineEdit *ThumbnailEdit = new QLineEdit();
	ThumbnailEdit->setText( mThumbnail );

	QToolButton *ThumbnailButton = new QToolButton();
	ThumbnailButton->setText( "..." );

	QHBoxLayout *ThumbnailLayout = new QHBoxLayout();
	ThumbnailLayout->setContentsMargins( 0, 0, 0, 0 );
	ThumbnailLayout->addWidget( ThumbnailEdit );
	ThumbnailLayout->addWidget( ThumbnailButton );

	QWidget *ThumbnailWidget = new QWidget();
	ThumbnailWidget->setLayout( ThumbnailLayout );

	FormLayout->addRow( "Thumbnail:", ThumbnailWidget );

	QTreeWidget *TagsTree = new QTreeWidget( &Dialog );
	TagsTree->setHeaderHidden( true );
	TagsTree->setUniformRowHeights( true );
	TagsTree->setRootIsDecorated( false );
	FormLayout->addRow( "Tags:", TagsTree );

	for( int TagIdx = 0; TagIdx < std::size( gTags ); TagIdx++ )
	{
		const char *Tag = gTags[ TagIdx ];
		QTreeWidgetItem *Item = new QTreeWidgetItem( TagsTree, QStringList() << Tag );
		Item->setCheckState( 0, mTags.contains( Tag ) ? Qt::Checked : Qt::Unchecked );
	}

	QFrame *Frame = new QFrame();
	Frame->setFrameShape( QFrame::HLine );
	Frame->setFrameShadow( QFrame::Raised );
	Layout->addWidget( Frame );

	QDialogButtonBox *ButtonBox = new QDialogButtonBox( &Dialog );
	ButtonBox->setOrientation( Qt::Horizontal );
	ButtonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
	ButtonBox->setCenterButtons( true );

	Layout->addWidget( ButtonBox );

	auto ThumbnailBrowse = [ = ]()
		{
			QString FileName = QFileDialog::getOpenFileName( this, "Open Thumbnail", QString(), "All Files (*.*)" );
			if( !FileName.isEmpty() )
				ThumbnailEdit->setText( FileName );
		};

	connect( ThumbnailButton, &QToolButton::clicked, ThumbnailBrowse );
	connect( ButtonBox, SIGNAL( accepted() ), &Dialog, SLOT( accept() ) );
	connect( ButtonBox, SIGNAL( rejected() ), &Dialog, SLOT( reject() ) );

	if( Dialog.exec() != QDialog::Accepted )
		return;

	mTitle = TitleWidget->text();
	mDescription = DescriptionWidget->text();
	mThumbnail = ThumbnailEdit->text();
	mTags.clear();

	for( int ChildIdx = 0; ChildIdx < TagsTree->topLevelItemCount(); ChildIdx++ )
	{
		QTreeWidgetItem *Child = TagsTree->topLevelItem( ChildIdx );
		if( Child->checkState( 0 ) == Qt::Checked )
			mTags.append( Child->text( 0 ) );
	}

	if( !SteamUGC() )
	{
		QMessageBox::information( this, "Error", "Could not initialize Steam, make sure you're running the launcher from the Steam client." );
		return;
	}

	if( !mFileId )
	{
		SteamAPICall_t SteamAPICall = SteamUGC()->CreateItem( AppId, k_EWorkshopFileTypeCommunity );
		mSteamCallResultCreateItem.Set( SteamAPICall, this, &MLXMainWindow::OnCreateItemResult );
	}
	else
		UpdateWorkshopItem();
}

void MLXMainWindow::UpdateWorkshopItem()
{
	QJsonObject Root;

	Root[ "PublisherID" ] = QString::number( mFileId );
	Root[ "Title" ] = mTitle;
	Root[ "Description" ] = mDescription;
	Root[ "Thumbnail" ] = mThumbnail;
	Root[ "Type" ] = mType;
	Root[ "FolderName" ] = mFolderName;
	Root[ "Tags" ] = mTags.join( ',' );

	QString WorkshopFile( mWorkshopFolder + "/workshop.json" );
	QFile File( WorkshopFile );

	if( !File.open( QIODevice::WriteOnly ) )
	{
		QMessageBox::warning( this, "Error", QString( "Error writing to file '%1'." ).arg( WorkshopFile ) );
		return;
	}

	File.write( QJsonDocument( Root ).toJson() );
	File.close();

	UGCUpdateHandle_t UpdateHandle = SteamUGC()->StartItemUpdate( AppId, mFileId );
	SteamUGC()->SetItemTitle( UpdateHandle, mTitle.toLatin1().constData() );
	SteamUGC()->SetItemDescription( UpdateHandle, mDescription.toLatin1().constData() );
	SteamUGC()->SetItemPreview( UpdateHandle, mThumbnail.toLatin1().constData() );
	SteamUGC()->SetItemContent( UpdateHandle, mWorkshopFolder.toLatin1().constData() );

	const char *TagList[ std::size( gTags ) ];
	SteamParamStringArray_t Tags;
	Tags.m_ppStrings = TagList;
	Tags.m_nNumStrings = 0;

	for( const QString &Tag : mTags )
	{
		QByteArray TagStr = Tag.toLatin1();

		for( int TagIdx = 0; TagIdx < std::size( gTags ); TagIdx++ )
		{
			if( TagStr == gTags[ TagIdx ] )
			{
				TagList[ Tags.m_nNumStrings++ ] = gTags[ TagIdx ];
				if( Tags.m_nNumStrings == std::size( TagList ) )
					break;
			}
		}
	}

	SteamUGC()->SetItemTags( UpdateHandle, &Tags );

	SteamAPICall_t SteamAPICall = SteamUGC()->SubmitItemUpdate( UpdateHandle, "" );
	mSteamCallResultUpdateItem.Set( SteamAPICall, this, &MLXMainWindow::OnUpdateItemResult );

	QProgressDialog Dialog( this );
	Dialog.setLabelText( QString( "Uploading workshop item '%1'..." ).arg( QString::number( mFileId ) ) );
	Dialog.setCancelButton( NULL );
	Dialog.setWindowModality( Qt::WindowModal );
	Dialog.show();

	for( ;;)
	{
		uint64 Processed, Total;

		const auto Status = SteamUGC()->GetItemUpdateProgress( SteamAPICall, &Processed, &Total );
		// if we get an invalid status exit out, it could mean we're finished or there's an actual problem
		if( Status == k_EItemUpdateStatusInvalid )
		{
			break;
		}

		QString status_msg;

		switch( Status )
		{
			case EItemUpdateStatus::k_EItemUpdateStatusInvalid:
				status_msg = "Invalid";
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingConfig:
				status_msg = "Preparing Config";
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingContent:
				status_msg = "Preparing Content";
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingContent:
				status_msg = "Uploading Content";
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingPreviewFile:
				status_msg = "Uploading Preview File";
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusCommittingChanges:
				status_msg = "Comitting changes";
				break;
		}

		Dialog.setLabelText( QString( "Uploading workshop item '%1': %2" ).arg( mFileId ).arg( status_msg ) );
		Dialog.setMaximum( Total );
		Dialog.setValue( Processed );
		QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
		Sleep( 100 );
	}
}


//======================================================
//===================== EXPORT2BIN =====================
//======================================================

Export2BinGroupBox::Export2BinGroupBox( QWidget *parent, MLXMainWindow *parent_window ): QGroupBox( parent ), parentWindow( parent_window )
{
	this->setAcceptDrops( true );
}

void MLXMainWindow::InitExport2BinGUI()
{
	QDockWidget *dock = new QDockWidget( this );
	dock->setWindowTitle( "Export2Bin" );
	dock->setFloating( true );

	QWidget *widget = new QWidget( dock );
	QGridLayout *gridLayout = new QGridLayout();
	widget->setLayout( gridLayout );
	dock->setWidget( widget );

	Export2BinGroupBox *groupBox = new Export2BinGroupBox( dock, this );
	gridLayout->addWidget( groupBox, 0, 0 );

	QLabel *label = new QLabel( "Drag Files Here", groupBox );
	label->setAlignment( Qt::AlignCenter );
	QVBoxLayout *groupBoxLayout = new QVBoxLayout( groupBox );
	groupBoxLayout->addWidget( label );
	groupBox->setLayout( groupBoxLayout );

	mExport2BinOverwriteWidget = new QCheckBox( "&Overwrite Existing Files", widget );
	gridLayout->addWidget( mExport2BinOverwriteWidget, 1, 0 );

	QSettings Settings;
	mExport2BinOverwriteWidget->setChecked( Settings.value( "Export2Bin_OverwriteFiles", true ).toBool() );

	QHBoxLayout *dirLayout = new QHBoxLayout();
	QLabel *dirLabel = new QLabel( "Ouput Directory:", widget );
	mExport2BinTargetDirWidget = new QLineEdit( widget );
	QToolButton *dirBrowseButton = new QToolButton( widget );
	dirBrowseButton->setText( "..." );

	const QDir defaultPath = QString( "%1/model_export/export2bin/" ).arg( MLXUtils::BO3_ROOT_PATH );
	mExport2BinTargetDirWidget->setText( Settings.value( "Export2Bin_TargetDir", defaultPath.absolutePath() ).toString() );

	connect( dirBrowseButton, SIGNAL( clicked() ), this, SLOT( OnExport2BinChooseDirectory() ) );
	connect( mExport2BinOverwriteWidget, SIGNAL( clicked() ), this, SLOT( OnExport2BinToggleOverwriteFiles() ) );

	dirBrowseButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
	dirLayout->addWidget( dirLabel );
	dirLayout->addWidget( mExport2BinTargetDirWidget );
	dirLayout->addWidget( dirBrowseButton );

	gridLayout->addLayout( dirLayout, 2, 0 );

	groupBox->setAcceptDrops( true );

	dock->resize( QSize( 256, 256 ) );

	mExport2BinGUIWidget = dock;
}

void Export2BinGroupBox::dragEnterEvent( QDragEnterEvent *event )
{
	event->acceptProposedAction();
}

void Export2BinGroupBox::dropEvent( QDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();

	if( parentWindow == NULL )
	{
		return;
	}

	if( mimeData->hasUrls() )
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		QDir working_dir( MLXUtils::BO3_ROOT_PATH );
		for( int i = 0; i < urlList.size(); i++ )
		{
			pathList.append( urlList.at( i ).toLocalFile() );
		}

		QProcess *Process = new QProcess();
		connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );

		bool allowOverwrite = this->parentWindow->mExport2BinOverwriteWidget->isChecked();

		QString outputDir = parentWindow->mExport2BinTargetDirWidget->text();
		parentWindow->StartConvertThread( pathList, outputDir, allowOverwrite );

		event->acceptProposedAction();
	}
}

void Export2BinGroupBox::dragLeaveEvent( QDragLeaveEvent *event )
{
	event->accept();
}
