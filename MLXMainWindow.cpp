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

#include <windows.h>
#include <utility>

#pragma comment(lib, "steam_api64.lib")

const int AppId = 311210;

const char *gTags[] = { "Animation", "Audio", "Character", "Map", "Mod", "Mode", "Model", "Multiplayer", "Scorestreak", "Skin", "Specialist", "Texture", "UI", "Vehicle", "Visual Effect", "Weapon", "WIP", "Zombies" };


MLXMainWindow::MLXMainWindow()
{
	QSettings Settings;

	mBuildThread = NULL;
	mBuildLanguage = Settings.value( "BuildLanguage", "english" ).toString();
	mTheme = Settings.value( "Theme", "T7x" ).toString();

	// Qt prefers '/' over '\\'
	mGamePath = QString( getenv( "TA_GAME_PATH" ) ).replace( '\\', '/' );
	mToolsPath = QString( getenv( "TA_TOOLS_PATH" ) ).replace( '\\', '/' );

	//qApp->setFont( QFont( GetCustomFont( LATO ).family(), 11 ) );

	setWindowIcon( QIcon( ":/resources/ModLauncher.png" ) );
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

	mFileListWidget = new QTreeWidget();
	mFileListWidget->setHeaderHidden( true );
	mFileListWidget->setUniformRowHeights( true );
	//mFileListWidget->setRootIsDecorated(false);
	mFileListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
	TopLayout->addWidget( mFileListWidget );

	connect( mFileListWidget, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( ContextMenuRequested() ) );

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
	ActionsLayout->addWidget( mRunOptionsWidget );

	mBuildButton = new QPushButton( "Build" );
	connect( mBuildButton, SIGNAL( clicked() ), mActionEditBuild, SLOT( trigger() ) );
	mBuildButton->setToolTip( mlxToolTips[ TOOLTIP_BUILD ] );
	ActionsLayout->addWidget( mBuildButton );

	mDvarsButton = new QPushButton( "Dvars" );
	connect( mDvarsButton, SIGNAL( clicked() ), this, SLOT( OnEditDvars() ) );
	mDvarsButton->setToolTip( mlxToolTips[ TOOLTIP_DVARS ] );
	ActionsLayout->addWidget( mDvarsButton );

	mSaveOutputButton = new QPushButton( "Save Output" );
	connect( mSaveOutputButton, SIGNAL( clicked() ), this, SLOT( OnSaveOutputLog() ) );
	mSaveOutputButton->setToolTip( mlxToolTips[ TOOLTIP_SAVE_OUTPUT ] );
	ActionsLayout->addWidget( mSaveOutputButton );

	mOpenLogButton = new QPushButton( "Open Log File" );
	connect( mOpenLogButton, SIGNAL( clicked() ), this, SLOT( OnOpenConsoleMPLog() ) );
	mOpenLogButton->setToolTip( mlxToolTips[ TOOLTIP_OPEN_LOG ] );
	ActionsLayout->addWidget( mOpenLogButton );

	mDebugButton = new QPushButton( "Apply Theme [DEBUG]" );
	connect( mDebugButton, SIGNAL( clicked() ), this, SLOT( OnDebugButtonPressed() ) );
	ActionsLayout->addWidget( mDebugButton );

	mIgnoreErrorsWidget = new QCheckBox( "Ignore Errors" );
	mIgnoreErrorsWidget->setToolTip( mlxToolTips[ TOOLTIP_IGNORE_ERRORS ] );
	ActionsLayout->addWidget( mIgnoreErrorsWidget );

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
	connect( SearchBar, SIGNAL( SearchBar->textEdited() ), this, SLOT( OnSearchTextChanged( const QString & text ) ) );
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

	mOutputWidget = new MLXPlainTextEdit( this );
	CentralWidget->addWidget( mOutputWidget );

	//mOutputWidget->DefaultOutputColor = mOutputWidget->textCursor().charFormat().foreground().color();

	LastMatchCursor = mOutputWidget->textCursor();

	setCentralWidget( CentralWidget );

	mShippedMapList << "mp_aerospace" << "mp_apartments" << "mp_arena" << "mp_banzai" << "mp_biodome"
		<< "mp_chinatown" << "mp_city" << "mp_conduit" << "mp_crucible" << "mp_cryogen" << "mp_ethiopia"
		<< "mp_freerun_01" << "mp_freerun_02" << "mp_freerun_03" << "mp_freerun_04" << "mp_havoc" << "mp_infection"
		<< "mp_kung_fu" << "mp_metro" << "mp_miniature" << "mp_nuketown_x" << "mp_redwood" << "mp_rise" << "mp_rome"
		<< "mp_ruins" << "mp_sector" << "mp_shrine" << "mp_skyjacked" << "mp_spire" << "mp_stronghold" << "mp_veiled"
		<< "mp_waterpark" << "mp_western" << "zm_castle" << "zm_factory" << "zm_genesis" << "zm_island" << "zm_levelcommon"
		<< "zm_stalingrad" << "zm_zod";

	Settings.beginGroup( "MainWindow" );
	resize( QSize( 800, 600 ) );
	move( QPoint( 200, 200 ) );
	restoreGeometry( Settings.value( "Geometry" ).toByteArray() );
	restoreState( Settings.value( "State" ).toByteArray() );
	Settings.endGroup();

	SteamAPI_Init();

	connect( &mTimer, SIGNAL( timeout() ), this, SLOT( SteamUpdate() ) );
	mTimer.start( 1000 );

	PopulateFileList();
	UpdateTheme();
}

MLXMainWindow::~MLXMainWindow()
{
}

void MLXMainWindow::HighlightAllMatches( const QString &text )
{
	// Clear any previous highlights
	ClearHighlights();

	QTextDocument *document = mOutputWidget->document();
	QTextCursor cursor( document );

	QTextCharFormat highlightFormat;
	highlightFormat.setBackground( HighlightColor );

	int matchCount = 0;
	while( !cursor.isNull() && !cursor.atEnd() )
	{
		// Find the next occurrence of the text
		cursor = document->find( text, cursor, GetSearchFlags() );

		if( !cursor.isNull() )
		{
			// Highlight the text
			cursor.mergeCharFormat( highlightFormat );

			// Save the first match as the one to jump to
			if( matchCount == 0 )
			{
				LastMatchCursor = cursor;
				mOutputWidget->setTextCursor( LastMatchCursor );
			}

			matchCount++;
		}
	}

	// If no matches were found, show a pop-up
	if( matchCount == 0 )
		ShowNoResultsPopup();
}

void MLXMainWindow::ClearHighlights()
{
	// Clear all formatting from the document
	QTextDocument *document = mOutputWidget->document();
	QTextCursor cursor( document );

	QTextCharFormat defaultFormat;
	defaultFormat.setBackground( Qt::transparent );

	while( !cursor.isNull() && !cursor.atEnd() )
	{
		cursor.select( QTextCursor::WordUnderCursor );
		cursor.setCharFormat( defaultFormat );
		cursor.movePosition( QTextCursor::NextWord );
	}

	// Reset the last match cursor
	LastMatchCursor = mOutputWidget->textCursor();
}

void MLXMainWindow::ShowNoResultsPopup()
{
	QMessageBox::information( this, "Search", "No results found." );
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
	mlxToolTips[ TOOLTIP_COMPILE ] =
		"Build/apply changes made in Radaint since last compile." +
		QString( "\nIf no changes were made, this doesn't need to be checked." );

	mlxToolTips[ TOOLTIP_LIGHT ] =
		"Rebuild lighting from scratch. To use lighting built in Radiant instead,\ngo to Radiant --> File " +
		QString( "--> Export Lighting, and make sure \n\"Export build lighting\" is checked (leave other checkboxes as default)." );

	mlxToolTips[ TOOLTIP_LINK ] =
		"Link all files (scripts, compiled Radiant map, sounds, lighting info, assets, etc.) together." +
		QString( "This must always be checked, as the Linker generates the files the game actually reads." );

	mlxToolTips[ TOOLTIP_RUN ] =
		"Runs the game after all build processes have completed.\nNote: This option will be ignored if there are any errors.";

	mlxToolTips[ TOOLTIP_EXEC_ARGS ] =
		"Extra arguments to run the game with.";

	mlxToolTips[ TOOLTIP_BUILD ] =
		"Executes all of the above checked components.";

	mlxToolTips[ TOOLTIP_DVARS ] =
		"Edit Dvars that the game runs with.";

	mlxToolTips[ TOOLTIP_SAVE_OUTPUT ] =
		"Save the current output of the launcher (the big text box at the bottom) to a \"logs\" folder in your BO3 root.";

	mlxToolTips[ TOOLTIP_OPEN_LOG ] =
		"Opens console_mp.log found in the BO3 root. This file is a log of everything printed to" +
		QString( "\nthe console during the game's runtime. You can enable the log in the Dvars menu above (under \"Build\")." );

	mlxToolTips[ TOOLTIP_IGNORE_ERRORS ] =
		"For advanced users only. This allows you to ignore errors that would otherwise halt the build process," +
		QString( "\nand continue building your map. Please don't use this unless you know what you're doing, as, you" ) +
		QString( "\nshould fix errors if they're not letting you build." );
}

void MLXMainWindow::CreateActions()
{
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

	mActionFileExit = new QAction( "E&xit", this );
	connect( mActionFileExit, SIGNAL( triggered() ), this, SLOT( close() ) );

	mActionEditBuild = new QAction( QIcon( ":/resources/Go.png" ), "Build", this );
	mActionEditBuild->setShortcut( QKeySequence( "Ctrl+B" ) );
	connect( mActionEditBuild, SIGNAL( triggered() ), this, SLOT( OnEditBuild() ) );

	mActionEditPublish = new QAction( QIcon( ":/resources/publish.png" ), "Publish", this );
	mActionEditPublish->setShortcut( QKeySequence( "Ctrl+P" ) );
	connect( mActionEditPublish, SIGNAL( triggered() ), this, SLOT( OnEditPublish() ) );

	mActionEditOptions = new QAction( "&Options...", this );
	connect( mActionEditOptions, SIGNAL( triggered() ), this, SLOT( OnEditOptions() ) );

	mActionHelpAbout = new QAction( "&About...", this );
	connect( mActionHelpAbout, SIGNAL( triggered() ), this, SLOT( OnHelpAbout() ) );
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
	MenuBar->addAction( FileMenu->menuAction() );

	QMenu *EditMenu = new QMenu( "&Edit", MenuBar );
	EditMenu->addAction( mActionEditBuild );
	EditMenu->addAction( mActionEditPublish );
	EditMenu->addSeparator();
	EditMenu->addAction( mActionEditOptions );
	MenuBar->addAction( EditMenu->menuAction() );

	QMenu *HelpMenu = new QMenu( "&Help", MenuBar );
	HelpMenu->addAction( mActionHelpAbout );
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

	const QDir defaultPath = QString( "%1/model_export/export2bin/" ).arg( mToolsPath );
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
	Commands.append( QPair<QString, QStringList>( QString( "%1/gdtdb/gdtdb.exe" ).arg( mToolsPath ), QStringList() << "/update" ) );

	StartBuildThread( Commands );
}

void MLXMainWindow::StartBuildThread( const QList<QPair<QString, QStringList>> &Commands )
{
	mBuildButton->setText( "Cancel" );
	mOutputWidget->clear();

	mBuildThread = new mlBuildThread( Commands, mIgnoreErrorsWidget->isChecked() );
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

void MLXMainWindow::PopulateFileList()
{
	mFileListWidget->clear();

	QString UserMapsFolder = QDir::cleanPath( QString( "%1/usermaps/" ).arg( mGamePath ) );
	QStringList UserMaps = QDir( UserMapsFolder ).entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	QTreeWidgetItem *MapsRootItem = new QTreeWidgetItem( mFileListWidget, QStringList() << "Maps" );

	QFont Font = MapsRootItem->font( 0 );
	Font.setBold( true );
	MapsRootItem->setFont( 0, Font );

	for( QString MapName : UserMaps )
	{
		QString ZoneFileName = QString( "%1/%2/zone_source/%3.zone" ).arg( UserMapsFolder, MapName, MapName );

		if( QFileInfo( ZoneFileName ).isFile() )
		{
			QTreeWidgetItem *MapItem = new QTreeWidgetItem( MapsRootItem, QStringList() << MapName );
			MapItem->setCheckState( 0, Qt::Unchecked );
			MapItem->setData( 0, Qt::UserRole, ML_ITEM_MAP );
		}
	}

	QString ModsFolder = QDir::cleanPath( QString( "%1/mods/" ).arg( mGamePath ) );
	QStringList Mods = QDir( ModsFolder ).entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	QTreeWidgetItem *ModsRootItem = new QTreeWidgetItem( mFileListWidget, QStringList() << "Mods" );
	ModsRootItem->setFont( 0, Font );
	const char *Files[ 4 ] = { "core_mod", "mp_mod", "cp_mod", "zm_mod" };

	for( QString ModName : Mods )
	{
		QTreeWidgetItem *ParentItem = NULL;

		for( int FileIdx = 0; FileIdx < 4; FileIdx++ )
		{
			QString ZoneFileName = QString( "%1/%2/zone_source/%3.zone" ).arg( ModsFolder, ModName, Files[ FileIdx ] );

			if( QFileInfo( ZoneFileName ).isFile() )
			{
				if( !ParentItem )
					ParentItem = new QTreeWidgetItem( ModsRootItem, QStringList() << ModName );

				QTreeWidgetItem *ModItem = new QTreeWidgetItem( ParentItem, QStringList() << Files[ FileIdx ] );
				ModItem->setCheckState( 0, Qt::Unchecked );
				ModItem->setData( 0, Qt::UserRole, ML_ITEM_MOD );
			}
		}
	}

	mFileListWidget->expandAll();
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

void MLXMainWindow::UpdateTheme()
{
	if( mTheme == "Default" )
	{
		qApp->setStyle( "WindowsVista" );
		qApp->setStyleSheet( "" );
		mOutputWidget->setFont( DefaultFont );
		if( mOutputWidget != nullptr )
		{
			mOutputWidget->DefaultOutputColor = QColor( "#242323" );
		}
	}
	else if( mTheme == "Default Dark" )
	{
		qApp->setStyle( "Fusion" );
		qApp->setStyleSheet( "" );
		mOutputWidget->setFont( DefaultFont );
		if( mOutputWidget != nullptr )
		{
			mOutputWidget->DefaultOutputColor = QColor( "#242323" );
		}
	}
	else if( mTheme == "Modern" )
	{
		qApp->setStyle( "Windows11" );
		qApp->setStyleSheet( "" );
		mOutputWidget->setFont( DefaultFont );
		if( mOutputWidget != nullptr )
		{
			mOutputWidget->DefaultOutputColor = QColor( "#242323" );
		}
	}
	else if( mTheme == "Treyarch" )
	{
		qApp->setStyle( "Windows" );
		QFile file( QString( "%1/radiant/stylesheet.qss" ).arg( mToolsPath ) );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		file.close();
		qApp->setStyleSheet( styleSheet );
		mOutputWidget->setFont( DefaultFont );
	}
	else if( mTheme == "T7x" )
	{
		qApp->setStyle( "Windows" );
		QFile file( QString( "%1/bin/t7x_themes/pv_t7x_stylesheet.qss" ).arg( mToolsPath ) );
		file.open( QFile::ReadOnly );
		QString styleSheet = QLatin1String( file.readAll() );
		file.close();
		qApp->setStyleSheet( styleSheet );
		mOutputWidget->setFont( QFont( QFontDatabase::applicationFontFamilies( RegisteredFonts[ FIRA_CODE ] ).at( 0 ) ) );
		if( mOutputWidget != nullptr )
		{
			mOutputWidget->DefaultOutputColor = QColor( "#b2b2b2" );
		}
	}
}

void MLXMainWindow::RegisterFonts()
{
	RegisteredFonts[ FIRA_CODE ] = QFontDatabase::addApplicationFont( ":resources/fonts/FiraCode-Regular.ttf" );
	RegisteredFonts[ INTER ] = QFontDatabase::addApplicationFont( ":resources/fonts/Inter_18pt-Regular.ttf" );
	RegisteredFonts[ LATO ] = QFontDatabase::addApplicationFont( ":resources/fonts/Lato-Regular.ttf" );

	QString font_out = QString( "'%1'\n" ).arg( qApp->font().family() );
	for( int id : RegisteredFonts.values() )
	{
		if( id == -1 )
		{
			QMessageBox::critical( this, QString( "ERROR" ), QString( "One or more fonts failed to register!" ) );
		}

		font_out.append( QString( "'%1'\n" ).arg( QFontDatabase::applicationFontFamilies( id ).at( 0 ) ) );
	}

	DefaultFont = qApp->font().family();
	//QMessageBox::information( this, QString( "Fonts" ), QString( "Successfully loaded fonts:\n%1" ).arg( font_out ) );
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

		switch( Status )
		{
			case EItemUpdateStatus::k_EItemUpdateStatusInvalid:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Invalid" ) ) );
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingConfig:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Preparing Config" ) ) );
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusPreparingContent:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Preparing Content" ) ) );
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingContent:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Uploading Content" ) ) );
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusUploadingPreviewFile:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Uploading Preview file" ) ) );
				break;
			case EItemUpdateStatus::k_EItemUpdateStatusCommittingChanges:
				Dialog.setLabelText(
					QString( "Uploading workshop item '%1': %2" ).arg( QString::number( mFileId ), QString( "Committing Changes" ) ) );
				break;
		}

		Dialog.setMaximum( Total );
		Dialog.setValue( Processed );
		QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
		Sleep( 100 );
	}
}



Export2BinGroupBox::Export2BinGroupBox( QWidget *parent, MLXMainWindow *parent_window ): QGroupBox( parent ), parentWindow( parent_window )
{
	this->setAcceptDrops( true );
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

		QDir working_dir( parentWindow->mToolsPath );
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
