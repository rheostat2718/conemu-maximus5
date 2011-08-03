
#pragma once

enum {
	REPluginName=0,
	REBarShiftF1,
	REBarShiftF1L,
	REBarShiftF2,
	REBarShiftF2L,
	REBarShiftF2Virt,
	REBarShiftF2VirtL,
	REBarF3,
	REBarF3L,
	REBarAltF3,
	REBarAltF3L,
	REBarAltShiftF3,
	REBarAltShiftF3L,
	REBarShiftF3,
	REBarShiftF3L,
	REBarCtrlAltF3,
	REBarCtrlAltF3L,
	REBarF4,
	REBarF4L,
	REBarAltF4,
	REBarAltF4L,
	REBarAltShiftF4,
	REBarAltShiftF4L,
	REBarCtrlAltF4,
	REBarCtrlAltF4L,
	REBarShiftF4,
	REBarShiftF4L,
	REBarF5,
	REBarF5L,
	REBarShiftF5,
	REBarShiftF5L,
	REBarShiftF6,
	REBarShiftF6L,
	REBarF7,
	REBarF7L,
	REBarAltShiftF7,
	REBarAltShiftF7L,
	REBarShiftF7,
	REBarShiftF7L,
	REPanelx32Label,
	REPanelx32VirtLabel,
	REPanelx64Label,
	REPanelx64VirtLabel,
	REPanelVirtLabel,
	REPanelBackupPrefix,
	REPanelBackupRestorePrefix,
	REDeletedKey,
	REDeletedValue,
	REBookmarksTitle,
	REBookmarksExtTitle,
	REBookmarksFooter,
	REBookmarksExtFooter,
	REConfigTitle,
	REAddToPluginsMenu,
	REAddToDisksMenu,
	REDisksMenuHotkey,
	REPrefixSeparator,
	REPrefixLabel,
	REPanelPrefixLabel,
	REWow6432,
	REWow64process,
	REWow64on32,
	REBrowseRegFiles,
	REBrowseHives,
	RESkipAccessDeniedMessage,
	REUseBackupRestore,
	RESkipPrivilegesDeniedMessage,
	REShowKeysAsDirs,
	REExportDefaultValueFirst,
	RECreateUnicodeFiles,
	REAnsiCpLabel,
	REEditBinaryAsText,
	REEscapeRNonExporting,
	RESpeedUpLargeKeysSeparator,
	REUnsortLargeKeys,
	RELoadDescriptions,
	REAutoRefreshSeparator,
	REAutoRefreshChanges,
	RERefreshKeyTimeoutLabel,
	RERefreshSubkeyTimeoutLabel,
	RECheckMacroSeq,
	RECheckMacroVar,
	REStorePanelMode,
	REBtnOK,
	REBtnCancel,
	REBtnDiscard,
	REBtnContinue,
	REBtnEditor,
	REBtnOverwrite,
	REBtnAppend,
	REBtnVisual,
	REChangeNumber,
	REValueNameLabel,
	REValueTypeLabel,
	REValueDataLabel,
	REValueBaseLabel,
	REValueBaseHex,
	REValueBaseUnsigned,
	REValueBaseSigned,
	REValueSizeStatic,
	REValueShowAltStatic,
	REValueErrorLabel,
	RENewValueTitle,
	RENewValueNameLabel,
	RENewValueName,
	REChangeDescTitle,
	REChangeDescLabel,
	RERenameKeyTitle,
	RECopyKeyTitle,
	RERenameKeyLabel,
	RERenameValueTitle,
	RECopyValueTitle,
	RERenameValueLabel,
	RECreateKeyTitle,
	RECreateKeyLabel,
	REImportValueTitle,
	REImportValueLabel,
	REImportUnicodeStrings,
	REImportForAll,
	REGuiPermissionEdit,
	REGuiPermissionTitle,
	REConnectRemoteTitle,
	REConnectServerNameLabel,
	REConnectUserNameLabel,
	REConnectPasswordLabel,
	REConnectMedia,
	REBtnConnect,
	REBtnLocal,
	REExportDlgTitle,
	RESaveRegDlgTitle,
	REExportItemLabel,
	REExportItemsLabel,
	RESaveRegItemLabel,
	REExportItemsProgress,
	RECopyDlgTitle,
	RECopyItemLabel,
	RECopyItemsLabel,
	REImportDlgTitle,
	RELoadingRegFileTitle,
	RELoadingRegFileText,
	REExportFormatLabel,
	REExportReg4,
	REExportReg5,
	REExportCmd,
	REExportHive,
	REExportRaw,
	REExportHiveRoot,
	REExportRawUnicode,
	REFormatFailLabel,
	REFormatFailLineInfo,
	REActionBrowse,
	REActionImport,
	REActionImport32,
	REActionImport64,
	REActionImportHere,
	REActionImportValuesHere,
	REActionImportRegAsRaw,
	REImportStyleKeyTitle,
	REImportStyleFileLabel,
	REImportStyleKeyLabel,
	RE_REG_NONE,
	RE_REG_SZ,
	RE_REG_EXPAND_SZ,
	RE_REG_BINARY,
	RE_REG_DWORD,
	RE_REG_DWORD_BIG_ENDIAN,
	RE_REG_LINK,
	RE_REG_MULTI_SZ,
	RE_REG_RESOURCE_LIST,
	RE_REG_FULL_RESOURCE_DESCRIPTOR,
	RE_REG_RESOURCE_REQUIREMENTS_LIST,
	RE_REG_QWORD,
	RECmdHeader,
	RECmdVarReset,
	RECmdUTF8Warn,
	RECmdErrCheck,
	RECmdErrFin,
	REM_CmdWarnUtf8,
	REM_CmdWarnOEM,
	REM_Warning,
	REM_WarningDelete,
	REM_ConfirmDelete,
	REM_NumberRequireName,
	REM_CantAjustPrivileges,
	REM_CantOpenKeyRead,
	REM_CantOpenKeyWrite,
	REM_CantOpenKeyDelete,
	REM_KeyNotExists,
	REM_CurrentPath,
	REM_KeyNameTooLong,
	REM_ValueNotExists,
	REM_CantOpenValueRead,
	REM_CantOpenValueWrite,
	REM_CantRenameDefaultValue,
	REM_NotEnoughRighsForCopy,
	REM_KeyAlreadyExists,
	REM_ValueAlreadyExists,
	REM_KeyWillBeErased,
	REM_ValueWillBeErased,
	REM_ValueForcedToMultiSZ,
	REM_CantSaveKey,
	REM_CantLoadKey,
	REM_CantRenameKey,
	REM_CantDeleteKey,
	REM_CantDeleteValue,
	REM_RegFileLoadFailed,
	REM_CmdLineFail_Export,
	REM_CmdLineFail_Import,
	REM_CmdLineFail_OpenRegFile,
	REM_CmdLineFail_Remote,
	REM_CmdLineFail_RemoteArg,
	REM_CmdLineFail_UnknownCommand,
	REM_ImportConfirm,
	REM_ImportSuccess,
	REM_ImportFailed,
	REM_ImportCreateKeyFailed,
	REM_ImportDeleteKeyFailed,
	REM_CantCreateTempFolder,
	REM_CantCreateTempFile,
	REM_CantOpenFileReading,
	REM_CantReadFile,
	REM_CantAllocateMemory,
	REM_FileAlreadyExist,
	REM_FileExistInfo,
	REM_OperationCancelledByUser,
	REM_ConfirmOperationCancel,
	REM_SetDirectoryFailed,
	REM_OpenPluginPrefixFailed,
	REM_HiveOpenFailed,
	REM_Duration,
	REM_CmdLineFail_HiveFile,
	REM_CmdLineFail_HiveKey,
	REM_CmdLineFail_Unmount,
	REM_Unmount_Confirm,
	REM_Mount_Fail,
	REM_Unmount_Fail,
	REM_LogonFailed,
	REM_ImpersonateFailed,
	REM_RevertToSelfFailed,
	REM_RemoteConnecting,
	REM_RemoteKeyFailed,
	REM_UnknownFileFormat,
	REM_ConfirmBookmarkReset,
	REM_EmptyKeyNameNotAllowed,
	REM_EmptyValueNameNotAllowed,
	REM_BadDoubleZero,
	REM_CmdBadKeyName,
	REM_CmdBadValueName,
	REM_CmdBadValueMSZ,
	REM_CmdErrMsz0,
	REM_CreateToolhelp32SnapshotFailed,
	REM_RegeditExeFailed,
	REM_RegeditWindowFailed,
	REM_RegeditWindowHung,
	REM_RegeditCommunicationFailed,
	REM_RegeditFailed_VirtualAllocEx,
	REM_RegeditFailed_WriteProcessMemory,
	REM_RegeditFailed_ReadProcessMemory,
	REM_RegeditFailed_LoadLibraryPtr,
	REM_RegeditFailed_CreateRemoteThread,
	REM_RegeditFailed_TVM_GETITEMW,
	REM_RegeditFailed_WaitThreadReady,
	REM_RegeditFailed_Pipe,
	REM_RegeditFailed_NullArgs,
	REM_RegeditFailed_NoWindow,
	REM_RegeditFailed_KeyNotFound,
	REM_RegeditFailed_ValueNotFound,
	REM_RegeditFailed_NeedHandle,
	REM_RegeditFailed_RequireH64,
	REStartingRegedit,
	REWaitForIdleRegedit,
	REM_MPEC_UNRECOGNIZED_KEYWORD,
	REM_MPEC_UNRECOGNIZED_FUNCTION,
	REM_MPEC_FUNC_PARAM,
	REM_MPEC_NOT_EXPECTED_ELSE,
	REM_MPEC_NOT_EXPECTED_END,
	REM_MPEC_UNEXPECTED_EOS,
	REM_MPEC_EXPECTED_TOKEN,
	REM_MPEC_BAD_HEX_CONTROL_CHAR,
	REM_MPEC_BAD_CONTROL_CHAR,
	REM_MPEC_VAR_EXPECTED,
	REM_MPEC_EXPR_EXPECTED,
	REM_MPEC_ZEROLENGTHMACRO,
	REM_MPEC_INTPARSERERROR,
	REM_MPEC_CONTINUE_OTL,
	REM_GetKeySecurityFail,
	REM_SetKeyOwnerFail,
	REM_SetKeySecurityFail,
	REM_DescrToStringFail,
	REM_StringToDescrFail,
	};
