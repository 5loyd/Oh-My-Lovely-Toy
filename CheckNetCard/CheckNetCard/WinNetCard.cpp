#include "StdAfx.h"
#include "WinNetCard.h"


WinNetCard::WinNetCard(void)
{
}


WinNetCard::~WinNetCard(void)
{
}

bool WinNetCard::GetRegistryProperty(HDEVINFO DeviceInfoSet,   
	PSP_DEVINFO_DATA DeviceInfoData, 
	ULONG Property, 
	PVOID Buffer,
	PULONG Length)   
{   
	while (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,   
		DeviceInfoData, Property, NULL, (BYTE *)*(TCHAR **)Buffer, *Length, Length))   
	{   
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)   
		{   
			if (*(LPTSTR *)Buffer) LocalFree(Buffer);   
			*(PCHAR *)Buffer = (PCHAR)LocalAlloc(LPTR,*Length);   
		}   
		else return false;   
	}   
	return (*(LPTSTR *)Buffer)[0];
}

void WinNetCard::EnumNetCards(NetCardList *NetDeviceList)   
{   
	CString     DevValue;      
	DWORD  Status, Problem;   
	LPTSTR Buffer   = NULL;   
	DWORD  BufSize  = 0;   
	HDEVINFO hDevInfo   = 0;  

	if( INVALID_HANDLE_VALUE == (hDevInfo = SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES)))   
		return; 

	SP_DEVINFO_DATA  DeviceInfoData ={sizeof(SP_DEVINFO_DATA)};

	PNetCardStruct NetCard = new TNetCardStruct;

	for(DWORD DeviceId=0;SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&DeviceInfoData);DeviceId++)   
	{   
		if (CM_Get_DevNode_Status(&Status, &Problem, DeviceInfoData.DevInst,0) != CR_SUCCESS)   
			continue;   

		if(GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS , &Buffer, (PULONG)&BufSize))   
			DevValue = Buffer;   

		if (DevValue == _T("Net"))   
		{   
			if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_ENUMERATOR_NAME , &Buffer, (PULONG)&BufSize))   
				DevValue = Buffer;   

			if (/*DevValue != "ROOT"*/1)   
			{   
				NetCard->Id = DeviceId;   
				NetCard->Name = "<Unknown Device>";   
				if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DRIVER , &Buffer, (PULONG)&BufSize))   
					if (GetRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC , &Buffer, (PULONG)&BufSize))   
						NetCard->Name = Buffer;   

				NetCard->Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem);   
				NetCard->Changed = false;   
				NetDeviceList->push_back(*NetCard);
			}   
		}   
	}   
	delete NetCard;
}   
//---------------------------------------------------------------------------
//功能:NetCardStateChange 网卡的启用与禁用
//参数:NetCardPoint 是 PNetCardStruct 的指针.
//参数:Enabled   true = 启用   false = 禁用
//---------------------------------------------------------------------------

bool WinNetCard::NetCardStateChange(void * NetCardPoint, bool Enabled)
{
	PNetCardStruct NetCard = (PNetCardStruct)NetCardPoint;
    DWORD DeviceId = NetCard->Id;
    HDEVINFO hDevInfo = 0;
	
	
	
	if (INVALID_HANDLE_VALUE == (hDevInfo =
                SetupDiGetClassDevs(NULL,NULL,0,DIGCF_PRESENT|DIGCF_ALLCLASSES)))
	{
        return false;
	}
	/*
	SetupDiGetClassDevs( 
      (LPGUID) &GUID_DEVCLASS_NET,    // GUID_DEVCLASS_NET表示仅列出网络设备 
      NULL,  
      this->m_hWnd,  
      DIGCF_PRESENT); 
	  */
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)};
    DWORD Status, Problem;

	if (!SetupDiEnumDeviceInfo(hDevInfo,DeviceId,&DeviceInfoData))  //
        return false;

    if (CM_Get_DevNode_Status(&Status, &Problem,
                DeviceInfoData.DevInst,0) != CR_SUCCESS)   //读取网卡状态
        return false;

	SP_PROPCHANGE_PARAMS PropChangeParams = {sizeof(SP_CLASSINSTALL_HEADER)};
    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    PropChangeParams.Scope = DICS_FLAG_GLOBAL;

	if (Enabled)
    {
        if (!((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem)))
        {
            NetCard->Disabled = false;
            return false;
        }
        PropChangeParams.StateChange = DICS_ENABLE;
    }
    else
    {
        if ((Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem))
        {
            NetCard->Disabled = true;
            return false;
        }
        if (!((Status & DN_DISABLEABLE) && (CM_PROB_HARDWARE_DISABLED != Problem)))
            return false;
        PropChangeParams.StateChange = DICS_DISABLE;
    }
	
	if (!SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData,(SP_CLASSINSTALL_HEADER *)&PropChangeParams, sizeof(PropChangeParams)))
	{//功能:设置网卡有效或无效
        return false;
	}
    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData))
	{
		return false;
	}
    if (CM_Get_DevNode_Status(&Status, &Problem,DeviceInfoData.DevInst,0) == CR_SUCCESS)
	{
        NetCard->Disabled = (Status & DN_HAS_PROBLEM) && (CM_PROB_DISABLED == Problem);
	}
    return true;
}