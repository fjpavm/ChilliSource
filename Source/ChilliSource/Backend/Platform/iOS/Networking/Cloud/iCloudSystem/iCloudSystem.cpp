    //
//  CiCloudSystem.cpp
//  MMCoHMoFlow
//
//  Created by Hugh McLaughlin on 13/07/2012.
//  Copyright (c) 2012 Tag Games Ltd. All rights reserved.
//

#include <ChilliSource/Backend/Platform/iOS/Networking/Cloud/iCloudSystem/iCloudSystem.h>
#include <ChilliSource/Backend/Platform/iOS/Networking/Cloud/iCloudSystem/ICloudSystemController.h>
#include <ChilliSource/Core/File/FileStream.h>
#include <ChilliSource/Core/Base/Application.h>
#include <sys/stat.h>
#import <UIKit/UIKit.h>
#import <sys/utsname.h>

// The device must be running running iOS 4.1 or later.
NSString *reqSysVer = @"5.0";

std::string EMPTY_STD_STRING = "Empty";

namespace ChilliSource
{
	namespace iOS
    {
        DEFINE_NAMED_INTERFACE(CiCloudSystem);
        
        using namespace ChilliSource::Core;
        
        CiCloudSystem::CiCloudSystem(ChilliSource::Networking::IHttpConnectionSystem* inpcHttpConnectionSystem)
        {
            //You should be checking if IsSupported before creating the system as it is only compatible with >= iOS 5.0 OS's
            if(!IsSupported())
                assert(false);
            
            mpcFileManager = [NSFileManager defaultManager];
            
            //Create all default folders if needed
            mstrICloudDirectoryPath = GetCloudStoragePath();
            
            [CiCloudSystemController sharedInstance];
            if(mstrICloudDirectoryPath != "")
            {
                std::string strPath = GetCloudStoragePath();
                
                for(u32 i=0; i<(u32)CloudStorageArea::k_total; ++i)
                {
                    CreateDirectory(CloudStorageArea((u32)CloudStorageArea::k_sharedDocuments + i), "");
                }
            }
            
            //Initially query for all iCloud files to keep initial cache
            QueryForAllCloudFiles();
            
            mpcHttpConnectionSystem = inpcHttpConnectionSystem;
        }

        //-----------------------------------------------
        /// Is A
        ///
        /// @param Type ID
        /// @return Whether the request is of given type
        //-----------------------------------------------
        bool CiCloudSystem::IsA(Core::InterfaceIDType inInterfaceID) const 
        {
            return inInterfaceID == CiCloudSystem::InterfaceID;
        }
        
        //Returns if the client meets the minimum OS version to use this feature, this should be used to decide to create the system or not
        bool CiCloudSystem::IsSupported()
        {
#if TARGET_IPHONE_SIMULATOR
            CS_ERROR_LOG("CiCloudSystem::IsSupported - System not supported on simulator");
            return false;
#endif
            
            // Check for NSMetadataQuery class
            Class pClass = (NSClassFromString(@"NSMetadataQuery"));
			// Check what OS we are running
            NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
            bool osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
            // If our NSMetadataQuery class is present and we are running
			// at least iOS 5.0, then we are good to go.
            if((pClass != nil) && osVersionSupported)
            {
                CS_DEBUG_LOG("ICloudSupported");
                return true;
            }
            else
            {
                CS_ERROR_LOG("CiCloudSystem::IsSupported - System not supported on current device, needs to be iOS 5.0 at least");
                return false;
            }
        }

        //Returns if iCloud has been enabled on the client device
        bool CiCloudSystem::IsCloudStorageEnabled() const
        {
            NSString* strURLAsString = [[[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil] absoluteString];
            
            if(!strURLAsString || [strURLAsString length] == 0)
            {
                CS_ERROR_LOG("iCloud not enabled on device");
                return false;
            }
            
            NSURL *ubiq = [[NSURL alloc] initWithString:strURLAsString];
            if (ubiq) 
            {
                [ubiq release];
                return true; 
            } 
            else 
            {
                [ubiq release];
                CS_ERROR_LOG("CiCloudSystem::IsCloudStorageEnabled() - ICloud Not Enabled On Device!");
                return false;
            }
            
        }

        //Performs any initialisation required
        void CiCloudSystem::Initialise()
        {
            
        }

        //Manually Refresh the cache of files stored in the cloud
        void CiCloudSystem::RefreshCloudContent()
        {
            QueryForAllCloudFiles();
        }
        
        //If cloud service is enabled returns the path to the storage on device
        std::string CiCloudSystem::GetCloudStoragePath() const
        {        
            if(IsCloudStorageEnabled())
            {
                NSURL *storageURL = [[NSFileManager defaultManager] 
                                        URLForUbiquityContainerIdentifier:nil];
                
                return ChilliSource::Core::CStringUtils::NSStringToString([storageURL absoluteString]);
            }
            else 
            {
                return "";
            }
        }

        void CiCloudSystem::CreateDirectory(CloudStorageArea ineStorageArea, const std::string& instrDir)
        {
            if(mstrICloudDirectoryPath == "")
            {
                mstrICloudDirectoryPath = GetCloudStoragePath();
            }
            
            if(mstrICloudDirectoryPath != "")
            {
                std::string strConstructedPath = mstrICloudDirectoryPath + GetAppendedFilePathToStorageArea(ineStorageArea, instrDir);
                NSURL *ubiq = [[NSURL alloc] initWithString:[[[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil] absoluteString]];
                NSURL *ubiquitousPackage = [[NSURL alloc] initWithString:[[ubiq URLByAppendingPathComponent:(CStringUtils::StringToNSString(GetAppendedFilePathToStorageArea(ineStorageArea, instrDir)))] absoluteString]];

                BOOL* isDir;
                if(![mpcFileManager fileExistsAtPath:[ubiquitousPackage absoluteString] isDirectory:isDir])
                {
                    if(![mpcFileManager createDirectoryAtURL:ubiquitousPackage withIntermediateDirectories:YES attributes:nil error:nil])
                    {
                        CS_ERROR_LOG("CiCloudSystem::CreateDirectoryURL - Failed to create Directory - " + strConstructedPath);
                    }
                }
                else 
                {
                    CS_DEBUG_LOG("Directory already exists");
                }
                
                [ubiq release];
                [ubiquitousPackage release];
            }
            else 
            {
                CS_ERROR_LOG("CiCloudSystem::CreateDirectory - Failed to create Directory - ICloud Not Enabled!");
            }
        }

        //Create file in cloud only, use syncFile to maintain files in both locations
        void CiCloudSystem::CreateFile(CloudStorageArea ineStorageArea, const std::string& instrFileName, const std::string& instrData)
        {
            NSMutableData *data = [[NSMutableData alloc] initWithBytes:instrData.data() length:instrData.length()];
            [[CiCloudSystemController sharedInstance] writeDocumentWithAbsolutePath:ChilliSource::Core::CStringUtils::StringToNSString(GetAppendedFilePathToStorageArea(ineStorageArea, instrFileName))
                                                                                  :data
                                                                                  :nil];
            return;
        }

        void CiCloudSystem::DeleteFile(CloudStorageArea ineStorageArea, const std::string& instrFileName)
        {
            //TODO:: Implement
            CS_ERROR_LOG("CiCloudSystem::DeleteFile:: Not Implemented");
        }
        
        const std::string CiCloudSystem::GetAppendedFilePathToStorageArea(CloudStorageArea ineArea, const std::string& instrRelativeFilePath) const
        {
            return "Documents/" + GetStringForStorageArea(ineArea) + "/" + instrRelativeFilePath;
        }
        
#pragma mark File/Folder Queries
        
        bool CiCloudSystem::SyncFileToCloud(ChilliSource::Core::StorageLocation ineStorageLocation, const std::string& instrFilePath, ICloudStorageSystem::OnSyncFileCompletedDelegate inSyncCompleteDelegate, ICloudStorageSystem::OnSyncConflictDelegate inSyncConflictDelegate)
        {
            if(!IsCloudStorageEnabled())
                return false;
            
            if(mpcHttpConnectionSystem && !mpcHttpConnectionSystem->CheckReachability())
            {
                CS_ERROR_LOG("Cannot sync file to cloud, network unreachable");
                return false;
            }
            
            //Show up a warning message if called from a thread other than main
            if(![NSThread isMainThread])
            {
                CS_WARNING_LOG("CiCloudSystem::SyncFileToCloud -> You are calling this function from inside a thread, objects that rely on autoreleasing in this system will be leaked!");
                CS_WARNING_LOG("You should create an NSAutoReleasePool at the beginning of your thread and call release when its finished (Ingnore this warning if already implemented)");
            }
            
            //First we need to check if this file exists locally
            bool bExists = ChilliSource::Core::CApplication::GetFileSystemPtr()->DoesFileExist(ineStorageLocation, instrFilePath);

            int dwFilePathOffset = instrFilePath.find_last_of("/");
            
            if(dwFilePathOffset > -1)
                CreateDirectory(GetCloudStorageAreaForStorageArea(ineStorageLocation), instrFilePath.substr(0, dwFilePathOffset));
            
            std::string strConstructedPath = GetAppendedFilePathToStorageArea(GetCloudStorageAreaForStorageArea(ineStorageLocation), instrFilePath);
            
            CS_DEBUG_LOG(GetCloudStoragePath() + strConstructedPath);
            
            //We store the query delegates along with the file name
            mmFileToSyncDelegateMap.insert(std::make_pair(GetCloudStoragePath() + strConstructedPath, CloudFileSyncRequest(ineStorageLocation, instrFilePath, inSyncConflictDelegate, inSyncCompleteDelegate)));
            
            //We need to Open the file stored in the cloud or create it if it doesnt exist, NOTE::we only want to create the cloud file if a local file exists and the cloud file doesnt
            [[CiCloudSystemController sharedInstance] openDocument:CStringUtils::StringToNSString(strConstructedPath) :(OnOpenCloudFileCompletedDelegate(this, &CiCloudSystem::OnCloudFileOpened)) :bExists];
            
            return true;
        }
        
        void CiCloudSystem::OnCloudFileOpened(MoFlowUIDocument* incOpenedDoc, BOOL inbJustCreated)
        {
            std::string strFileState = CStringUtils::NSStringToString([CiCloudSystemController stringForState:[incOpenedDoc documentState]]);
            CS_DEBUG_LOG("CiCloudSystem::OnCloudFileOpened with state " + strFileState);
            
            //Should always callback with a MoFloUIDocument - whether valid or not
            if(!incOpenedDoc)
            {
                assert(false);
            }

            //Get the file url string
            std::string mstrFileName = CStringUtils::NSStringToString([[incOpenedDoc fileURL] absoluteString]);
            CS_DEBUG_LOG(mstrFileName);
            
            //Find the delegates this url refares to
            FileToSyncDelegateMap::iterator it;
            it = mmFileToSyncDelegateMap.find(mstrFileName);

            if((it) == mmFileToSyncDelegateMap.end())
                assert(false);
            
            CloudFileSyncRequest psRequest = it->second;
            
            //If the document state is not normal at this stage then we should return and call the delegate as the doc is non-existant or an error has occured
            if ([incOpenedDoc documentState] != UIDocumentStateNormal) 
            {
                //Call the onSync Completed delegate, if any
                if(psRequest.mpcSyncCompletedDelegate)
                    psRequest.mpcSyncCompletedDelegate();
                
                return;
            }
            
            
            //We want to copy our local copies contents straight to the server
            FileStreamPtr pFileStream = CApplication::GetFileSystemPtr()->CreateFileStream(psRequest.meLocalStorageLocation, psRequest.mstrLocalFilePath, ChilliSource::Core::FileMode::k_read);
            
            std::string strLocalContents = "";
            
            if(pFileStream)
                pFileStream->GetAll(strLocalContents);
            
            std::string strCloudContents = [incOpenedDoc contentsAsSTDString];
            
            bool bExists = ChilliSource::Core::CApplication::GetFileSystemPtr()->DoesFileExist(psRequest.meLocalStorageLocation, psRequest.mstrLocalFilePath);
            
            //If no local file we still want to sync if cloud file exists
            if(!bExists)
            {
                //Cloud version exists, local version does not - create local from cloud
                FileStreamPtr pFileStream = CApplication::GetFileSystemPtr()->CreateFileStream(psRequest.meLocalStorageLocation, psRequest.mstrLocalFilePath, ChilliSource::Core::FileMode::k_write);
                pFileStream->Write(strCloudContents);
                pFileStream->Close();
                
                //Call the onSync Completed delegate, if any
                if(psRequest.mpcSyncCompletedDelegate)
                    psRequest.mpcSyncCompletedDelegate();
                
                return;
            }
            
            if(strLocalContents.compare(strCloudContents) == 0)
            {
                CS_DEBUG_LOG("CiCloudSystem::OnCloudFileOpened - File Contents are Equal no Changes needed");
                
                //Call the onSync Completed delegate, if any
                if(psRequest.mpcSyncCompletedDelegate)
                    psRequest.mpcSyncCompletedDelegate();
                
                return;
            }
            
            if(inbJustCreated)
            {
                NSMutableData *data = [[NSMutableData alloc] initWithBytes:strLocalContents.data() length:strLocalContents.length()];
                [[CiCloudSystemController sharedInstance] writeDocumentWithAbsolutePath:[[incOpenedDoc fileURL] absoluteString]
                                                                                      :data
                                                                                      :psRequest.mpcSyncCompletedDelegate];
            }
            else 
            {
                //We need to determine whether the file on the server is newer than the one on local system
                struct timespec localModifiedTime;
                struct stat output;
                
                //get the filepath
                std::string path = CApplication::GetFileSystemPtr()->GetStorageLocationDirectory(it->second.meLocalStorageLocation) + it->second.mstrLocalFilePath;
                
                //Get the stat for the file
                stat(path.c_str(), &output);
                
                //Calculate the last modification time
                localModifiedTime = output.st_mtimespec;
                
                NSDate *pLocalModifiedTime = [NSDate dateWithTimeIntervalSince1970:localModifiedTime.tv_sec];
                NSDate *pCloudLastEdited = [incOpenedDoc fileModificationDate];
                
                //The locally stored version is the latest version
                if([pLocalModifiedTime compare:pCloudLastEdited] == NSOrderedDescending)
                {
                    CS_DEBUG_LOG("CiCloudSystem::OnCloudFileOpened - Local version is the later version, commiting to iCloud!");
                    
                    //Set the file contents to that of our local file
                    NSMutableData *data = [[NSMutableData alloc] initWithBytes:strLocalContents.data() length:strLocalContents.length()];
                    s8* pData = (s8*)[data mutableBytes];
                    std::string strData = std::string(pData, [data length]);
                    CS_DEBUG_LOG(strData);
                    
                    if([data length] > 0)
                    {
                        [[CiCloudSystemController sharedInstance] writeDocumentWithAbsolutePath:[[incOpenedDoc fileURL] absoluteString]
                                                                                              :data
                                                                                              :psRequest.mpcSyncCompletedDelegate];
                    }
                    else
                    {
                        CS_ERROR_LOG("CiCloudSystem::OnCloudFileOpened - Local file is latest version, but has NULL contents, doing nothing ... ");
                        if (psRequest.mpcSyncCompletedDelegate)
                        {
                            psRequest.mpcSyncCompletedDelegate();
                        }
                    }
                    
                }
                else // We have a conflict, the iCloud version is the newer version - if a delegate exists then we call back for them to decide, otherwise do nothing
                {
                    if(psRequest.mpcSyncConflictDelegate)
                    {
                        FileSyncConflict* pConflict = new Networking::ICloudStorageSystem::FileSyncConflict(psRequest.meLocalStorageLocation,
                                                                    psRequest.mstrLocalFilePath,
                                                                    GetCloudStorageAreaForStorageArea(psRequest.meLocalStorageLocation),
                                                                    CStringUtils::NSStringToString([[incOpenedDoc fileURL] absoluteString]),
                                                                    strLocalContents,
                                                                    strCloudContents);
                        
                        mvsCachedConflicts.push_back(pConflict);
                        //Call the delegate
                        psRequest.mpcSyncConflictDelegate(Networking::ICloudStorageSystem::OnConflictResolvedDelegate(this, &CiCloudSystem::OnConflictResolved), pConflict);
                    }
                    else
                    {
                        CS_ERROR_LOG("CiCloudSystem::OnCloudFileOpened - A later version of the file exists on server and no conflict delegate has been provided - doing nothing!");
                    }
                }
            }
        }
        
        void CiCloudSystem::OnConflictResolved(Networking::ICloudStorageSystem::FileConflictChoice ineChoice, Networking::ICloudStorageSystem::FileSyncConflict* insFileSyncConflict, Networking::ICloudStorageSystem::OnSyncFileCompletedDelegate inpSyncCompleteDelegate)
        {
            switch (ineChoice)
            {
                case ICloudStorageSystem::FileConflictChoice::k_copyCloudToLocal:
                {
                    FileStreamPtr pFileStream = CApplication::GetFileSystemPtr()->CreateFileStream(insFileSyncConflict->meLocalFileLocation, insFileSyncConflict->mstrLocalFilePath, ChilliSource::Core::FileMode::k_write);
                    
                    if(pFileStream)
                    {
                        pFileStream->Write(insFileSyncConflict->mstrCloudContents);
                        pFileStream->Close();
                    }
                    
                    std::vector<FileSyncConflict*>::iterator it = mvsCachedConflicts.begin();
                    
                    //Remove from the cached Conflicts
                    for(; it != mvsCachedConflicts.end();)
                    {
                        if((*it) == insFileSyncConflict)
                        {
                            it = mvsCachedConflicts.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                    
                    //Call the complete delegate, if any
                    if(inpSyncCompleteDelegate)
                        inpSyncCompleteDelegate();
                    
                    break;
                }
                case ICloudStorageSystem::FileConflictChoice::k_copyLocalToCloud:
                {
                    //We want to copy our local copies contents straight to the server
                    FileStreamPtr pFileStream = CApplication::GetFileSystemPtr()->CreateFileStream(insFileSyncConflict->meLocalFileLocation, insFileSyncConflict->mstrLocalFilePath, ChilliSource::Core::FileMode::k_read);
                    
                    if(pFileStream)
                    {
                        std::string strLocalContents;
                        pFileStream->GetAll(strLocalContents);
                        
                        std::string strFilePath = insFileSyncConflict->mstrCloudFilePath;
                        NSMutableData *data = [[NSMutableData alloc] initWithBytes:strLocalContents.data() length:strLocalContents.length()];
                        [[CiCloudSystemController sharedInstance] writeDocumentWithAbsolutePath:CStringUtils::StringToNSString(strFilePath)
                                                                                              :data
                                                                                              :inpSyncCompleteDelegate];
                        
                        std::vector<FileSyncConflict*>::iterator it = mvsCachedConflicts.begin();
                        
                        //Remove from the cached Conflicts
                        for(; it != mvsCachedConflicts.end();)
                        {
                            if((*it) == insFileSyncConflict)
                            {
                                it = mvsCachedConflicts.erase(it);
                            }
                            else
                            {
                                ++it;
                            }
                        }
                    }
                    break;
                }
                    
                default:
                    break;
            }
        }
        
        void CiCloudSystem::QueryForAllCloudFiles()
        {
            if(IsCloudStorageEnabled())
                [[CiCloudSystemController sharedInstance] queryContentsOfICloudDirectory:(Networking::ICloudStorageSystem::OnQueryFilesCompletedDelegate(this, &CiCloudSystem::QueryDidFinishGathering))];
        }
        
        //Callback from any query made to retrieve files from cloud
        void CiCloudSystem::QueryDidFinishGathering(Networking::ICloudStorageSystem::ICloudFileList invFileList)
        {
            mvCachedCloudFiles = invFileList;
            
            if(mOnQueryFilesCompletedDelegate)
                mOnQueryFilesCompletedDelegate(invFileList);
        }
    }
}
