#include <iostream>
#include <windows.h>
#include <errhandlingapi.h>
#include <initguid.h>
#include <SetupAPI.h>
#include <devpkey.h>
#include <String>

#pragma comment (lib, "Setupapi.lib")

std::string GetLastErrorAsString()
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

int main()
{
    time_t now = time(0);
    tm tmInfo;
    localtime_s(&tmInfo, &now);
    char buffer[101];
    strftime(buffer + 21, 80, "%Y-%m-%d %X", &tmInfo);
    memcpy(buffer, "Connected Devices at ", 21);
    SetConsoleTitleA(buffer);


    HDEVINFO device_info_list = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (device_info_list != INVALID_HANDLE_VALUE)
    {
        SP_DEVINFO_DATA device_info_data;
        memset(&device_info_data, 0, sizeof(SP_DEVINFO_DATA));
        device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

        int device_index = 0;

        while (SetupDiEnumDeviceInfo(device_info_list, device_index, &device_info_data))
        {
            device_index++;


            DEVPROPKEY friendly_name_property = DEVPKEY_NAME;
            DEVPROPTYPE friendly_name_property_type;
            DWORD friendly_name_property_byte_size = 0;

            SetupDiGetDevicePropertyW(
                device_info_list,
                &device_info_data,
                &friendly_name_property,
                &friendly_name_property_type,
                NULL,
                0,
                &friendly_name_property_byte_size,
                0
            );

            char* out_buffer = new char[friendly_name_property_byte_size];
            memset(out_buffer, 0, friendly_name_property_byte_size);


            std::cout << "Device " << device_index << ": ";

            if (SetupDiGetDevicePropertyW(
                device_info_list,
                &device_info_data,
                &friendly_name_property,
                &friendly_name_property_type,
                (PBYTE)out_buffer,
                friendly_name_property_byte_size,
                &friendly_name_property_byte_size,
                0
            ))
            {
                for (int i = 0; i < friendly_name_property_byte_size; i += 2)
                {
                    std::cout << out_buffer[i];
                }
                std::cout << "\n";
            }
            else
            {
                std::cout << "SetupDiGetDevicePropertyW error " << GetLastError() << " " << GetLastErrorAsString();
            }
            delete[] out_buffer;



        }

        DWORD error = GetLastError();
        if (error == ERROR_NO_MORE_ITEMS)
        {
            std::cout << "No More Connected Devices to Enumerate\n";
        }
        else
        {
            std::cout << "SetupDiEnumDeviceInfo Unknown Error " << error << " \n";
        }

    }
    else
    {
        std::cout << "SetupDiGetClassDevs invalid handle returned\n";
    }

    SetupDiDestroyDeviceInfoList(device_info_list);
    std::cin.get();
}
