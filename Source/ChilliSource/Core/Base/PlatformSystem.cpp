//
//  PlatformSystem.cpp
//  ChilliSource
//
//  Created by Ian Copland on 24/02/2014.
//  Copyright (c) 2014 Tag Games Ltd. All rights reserved.
//

#include <ChilliSource/Core/Base/PlatformSystem.h>

#ifdef CS_TARGETPLATFORM_IOS
#   include <ChilliSource/Backend/Platform/iOS/Core/Base/PlatformSystem.h>
#elif defined CS_TARGETPLATFORM_ANDROID
#   include <ChilliSource/Backend/Platform/Android/Core/Base/PlatformSystem.h>
#elif defined CS_TARGETPLATFORM_WINDOWS
#   include <ChilliSource/Backend/Platform/Windows/Core/Base/PlatformSystem.h>
#endif

namespace ChilliSource
{
    namespace Core
    {
        //-----------------------------------------
        //-----------------------------------------
        PlatformSystemUPtr PlatformSystem::Create()
        {
#ifdef CS_TARGETPLATFORM_IOS
            return PlatformSystemUPtr(new iOS::PlatformSystem());
#elif defined CS_TARGETPLATFORM_ANDROID
            return PlatformSystemUPtr(new Android::PlatformSystem());
#elif defined CS_TARGETPLATFORM_WINDOWS
			return PlatformSystemUPtr(new Windows::PlatformSystem());
#else
            return nullptr;
#endif
        }
    }
}
