//
//  PlatformSystem.cpp
//  ChilliSource
//  Created by Scott Downie on 24/11/2010.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2010 Tag Games Limited
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

#include <CSBackend/Platform/Linux/Core/Base/PlatformSystem.h>
#include <CSBackend/Platform/Linux/SFML/Base/SFMLWindow.h>
#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Threading/TaskScheduler.h>
#include <ChilliSource/UI/Base/CursorSystem.h>

#include <chrono>

namespace CSBackend 
{
	namespace Linux
	{
		CS_DEFINE_NAMEDTYPE(PlatformSystem);
		//-----------------------------------------
		//-----------------------------------------
		PlatformSystem::PlatformSystem() 
		{
		}
		//--------------------------------------------------
		//--------------------------------------------------
		bool PlatformSystem::IsA(ChilliSource::InterfaceIDType in_interfaceId) const
		{
			return (ChilliSource::PlatformSystem::InterfaceID == in_interfaceId || PlatformSystem::InterfaceID == in_interfaceId);
		}
		//-------------------------------------------------
		//-------------------------------------------------
		void PlatformSystem::CreateDefaultSystems(ChilliSource::Application* in_application)
		{
			in_application->CreateSystem<ChilliSource::CursorSystem>();
		}
		//-------------------------------------------------
		//-------------------------------------------------
		void PlatformSystem::SetPreferredFPS(u32 in_fps)
		{
			ChilliSource::Application::Get()->GetTaskScheduler()->ScheduleTask(ChilliSource::TaskType::k_system, [=](const ChilliSource::TaskContext& in_taskContext)
			{
				SFMLWindow::Get()->SetPreferredFPS(in_fps);
			});
		}
		//---------------------------------------------------
		//---------------------------------------------------
		void PlatformSystem::SetVSyncEnabled(bool in_enabled)
		{
			ChilliSource::Application::Get()->GetTaskScheduler()->ScheduleTask(ChilliSource::TaskType::k_system, [=](const ChilliSource::TaskContext& in_taskContext)
			{
				SFMLWindow::Get()->SetVSyncEnabled(in_enabled);
			});
		}
		//--------------------------------------------
		//--------------------------------------------
		void PlatformSystem::Quit()
		{
			ChilliSource::Application::Get()->GetTaskScheduler()->ScheduleTask(ChilliSource::TaskType::k_system, [=](const ChilliSource::TaskContext& in_taskContext)
			{
				SFMLWindow::Get()->Quit();
			});
		}
		//--------------------------------------------------
		//--------------------------------------------------
		u64 PlatformSystem::GetSystemTimeMS() const
		{
			return (u64) std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();  
		}
	}
}

#endif
