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
#include "MLXOutputHighlighter.h"
#include "MLXThreads.h"
#include "MLXMapList.h"


enum mlx_fonts
{
	FIRA_CODE,
	INTER,
	LATO
};



class MLXMainWindow: public QMainWindow
{
	Q_OBJECT

	friend class Export2BinGroupBox;

	public:
	MLXMainWindow();
	~MLXMainWindow();

	void UpdateDB();
	QFont GetFont( mlx_fonts font_enum );

	void OnCreateItemResult( CreateItemResult_t *CreateItemResult, bool IOFailure );
	CCallResult<MLXMainWindow, CreateItemResult_t> mSteamCallResultCreateItem;

	void OnUpdateItemResult( SubmitItemUpdateResult_t *UpdateItemResult, bool IOFailure );
	CCallResult<MLXMainWindow, SubmitItemUpdateResult_t> mSteamCallResultUpdateItem;

	void OnUGCRequestUGCDetails( SteamUGCRequestUGCDetailsResult_t *RequestDetailsResult, bool IOFailure );
	CCallResult<MLXMainWindow, SteamUGCRequestUGCDetailsResult_t> mSteamCallResultRequestDetails;

	void UpdateTheme();

	void crash_handler();

	static void set_crash_handler()
	{
		if( _instance )
		{
			_instance->crash_handler();
		}
	}

	void setup_crash_handler()
	{
		_instance = this;
		std::set_terminate( []()
							{
								MLXMainWindow::set_crash_handler();
							} );
	}

	static MLXMainWindow *_instance;

	//protected slots:
	public slots:
	void OnFileNew();
	void OnFileAssetEditor();
	void OnFileLevelEditor();
	void OnFileExport2Bin();
	void OnEditBuild();
	void OnEditPublish();
	void OnEditOptions();
	void OnEditDvars();
	void OnHelpAbout();
	void OnHelpOpenGitHubRepo();
	void OnHelpReportIssue();
	void OnOpenZoneFile();
	void OnOpenSZCFile();
	void OnOpenScriptFile( const char *_ext );
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
	//void ContextMenuRequested(); // Moved to MLXMapList class
	void SteamUpdate();

	//protected:
	public:

	enum ToolTipDictMap
	{
		TOOLTIP_COMPILE = 1,
		TOOLTIP_LIGHT,
		TOOLTIP_LINK,
		TOOLTIP_RUN,
		TOOLTIP_EXEC_ARGS,
		TOOLTIP_BUILD,
		TOOLTIP_DVARS,
		TOOLTIP_COPY_OUTPUT,
		TOOLTIP_SAVE_OUTPUT,
		TOOLTIP_OPEN_LOG,
		TOOLTIP_IGNORE_ERRORS,
		TOOLTIP_OUTPUT_USES_MONO_FONT,
		TOOLTIP_OUTPUT_USES_COLOR_CODES,
		TOOLTIP_SEARCH_HIGHLIGHT_COLOR,
		TOOLTIP_THEME,
		TOOLTIP_CLEAN_XPAKS
	};

	enum HighlightColors
	{
		HIGHLIGHT_RED,
		HIGHLIGHT_ORANGE,
		HIGHLIGHT_YELLOW,
		HIGHLIGHT_GREEN,
		HIGHLIGHT_BLUE,
		HIGHLIGHT_PURPLE
	};

	// Seems like this isn't used - pv
	//void closeEvent( QCloseEvent *Event );

	void StartBuildThread( const QList<QPair<QString, QStringList>> &Commands );
	void StartConvertThread( QStringList &pathList, QString &outputDir, bool allowOverwrite );

	void PopulateFileList();
	void UpdateWorkshopItem();
	void ShowPublishDialog();
	void RegisterFonts();
	void DefineToolTips();

	void CreateActions();
	void CreateMenu();
	void CreateToolBar();

	void InitExport2BinGUI();

	QMap< mlx_fonts, int > RegisteredFonts;
	QFont DefaultFont;
	QFont mlx_output_font;

	QAction *mActionFileNew;
	QAction *mActionFileAssetEditor;
	QAction *mActionFileLevelEditor;
	QAction *mActionFileExport2Bin;
	QAction *mActionFileExit;
	QAction *mActionEditBuild;
	QAction *mActionEditPublish;
	QAction *mActionEditOptions;
	QAction *mActionHelpAbout;
	QAction *mActionHelpOpenGitHub;
	QAction *mActionHelpReportIssue;
	QAction *mActionToolsSaveLauncherOutput;
	QAction *mActionToolsCopyLauncherOutput;
	QAction *mActionToolsConsoleMPLog;
	QAction *mActionToolsMLXDebugLog;
	QAction *mActionToolsIgnoreErrors;
	QAction *mActionFolderRoot;
	QAction *mActionFolderUsermaps;
	QAction *mActionFolderShare;
	QAction *mActionFolderScriptsGlobal;
	QAction *mActionFolderSndAliases;

	MLXMapList *mFileListWidget;
	QPlainTextEdit *mOutputWidget;

	QPushButton *mBuildButton;
	QPushButton *mDvarsButton;
	QPushButton *mSaveOutputButton;
	QPushButton *mOpenLogButton;
	QPushButton *mDebugButton;
	QCheckBox *mCleanXPAKsWidget;
	QCheckBox *mCompileEnabledWidget;
	QComboBox *mCompileModeWidget;
	QCheckBox *mLightEnabledWidget;
	QComboBox *mLightQualityWidget;
	QCheckBox *mLinkEnabledWidget;
	QCheckBox *mRunEnabledWidget;
	QLineEdit *mRunOptionsWidget;
	QCheckBox *mIgnoreErrorsWidget;
	MLXOutputHighlighter *syntax_highlighter;
	QColor SearchHighlightColor;

	QStatusBar *StatusBar;

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

	QStringList mRunDvars;

	/// Contains a mapped dict of tooltips for easy definition and application
	std::map< ToolTipDictMap, QString > mlxToolTips;

	// Search bar stuff
	protected slots:
	void OnSearchTextChanged( const QString &text );
	void OnFindNext();
	void OnReturnPressed();
	void ClearHighlights();

	protected:
	void HighlightAllMatches( const QString &text );
	QTextDocument::FindFlags GetSearchFlags();

	QLineEdit *SearchBar;
	QPushButton *FindNextButton;
	QCheckBox *CaseSensitiveCheck;
	QCheckBox *WholeWordCheck;
	QTextCursor LastMatchCursor;
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
