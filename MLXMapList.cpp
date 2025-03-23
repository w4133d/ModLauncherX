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
#include "MLXMapList.h"
#include "MLXUtils.h"
#include "MLXMainWindow.h"

MLXMapList::MLXMapList( QWidget *parent )
	: QTreeWidget( parent )
{
	// Set parameters
	setHeaderHidden( true );
	setUniformRowHeights( true );
	//setDragEnabled( true );
	setDropIndicatorShown( true );
	setAcceptDrops( true );
	setDragDropMode( QAbstractItemView::InternalMove );
	setContextMenuPolicy( Qt::CustomContextMenu );
}



void MLXMapList::populate_file_tree()
{
	qDebug() << "MLXMapList: Context menu requested";
	clear();

	// Load maps

	// Optimisation - use hash table here if population operations are slow
	QSet< QString> registered_usermaps;
	valid_usermaps.clear();

	QString usermaps_folder = QDir::cleanPath( QString( "%1/usermaps/" ).arg( MLXUtils::BO3_ROOT_PATH ) );
	QStringList usermaps = QDir( usermaps_folder ).entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	QTreeWidgetItem *maps_root_item;

	//QFont Font = maps_root_item->font( 0 );
	//Font.setBold( true );
	//maps_root_item->setFont( 0, Font );
	//maps_root_item->setData( 0, Qt::UserRole, ITEM_PARENT );
	//maps_root_item->setFlags( maps_root_item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsDragEnabled );

	// Register all maps in the usermaps directory
	for( QString MapName : usermaps )
	{
		QString ZoneFileName = QString( "%1/%2/zone_source/%3.zone" ).arg( usermaps_folder, MapName, MapName );

		if( QFileInfo( ZoneFileName ).isFile() )
		{
			registered_usermaps << MapName;

			/*QTreeWidgetItem *map_item = new QTreeWidgetItem( maps_root_item, QStringList() << MapName );
			map_item->setCheckState( 0, Qt::Unchecked );
			map_item->setData( 0, Qt::UserRole, ITEM_MAP );
			map_item->setFlags( map_item->flags() & ~Qt::ItemIsDropEnabled );*/
		}
	}

	// Load all stuff from JSON save file
	if( load_tree_structure() )
	{
		qDebug() << "Successfully loaded tree structure from JSON";
		// Time to 2x check if all saved maps are still valid...
		maps_root_item = topLevelItem( 0 );
		helper_remove_invalid_tree_items( maps_root_item, registered_usermaps ); // Recursive map validation check

		qDebug() << "Found" << invalid_tree_item_count
			<< "maps that existed in the JSON, but don't"
			<< "exist in the usermaps folder anymore.";

		// So helper_remove_invalid_tree_items() has just populated the valid_usermaps set.
		// Now, any usermaps that are in registered_usermaps, but aren't in valid_usermaps
		// are the ones that we haven't loaded. So we just stick em on the end as children
		// of maps_root_item

		// Find the difference:
		qDebug() << "Validated usermaps from the save:" << valid_usermaps;
		qDebug() << "Registered usermaps: " << registered_usermaps;
		registered_usermaps.subtract( valid_usermaps );
		qDebug() << "Registered usermaps after subtraction: " << registered_usermaps;
	}
	else
	{
		qDebug() << "load_tree_structure() is false, JSON failed";
		// JSON failed - just load normally
		maps_root_item = new QTreeWidgetItem( this, QStringList() << "Maps" );
	}

	// When load_tree_structure() is TRUE, then registered_usermaps will be modified
	// to only contain maps that were in the usermaps folder, but not in the saved folders.
	// The modification discards any leftovers from the valid_usermaps set. In other words,
	// any usermaps that WERE in the save file, but no longer exist on the system are
	// discarded along with the valid_usermaps set.
	// registered_usermaps basically contains all map names that aren't in folders, cause
	// usermap names are only stored if they're in a folder.
	for( QString usermap : registered_usermaps )
	{
		QTreeWidgetItem *map_item = new QTreeWidgetItem( maps_root_item, QStringList() << usermap );
		map_item->setCheckState( 0, Qt::Unchecked );
		map_item->setData( 0, Qt::UserRole, ITEM_MAP );
		map_item->setFlags( map_item->flags() & ~Qt::ItemIsDropEnabled );
	}

	QFont font = maps_root_item->font( 0 );
	font.setBold( true );
	maps_root_item->setFont( 0, font );
	maps_root_item->setData( 0, Qt::UserRole, ITEM_PARENT );
	maps_root_item->setFlags( maps_root_item->flags() & /*~Qt::ItemIsSelectable &*/ ~Qt::ItemIsDragEnabled );

	// Load Mods

	QString mods_folder = QDir::cleanPath( QString( "%1/mods/" ).arg( MLXUtils::BO3_ROOT_PATH ) );
	QStringList mods_list = QDir( mods_folder ).entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	QTreeWidgetItem *mods_root_item = new QTreeWidgetItem( this, QStringList() << "Mods" );
	mods_root_item->setFont( 0, font );
	mods_root_item->setFlags( mods_root_item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsDragEnabled );

	const char *Files[ 4 ] = { "core_mod", "mp_mod", "cp_mod", "zm_mod" };

	for( QString mod_name : mods_list )
	{
		QTreeWidgetItem *parent_item = NULL;

		for( int FileIdx = 0; FileIdx < 4; FileIdx++ )
		{
			QString ZoneFileName = QString( "%1/%2/zone_source/%3.zone" ).arg( mods_folder, mod_name, Files[ FileIdx ] );

			if( QFileInfo( ZoneFileName ).isFile() )
			{
				if( !parent_item )
					parent_item = new QTreeWidgetItem( mods_root_item, QStringList() << mod_name );

				parent_item->setData( 0, Qt::UserRole, ITEM_MOD_PARENT );
				QTreeWidgetItem *mod_item = new QTreeWidgetItem( parent_item, QStringList() << Files[ FileIdx ] );
				mod_item->setCheckState( 0, Qt::Unchecked );
				mod_item->setData( 0, Qt::UserRole, ITEM_MOD );
			}
		}
	}

	save_tree_structure();

	collapseAll();
	maps_root_item->setExpanded( true );
	mods_root_item->setExpanded( true );

	// Expand all the mod folders out?
	for( int i = 0; i < mods_root_item->childCount(); ++i )
	{
		mods_root_item->child( i )->setExpanded( true );
	}
}


void MLXMapList::on_context_menu_requested()
{
	QList<QTreeWidgetItem *> item_list = selectedItems();
	if( item_list.isEmpty() )
		return;

	QMenu *menu = new QMenu;

	QTreeWidgetItem *item = item_list[ 0 ];
	QString item_type;

	QIcon clean_icon( ":/resources/clean.png" );
	QIcon new_folder( ":/resources/NewFolder.png" );

	switch( item->data( 0, Qt::UserRole ).toInt() )
	{
		case ITEM_MAP:
			item_type = "Map";
			break;
		case ITEM_MOD:
			item_type = "Mod";
			break;
		case ITEM_PARENT:
			//item_type = "Parent";
			menu->addAction( new_folder, "New Folder", this, SLOT( on_create_folder() ) );
			menu->exec( QCursor::pos() );
			return;
		case ITEM_MAP_FOLDER:
			//item_type = "Folder";
			menu->addAction( new_folder, "New Folder", this, SLOT( on_create_folder() ) );
			menu->addSeparator();
			menu->addAction( clean_icon, "Remove Folder", this, SLOT( on_delete_folder() ) );
			menu->exec( QCursor::pos() );
			return;
		default: return;
	}

	QIcon GameIcon( ":/resources/BlackOps3.png" );
	menu->addAction( GameIcon, QString( "Run %1" ).arg( item_type ), MLXMainWindow::_instance, SLOT( OnRunMapOrMod() ) );

	if( item->data( 0, Qt::UserRole ).toInt() == ITEM_MAP )
		menu->addAction( MLXMainWindow::_instance->mActionFileLevelEditor );

	menu->addSeparator();

	QIcon szc_icon( ":/resources/json.png" );
	QIcon folder_icon( ":/resources/folder.png" );
	QIcon zone_icon( ":/resources/zone.png" );

	menu->addAction( zone_icon, "Open Zone File", MLXMainWindow::_instance, SLOT( OnOpenZoneFile() ) );
	menu->addAction( szc_icon, "Open SZC FIle", MLXMainWindow::_instance, SLOT( OnOpenSZCFile() ) );
	menu->addAction( folder_icon, QString( "Open %1 Folder" ).arg( item_type ), MLXMainWindow::_instance, SLOT( OnOpenModRootFolder() ) );


	if( item_type == "Map" )
	{
		QIcon scripting_icon( ":/resources/gsc.png" );

		menu->addSeparator();

		menu->addAction( scripting_icon, "Open GSC File", this, []()
						 {
							 MLXMainWindow::_instance->OnOpenScriptFile( "gsc" );
						 }
		);
		menu->addAction( scripting_icon, "Open CSC File", this, []()
						 {
							 MLXMainWindow::_instance->OnOpenScriptFile( "csc" );
						 }
		);
	}

	menu->addSeparator();

	QIcon delete_icon( ":/resources/delete.png" );

	menu->addAction( delete_icon, "Delete", MLXMainWindow::_instance, SLOT( OnDelete() ) );
	//menu->addAction( clean_icon, "Clean XPaks", MLXMainWindow::_instance, SLOT( OnCleanXPaks() ) );

	menu->exec( QCursor::pos() );
}


void MLXMapList::on_create_folder()
{
	QList<QTreeWidgetItem *> item_list = selectedItems();

	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *parent = item_list[ 0 ];

	QString folder_name = QInputDialog::getText( this, "New Map Folder", "Enter a folder name..." );

	// VALIDATION CHECKS:

	// Cancelled or entered nothing
	if( folder_name == "" )
		return;

	// Entered an invalid character
	QRegularExpressionValidator *validator = new QRegularExpressionValidator( QRegularExpression( "[a-zA-Z0-9_]*" ), this );

	int pos = 0;
	if( validator->validate( folder_name, pos ) != QValidator::Acceptable )
	{
		QMessageBox::warning(
			this, "Invalid Folder Name",
			"Invalid characters in folder name.\nAccepted characters are letters, numbers and underscores.",
			QMessageBox::Ok
		);

		return;
	}

	// Entered a folder name that's the same as a sibling
	for( int i = 0; i < parent->childCount(); ++i )
	{
		if( parent->child( i )->text( 0 ) == folder_name )
		{
			QMessageBox::warning(
				this, "Invalid Folder Name",
				"A folder of the same name already exists! Please choose a different one.",
				QMessageBox::Ok
			);

			return;
		}
	}

	QTreeWidgetItem *new_folder = new QTreeWidgetItem( this, QStringList() << folder_name );
	parent->addChild( new_folder );
	parent->setExpanded( true );

	//new_folder->setData( COL_PARENT, Qt::UserRole, parent->text( 0 ) );

	sortItems( 0, Qt::AscendingOrder );

	save_tree_structure();
}


void MLXMapList::on_delete_folder()
{
	/* Steps:
	* 1 | Remove item from tree
	* 2 | Remove folder from JSON file
	* 3 | Repopulate the tree
	* 4 | ye
	*/

	QList<QTreeWidgetItem *> item_list = selectedItems();

	if( item_list.isEmpty() )
		return;

	QTreeWidgetItem *folder = item_list[ 0 ];

	// Make sure we actually are removing a 
	if( folder->data( 0, Qt::UserRole ).toInt() != ITEM_MAP_FOLDER )
		return;

	// Step 1
	QTreeWidgetItem *parent = folder->parent();
	parent->removeChild( folder );
	//all_folders->remove( folder );

	// Step 2 - Update save file
	save_tree_structure();

	// Step 3
	populate_file_tree();
}


void MLXMapList::save_tree_structure()
{
	QJsonArray a_root;

	a_root.append( helper_tree_item_to_json( topLevelItem( 0 ) ) );

	QJsonDocument doc( a_root );
	QFile file( MLXUtils::MAP_LIST_SAVE_PATH );

	QDir dir = QFileInfo( MLXUtils::MAP_LIST_SAVE_PATH ).absolutePath();
	if( !dir.exists() )
	{
		dir.mkpath( "." );
	}

	if( !file.open( QIODevice::WriteOnly ) )
	{
		qWarning() << "Unable to open file for writing:" << file.errorString();
		return;
	}

	file.write( doc.toJson( QJsonDocument::Indented ) ); // Pretty print JSON
	file.close();

	qDebug() << "Tree structure saved to" << MLXUtils::MAP_LIST_SAVE_PATH;

}

bool MLXMapList::load_tree_structure()
{
	QFile file( MLXUtils::MAP_LIST_SAVE_PATH );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		qWarning() << "Unable to open file for reading:" << file.errorString();
		return false;
	}

	QByteArray json_data = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson( json_data );
	if( doc.isNull() || !doc.isArray() )
	{
		qWarning() << "Invalid JSON format";
		return false;
	}

	QJsonArray a_root = doc.array();
	for( int i = 0; i < a_root.size(); ++i )
	{
		QJsonObject obj = a_root[ i ].toObject();
		QTreeWidgetItem *item = helper_json_to_tree_item( obj ); // Recursively adds children to tree item
		addTopLevelItem( item );
	}

	qDebug() << "Tree structure loaded from" << MLXUtils::MAP_LIST_SAVE_PATH;

	return true;
}


// Loader / saver helper functions

QJsonObject MLXMapList::helper_tree_item_to_json( QTreeWidgetItem *item )
{
	QJsonObject obj;

	obj[ "name" ] = item->text( 0 );
	obj[ "item_type" ] = item->data( 0, Qt::UserRole ).toString();

	QJsonArray children;
	for( int i = 0; i < item->childCount(); ++i )
	{
		QTreeWidgetItem *child = item->child( i );
		int child_type = child->data( 0, Qt::UserRole ).toInt();

		if( ITEM_MAP_FOLDER == child_type )
			children.append( helper_tree_item_to_json( child ) );
	}

	obj[ "children" ] = children;

	return obj;
}

QTreeWidgetItem *MLXMapList::helper_json_to_tree_item( const QJsonObject &obj )
{
	QTreeWidgetItem *item = new QTreeWidgetItem();

	item->setText( 0, obj[ "name" ].toString() );
	item->setData( 0, Qt::UserRole, obj[ "item_type" ].toInt( -1 ) );

	if( obj[ "item_type" ].toInt() == ITEM_MAP )
	{

	}

	QJsonArray a_children = obj[ "children" ].toArray();
	for( int i = 0; i < a_children.size(); ++i )
	{
		QJsonObject child_obj = a_children[ i ].toObject();
		QTreeWidgetItem *child_item = helper_json_to_tree_item( child_obj );
		item->addChild( child_item );
	}

	return item;
}

void MLXMapList::helper_remove_invalid_tree_items( QTreeWidgetItem *parent, QSet<QString> &registered_usermaps )
{
	for( int i = 0; i < parent->childCount(); ++i )
	{
		QTreeWidgetItem *child = parent->child( i );

		// Execute on any children first
		if( child->childCount() )
			helper_remove_invalid_tree_items( child, registered_usermaps );

		if( child->data( 0, Qt::UserRole ).toInt() != ITEM_MAP )
			continue;

		if( registered_usermaps.contains( child->text( 0 ) ) )
		{
			valid_usermaps << child->text( 0 );
			continue;
		}

		// Map name not found in valid list - remove
		parent->removeChild( child );
		++invalid_tree_item_count;
	}
}