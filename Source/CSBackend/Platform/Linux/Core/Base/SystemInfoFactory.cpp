//  The MIT License (MIT)
//
//  Copyright (c) 2016 Tag Games Limited
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

#include <ChilliSource/Core/Base/DeviceInfo.h>
#include <ChilliSource/Core/Base/ScreenInfo.h>
#include <ChilliSource/Core/Base/SystemInfo.h>
#include <ChilliSource/Core/String/StringUtils.h>

#include <CSBackend/Platform/Linux/Core/Base/Screen.h>
#include <CSBackend/Platform/Linux/Core/Base/SystemInfoFactory.h>
#include <CSBackend/Rendering/OpenGL/Base/RenderInfoFactory.h>

#include <vector>
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
            const std::string k_defaultOSVersion = "UnknownVersion";
            const std::string k_defaultDeviceModel = "Linux";
            const std::string k_deviceModelType = "PC";
            const std::string k_defaultDeviceManufacturer = "OpenSource";
            const std::string k_defaultDeviceUdid = "FAKE ID";
            

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
				return k_defaultDeviceUdid;
            }

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
				return k_defaultDeviceModel;
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
				return k_defaultDeviceManufacturer;
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
				return k_defaultOSVersion;
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

            /// Returns the language portion of a locale code.
            ///
            /// @param The locale code.
            ///
            /// @return The language code.
            ///
            std::string ParseLanguageFromLocale(const std::string& in_locale) noexcept
            {
                std::vector<std::string> strLocaleBrokenUp = ChilliSource::StringUtils::Split(in_locale, "_", 0);

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
			/// @return The number of cores.
			//----------------------------------------------
            u32 GetNumberOfCPUCores()
            {
				return std::thread::hardware_concurrency();
            }
        

            /// @return The screen's current resolution.
            ///
            ChilliSource::Vector2 GetScreenResolution() noexcept
            {
                ChilliSource::Integer2 resolution = SFMLWindow::Get()->GetWindowSize();
                return ChilliSource::Vector2((f32)resolution.x, (f32)resolution.y);
            }

            /// @return A list of resolutions supported by the display
            ///
            std::vector<ChilliSource::Integer2> GetSupportedFullscreenResolutions() noexcept
            {
                return SFMLWindow::Get()->GetSupportedFullscreenResolutions();
            }

        }

        //--------------------------------------------------------------------------------
        ChilliSource::SystemInfoCUPtr SystemInfoFactory::CreateSystemInfo() noexcept
        {
             // Create DeviceInfo.
            ChilliSource::DeviceInfo deviceInfo(GetDeviceModel(), k_deviceModelType, GetDeviceManufacturer(), GetUDID(), GetLocale(), ParseLanguageFromLocale(GetLocale()), GetOSVersion(), GetNumberOfCPUCores());

            // Create ScreenInfo.
            ChilliSource::ScreenInfo screenInfo(GetScreenResolution(), 1.0f, 1.0f, GetSupportedFullscreenResolutions());

			//Create RenderInfo
    		ChilliSource::RenderInfo renderInfo = OpenGL::RenderInfoFactory::CreateRenderInfo();

            // Create SystemInfo.
            ChilliSource::SystemInfoUPtr systemInfo(new ChilliSource::SystemInfo(deviceInfo, screenInfo, renderInfo, ""));

            return std::move(systemInfo);
        }
        
    }
}

#endif