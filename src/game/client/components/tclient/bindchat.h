
#ifndef GAME_CLIENT_COMPONENTS_BINDCHAT_H
#define GAME_CLIENT_COMPONENTS_BINDCHAT_H
#include <game/client/component.h>
class IConfigManager;

enum
{
	BINDCHAT_MAX_NAME = 64,
	BINDCHAT_MAX_CMD = 1024,
	BINDCHAT_MAX_BINDS = 256,
};

class CBindChat : public CComponent
{
	static void ConAddBindchat(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveBindchat(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveAllBindchatBinds(IConsole::IResult *pResult, void *pUserData);

	static void ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData);

public:
	class CBind
	{
	public:
		char m_aName[BINDCHAT_MAX_NAME];
		char m_aCommand[BINDCHAT_MAX_CMD];

		bool operator==(const CBind &Other) const
		{
			return str_comp(m_aName, Other.m_aName) == 0 && str_comp(m_aCommand, Other.m_aCommand) == 0;
		}
	};

	std::vector<CBind> m_vBinds;

	CBindChat();
	virtual int Sizeof() const override { return sizeof(*this); }

	virtual void OnConsoleInit() override;

	void AddBind(const char *Name, const char *Command);
	void RemoveBind(const char *Name, const char *Command);
	void RemoveBind(int Index);
	void RemoveAllBinds();

	void ExecuteBind(int Bind, const char *pArgs);

	bool ChatDoBinds(const char *pText);
	bool ChatDoAutocomplete(bool ShiftPressed);
};

#endif
