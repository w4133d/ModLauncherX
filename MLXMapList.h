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

class MLXMapList: public QTreeWidget
{
	Q_OBJECT

	public:
	explicit MLXMapList( QWidget *parent = nullptr );

	void populate_file_tree();
	// void ...

	enum ListItem: int
	{
		ITEM_UNKNOWN = -1,
		ITEM_MAP,
		ITEM_MOD,
		ITEM_PARENT,
		ITEM_MAP_FOLDER,
		ITEM_MOD_PARENT
	};

	public slots:
	// Callbacks
	void on_context_menu_requested();

	protected slots:
	// Callbacks
	void on_create_folder();
	void on_delete_folder();

	// Structure system
	void save_tree_structure();
	bool load_tree_structure();

	// Helpers
	QJsonObject helper_tree_item_to_json( QTreeWidgetItem *folder );
	QTreeWidgetItem *helper_json_to_tree_item( const QJsonObject &obj );
	void helper_remove_invalid_tree_items( QTreeWidgetItem *parent, QSet<QString> &registered_usermaps );

	private:
	QSet< QString > valid_usermaps;
	uint invalid_tree_item_count;
	//const MLXMainWindow *main_window = MLXMainWindow::_instance;
};
