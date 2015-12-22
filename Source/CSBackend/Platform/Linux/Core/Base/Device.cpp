//
//  Device.cpp
//  Chilli Source
//  Created by Ian Copland on 24/04/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifdef CS_TARGETPLATFORM_LINUX

#include <CSBackend/Platform/Linux/Core/Base/Device.h>

#include <algorithm>
#include <cstdio>
#include <locale>
#include <sys/utsname.h>
#include <thread>
#include <unistd.h>

namespace CSBackend
{
    namespace Linux
    {
        namespace
        {
			const std::string k_defaultLocale = "en_US";
			const std::string k_defaultLanguage = "en";
            //----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The device model name.
            //----------------------------------------------
            std::string GetDeviceModel()
            {
				struct utsname deviceInfo;
				if(uname(&deviceInfo) == 0)
				{
					return std::string(deviceInfo.sysname);
				}
				return "Linux";
            }
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The device model type name.
			//----------------------------------------------
            std::string GetDeviceModelType()
            {
				return "PC";
            }
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The device manufacturer name.
			//----------------------------------------------
            std::string GetDeviceManufacturer()
            {
				struct utsname deviceInfo;
				if(uname(&deviceInfo) == 0)
				{
					return std::string(deviceInfo.machine);
				}
				return "OpenSource";
            }
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The OS version number string.
			//----------------------------------------------
            std::string GetOSVersion()
            {
				struct utsname deviceInfo;
				if(uname(&deviceInfo) == 0)
				{
					return std::string(deviceInfo.release);
				}
				return "UnknownVersion";
            }
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The current locale.
			//----------------------------------------------
            std::string GetLocale()
            {
				std::locale l;
				if (l.name().empty() == false)
				{
					return l.name();
				}

				return k_defaultLocale;
            }
			//----------------------------------------------------
			/// Returns the language portion of a locale code.
			///
			/// @author Ian Copland
			///
			/// @param The locale code.
			///
			/// @return The language code.
			//----------------------------------------------------
			std::string ParseLanguageFromLocale(const std::string& in_locale)
			{
				std::vector<std::string> strLocaleBrokenUp = CSCore::StringUtils::Split(in_locale, "_", 0);

				if (strLocaleBrokenUp.size() > 0)
				{
					return strLocaleBrokenUp[0];
				}
				else
				{
					return k_defaultLanguage;
				}
			}
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The UDID.
			//----------------------------------------------
            std::string GetUDID()
            {
				const int k_bufferSize = 256;
				char buf[k_bufferSize];
				long uid = gethostid();
				if(snprintf(buf, k_bufferSize, "%ld", uid) != 0)
				{
					return std::string(buf);
				}
				return "FAKE ID";
            }
			//----------------------------------------------
			/// @author Ian Copland
			///
			/// @return The number of cores.
			//----------------------------------------------
            u32 GetNumberOfCPUCores()
            {
				return std::thread::hardware_concurrency();
            }
        }
        
        CS_DEFINE_NAMEDTYPE(Device);
        //----------------------------------------------------
        //----------------------------------------------------
        Device::Device()
			: m_locale(k_defaultLocale), m_language(k_defaultLanguage)
        {
			m_model = CSBackend::Linux::GetDeviceModel();
			m_modelType = CSBackend::Linux::GetDeviceModelType();
			m_manufacturer = CSBackend::Linux::GetDeviceManufacturer();
			m_locale = CSBackend::Linux::GetLocale();
			m_language = CSBackend::Linux::ParseLanguageFromLocale(m_locale);
			m_osVersion = CSBackend::Linux::GetOSVersion();
			m_udid = CSBackend::Linux::GetUDID();
            m_numCPUCores = CSBackend::Linux::GetNumberOfCPUCores();
        }
        //-------------------------------------------------------
        //-------------------------------------------------------
        bool Device::IsA(CSCore::InterfaceIDType in_interfaceId) const
        {
            return (CSCore::Device::InterfaceID == in_interfaceId || Device::InterfaceID == in_interfaceId);
        }
        //---------------------------------------------------
        //---------------------------------------------------
        const std::string& Device::GetModel() const
        {
            return m_model;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetModelType() const
        {
            return m_modelType;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetManufacturer() const
        {
            return m_manufacturer;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetLocale() const
        {
            return m_locale;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetLanguage() const
        {
            return m_language;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetOSVersion() const
        {
            return m_osVersion;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		const std::string& Device::GetUDID() const
        {
            return m_udid;
        }
        //---------------------------------------------------
        //---------------------------------------------------
		u32 Device::GetNumberOfCPUCores() const
        {
            return m_numCPUCores;
        }
    }
}

#endif
