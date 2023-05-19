#include <iostream>
#include <windows.h>
#include <errhandlingapi.h>
#include <initguid.h>
#include <SetupAPI.h>
#include <devpkey.h>
#include <String>

//MSVC link against setupapi library
#pragma comment (lib, "Setupapi.lib")

std::string get_last_error_as_string();
void set_console_title();

int main()
{
    //set the console title with the date and time
    set_console_title();

    //get the device list and check for valid handle
    //gets the device list of all device types that are currently connected
    HDEVINFO device_info_list = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (device_info_list == INVALID_HANDLE_VALUE)
    {
        SetupDiDestroyDeviceInfoList(device_info_list);
        printf("SetupDiGetClassDevs invalid handle returned\n");
        std::cin.get();
        exit(1);
    }

    //initialize the data structure to be filled with individual device data
    SP_DEVINFO_DATA device_info_data;
    ZeroMemory(&device_info_data, sizeof(device_info_data));
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

    //eniumerate all devices in the device list
    int device_index = 0;
    while (SetupDiEnumDeviceInfo(device_info_list, device_index, &device_info_data))
    {
        device_index++;

        DEVPROPKEY device_name = DEVPKEY_NAME;
        DEVPROPTYPE name_type;
        DWORD name_size = 0;
        
        //first call is to get the size of the name
        SetupDiGetDevicePropertyW(device_info_list, &device_info_data, &device_name, &name_type, NULL, 0, &name_size, 0);

        //actually get and store the name
        std::wstring name(name_size, L' ');
        BOOL got_name = SetupDiGetDevicePropertyW(device_info_list, &device_info_data, &device_name, &name_type, (PBYTE)name.data(), name_size, &name_size, 0);

        if (!got_name)
        {
            printf("Unable to get name for device[%d]: %s\n", device_index, get_last_error_as_string().c_str());
            continue;
        }

        printf("Device[%d]: %ls\n", device_index, name.c_str());
    }


    //make sure that we have no more devices and not some other error
    if (GetLastError() == ERROR_NO_MORE_ITEMS)
    {
        printf("No More Connected Devices to Enumerate\n");
    }
    else
    {
        printf("SetupDiEnumDeviceInfo Error: %s\n", get_last_error_as_string().c_str());
    }

    SetupDiDestroyDeviceInfoList(device_info_list);
    std::cin.get();
}

//sets console title as current date and time 
void set_console_title()
{
    char buffer[101];
    memcpy(buffer, "Connected Devices at ", 21);

    time_t now = time(0);
    tm tmInfo;
    localtime_s(&tmInfo, &now);
    strftime(buffer + 21, 80, "%Y-%m-%d %X", &tmInfo);
    SetConsoleTitleA(buffer);
}

//gets the last win32 api error as a std::string
std::string get_last_error_as_string()
{
    DWORD error = GetLastError();
    if (error == 0)
    {
        return std::string();
    }

    LPSTR message_buffer;


    DWORD message_size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message_buffer,
        0,
        NULL
    );

    std::string message(message_buffer, message_size);

    LocalFree(message_buffer);

    return message;
}