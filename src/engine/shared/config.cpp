/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/config.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <engine/storage.h>

CConfig g_Config;

void EscapeParam(char *pDst, const char *pSrc, int Size)
{
	str_escape(&pDst, pSrc, pDst + Size);
}

CConfigManager::CConfigManager()
{
	m_pStorage = 0;
	m_ConfigFile = 0;
	m_NumCallbacks = 0;
	m_NumTCallbacks = 0;
	m_Failed = false;
}

void CConfigManager::Init()
{
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	Reset();
}

void CConfigManager::Reset()
{
#define MACRO_CONFIG_INT(Name, ScriptName, def, min, max, flags, desc) g_Config.m_##Name = def;
#define MACRO_CONFIG_COL(Name, ScriptName, def, flags, desc) MACRO_CONFIG_INT(Name, ScriptName, def, 0, 0, flags, desc)
#define MACRO_CONFIG_STR(Name, ScriptName, len, def, flags, desc) str_copy(g_Config.m_##Name, def, len);

#include "config_variables.h"

#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_COL
#undef MACRO_CONFIG_STR
}

void CConfigManager::Reset(const char *pScriptName)
{
#define MACRO_CONFIG_INT(Name, ScriptName, def, min, max, flags, desc) \
	if(str_comp(pScriptName, #ScriptName) == 0) \
	{ \
		g_Config.m_##Name = def; \
		return; \
	};
#define MACRO_CONFIG_COL(Name, ScriptName, def, flags, desc) MACRO_CONFIG_INT(Name, ScriptName, def, 0, 0, flags, desc)
#define MACRO_CONFIG_STR(Name, ScriptName, len, def, flags, desc) \
	if(str_comp(pScriptName, #ScriptName) == 0) \
	{ \
		str_copy(g_Config.m_##Name, def, len); \
		return; \
	};

#include "config_variables.h"

#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_COL
#undef MACRO_CONFIG_STR
}

bool CConfigManager::Save()
{
	if(!m_pStorage || !g_Config.m_ClSaveSettings)
		return true;

	char aConfigFileTmp[IO_MAX_PATH_LENGTH];
	m_ConfigFile = m_pStorage->OpenFile(IStorage::FormatTmpPath(aConfigFileTmp, sizeof(aConfigFileTmp), CONFIG_FILE), IOFLAG_WRITE, IStorage::TYPE_SAVE);

	if(!m_ConfigFile)
	{
		dbg_msg("config", "ERROR: opening %s failed", aConfigFileTmp);
		return false;
	}

	m_Failed = false;

	char aLineBuf[1024 * 2];
	char aEscapeBuf[1024 * 2];

#define MACRO_CONFIG_INT(Name, ScriptName, def, min, max, flags, desc) \
	if((flags)&CFGFLAG_SAVE && g_Config.m_##Name != (def)) \
	{ \
		str_format(aLineBuf, sizeof(aLineBuf), "%s %i", #ScriptName, g_Config.m_##Name); \
		WriteLine(aLineBuf); \
	}
#define MACRO_CONFIG_COL(Name, ScriptName, def, flags, desc) \
	if((flags)&CFGFLAG_SAVE && g_Config.m_##Name != (def)) \
	{ \
		str_format(aLineBuf, sizeof(aLineBuf), "%s %u", #ScriptName, g_Config.m_##Name); \
		WriteLine(aLineBuf); \
	}
#define MACRO_CONFIG_STR(Name, ScriptName, len, def, flags, desc) \
	if((flags)&CFGFLAG_SAVE && str_comp(g_Config.m_##Name, def) != 0) \
	{ \
		EscapeParam(aEscapeBuf, g_Config.m_##Name, sizeof(aEscapeBuf)); \
		str_format(aLineBuf, sizeof(aLineBuf), "%s \"%s\"", #ScriptName, aEscapeBuf); \
		WriteLine(aLineBuf); \
	}

#include "config_variables.h"

#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_COL
#undef MACRO_CONFIG_STR

	for(int i = 0; i < m_NumCallbacks; i++)
		m_aCallbacks[i].m_pfnFunc(this, m_aCallbacks[i].m_pUserData);

	if(io_sync(m_ConfigFile) != 0)
	{
		m_Failed = true;
	}

	if(io_close(m_ConfigFile) != 0)
		m_Failed = true;

	m_ConfigFile = 0;

	if(m_Failed)
	{
		dbg_msg("config", "ERROR: writing to %s failed", aConfigFileTmp);
		return false;
	}

	if(!m_pStorage->RenameFile(aConfigFileTmp, CONFIG_FILE, IStorage::TYPE_SAVE))
	{
		dbg_msg("config", "ERROR: renaming %s to " CONFIG_FILE " failed", aConfigFileTmp);
		return false;
	}
	TSave();
	return true;
}

bool CConfigManager::TSave()
{
	if(!m_pStorage || !g_Config.m_ClSaveSettings)
		return true;

	char aConfigFileTmp[IO_MAX_PATH_LENGTH];
	m_ConfigFile = m_pStorage->OpenFile(IStorage::FormatTmpPath(aConfigFileTmp, sizeof(aConfigFileTmp), TCONFIG_FILE), IOFLAG_WRITE, IStorage::TYPE_SAVE);

	if(!m_ConfigFile)
	{
		dbg_msg("config", "ERROR: opening %s failed", aConfigFileTmp);
		return false;
	}

	m_Failed = false;

	char aLineBuf[1024 * 2];
	char aEscapeBuf[1024 * 2];

#define MACRO_CONFIG_INT(Name, ScriptName, def, min, max, flags, desc) \
	if((flags)&CFGFLAG_SAVE && g_Config.m_##Name != def) \
	{ \
		str_format(aLineBuf, sizeof(aLineBuf), "%s %i", #ScriptName, g_Config.m_##Name); \
		WriteLine(aLineBuf); \
	}
#define MACRO_CONFIG_COL(Name, ScriptName, def, flags, desc) \
	if((flags)&CFGFLAG_SAVE && g_Config.m_##Name != def) \
	{ \
		str_format(aLineBuf, sizeof(aLineBuf), "%s %u", #ScriptName, g_Config.m_##Name); \
		WriteLine(aLineBuf); \
	}
#define MACRO_CONFIG_STR(Name, ScriptName, len, def, flags, desc) \
	if((flags)&CFGFLAG_SAVE && str_comp(g_Config.m_##Name, def) != 0) \
	{ \
		EscapeParam(aEscapeBuf, g_Config.m_##Name, sizeof(aEscapeBuf)); \
		str_format(aLineBuf, sizeof(aLineBuf), "%s \"%s\"", #ScriptName, aEscapeBuf); \
		WriteLine(aLineBuf); \
	}

#include "././game/tater_variables.h"

#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_COL
#undef MACRO_CONFIG_STR

	for(int i = 0; i < m_NumTCallbacks; i++)
		m_aTCallbacks[i].m_pfnFunc(this, m_aTCallbacks[i].m_pUserData);


	if(io_sync(m_ConfigFile) != 0)
	{
		m_Failed = true;
	}

	if(io_close(m_ConfigFile) != 0)
		m_Failed = true;

	m_ConfigFile = 0;

	if(m_Failed)
	{
		dbg_msg("config", "ERROR: writing to %s failed", aConfigFileTmp);
		return false;
	}

	if(!m_pStorage->RenameFile(aConfigFileTmp, TCONFIG_FILE, IStorage::TYPE_SAVE))
	{
		dbg_msg("config", "ERROR: renaming %s to " TCONFIG_FILE " failed", aConfigFileTmp);
		return false;
	}

	return true;
}

void CConfigManager::RegisterCallback(SAVECALLBACKFUNC pfnFunc, void *pUserData)
{
	dbg_assert(m_NumCallbacks < MAX_CALLBACKS, "too many config callbacks");
	m_aCallbacks[m_NumCallbacks].m_pfnFunc = pfnFunc;
	m_aCallbacks[m_NumCallbacks].m_pUserData = pUserData;
	m_NumCallbacks++;
}

void CConfigManager::RegisterTCallback(SAVECALLBACKFUNC pfnFunc, void *pUserData)
{
	dbg_assert(m_NumTCallbacks < MAX_CALLBACKS, "too many tater config callbacks");
	m_aTCallbacks[m_NumTCallbacks].m_pfnFunc = pfnFunc;
	m_aTCallbacks[m_NumTCallbacks].m_pUserData = pUserData;
	m_NumTCallbacks++;
}


void CConfigManager::WriteLine(const char *pLine)
{
	if(!m_ConfigFile ||
		io_write(m_ConfigFile, pLine, str_length(pLine)) != static_cast<unsigned>(str_length(pLine)) ||
#if defined(CONF_FAMILY_WINDOWS)
		io_write_newline(m_ConfigFile) != 2)
#else
		io_write_newline(m_ConfigFile) != 1)
#endif
		m_Failed = true;
}

IConfigManager *CreateConfigManager() { return new CConfigManager; }
