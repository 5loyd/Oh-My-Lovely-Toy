#pragma once
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include <map>
#include <list>
#pragma comment(lib, "Setupapi.lib")

typedef struct NetCardStruct
{
	DWORD Id;           // �����豸��
	CString Name;    // ������
	bool Disabled;      // ��ǰ�Ƿ����
	bool Changed;       // �Ƿ���Ĺ�
} TNetCardStruct;
typedef TNetCardStruct* PNetCardStruct;

typedef std::list<TNetCardStruct> NetCardList;

class WinNetCard
{
public:
	WinNetCard(void);
	~WinNetCard(void);
private:
	static bool GetRegistryProperty(HDEVINFO DeviceInfoSet,   PSP_DEVINFO_DATA DeviceInfoData, ULONG Property, PVOID Buffer,PULONG Length);
public:
	static bool NetCardStateChange(void * NetCardPoint, bool Enabled);
	static void EnumNetCards(NetCardList *NetDeviceList)   ;
};

