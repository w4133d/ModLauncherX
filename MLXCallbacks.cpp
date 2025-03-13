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

#include <iosfwd>
#include <sstream>
#include <windows.h>
#include <iomanip>
#include <string>

/*
* ========= ModLauncherX Callbacks =========
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
	{ "ai_disableSpawn", "Disable AI from spawning", DVAR_VALUE_BOOL },
	{ "developer", "Run developer mode", DVAR_VALUE_INT, 0, 2 },
	{ "g_password", "Password for your server", DVAR_VALUE_STRING },
	{ "logfile", "Console log information written to current fs_game", DVAR_VALUE_INT, 0, 2 },
	{ "scr_mod_enable_devblock", "Developer blocks are executed in mods ", DVAR_VALUE_BOOL },
	{ "connect", "Connect to a specific server", DVAR_VALUE_STRING, NULL, NULL, true },
	{ "set_gametype", "Set a gametype to load with map", DVAR_VALUE_STRING, NULL, NULL, true },
	{ "splitscreen", "Enable splitscreen", DVAR_VALUE_BOOL },
	{ "splitscreen_playerCount", "Allocate the number of instances for splitscreen", DVAR_VALUE_INT, 0,  2}
};


void MLXMainWindow::OnSearchTextChanged( const QString &text )
{
	// Clear all highlights if the text is empty
	if( text.isEmpty() )
	{
		ClearHighlights();
		return;
	}

	// Highlight all occurrences of the text
	HighlightAllMatches( text );
}


void MLXMainWindow::OnFindNext()
{
	QString text = SearchBar->text();
	if( text.isEmpty() )
		return;

	// Find the next occurrence of the text
	LastMatchCursor = mOutputWidget->document()->find( text, LastMatchCursor, GetSearchFlags() );

	if( !LastMatchCursor.isNull() )
	{
		// Move the cursor to the next occurrence
		mOutputWidget->setTextCursor( LastMatchCursor );
	}
	else
	{
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


void MLXMainWindow::ContextMenuRequested()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];
	QString ItemType = ( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP ) ? "Map" : "Mod";

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_UNKNOWN )
		return;

	QIcon GameIcon( ":/resources/BlackOps3.png" );

	QMenu *Menu = new QMenu;
	Menu->addAction( GameIcon, QString( "Run %1" ).arg( ItemType ), this, SLOT( OnRunMapOrMod() ) );

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
		Menu->addAction( mActionFileLevelEditor );

	Menu->addSeparator();

	Menu->addAction( "Open Zone File", this, SLOT( OnOpenZoneFile() ) );
	Menu->addAction( "Open SZC FIle", this, SLOT( OnOpenSZCFile() ) );
	Menu->addAction( QString( "Open %1 Folder" ).arg( ItemType ), this, SLOT( OnOpenModRootFolder() ) );

	Menu->addSeparator();

	Menu->addAction( "Delete", this, SLOT( OnDelete() ) );
	Menu->addAction( "Clean XPaks", this, SLOT( OnCleanXPaks() ) );

	Menu->exec( QCursor::pos() );
}


void MLXMainWindow::OnFileAssetEditor()
{
	QProcess *Process = new QProcess();
	connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );
	Process->start( QString( "%1/bin/AssetEditor_modtools.exe" ).arg( mToolsPath ), QStringList() );
}


void MLXMainWindow::OnFileLevelEditor()
{
	QProcess *Process = new QProcess();
	connect( Process, SIGNAL( finished( int ) ), Process, SLOT( deleteLater() ) );

	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.count() && ItemList[ 0 ]->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = ItemList[ 0 ]->text( 0 );
		Process->start( QString( "%1/bin/radiant_modtools.exe" ).arg( mToolsPath ), QStringList() << QString( "%1/map_source/%2/%3.map" ).arg( mGamePath, MapName.left( 2 ), MapName ) );
	}
	else
	{
		Process->start( QString( "%1/bin/radiant_modtools.exe" ).arg( mToolsPath ), QStringList() );
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
	QDir TemplatesFolder( QString( "%1/rex/templates" ).arg( mToolsPath ) );
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

	QString Name = NameWidget->text();

	if( Name.isEmpty() )
	{
		QMessageBox::information( this, "Error", "Map name cannot be empty." );
		return;
	}

	if( mShippedMapList.contains( Name, Qt::CaseInsensitive ) )
	{
		QMessageBox::information( this, "Error", "Map name cannot be the same as a built-in map." );
		return;
	}

	QByteArray MapName = NameWidget->text().toLatin1().toLower();
	QString Output;

	QString Template = Templates[ TemplateWidget->currentIndex() ];

	if( ( Template == "MP Mod Level" && !MapName.startsWith( "mp_" ) ) || ( Template == "ZM Mod Level" && !MapName.startsWith( "zm_" ) ) )
	{
		QMessageBox::information( this, "Error", "Map name must start with 'mp_' or 'zm_'." );
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

	if( RecursiveCopy( TemplatesFolder.absolutePath() + QDir::separator() + Templates[ TemplateWidget->currentIndex() ], QDir::cleanPath( mGamePath ) ) )
	{
		PopulateFileList();

		QMessageBox::information( this, "New Map Created", QString( "Files created:\n" ) + Output );
	}
	else
		QMessageBox::information( this, "Error", "Error creating map files." );
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
			Commands.append( QPair<QString, QStringList>( QString( "%1/gdtdb/gdtdb.exe" ).arg( mToolsPath ), QStringList() << "/update" ) );
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

	QStringList LanguageArgs;
	LanguageArgs;

	if( mBuildLanguage != "All" )
		LanguageArgs << "-language" << mBuildLanguage;
	else 
		for( const QString &Language : gLanguages )
			LanguageArgs << "-language" << Language;

	for( QTreeWidgetItem *Item : CheckedItems )
	{
		if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
		{
			QString MapName = Item->text( 0 );

			if( mCompileEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				QStringList Args;
				Args << "-platform" << "pc";

				if( mCompileModeWidget->currentIndex() == 0 )
					Args << "-onlyents";
				else
					Args << "-navmesh" << "-navvolume";

				Args << "-loadFrom" << QString( "%1\\map_source\\%2\\%3.map" ).arg( mGamePath, MapName.left( 2 ), MapName );
				Args << QString( "%1\\share\\raw\\maps\\%2\\%3.d3dbsp" ).arg( mGamePath, MapName.left( 2 ), MapName );

				Commands.append( QPair<QString, QStringList>( QString( "%1\\bin\\cod2map64.exe" ).arg( mToolsPath ), Args ) );
			}

			if( mLightEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				QStringList Args;
				Args << "-ledSilent";

				switch( mLightQualityWidget->currentIndex() )
				{
					case 0:
						Args << "+low";
						break;

					default:
					case 1:
						Args << "+medium";
						break;

					case 2:
						Args << "+high";
						break;
				}

				Args << "+localprobes" << "+forceclean" << "+recompute" << QString( "%1/map_source/%2/%3.map" ).arg( mGamePath, MapName.left( 2 ), MapName );
				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/radiant_modtools.exe" ).arg( mToolsPath ), Args ) );
			}

			if( mLinkEnabledWidget->isChecked() )
			{
				AddUpdateDBCommand();

				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/linker_modtools.exe" ).arg( mToolsPath ), QStringList() << LanguageArgs << "-modsource" << MapName ) );
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
				Commands.append( QPair<QString, QStringList>( QString( "%1/bin/linker_modtools.exe" ).arg( mToolsPath ), QStringList() << LanguageArgs << "-fs_game" << ModName << "-modsource" << ZoneName ) );
			}

			LastMod = ModName;
		}
	}

	if( mRunEnabledWidget->isChecked() && ( !LastMod.isEmpty() || !LastMap.isEmpty() ) )
	{
		QStringList Args;

		if( !mRunDvars.isEmpty() )
			Args << mRunDvars;

		Args << "+set" << "fs_game" << ( LastMod.isEmpty() ? LastMap : LastMod );

		if( !LastMap.isEmpty() )
			Args << "+devmap" << LastMap;

		QString ExtraOptions = mRunOptionsWidget->text();
		if( !ExtraOptions.isEmpty() )
			Args << ExtraOptions.split( ' ' );

		Commands.append( QPair<QString, QStringList>( QString( "%1/BlackOps3.exe" ).arg( mGamePath ), Args ) );
	}

	if( Commands.size() == 0 && !UpdateAdded )
	{
		QMessageBox::information( this, "No Tasks", "Please selected at least one file from the list and one action to be performed." );
		return;
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
	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
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

	mWorkshopFolder = QString( "%1/%2/zone" ).arg( mGamePath, Folder );
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
	Dialog.setWindowTitle( "Options" );
	Dialog.resize( QSize( 250, 30 ) );
	//QMessageBox::information( this, QString( "Yo" ), QString( "Width: %1\nHeight: %2" ).arg( QString().setNum( Dialog.size().width()  ), QString().setNum( Dialog.size().height() ) ) );

	QVBoxLayout *Layout = new QVBoxLayout( &Dialog );

	QSettings Settings;

	//QCheckBox* TreyarchThemeCheckbox = new QCheckBox("Use Treyarch Theme");
	//TreyarchThemeCheckbox->setToolTip("Toggle between the dark grey Treyarch colors and the default Windows colors");
	//TreyarchThemeCheckbox->setChecked(Settings.value("UseDarkTheme", false).toBool());
	//Layout->addWidget( TreyarchThemeCheckbox );

	// THEME //

	QHBoxLayout *ThemeLayout = new QHBoxLayout();
	ThemeLayout->addWidget( new QLabel( "Theme" ) );

	QStringList Themes;
	Themes << "Default" << "Default Dark" << "Modern" << "Treyarch" << "T7x";

	QComboBox *ThemeCombo = new QComboBox();
	ThemeCombo->addItems( Themes );
	ThemeCombo->setCurrentText( mTheme );
	//ThemeCombo->setFixedWidth( 110 );
	//ThemeCombo->setFixedHeight( 20 );
	ThemeLayout->addWidget( ThemeCombo );

	Layout->addLayout( ThemeLayout );

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

	mBuildLanguage = LanguageCombo->currentText();
	mTheme = ThemeCombo->currentText();

	Settings.setValue( "BuildLanguage", mBuildLanguage );
	Settings.setValue( "Theme", mTheme );

	UpdateTheme();
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

	for( int DvarIdx = 0; DvarIdx < ARRAYSIZE( gDvars ); DvarIdx++ )
		Dvar( gDvars[ DvarIdx ], DvarTree );

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


void MLXMainWindow::OnOpenZoneFile()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		ShellExecute( NULL, "open", QString( "\"%1/usermaps/%2/zone_source/%3.zone\"" ).arg( mGamePath, MapName, MapName ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = Item->parent()->text( 0 );
		QString ZoneName = Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2/zone_source/%3.zone\"" ).arg( mGamePath, ModName, ZoneName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
}


void MLXMainWindow::OnOpenSZCFile()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		ShellExecute( NULL, "open", QString( "\"%1/usermaps/%2/sound/zoneconfig/%3.szc\"" ).arg( mGamePath, MapName, MapName ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = Item->parent()->text( 0 );
		QString ZoneName = Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2/sound/zoneconfig/%3.szc\"" ).arg( mGamePath, ModName, ZoneName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}

}


void MLXMainWindow::OnOpenModRootFolder()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/usermaps/%2\"" ).arg( mGamePath, MapName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
	else
	{
		QString ModName = Item->parent() ? Item->parent()->text( 0 ) : Item->text( 0 );
		ShellExecute( NULL, "open", ( QString( "\"%1/mods/%2\"" ).arg( mGamePath, ModName ) ).toLatin1().constData(), "", NULL, SW_SHOWDEFAULT );
	}
}


void MLXMainWindow::OnRunMapOrMod()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];

	QStringList Args;

	if( !mRunDvars.isEmpty() )
		Args << mRunDvars;

	Args << "+set" << "fs_game";

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		Args << MapName;
		Args << "+devmap" << MapName;
	}
	else
	{
		QString ModName = Item->parent() ? Item->parent()->text( 0 ) : Item->text( 0 );
		Args << ModName;
	}

	QString ExtraOptions = mRunOptionsWidget->text();
	if( !ExtraOptions.isEmpty() )
		Args << ExtraOptions.split( ' ' );

	QList<QPair<QString, QStringList>> Commands;
	Commands.append( QPair<QString, QStringList>( QString( "%1/BlackOps3.exe" ).arg( mGamePath ), Args ) );
	StartBuildThread( Commands );
}


void MLXMainWindow::OnSaveOutputLog() const
{
	// want to make a logs directory for easy management of launcher logs (exe_dir/logs)
	const auto dir = QDir {};
	if( !dir.exists( mToolsPath + "logs" ) )
	{
		const auto result = dir.mkdir( mToolsPath + "logs" );
		if( !result )
		{
			QMessageBox::warning( nullptr, "Error", QString( "Could not create the \"logs\" directory" ) );
			return;
		}
	}

	const auto time = std::time( nullptr );
	auto ss = std::stringstream {};
	const auto timeStr = std::put_time( std::localtime( &time ), "%F_%T" );

	ss << timeStr;

	auto dateStr = ss.str();
	std::replace( dateStr.begin(), dateStr.end(), ':', '_' );

	QFile log( QString( "%1logs/modlog_%2.txt" ).arg( mToolsPath, dateStr.c_str() ) );

	if( !log.open( QIODevice::WriteOnly ) )
		return;

	QTextStream stream( &log );
	stream << mOutputWidget->toPlainText();

	QMessageBox::information(
		nullptr,
		QString( "Launcher Log Output Saved" ),
		QString( "The current output of the Launcher's log has been saved to:\n\n%1" ).arg( log.fileName() )
	);
}


void MLXMainWindow::OnOpenConsoleMPLog() const
{
	ShellExecute( 0, 0, QString { mToolsPath + "console_mp.log" }.toLocal8Bit().constData(), 0, 0, SW_HIDE );
}


void MLXMainWindow::OnDebugButtonPressed()
{
	//QMessageBox::information( this, "Color", "The color of the text cursor is: " + QString( mOutputWidget->getSelectedTextColor().name() ) );
	UpdateTheme();
	mOutputWidget->appendColoredText( QString( "[ ModLauncherX ] Theme updated: '%1'\n" ).arg( mTheme ) );
}


void MLXMainWindow::OnCleanXPaks()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];
	QString Folder;

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		Folder = QString( "%1/usermaps/%2" ).arg( mGamePath, MapName );
	}
	else
	{
		QString ModName = Item->parent() ? Item->parent()->text( 0 ) : Item->text( 0 );
		Folder = QString( "%1/mods/%2" ).arg( mGamePath, ModName );
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

	QString relativeFolder = QDir( mGamePath ).relativeFilePath( Folder );

	if( fileList.count() == 0 )
	{
		QMessageBox::information( this, QString( "Clean XPaks (%1)" ).arg( relativeFolder ), QString( "There are no XPak's to clean!" ) );
		return;
	}

	if( QMessageBox::question( this,
							   QString( "Clean XPaks (%1)" ).arg( relativeFolder ),
							   QString( "Are you sure you want to delete the following files?" + fileListString ),
							   QMessageBox::Yes | QMessageBox::No )
		!= QMessageBox::Yes )
		return;

	for( auto &file : fileList )
	{
		qDebug() << file;
		QFile( file ).remove();
	}
}


void MLXMainWindow::OnDelete()
{
	QList<QTreeWidgetItem *> ItemList = mFileListWidget->selectedItems();
	if( ItemList.isEmpty() )
		return;

	QTreeWidgetItem *Item = ItemList[ 0 ];
	QString Folder;

	QMessageBox::StandardButton result;

	if( Item->data( 0, Qt::UserRole ).toInt() == ML_ITEM_MAP )
	{
		QString MapName = Item->text( 0 );
		Folder = QString( "%1/usermaps/%2" ).arg( mGamePath, MapName );
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
		Folder = QString( "%1/mods/%2" ).arg( mGamePath, ModName );
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
	PopulateFileList();
}


void MLXMainWindow::OnExport2BinChooseDirectory()
{
	const QString dir = QFileDialog::getExistingDirectory( mExport2BinGUIWidget, tr( "Open Directory" ), mToolsPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );
	this->mExport2BinTargetDirWidget->setText( dir );

	QSettings Settings;
	Settings.setValue( "Export2Bin_TargetDir", dir );
}


void MLXMainWindow::OnExport2BinToggleOverwriteFiles()
{
	QSettings Settings;
	Settings.setValue( "Export2Bin_OverwriteFiles", mExport2BinOverwriteWidget->isChecked() );
}


void MLXMainWindow::BuildOutputReady( QString Output )
{
	mOutputWidget->appendColoredText( Output );
}


void MLXMainWindow::BuildFinished()
{
	mBuildButton->setText( "Build" );
	mBuildThread->deleteLater();
	mBuildThread = NULL;
}

