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
#include "MLXPlainTextEdit.h"
#include "MLXThreads.h"



enum mlItemType
{
	ML_ITEM_UNKNOWN,
	ML_ITEM_MAP,
	ML_ITEM_MOD
};


class MLXMainWindow: public QMainWindow
{
	Q_OBJECT

		friend class Export2BinGroupBox;

	public:
	MLXMainWindow();
	~MLXMainWindow();

	void UpdateDB();

	void OnCreateItemResult( CreateItemResult_t *CreateItemResult, bool IOFailure );
	CCallResult<MLXMainWindow, CreateItemResult_t> mSteamCallResultCreateItem;

	void OnUpdateItemResult( SubmitItemUpdateResult_t *UpdateItemResult, bool IOFailure );
	CCallResult<MLXMainWindow, SubmitItemUpdateResult_t> mSteamCallResultUpdateItem;

	void OnUGCRequestUGCDetails( SteamUGCRequestUGCDetailsResult_t *RequestDetailsResult, bool IOFailure );
	CCallResult<MLXMainWindow, SteamUGCRequestUGCDetailsResult_t> mSteamCallResultRequestDetails;

	protected slots:
	void OnFileNew();
	void OnFileAssetEditor();
	void OnFileLevelEditor();
	void OnFileExport2Bin();
	void OnEditBuild();
	void OnEditPublish();
	void OnEditOptions();
	void OnEditDvars();
	void OnHelpAbout();
	void OnOpenZoneFile();
	void OnOpenSZCFile();
	void OnOpenModRootFolder();
	void OnRunMapOrMod();
	void OnSaveOutputLog() const;
	void OnOpenConsoleMPLog() const;
	void OnCleanXPaks();
	void OnDelete();
	void OnDebugButtonPressed();
	void OnExport2BinChooseDirectory();
	void OnExport2BinToggleOverwriteFiles();
	void BuildOutputReady( QString Output );
	void BuildFinished();
	void ContextMenuRequested();
	void SteamUpdate();

	protected:
	enum mlxFonts
	{
		FIRA_CODE,
		INTER,
		LATO
	};

	enum ToolTipDictMap
	{
		TOOLTIP_COMPILE = 1,
		TOOLTIP_LIGHT,
		TOOLTIP_LINK,
		TOOLTIP_RUN,
		TOOLTIP_EXEC_ARGS,
		TOOLTIP_BUILD,
		TOOLTIP_DVARS,
		TOOLTIP_SAVE_OUTPUT,
		TOOLTIP_OPEN_LOG,
		TOOLTIP_IGNORE_ERRORS,
	};

	// Seems like this isn't used - pv
	//void closeEvent( QCloseEvent *Event );

	void StartBuildThread( const QList<QPair<QString, QStringList>> &Commands );
	void StartConvertThread( QStringList &pathList, QString &outputDir, bool allowOverwrite );

	void PopulateFileList();
	void UpdateWorkshopItem();
	void ShowPublishDialog();
	void UpdateTheme();
	void RegisterFonts();
	void DefineToolTips();

	void CreateActions();
	void CreateMenu();
	void CreateToolBar();

	void InitExport2BinGUI();

	QMap< mlxFonts, int > RegisteredFonts;
	QFont DefaultFont;

	QAction *mActionFileNew;
	QAction *mActionFileAssetEditor;
	QAction *mActionFileLevelEditor;
	QAction *mActionFileExport2Bin;
	QAction *mActionFileExit;
	QAction *mActionEditBuild;
	QAction *mActionEditPublish;
	QAction *mActionEditOptions;
	QAction *mActionHelpAbout;

	QTreeWidget *mFileListWidget;
	MLXPlainTextEdit *mOutputWidget;

	QPushButton *mBuildButton;
	QPushButton *mDvarsButton;
	QPushButton *mSaveOutputButton;
	QPushButton *mOpenLogButton;
	QPushButton *mDebugButton;
	QCheckBox *mCompileEnabledWidget;
	QComboBox *mCompileModeWidget;
	QCheckBox *mLightEnabledWidget;
	QComboBox *mLightQualityWidget;
	QCheckBox *mLinkEnabledWidget;
	QCheckBox *mRunEnabledWidget;
	QLineEdit *mRunOptionsWidget;
	QCheckBox *mIgnoreErrorsWidget;

	mlBuildThread *mBuildThread;
	mlConvertThread *mConvertThread;

	QDockWidget *mExport2BinGUIWidget;
	QCheckBox *mExport2BinOverwriteWidget;
	QLineEdit *mExport2BinTargetDirWidget;

	QString mTheme;
	QString mBuildLanguage;

	QStringList mShippedMapList;
	QTimer mTimer;

	quint64 mFileId;
	QString mTitle;
	QString mDescription;
	QString mThumbnail;
	QString mWorkshopFolder;
	QString mFolderName;
	QString mType;
	QStringList mTags;

	QString mGamePath;
	QString mToolsPath;

	QStringList mRunDvars;

	/// Contains a mapped dict of tooltip definitions for easy definition and application
	std::map< ToolTipDictMap, QString > mlxToolTips;

	// Search bar stuff
	private slots:
	void OnSearchTextChanged( const QString &text );
	void OnFindNext();
	void OnReturnPressed();
	void ClearHighlights();

	private:
	void HighlightAllMatches( const QString &text );
	void ShowNoResultsPopup();
	QTextDocument::FindFlags GetSearchFlags();

	QLineEdit *SearchBar;
	QPushButton *FindNextButton;
	QCheckBox *CaseSensitiveCheck;
	QCheckBox *WholeWordCheck;
	QTextCursor LastMatchCursor;
	QColor HighlightColor;
	uint WrapCount;
};


class Export2BinGroupBox: public QGroupBox
{
	private:
	MLXMainWindow *parentWindow;

	protected:
	void dragEnterEvent( QDragEnterEvent *event );
	void dragLeaveEvent( QDragLeaveEvent *event );
	void dropEvent( QDropEvent *event );

	public:
	Export2BinGroupBox( QWidget *parent, MLXMainWindow *parent_window );
};
