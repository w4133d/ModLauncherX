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
#include "dvar.h"
#include "MLXUtils.h"

#include <iosfwd>
#include <sstream>
#include <windows.h>
#include <iomanip>
#include <string>

/*
* ========= ModLauncherX Main Window Callbacks =========
* I've split the GUI callbacks for the main window
* into here, because MLXMainWindow.cpp was getting
* cluttered. Literally, MLXMainWindow.cpp was like
* 2k lines long lmao, I got sick of it. So ye that's
* all this file is.
*/


const char *gLanguages[] = {
	"english", "french", "italian", "spanish", "german", "portuguese", "russian",
	"polish", "japanese", "traditionalchinese", "simplifiedchinese", "englisharabic"
};
dvar_s gDvars[] = {
	{ "developer", "Shows error messages instead of crashing the game. Dev 2 catches all errors, mainly used for scripting.", DVAR_VALUE_INT, 0, 2 },
	{ "logfile", "Outputs the game's console to \"console_mp.log\" in the root folder. Useful for debugging in dev 1.", DVAR_VALUE_INT, 0, 2 },
	{ "com_clientFieldsDebug", "Allows you to debug clientfield errors. The error with this enabled will expose what scripts are causing the issue.", DVAR_VALUE_BOOL },
	{ "scr_mod_enable_devblock", "Developer blocks are executed in mods ", DVAR_VALUE_BOOL },
	/*{ "connect", "Connect to a specific server", DVAR_VALUE_STRING, NULL, NULL, true },*/
	/*{ "g_password", "Password for your server", DVAR_VALUE_STRING },*/
	{ "splitscreen", "Enable splitscreen", DVAR_VALUE_BOOL },
	{ "splitscreen_playerCount", "Allocate the number of instances for splitscreen", DVAR_VALUE_INT, 0,  2},
	{ "ai_disableSpawn", "Disable AI from spawning (doesn't work for some reason, you have to enable this in-game)", DVAR_VALUE_BOOL },
	{ "set_gametype", "Set a gametype to load with map", DVAR_VALUE_STRING, NULL, NULL, true }
};


void MLXMainWindow::OnSearchTextChanged( const QString &text )
{
	ClearHighlights();
	if( text.isEmpty() )
	{
		return;
	}

	HighlightAllMatches( text );
}


void MLXMainWindow::OnFindNext()
{
	//ClearHighlights();

	QString text = SearchBar->text();
	if( text.isEmpty() )
		return;

	// Find the next ocurrence of the text
	LastMatchCursor = mOutputWidget->document()->find( text, LastMatchCursor, GetSearchFlags() );

	if( !LastMatchCursor.isNull() )
	{
		// Move the cursor to the next occurrence
		mOutputWidget->setTextCursor( LastMatchCursor );
		//HighlightAllMatches( text );
	}
	else
	{
		// Break wrapping so we don't infinitely wrap
		if( WrapCount > 1 )
		{
			WrapCount = 0;
			return;
		}

		// If no more results, wrap to the top and find again
		LastMatchCursor = mOutputWidget->textCursor();
		LastMatchCursor.movePosition( QTextCursor::Start );
		WrapCount++;
		OnFindNext();
	}
}


void MLXMainWindow::OnReturnPressed()
{

	OnFindNext();
}


void MLXMainWindow::SteamUpdate()
{
	SteamAPI_RunCallbacks();
}


void MLXMainWindow::OnFileAssetEditor()
{
	QProcess *Process = new QProcess();
	connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );
	Process->start( QString( "%1/bin/AssetEditor_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() );
}


void MLXMainWindow::OnFileLevelEditor()
{
	QProcess *Process = new QProcess();
	connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );

	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.count() && item_list[ 0 ]->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = item_list[ 0 ]->text( 0 );
		Process->start( QString( "%1/bin/radiant_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() << QString( "%1/map_source/%2/%3.map" ).arg( MLXUtils::BO3_ROOT_PATH, MapName.left( 2 ), MapName ) );
	}
	else
	{
		Process->start( QString( "%1/bin/radiant_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() );
	}
}


void MLXMainWindow::OnFileExport2Bin()
{
	if( mExport2BinGUIWidget == NULL )
	{
		InitExport2BinGUI();
		mExport2BinGUIWidget->hide(); // Ensure the window is hidden (just in case)
	}

	mExport2BinGUIWidget->isVisible() ? mExport2BinGUIWidget->hide() : mExport2BinGUIWidget->show();
}


void MLXMainWindow::OnFileNew()
{
	QDir TemplatesFolder( QString( "%1/rex/templates" ).arg( MLXUtils::BO3_ROOT_PATH ) );
	QStringList Templates = TemplatesFolder.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

	if( Templates.isEmpty() )
	{
		QMessageBox::information( this, "Error", "Could not find any map templates." );
		return;
	}

	QDialog Dialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	Dialog.setWindowTitle( "New Map or Mod" );

	QVBoxLayout *Layout = new QVBoxLayout( &Dialog );

	QFormLayout *FormLayout = new QFormLayout();
	Layout->addLayout( FormLayout );

	QLineEdit *NameWidget = new QLineEdit();
	NameWidget->setValidator( new QRegularExpressionValidator( QRegularExpression( "[a-zA-Z0-9_]*" ), this ) );
	FormLayout->addRow( "Name:", NameWidget );

	QComboBox *TemplateWidget = new QComboBox();
	TemplateWidget->addItems( Templates );
	FormLayout->addRow( "Template:", TemplateWidget );

	QFrame *Frame = new QFrame();
	Frame->setFrameShape( QFrame::HLine );
	Frame->setFrameShadow( QFrame::Raised );
	Layout->addWidget( Frame );

	QDialogButtonBox *ButtonBox = new QDialogButtonBox( &Dialog );
	ButtonBox->setOrientation( Qt::Horizontal );
	ButtonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
	ButtonBox->setCenterButtons( true );

	Layout->addWidget( ButtonBox );

	connect( ButtonBox, SIGNAL( accepted() ), &Dialog, SLOT( accept() ) );
	connect( ButtonBox, SIGNAL( rejected() ), &Dialog, SLOT( reject() ) );

	if( Dialog.exec() != QDialog::Accepted )
		return;

	QString Name = NameWidget->text().toLower();

	if( Name.isEmpty() )
	{
		QMessageBox::critical( this, "Invalid Name", "Map name cannot be empty." );
		return;
	}

	if( mShippedMapList.contains( Name ) )
	{
		QMessageBox::critical( this, "Invalid Name", "Map name cannot be the same as a stock map. Those names are preserved." );
		return;
	}

	if( mFileListWidget->findItems( Name, Qt::MatchExactly ).size() )
	{
		QMessageBox::critical( this, "Map Already Exists", "A map with that name already exists in the launcher!" );
		return;
	}

	QByteArray MapName = Name.toLatin1();
	QString Output;

	QString Template = Templates[ TemplateWidget->currentIndex() ];

	if( ( Template == "MP Mod Level" && !MapName.startsWith( "mp_" ) ) )
	{
		QMessageBox::critical( this, "Invalid Name", "Multiplayer maps must start with 'mp_'!" );
		return;
	}

	if( Template == "ZM Mod Level" && !MapName.startsWith( "zm_" ) )
	{
		QMessageBox::critical( this, "Invalid Name", "Zombies maps must start with 'zm_'!" );
		return;
	}

	std::function<bool( const QString &, const QString & )> RecursiveCopy = [ & ]( const QString &SourcePath, const QString &DestPath ) -> bool
	{
		QDir Dir( SourcePath );
		if( !Dir.exists() )
			return false;

		foreach( QString DirEntry, Dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
		{
			QString NewPath = QString( DestPath + QDir::separator() + DirEntry ).replace( QString( "template" ), MapName );
			Dir.mkpath( NewPath );
			if( !RecursiveCopy( SourcePath + QDir::separator() + DirEntry, NewPath ) )
				return false;
		}

		foreach( QString DirEntry, Dir.entryList( QDir::Files ) )
		{
			QFile SourceFile( SourcePath + QDir::separator() + DirEntry );
			QString DestFileName = QString( DestPath + QDir::separator() + DirEntry ).replace( QString( "template" ), MapName );
			QFile DestFile( DestFileName );

			if( !SourceFile.open( QFile::ReadOnly ) || !DestFile.open( QFile::WriteOnly ) )
				return false;

			while( !SourceFile.atEnd() )
			{
				QByteArray Line = SourceFile.readLine();

				if( Line.contains( "guid" ) )
				{
					QString LineString( Line );
					LineString.replace( QRegularExpression( "guid \"\\{(.*)\\}\"" ), QString( "guid \"%1\"" ).arg( QUuid::createUuid().toString() ) );
					Line = LineString.toLatin1();
				}
				else
					Line.replace( "template", MapName );

				DestFile.write( Line );
			}

			Output += DestFileName + "\n";
		}

		return true;
	};

	if( RecursiveCopy( TemplatesFolder.absolutePath() + QDir::separator() + Templates[ TemplateWidget->currentIndex() ], QDir::cleanPath( MLXUtils::BO3_ROOT_PATH ) ) )
	{
		//PopulateFileList();
		mFileListWidget->populate_file_tree();

		QMessageBox::information( this, "New Map Created", QString( "Files created:\n" ) + Output );
	}
	else
		QMessageBox::critical( this, "Error", "Error creating map files." );
}


void MLXMainWindow::OnEditBuild()
{
	if( mBuildThread )
	{
		mBuildThread->Cancel();
		return;
	}

	QList<QPair<QString, QStringList>> Commands;
	bool UpdateAdded = false;

	auto AddUpdateDBCommand = [ & ]()
	{
		if( !UpdateAdded )
		{
			Commands.append( QPair<QString, QStringList>( QString( "%1/gdtdb/gdtdb.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() << "/update" ) );
			UpdateAdded = true;
		}
	};

	QList<QTreeWidgetItem *> CheckedItems;

	std::function<void( QTreeWidgetItem * )> SearchCheckedItems = [ & ]( QTreeWidgetItem *ParentItem ) -> void
	{
		for( int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++ )
		{
			QTreeWidgetItem *Child = ParentItem->child( ChildIdx );
			if( Child->checkState( 0 ) == Qt::Checked )
				CheckedItems.append( Child );
			else
				SearchCheckedItems( Child );
		}
	};

	SearchCheckedItems( mFileListWidget->invisibleRootItem() );
	QString LastMap, LastMod;

	QStringList Languageargs;
	Languageargs;

	if( mBuildLanguage != "All" )
		Languageargs << "-language" << mBuildLanguage;
	else 
		for( const QString &Language : gLanguages )
			Languageargs << "-language" << Language;

	for( QTreeWidgetItem *Item : CheckedItems )
	{
		if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
		{
			QString MapName = Item->text( 0 );

			if( mCompileEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				QStringList args;
				args << "-platform" << "pc";

				if( mCompileModeWidget->currentIndex() == 0 )
					args << "-onlyents";
				else
					args << "-navmesh" << "-navvolume";

				args << "-loadFrom" << QString( "%1\\map_source\\%2\\%3.map" ).arg( MLXUtils::BO3_ROOT_PATH, MapName.left( 2 ), MapName );
				args << QString( "%1\\share\\raw\\maps\\%2\\%3.d3dbsp" ).arg( MLXUtils::BO3_ROOT_PATH, MapName.left( 2 ), MapName );

				Commands.append( QPair<QString, QStringList>( QString( "%1\\bin\\cod2map64.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), args ) );
			}

			if( mLightEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				QStringList args;
				args << "-ledSilent";

				switch( mLightQualityWidget->currentIndex() )
				{
					case 0:
						args << "+low";
						break;

					default:
					case 1:
						args << "+medium";
						break;

					case 2:
						args << "+high";
						break;
				}

				args << "+localprobes" << "+forceclean" << "+recompute" << QString( "%1/map_source/%2/%3.map" ).arg( MLXUtils::BO3_ROOT_PATH, MapName.left( 2 ), MapName );
				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/radiant_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), args ) );
			}

			if( mLinkEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/linker_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() << Languageargs << "-modsource" << MapName ) );
			}

			LastMap = MapName;
		}
		else
		{
			QString ModName = Item->parent()->text( 0 );

			if( mLinkEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				QString ZoneName = Item->text( 0 );
				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/linker_modtools.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), QStringList() << Languageargs << "-fs_game" << ModName << "-modsource" << ZoneName ) );
			}

			LastMod = ModName;
		}
	}

	if( mRunEnabledWidget->isChecked() && ( !LastMod.isEmpty() || !LastMap.isEmpty() ) )
	{
		QStringList args;

		if( !mRunDvars.isEmpty() )
			args << mRunDvars;

		args << "+set" << "fs_game" << ( LastMod.isEmpty() ? LastMap : LastMod );

		if( !LastMap.isEmpty() )
			args << "+devmap" << LastMap;

		QString ExtraOptions = mRunOptionsWidget->text();
		if( !ExtraOptions.isEmpty() )
			args << ExtraOptions.split( ' ' );

		Commands.append( QPair<QString, QStringList>( QString( "%1/BlackOps3.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), args ) );
	}

	if( Commands.size() == 0 && !UpdateAdded )
	{
		QMessageBox::information( this, "No Tasks Selected", "Please selected at least one file from the list and one action to be performed." );
		return;
	}

	if( mCleanXPAKsWidget->isChecked() )
	{
		OnCleanXPaks();
	}

	StartBuildThread( Commands );
}


void MLXMainWindow::OnEditPublish()
{
	std::function<QTreeWidgetItem *( QTreeWidgetItem * )> SearchCheckedItem = [ & ]( QTreeWidgetItem *ParentItem ) -> QTreeWidgetItem *
	{
		for( int ChildIdx = 0; ChildIdx < ParentItem->childCount(); ChildIdx++ )
		{
			QTreeWidgetItem *Child = ParentItem->child( ChildIdx );
			if( Child->checkState( 0 ) == Qt::Checked )
				return Child;

			QTreeWidgetItem *Checked = SearchCheckedItem( Child );
			if( Checked )
				return Checked;
		}

		return nullptr;
	};

	QTreeWidgetItem *Item = SearchCheckedItem( mFileListWidget->invisibleRootItem() );
	if( !Item )
	{
		QMessageBox::warning( this, "Error", "No maps or mods checked." );
		return;
	}

	QString Folder;
	if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		Folder = "usermaps/" + Item->text( 0 );
		mType = "map";
		mFolderName = Item->text( 0 );
	}
	else
	{
		Folder = "mods/" + Item->parent()->text( 0 );
		mType = "mod";
		mFolderName = Item->parent()->text( 0 );
	}

	mWorkshopFolder = QString( "%1/%2/zone" ).arg( MLXUtils::BO3_ROOT_PATH, Folder );
	QFile File( mWorkshopFolder + "/workshop.json" );

	if( !QFileInfo( mWorkshopFolder ).isDir() )
	{
		QMessageBox::information( this, "Error", QString( "The folder '%1' does not exist." ).arg( mWorkshopFolder ) );
		return;
	}

	mFileId = 0;
	mTitle.clear();
	mDescription.clear();
	mThumbnail.clear();
	mTags.clear();

	if( File.open( QIODevice::ReadOnly ) )
	{
		QJsonDocument Document = QJsonDocument::fromJson( File.readAll() );
		QJsonObject Root = Document.object();

		mFileId = Root[ "PublisherID" ].toString().toULongLong();
		mTitle = Root[ "Title" ].toString();
		mDescription = Root[ "Description" ].toString();
		mThumbnail = Root[ "Thumbnail" ].toString();
		mTags = Root[ "Tags" ].toString().split( ',' );
	}

	if( mFileId )
	{
		SteamAPICall_t SteamAPICall = SteamUGC()->RequestUGCDetails( mFileId, 10 );
		mSteamCallResultRequestDetails.Set( SteamAPICall, this, &MLXMainWindow::OnUGCRequestUGCDetails );
	}
	else
		ShowPublishDialog();
}


void MLXMainWindow::OnUGCRequestUGCDetails( SteamUGCRequestUGCDetailsResult_t *RequestDetailsResult, bool IOFailure )
{
	if( IOFailure || RequestDetailsResult->m_details.m_eResult != k_EResultOK )
	{
		QMessageBox::warning( this, "Error", "Error retrieving item data from the Steam Workshop." );
		return;
	}

	SteamUGCDetails_t *Details = &RequestDetailsResult->m_details;

	mTitle = Details->m_rgchTitle;
	mDescription = Details->m_rgchDescription;
	mTags = QString( Details->m_rgchTags ).split( ',' );

	ShowPublishDialog();
}


void MLXMainWindow::OnEditOptions()
{
	QDialog Dialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	Dialog.setWindowTitle( "ModLauncherX Options" );
	Dialog.resize( QSize( 250, 30 ) );
	//QMessageBox::information( this, QString( "Yo" ), QString( "Width: %1\nHeight: %2" ).arg( QString().setNum( Dialog.size().width()  ), QString().setNum( Dialog.size().height() ) ) );

	QSettings Settings;

	QVBoxLayout *Layout = new QVBoxLayout( &Dialog );

	//QCheckBox* TreyarchThemeCheckbox = new QCheckBox("Use Treyarch Theme");
	//TreyarchThemeCheckbox->setToolTip("Toggle between the dark grey Treyarch colors and the default Windows colors");
	//TreyarchThemeCheckbox->setChecked(Settings.value("UseDarkTheme", false).toBool());
	//Layout->addWidget( TreyarchThemeCheckbox );

	// THEME STUFF //

	// Monospace font checkbox
	QCheckBox *UseMonoOutput = new QCheckBox( "Output Uses Monospace Font" );
	UseMonoOutput->setToolTip( mlxToolTips[ TOOLTIP_OUTPUT_USES_MONO_FONT ] );
	UseMonoOutput->setChecked( Settings.value( "UseMonoOutputFont", true ).toBool() );
	Layout->addWidget( UseMonoOutput );

	// Color codes checkbox
	QCheckBox *UseColorCodes = new QCheckBox( "Output Uses Color Codes" );
	UseColorCodes->setToolTip( mlxToolTips[ TOOLTIP_OUTPUT_USES_COLOR_CODES ] );
	UseColorCodes->setChecked( Settings.value( "UseColorCodes", true ).toBool() );
	Layout->addWidget( UseColorCodes );

	// Color codes checkbox
	QCheckBox *ClearOutputOnRebuild = new QCheckBox( "Output cleared on rebuild" );
	ClearOutputOnRebuild->setToolTip( mlxToolTips[ TOOLTIP_OUTPUT_USES_COLOR_CODES ] );
	ClearOutputOnRebuild->setChecked( Settings.value( "ClearOutputOnRebuild", true ).toBool() );
	Layout->addWidget( ClearOutputOnRebuild );

	// Theme dropdown
	QHBoxLayout *ThemeLayout = new QHBoxLayout();
	ThemeLayout->addWidget( new QLabel( "Theme" ) );

	QStringList Themes;
	Themes << "Default" << "Windows" << "Legacy" << "Modern" << "Treyarch" << "T7x";

	QComboBox *ThemeCombo = new QComboBox();
	ThemeCombo->addItems( Themes );
	ThemeCombo->setCurrentText( mTheme );
	//ThemeCombo->setFixedWidth( 110 );
	//ThemeCombo->setFixedHeight( 20 );
	ThemeLayout->addWidget( ThemeCombo );

	Layout->addLayout( ThemeLayout );

	// SEARCH HIGHLIGHT COLOUR //

	QHBoxLayout *search_colour_layout = new QHBoxLayout();
	search_colour_layout->addWidget( new QLabel( "Search Color" ) );

	QStringList highlight_colours;
	highlight_colours << "Red" << "Orange" << "Yellow"
		<< "Green" << "Cyan" << "Blue" << "Violet";

	QComboBox *search_colours_combo = new QComboBox();
	search_colours_combo->addItems( highlight_colours );
	search_colours_combo->setCurrentText( Settings.value( "SearchHighlightColor", "Yellow" ).toString() );
	search_colours_combo->setToolTip( mlxToolTips[ TOOLTIP_SEARCH_HIGHLIGHT_COLOR ] );

	search_colour_layout->addWidget( search_colours_combo );

	Layout->addLayout( search_colour_layout );

	// BUILD LANG //

	QHBoxLayout *LanguageLayout = new QHBoxLayout();
	LanguageLayout->addWidget( new QLabel( "Build Language" ) );

	QStringList Languages;
	Languages << "All";
	for( int LanguageIdx = 0; LanguageIdx < ARRAYSIZE( gLanguages ); LanguageIdx++ )
		Languages << gLanguages[ LanguageIdx ];

	QComboBox *LanguageCombo = new QComboBox();
	LanguageCombo->addItems( Languages );
	LanguageCombo->setCurrentText( mBuildLanguage );
	//LanguageCombo->setFixedWidth( 110 );
	//LanguageCombo->setFixedHeight( 20 );
	LanguageLayout->addWidget( LanguageCombo );

	Layout->addLayout( LanguageLayout );

	// OK CANCEL //

	QDialogButtonBox *ButtonBox = new QDialogButtonBox( &Dialog );
	ButtonBox->setOrientation( Qt::Horizontal );
	ButtonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
	ButtonBox->setCenterButtons( true );

	Layout->addWidget( ButtonBox );

	connect( ButtonBox, SIGNAL( accepted() ), &Dialog, SLOT( accept() ) );
	connect( ButtonBox, SIGNAL( rejected() ), &Dialog, SLOT( reject() ) );

	if( Dialog.exec() != QDialog::Accepted )
		return;

	// Apply settings to class
	mBuildLanguage = LanguageCombo->currentText();
	mTheme = ThemeCombo->currentText();
	//mlx_output_font = UseMonoOutput->isChecked() ? GetFont( FIRA_CODE ) : DefaultFont;
	syntax_highlighter->setDocument( UseColorCodes->isChecked() ? mOutputWidget->document() : nullptr);
	
	/*
	Color			Hex Code		Notes
	Red				#FFB3B3			Soft red, not too intense.
	Orange			#FFD9B3			Warm and pale, good for light or dark themes.
	Yellow			#FFF2B3			Muted pastel yellow, avoids eye strain.
	Green			#C2E5B3			Soft green, natural and readable.
	Cyan			#B3E5E5			Gentle cyan, cool and calm.
	Blue			#B3C6FF			Light blue, soft but visible.
	Violet			#D3B3FF			Pale purple, easy on the eyes.
	*/

	QString hl_color = search_colours_combo->currentText();
	if( hl_color == "Red" )
		SearchHighlightColor = QColor( "#FFB3B3" );
	else if( hl_color == "Orange" )
		SearchHighlightColor = QColor( "#FFD983" );
	else if( hl_color == "Yellow" )
		SearchHighlightColor = QColor( "#FFF2B3" );
	else if( hl_color == "Green" )
		SearchHighlightColor = QColor( "#C2E5B3" );
	else if( hl_color == "Cyan" )
		SearchHighlightColor = QColor( "#B3E5E5" );
	else if( hl_color == "Blue" )
		SearchHighlightColor = QColor( "#B3C6FF" );
	else if( hl_color == "Violet" )
		SearchHighlightColor = QColor( "#D3B3FF" );


	for( QString colour : highlight_colours )
	{
		if( search_colours_combo->currentText() == colour )
			SearchHighlightColor = colour;
	}

	// Save settings
	Settings.setValue( "BuildLanguage", mBuildLanguage );
	Settings.setValue( "Theme", mTheme );
	Settings.setValue( "UseMonoOutputFont", UseMonoOutput->isChecked() );
	Settings.setValue( "UseColorCodes", UseColorCodes->isChecked() );
	Settings.setValue( "SearchHighlightColor", hl_color );

	UpdateTheme();
	StatusBar->showMessage( "Theme updated. Please restart ModLauncherX if you encounter any visual bugs - pv", 7000 );
}


void MLXMainWindow::OnEditDvars()
{
	QDialog Dialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	Dialog.setWindowTitle( "Dvar Options" );

	QVBoxLayout *Layout = new QVBoxLayout( &Dialog );

	QLabel *Label = new QLabel( &Dialog );
	Label->setText(
		"Dvars that are to be used when you run the game.\nYou must press \"OK\" in order to save the values!\n" +
		QString( "Note: Restarting the launcher will stop applying these Dvars.\n" ) +
		QString( "You must reopen this window and click \"OK\" to re-apply them upon restarting." )
	);
	Layout->addWidget( Label );

	QTreeWidget *DvarTree = new QTreeWidget( &Dialog );
	DvarTree->setColumnCount( 2 );
	DvarTree->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
	DvarTree->setHeaderLabels( QStringList() << "Dvar" << "Value" );
	DvarTree->setUniformRowHeights( true );
	DvarTree->setRootIsDecorated( false );
	Layout->addWidget( DvarTree );

	QDialogButtonBox *ButtonBox = new QDialogButtonBox( &Dialog );
	ButtonBox->setOrientation( Qt::Horizontal );
	ButtonBox->setStandardButtons( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
	ButtonBox->setCenterButtons( true );

	Layout->addWidget( ButtonBox );

	for( int idx = 0; idx < ARRAYSIZE( gDvars ); idx++ )
		Dvar( gDvars[ idx ], DvarTree );

	connect( ButtonBox, SIGNAL( accepted() ), &Dialog, SLOT( accept() ) );
	connect( ButtonBox, SIGNAL( rejected() ), &Dialog, SLOT( reject() ) );

	if( Dialog.exec() != QDialog::Accepted )
		return;

	int size = 0;
	QSettings settings;
	QString dvarName, dvarValue;
	QTreeWidgetItemIterator it( DvarTree );

	mRunDvars.clear();
	while( *it && size < ARRAYSIZE( gDvars ) )
	{
		QWidget *widget = DvarTree->itemWidget( *it, 1 );
		dvarName = ( *it )->data( 0, 0 ).toString();
		dvar_s dvar = Dvar::findDvar( dvarName, DvarTree, gDvars, ARRAYSIZE( gDvars ) );
		switch( dvar.type )
		{
			case DVAR_VALUE_BOOL:
				dvarValue = Dvar::setDvarSetting( dvar, (QCheckBox *) widget );
				break;
			case DVAR_VALUE_INT:
				dvarValue = Dvar::setDvarSetting( dvar, (QSpinBox *) widget );
				break;
			case DVAR_VALUE_STRING:
				dvarValue = Dvar::setDvarSetting( dvar, (QLineEdit *) widget );
				break;
		}

		if( !dvarValue.toLatin1().isEmpty() )
		{
			if( !dvar.isCmd )
				mRunDvars << "+set" << dvarName;
			else			// hack for cmds
				mRunDvars << QString( "+%1" ).arg( dvarName );
			mRunDvars << dvarValue;
		}
		size++;
		++it;
	}
}


void MLXMainWindow::OnCreateItemResult( CreateItemResult_t *CreateItemResult, bool IOFailure )
{
	if( IOFailure )
	{
		QMessageBox::warning( this, "Error", "Disk Read error." );
		return;
	}

	if( CreateItemResult->m_eResult != k_EResultOK )
	{
		QMessageBox::warning( this, "Error", QString( "Error creating Steam Workshop item. Error code: %1\nVisit https://steamerrors.com/ for more information." ).arg( CreateItemResult->m_eResult ) );
		return;
	}

	mFileId = CreateItemResult->m_nPublishedFileId;

	UpdateWorkshopItem();
}


void MLXMainWindow::OnUpdateItemResult( SubmitItemUpdateResult_t *UpdateItemResult, bool IOFailure )
{
	if( IOFailure )
	{
		QMessageBox::warning( this, "Error", "Disk Read error." );
		return;
	}

	if( UpdateItemResult->m_eResult != k_EResultOK )
	{
		QMessageBox::warning( this, "Error", QString( "Error updating Steam Workshop item. Error code: %1\nVisit https://steamerrors.com/ for more information." ).arg( UpdateItemResult->m_eResult ) );
		return;
	}

	if( QMessageBox::question( this, "Update", "Workshop item successfully updated. Do you want to visit the Workshop page for this item now?", QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
		ShellExecute( NULL, "open", QString( "steam://url/CommunityFilePage/%1" ).arg( QString::number( mFileId ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
}


void MLXMainWindow::OnHelpAbout()
{
	QMessageBox::about(
		this,
		"About ModLauncherX",
		QString( "ModLauncherX is a fork of the original Black Ops III Mod Launcher, with added" ) +
		QString( " features and quality of life changes.\n\nIf you want to request a change/feature, " ) +
		QString( "please let me know on discord. Username: 'prov3ntus'" ) +
		QString( "\n\nPart of the T7x Mod Tools Suite" ) +
		QString( "\n\nMade by prov3ntus" )
	);
}


void MLXMainWindow::OnHelpOpenGitHubRepo()
{
	// Note: might change my name to 'prov3ntus' in the future...
	// Or I just leave it, idk not decided yet
	qDebug() << "Opening GitHub repo...";
	QDesktopServices::openUrl( QUrl( "https://github.com/w4133d/ModLauncherX" ) );
}


void MLXMainWindow::OnHelpReportIssue()
{
	// Message box pop up
	// 
	// - Spotted an issue? Please either DM me on discord
	// (username: prov3ntus) or open an issue on GitHub
	// 
	//	Open an Issue		Cancel

	qDebug() << "Open report issue dialog";
	
	QDialog *issue_window = new QDialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	issue_window->setWindowTitle( "Contact us about ModLauncherX" );
	//issue_window.resize( QSize( 250, 30 ) );

	QVBoxLayout *container = new QVBoxLayout( issue_window );

	QLabel *body_text =
		new QLabel(
			"Thanks for trying to help me improve ModLauncherX!"
			+ QString( "\n\nSpotted a bug? Want to request a feature? You opened the right window!" )
			+ QString( "\n\nPlease DM me on Discord, my username is 'prov3ntus' - my DMs are always open." )
			+ QString( "\nAlternatively, you can open an issue or discussion on the ModLauncherX GitHub." )
			+ QString( "\n\nOh yea, also INCLUDE THE LOG FILE for bug reports. It can be found here:" )
			+ QString( "\n\n" ) + MLXUtils::DEBUG_LOG_PATH
		);
	container->addWidget( body_text );

	QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Close );
	buttons->setCenterButtons( true );
	container->addWidget( buttons );

	// Bind cancel standard button to actually closing the window
	connect( buttons, &QDialogButtonBox::rejected, issue_window, &QDialog::reject );

	QPushButton *open_issue_btn = new QPushButton( "Open an Issue (GitHub)" );
	buttons->addButton( open_issue_btn, QDialogButtonBox::ActionRole );

	connect( open_issue_btn, &QPushButton::clicked, this, []()
			 {
				 QDesktopServices::openUrl( QUrl( "https://github.com/w4133d/ModLauncherX/issues/new" ) );
			 }
	);

	QPushButton *open_log_folder = new QPushButton( "Open Debug Log" );
	buttons->addButton( open_log_folder, QDialogButtonBox::ActionRole );

	connect( open_log_folder, &QPushButton::clicked, this, []()
			 {
				 //ShellExecute( NULL, "open", ( QString( "\"%1/logs\"" ).arg( MLXUtils::BO3_ROOT_PATH ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
				 MLXUtils::open_folder_select_file( MLXUtils::DEBUG_LOG_PATH );
			 }
	);

	issue_window->exec();
}


void MLXMainWindow::OnOpenZoneFile()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *Item = item_list[ 0 ];

	if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		ShellExecute( NULL, "open", QString( "\"%1/usermaps/%2/zone_source/%3.zone\"" ).arg( MLXUtils::BO3_ROOT_PATH, MapName, MapName ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = Item->parent()->text( 0 );
		QString ZoneName = Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2/zone_source/%3.zone\"" ).arg( MLXUtils::BO3_ROOT_PATH, ModName, ZoneName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
}


void MLXMainWindow::OnOpenSZCFile()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *Item = item_list[ 0 ];

	if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		ShellExecute( NULL, "open", QString( "\"%1/usermaps/%2/sound/zoneconfig/%3.szc\"" ).arg( MLXUtils::BO3_ROOT_PATH, MapName, MapName ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = Item->parent()->text( 0 );
		QString ZoneName = Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2/sound/zoneconfig/%3.szc\"" ).arg( MLXUtils::BO3_ROOT_PATH, ModName, ZoneName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
}


void MLXMainWindow::OnOpenScriptFile( const char* _ext )
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *Item = item_list[ 0 ];
	QString game_mode = Item->text( 0 ).left( 2 );
	qDebug() << "Game mode: " << game_mode;

	QString MapName = Item->text( 0 );
	ShellExecute( NULL, "open",
					QString( "\"%1/usermaps/%2/scripts/%3/%4.%5\"" ).arg(
						MLXUtils::BO3_ROOT_PATH, MapName, game_mode, MapName, _ext
					).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
}


void MLXMainWindow::OnOpenModRootFolder()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *item = item_list[ 0 ];

	if( item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/usermaps/%2\"" ).arg( MLXUtils::BO3_ROOT_PATH, MapName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = item->parent() ? item->parent()->text( 0 ) : item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2\"" ).arg( MLXUtils::BO3_ROOT_PATH, ModName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
}


void MLXMainWindow::OnRunMapOrMod()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *item = item_list[ 0 ];

	QStringList args;

	if( !mRunDvars.isEmpty() )
		args << mRunDvars;

	args << "+set" << "fs_game";

	if( item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = item->text( 0 );
		args << MapName;
		args << "+devmap" << MapName;
	}
	else
	{
		QString ModName = item->parent() ? item->parent()->text( 0 ) : item->text( 0 );
		args << ModName;
	}

	QString ExtraOptions = mRunOptionsWidget->text();
	if( !ExtraOptions.isEmpty() )
		args << ExtraOptions.split( ' ' );

	QList<QPair<QString, QStringList>> Commands;
	Commands.append( QPair<QString, QStringList>( QString( "%1/BlackOps3.exe" ).arg( MLXUtils::BO3_ROOT_PATH ), args ) );
	StartBuildThread( Commands );
}


void MLXMainWindow::OnSaveOutputLog() const
{
	// want to make a logs directory for easy management of launcher logs (exe_dir/logs)
	const auto dir = QDir {};
	if( !dir.exists( QString( "%1/logs/output" ).arg( MLXUtils::BO3_ROOT_PATH ) ) )
	{
		const auto result = dir.mkdir( QString( "%1/logs/output" ).arg( MLXUtils::BO3_ROOT_PATH ) );
		if( !result )
		{
			qWarning() << "OnSaveOutputLog() => Logs directory doesn't exist. Creation attempt mkdir() failed. mkdir() result = '"
				<< result << "'.";
			QMessageBox::warning( nullptr, "Error", QString( "Could not create the \"logs\" directory in the BO3 root folder." ) );
			return;
		}
	}

	const auto time = std::time( nullptr );
	auto ss = std::stringstream {};
	const auto timeStr = std::put_time( std::localtime( &time ), "%F_%T" );

	ss << timeStr;

	auto dateStr = ss.str();
	std::replace( dateStr.begin(), dateStr.end(), ':', '_' );

	QFile log( QString( "%1logs/output/mlx_output_%2.txt" ).arg( MLXUtils::BO3_ROOT_PATH, dateStr.c_str() ) );

	if( !log.open( QIODevice::WriteOnly ) )
		return;

	QTextStream stream( &log );
	stream << mOutputWidget->toPlainText();

	qDebug() << "Saved current launcher log output to:" << log.fileName();
	/*QMessageBox::information(
		nullptr,
		QString( "Launcher Log Output Saved" ),
		QString( "The current output of the Launcher's log has been saved to:\n\n%1" ).arg( log.fileName() )
	);*/
	MLXUtils::open_folder_select_file( log.fileName() );
}


void MLXMainWindow::OnOpenConsoleMPLog() const
{
	ShellExecute( 0, 0, QString { MLXUtils::BO3_ROOT_PATH + "console_mp.log" }.toLocal8Bit().constData(), 0, 0, SW_HIDE );
}


void MLXMainWindow::OnDebugButtonPressed()
{
	//QMessageBox::information( this, "Color", "The color of the text cursor is: " + QString( mOutputWidget->getSelectedTextColor().name() ) );
	UpdateTheme();
	mOutputWidget->appendPlainText( QString( "[ ModLauncherX ] Theme updated: '%1'\n" ).arg( mTheme ) );
}


void MLXMainWindow::OnCleanXPaks()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *Item = item_list[ 0 ];
	QString Folder;
	QString name;

	if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		name = Item->text( 0 );
		Folder = QString( "%1/usermaps/%2" ).arg( MLXUtils::BO3_ROOT_PATH, name );
	}
	else
	{
		name = Item->parent() ? Item->parent()->text( 0 ) : Item->text( 0 );
		Folder = QString( "%1/mods/%2" ).arg( MLXUtils::BO3_ROOT_PATH, name );
	}

	QString fileListString;
	QStringList fileList;
	QDirIterator it( Folder, QStringList() << "*.xpak", QDir::Files, QDirIterator::Subdirectories );
	while( it.hasNext() )
	{
		QString filepath = it.next();
		fileList.append( filepath );
		fileListString.append( "\n" + QDir( Folder ).relativeFilePath( filepath ) );
	}

	QString relativeFolder = QDir( MLXUtils::BO3_ROOT_PATH ).relativeFilePath( Folder );

	if( fileList.count() == 0 )
	{
		//QMessageBox::information( this, QString( "Clean XPaks (%1)" ).arg( relativeFolder ), QString( "There are no XPak's to clean!" ) );
		StatusBar->showMessage( QString( "There are no XPAKs to clean for %1!" ).arg( name ) );
		return;
	}

	/*
	if( QMessageBox::question( this,
							   QString( "Clean XPaks (%1)" ).arg( relativeFolder ),
							   QString( "Are you sure you want to clean XPaks the following files?" + fileListString ),
							   QMessageBox::Yes | QMessageBox::No )
		!= QMessageBox::Yes )
		return;
	*/

	for( auto &file : fileList )
	{
		qDebug() << file;
		QFile( file ).remove();
	}
}


void MLXMainWindow::OnDelete()
{
	QList<QTreeWidgetItem *> item_list = mFileListWidget->selectedItems();
	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *Item = item_list[ 0 ];
	QString Folder;
	QMessageBox::StandardButton result;

	if( Item->data( 0, Qt::UserRole ).toInt() == MLXMapList::ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		Folder = QString( "%1/usermaps/%2" ).arg( MLXUtils::BO3_ROOT_PATH, MapName );
		result = QMessageBox::question(
			this, "Delete Map",
			QString(
				"Are you sure you want to delete the map '%1'? Files will be deleted permanently!"
			).arg( MapName ),
			QMessageBox::Yes | QMessageBox::No
		);
	}
	else
	{
		QString ModName = Item->parent() ? Item->parent()->text( 0 ) : Item->text( 0 );
		Folder = QString( "%1/mods/%2" ).arg( MLXUtils::BO3_ROOT_PATH, ModName );
		result = QMessageBox::question(
			this, "Delete Mod",
			QString(
				"Are you sure you want to delete the mod '%1'? Files will be deleted permanently!"
			).arg( ModName ),
			QMessageBox::Yes | QMessageBox::No
		);
	}

	if( result != QMessageBox::Yes )
		return;

	QDir( Folder ).removeRecursively();
	//PopulateFileList();
	mFileListWidget->populate_file_tree();
}


void MLXMainWindow::crash_handler()
{
	qFatal() << "ModLauncherX has crashed! Log can be found at:\n" << MLXUtils::DEBUG_LOG_PATH;

	QDialog *error_window = new QDialog( this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint );
	error_window->setWindowTitle( "UNHANDED EXCEPTION" );
	//error_window.resize( QSize( 250, 30 ) );

	QVBoxLayout *container = new QVBoxLayout( error_window );

	QLabel *body_text =
		new QLabel( "An unhandled exception occured, and ModLauncherX has crashed. Not to worry though.\n"
					+ QString( "Click the button below to copy the error, and please report it to me, either\n" )
					+ QString( "by sending me a DM on discord [@prov3ntus] or opening a GitHub issue.\n" )
					+ QString( "P.S. My dm's are open ;) lmao" )
		);
	container->addWidget( body_text );

	QDialogButtonBox *buttons = new QDialogButtonBox( QDialogButtonBox::Close );
	buttons->setCenterButtons( true );
	container->addWidget( buttons );

	// Bind cancel standard button to actually closing the window
	connect( buttons, &QDialogButtonBox::rejected, error_window, &QDialog::reject );

	QPushButton *open_log_folder = new QPushButton( "Open Error Log" );
	buttons->addButton( open_log_folder, QDialogButtonBox::ActionRole );

	connect( open_log_folder, &QPushButton::clicked, this, []()
			 {
				 //ShellExecute( NULL, "open", ( QString( "\"%1/logs\"" ).arg( MLXUtils::BO3_ROOT_PATH ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
				 MLXUtils::open_folder_select_file( MLXUtils::DEBUG_LOG_PATH );
			 }
	);

	error_window->exec();

	std::abort();
}


void MLXMainWindow::BuildOutputReady( QString Output )
{
	const QString timestamp = QTime::currentTime().toString( "[hh:mm:ss.zzz] |> " );
	mOutputWidget->appendPlainText( timestamp + Output );
}


void MLXMainWindow::BuildFinished()
{
	mBuildButton->setText( "Build" );
	mBuildThread->deleteLater();
	mBuildThread = NULL;
}


//======================================================
//===================== EXPORT2BIN =====================
//======================================================

void MLXMainWindow::OnExport2BinChooseDirectory()
{
	const QString dir = QFileDialog::getExistingDirectory( mExport2BinGUIWidget, tr( "Open Directory" ), MLXUtils::BO3_ROOT_PATH, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	this->mExport2BinTargetDirWidget->setText( dir );

	QSettings Settings;
	Settings.setValue( "Export2Bin_TargetDir", dir );
}


void MLXMainWindow::OnExport2BinToggleOverwriteFiles()
{
	QSettings Settings;
	Settings.setValue( "Export2Bin_OverwriteFiles", mExport2BinOverwriteWidget->isChecked() );
}
