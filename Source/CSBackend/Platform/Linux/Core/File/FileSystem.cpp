//
//  FileSystem.cpp
//  Chilli Source
//  Created by Ian Copland on 25/03/2011.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2011 Tag Games Limited
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

#include <CSBackend/Platform/Linux/Core/File/FileSystem.h>

#include <ChilliSource/Core/Base/Utils.h>
#include <ChilliSource/Core/String/StringUtils.h>

#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <stack>
#include <sys/stat.h>
#include <sys/types.h>

namespace CSBackend
{
	namespace Linux 
	{
		namespace
		{
			const std::string k_saveDataPath = "SaveData/";
			const std::string k_cachePath = "Cache/";
			const std::string k_dlcPath = "DLC/";
		
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The file path.
			///
			/// @return Whether or not the given file path exists.
			//--------------------------------------------------------------
			bool DoesFileExist(const std::string& in_filePath)
			{
				std::ifstream f(in_filePath.c_str());
				if (f.good()) {
					f.close();
					return true;
				} 
				else 
				{
        				f.close();
					return false;
				}
			}
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The directory path.
			///
			/// @return Whether or not the given directory path exists.
			//--------------------------------------------------------------
			bool DoesDirectoryExist(const std::string& in_directoryPath)
			{
				struct stat sb;
				if (stat(in_directoryPath.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))
				{
					return true;
				}
				return false;
			}
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The directory path.
			///
			/// @return Whether or not the directory was successfully created.
			//--------------------------------------------------------------
			bool CreateDirectory(const std::string& in_directoryPath)
			{
				return (mkdir(in_directoryPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
			}
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The directory path.
			///
			/// @return Whether or not the directory path was successfully created.
			//--------------------------------------------------------------
			bool CreateDirectoryPath(std::string in_directoryPath)
			{
				// remove last forward slash if it exists so forward slash indicates parent dir
				if(ChilliSource::StringUtils::EndsWith(in_directoryPath, "/"))
				{
					in_directoryPath.pop_back();
				}			
				if (DoesDirectoryExist(in_directoryPath) == false)
				{
					size_t forwardSlashPos = in_directoryPath.rfind("/");
					if(forwardSlashPos != std::string::npos)
					{
						std::string parentDir = in_directoryPath.substr(0, forwardSlashPos);
						if(parentDir != "/" && parentDir != "" &&  DoesDirectoryExist(parentDir) == false)
						{
							CreateDirectoryPath(parentDir);
						}
					}
					if (CreateDirectory(in_directoryPath) == false)
					{
						CS_LOG_ERROR("File System: Failed to create directory '" + in_directoryPath + "'");
						return false;
					}
				}

				return true;
			}
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The directory path.
			///
			/// @return Whether or not the directory was successfully deleted.
			//--------------------------------------------------------------
			bool DeleteDirectory(const std::string& in_directoryPath)
			{
        			DIR *dir = NULL;
        			dir = opendir(in_directoryPath.c_str());
				if(dir == NULL)
				{
					return false;
				}
				else
				{
					struct dirent *entry = NULL;
        				while(entry = readdir(dir))
					{   
						if(std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
						{   
                        				std::string subPath = in_directoryPath + "/" + entry->d_name;
							if(DoesDirectoryExist(subPath))
							{   
								DeleteDirectory(subPath);
							}   
							else 
							{   
								if(DoesFileExist(subPath))
								{   
									remove(subPath.c_str());
								}   
							}   
						}   
					} 
				}  
        		if(remove(in_directoryPath.c_str()) != 0)
				{
					return false;
				}
				return true;
			}
			//--------------------------------------------------------------
			/// @author Francisco Mendonca
			///
			/// @param The source file path.
			/// @param The destination file path.
			///
			/// @return Whether or not the directory was successfully deleted.
			//--------------------------------------------------------------
			bool CopyFile(std::string in_sourcePath, std::string in_destPath)
			{
				FILE *fpSource, *fpDest;
				if((fpSource = fopen(in_sourcePath.c_str(), "rb")) == NULL)
				{
					return false;
				}
				if((fpDest = fopen(in_destPath.c_str(),"wb")) == NULL)
				{
					fclose(fpSource);
					return false;
				}
				char buffer[BUFSIZ];
				size_t size;
				while((size = fread(buffer, 1, BUFSIZ, fpSource)) != 0)
				{
					size_t sizeWritten;
					if(fwrite(buffer , 1, size, fpDest) != size)
					{
						break;
					}
				}
				bool success = !ferror(fpSource) && !ferror(fpDest);
 				fclose(fpSource);
				bool closeDestOk = (fclose(fpDest) == 0);
				success = success && closeDestOk;
				return success;
			}
			//--------------------------------------------------------------
			/// Lists all files and sub-directories inside the given directory.
			/// All paths will be relative to the given directory.
			///
			/// @author Francisco Mendonca
			///
			/// @param The directory.
			/// @param Whether or not to recurse into sub directories.
			/// @param [Out] The sub directories.
			/// @param [Out] The files.
			/// @param [Optional] The relative directory path. This is used
			/// in recursion and shouldn't be set outside of this function.
			/// @return Whether or not this succeeded.
			//--------------------------------------------------------------
			bool ListDirectoryContents(const std::string& in_directoryPath, bool in_recursive, std::vector<std::string>& out_directoryPaths, std::vector<std::string>& out_filePaths, 
				const std::string& in_relativeDirectoryPath = "")
			{
				DIR *dir = NULL;
        		dir = opendir(in_directoryPath.c_str());
				if(dir == NULL)
				{
					return false;
				}

				struct dirent *entry = NULL;
        			while(entry = readdir(dir))
				{ 
					if(std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
					{
						std::string subPath = ChilliSource::StringUtils::StandardiseDirectoryPath(in_directoryPath) + entry->d_name;
						if(CSBackend::Linux::DoesDirectoryExist(subPath))
						{
							std::string directoryName = entry->d_name;
							std::string relativeDirectoryPath = ChilliSource::StringUtils::StandardiseDirectoryPath(in_relativeDirectoryPath + directoryName);
							out_directoryPaths.push_back(relativeDirectoryPath);

							if (in_recursive == true)
							{
								std::string absoluteDirectoryPath = ChilliSource::StringUtils::StandardiseDirectoryPath(subPath);
								if (ListDirectoryContents(absoluteDirectoryPath, true, out_directoryPaths, out_filePaths, relativeDirectoryPath) == false)
								{
									return false;
								}
							}
						}
						else
						{
							std::string fileName = entry->d_name;
							std::string relativeFilePath = ChilliSource::StringUtils::StandardiseFilePath(in_relativeDirectoryPath + fileName);
							out_filePaths.push_back(relativeFilePath);
						}
					}
				} 

				return true;
			}
		}

		CS_DEFINE_NAMEDTYPE(FileSystem);
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		FileSystem::FileSystem()
		{
			std::string strWorkingDir = "./";

			m_packagePath = strWorkingDir + "assets/";
			m_documentsPath = strWorkingDir + "Documents/";

			CSBackend::Linux::CreateDirectoryPath(m_documentsPath);
			CS_ASSERT(CSBackend::Linux::DoesDirectoryExist(m_documentsPath), "Could not create Documents directory.");

			CSBackend::Linux::CreateDirectoryPath(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_saveData));
			CS_ASSERT(CSBackend::Linux::DoesDirectoryExist(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_saveData)), "Could not create SaveData storage location.");

			CSBackend::Linux::CreateDirectoryPath(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_cache));
			CS_ASSERT(CSBackend::Linux::DoesDirectoryExist(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_cache)), "Could not create Cache storage location.");

			CSBackend::Linux::CreateDirectoryPath(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_DLC));
			CS_ASSERT(CSBackend::Linux::DoesDirectoryExist(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_DLC)), "Could not create DLC storage location.");
		}
		//----------------------------------------------------------
		//----------------------------------------------------------
		bool FileSystem::IsA(ChilliSource::InterfaceIDType in_interfaceId) const
		{
			return (ChilliSource::FileSystem::InterfaceID == in_interfaceId || FileSystem::InterfaceID == in_interfaceId);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		ChilliSource::ITextInputStreamUPtr FileSystem::CreateTextInputStream(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath) const
        {
        	std::string absFilePath = "";
			if (in_storageLocation == ChilliSource::StorageLocation::k_DLC && DoesFileExistInCachedDLC(in_filePath) == false)
			{
				absFilePath = GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_package) + GetPackageDLCPath() + in_filePath;
			}
			else
			{
				absFilePath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath;
			}

			ChilliSource::ITextInputStreamUPtr output(new ChilliSource::TextInputStream(absFilePath));
            if (output->IsValid() == true)
			{
                return output;
            }
            else
            {
                return nullptr;
            }
        }
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		ChilliSource::IBinaryInputStreamUPtr FileSystem::CreateBinaryInputStream(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath) const
		{
			std::string absFilePath = "";
			if (in_storageLocation == ChilliSource::StorageLocation::k_DLC && DoesFileExistInCachedDLC(in_filePath) == false)
			{
				absFilePath = GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_package) + GetPackageDLCPath() + in_filePath;
			}
			else
			{
				absFilePath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath;
				}

			ChilliSource::IBinaryInputStreamUPtr output(new ChilliSource::BinaryInputStream(absFilePath));
			if (output->IsValid() == true)
			{
				return output;
			}
			else
			{
				return nullptr;
			}
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		ChilliSource::TextOutputStreamUPtr FileSystem::CreateTextOutputStream(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath, ChilliSource::FileWriteMode in_fileMode) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_storageLocation), "File System: Trying to write to read only storage location.");

			if (IsStorageLocationWritable(in_storageLocation))
			{
				std::string absFilePath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath;

				ChilliSource::TextOutputStreamUPtr output(new ChilliSource::TextOutputStream(absFilePath, in_fileMode));
				if (output->IsValid() == true)
				{
					return output;
				}
			}

			return nullptr;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		ChilliSource::BinaryOutputStreamUPtr FileSystem::CreateBinaryOutputStream(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath, ChilliSource::FileWriteMode in_fileMode) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_storageLocation), "File System: Trying to write to read only storage location.");

			if (IsStorageLocationWritable(in_storageLocation))
			{
				std::string absFilePath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath;

				ChilliSource::BinaryOutputStreamUPtr output(new ChilliSource::BinaryOutputStream(absFilePath, in_fileMode));
				if (output->IsValid() == true)
				{
					return output;
				}
			}

			return nullptr;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::CreateDirectoryPath(ChilliSource::StorageLocation in_storageLocation, const std::string& in_directoryPath) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_storageLocation), "File System: Trying to write to read only storage location.");

			std::string directoryPath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_directoryPath;
			return CSBackend::Linux::CreateDirectoryPath(directoryPath);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::CopyFile(ChilliSource::StorageLocation in_sourceStorageLocation, const std::string& in_sourceFilePath, 
			ChilliSource::StorageLocation in_destinationStorageLocation, const std::string& in_destinationFilePath) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_destinationStorageLocation), "File System: Trying to write to read only storage location.");
            
			std::string sourceFilePath;
			if (in_sourceStorageLocation == ChilliSource::StorageLocation::k_DLC && DoesFileExistInCachedDLC(in_sourceFilePath) == false)
			{
				sourceFilePath = GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_package) + GetPackageDLCPath() + ChilliSource::StringUtils::StandardiseFilePath(in_sourceFilePath);
			}
			else
			{
				sourceFilePath = GetAbsolutePathToStorageLocation(in_sourceStorageLocation) + ChilliSource::StringUtils::StandardiseFilePath(in_sourceFilePath);
			}

			if (CSBackend::Linux::DoesFileExist(sourceFilePath) == false)
			{
				CS_LOG_ERROR("File System: Trying to copy file '" + in_sourceFilePath + "' but it does not exist.");
				return false;
			}

			//get the path to the file
			std::string destinationFileName, destinationDirectoryPath;
			ChilliSource::StringUtils::SplitFilename(in_destinationFilePath, destinationFileName, destinationDirectoryPath);
            
			//create the output directory
			CreateDirectoryPath(in_destinationStorageLocation, destinationDirectoryPath);
            
			std::string destFilePath = GetAbsolutePathToStorageLocation(in_destinationStorageLocation) + in_destinationFilePath;
            //try and copy the files
			if (CSBackend::Linux::CopyFile(sourceFilePath, destFilePath) == false)
			{
				CS_LOG_ERROR("File System: Failed to copy file '" + in_sourceFilePath + "'");
				return false;
			}

			return true;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::CopyDirectory(ChilliSource::StorageLocation in_sourceStorageLocation, const std::string& in_sourceDirectoryPath, 
			ChilliSource::StorageLocation in_destinationStorageLocation, const std::string& in_destinationDirectoryPath) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_destinationStorageLocation), "File System: Trying to write to read only storage location.");

			if (DoesDirectoryExist(in_sourceStorageLocation, in_sourceDirectoryPath) == false)
			{
				CS_LOG_ERROR("File System: Trying to copy directory '" + in_sourceDirectoryPath + "' but it doesn't exist.");
				return false;
			}

			std::vector<std::string> filePaths = GetFilePaths(in_sourceStorageLocation, in_sourceDirectoryPath, true);

			//if the source directory is empty, just create the equivelent directory in the destination
			if (filePaths.size() == 0)
			{
				CreateDirectoryPath(in_destinationStorageLocation, in_destinationDirectoryPath);
			}
			else
			{
				std::string sourceDirectoryPath = ChilliSource::StringUtils::StandardiseDirectoryPath(in_sourceDirectoryPath);
				std::string destinationDirectoryPath = ChilliSource::StringUtils::StandardiseDirectoryPath(in_destinationDirectoryPath);
				for (const std::string& filePath : filePaths)
				{
					if (CopyFile(in_sourceStorageLocation, sourceDirectoryPath + filePath, in_destinationStorageLocation, destinationDirectoryPath + filePath) == false)
					{
						CS_LOG_ERROR("File System: Failed to copy directory '" + in_sourceDirectoryPath + "'");
						return false;
					}
				}
			}

			return true;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DeleteFile(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_storageLocation), "File System: Trying to delete from a read only storage location.");

			std::string filePath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath;
			if (remove(filePath.c_str()) != 0)
			{
				CS_LOG_ERROR("File System: Failed to delete file '" + in_filePath + "'");
				return false;
			}

			return true;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DeleteDirectory(ChilliSource::StorageLocation in_storageLocation, const std::string& in_directoryPath) const
		{
			CS_ASSERT(IsStorageLocationWritable(in_storageLocation), "File System: Trying to delete from a read only storage location.");

			std::string directoryPath = GetAbsolutePathToStorageLocation(in_storageLocation) + in_directoryPath;
			if (CSBackend::Linux::DeleteDirectory(directoryPath) == false)
			{
				CS_LOG_ERROR("File System: Failed to delete directory '" + in_directoryPath + "'");
				return false;
			}

			return true;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		std::vector<std::string> FileSystem::GetFilePaths(ChilliSource::StorageLocation in_storageLocation, const std::string& in_directoryPath, bool in_recursive) const
		{
			std::vector<std::string> possibleDirectories = GetPossibleAbsoluteDirectoryPaths(in_storageLocation, in_directoryPath);
            
			std::vector<std::string> output;
			std::vector<std::string> filePaths;
			std::vector<std::string> directoryPaths;
			for (const std::string& possibleDirectory : possibleDirectories)
			{
				filePaths.clear();
				directoryPaths.clear();

				ListDirectoryContents(possibleDirectory, in_recursive, directoryPaths, filePaths);
				output.insert(output.end(), filePaths.begin(), filePaths.end());
			}

			std::sort(output.begin(), output.end());
			std::vector<std::string>::iterator it = std::unique(output.begin(), output.end());
			output.resize(it - output.begin());
			return output;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		std::vector<std::string> FileSystem::GetDirectoryPaths(ChilliSource::StorageLocation in_storageLocation, const std::string& in_directoryPath, bool in_recursive) const
		{
			std::vector<std::string> possibleDirectories = GetPossibleAbsoluteDirectoryPaths(in_storageLocation, in_directoryPath);

			std::vector<std::string> output;
			std::vector<std::string> filePaths;
			std::vector<std::string> directoryPaths;
			for (const std::string& possibleDirectory : possibleDirectories)
			{
				filePaths.clear();
				directoryPaths.clear();

				ListDirectoryContents(possibleDirectory, in_recursive, directoryPaths, filePaths);
				output.insert(output.end(), directoryPaths.begin(), directoryPaths.end());
			}

			std::sort(output.begin(), output.end());
			std::vector<std::string>::iterator it = std::unique(output.begin(), output.end());
			output.resize(it - output.begin());
			return output;
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesFileExist(ChilliSource::StorageLocation in_storageLocation, const std::string& in_filePath) const
		{
			switch (in_storageLocation)
			{
				case ChilliSource::StorageLocation::k_DLC:
				{
					if (DoesItemExistInDLCCache(in_filePath, false) == true)
					{
						return true;
					}

					return DoesFileExist(ChilliSource::StorageLocation::k_package, GetPackageDLCPath() + in_filePath);
				}
				default:
				{
					std::string path = ChilliSource::StringUtils::StandardiseFilePath(GetAbsolutePathToStorageLocation(in_storageLocation) + in_filePath);
					return CSBackend::Linux::DoesFileExist(path);
				}
			}
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesFileExistInCachedDLC(const std::string& in_filePath) const
		{
			return DoesItemExistInDLCCache(in_filePath, false);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesFileExistInPackageDLC(const std::string& in_filePath) const
		{
			return DoesFileExist(ChilliSource::StorageLocation::k_package, GetPackageDLCPath() + in_filePath);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesDirectoryExist(ChilliSource::StorageLocation in_storageLocation, const std::string& in_directoryPath) const
		{
			switch (in_storageLocation)
			{
				case ChilliSource::StorageLocation::k_DLC:
				{
					if (DoesItemExistInDLCCache(in_directoryPath, true) == true)
					{
						return true;
					}

					return DoesDirectoryExist(ChilliSource::StorageLocation::k_package, GetPackageDLCPath() + in_directoryPath);
				}
				default:
				{
					std::string path = ChilliSource::StringUtils::StandardiseDirectoryPath(GetAbsolutePathToStorageLocation(in_storageLocation) + in_directoryPath);
					return CSBackend::Linux::DoesDirectoryExist(path);
				}
			}
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesDirectoryExistInCachedDLC(const std::string& in_directoryPath) const
		{
			return DoesItemExistInDLCCache(in_directoryPath, true);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesDirectoryExistInPackageDLC(const std::string& in_directoryPath) const
		{
			return DoesDirectoryExist(ChilliSource::StorageLocation::k_package, GetPackageDLCPath() + in_directoryPath);
		}
		//--------------------------------------------------------------
		//--------------------------------------------------------------
		std::string FileSystem::GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation in_storageLocation) const
		{
			switch (in_storageLocation)
			{
			case ChilliSource::StorageLocation::k_package:
				return m_packagePath + "AppResources/";
			case ChilliSource::StorageLocation::k_chilliSource:
				return m_packagePath + "CSResources/";
			case ChilliSource::StorageLocation::k_saveData:
				return m_documentsPath + k_saveDataPath;
			case ChilliSource::StorageLocation::k_cache:
				return m_documentsPath + k_cachePath;
			case ChilliSource::StorageLocation::k_DLC:
				return m_documentsPath + k_dlcPath;
			case ChilliSource::StorageLocation::k_root:
				return "";
				break;
			default:
				CS_LOG_ERROR("Storage Location not available on this platform!");
				return "";
			}
		}
				//--------------------------------------------------------------
		//--------------------------------------------------------------
		bool FileSystem::DoesItemExistInDLCCache(const std::string& in_path, bool in_isDirectory) const
		{
			std::string path = GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_DLC) + in_path;
			if (in_isDirectory == true)
			{
				return CSBackend::Linux::DoesDirectoryExist(ChilliSource::StringUtils::StandardiseDirectoryPath(path));
			}
			else
			{
				return CSBackend::Linux::DoesFileExist(ChilliSource::StringUtils::StandardiseFilePath(path));
			}
		}
		//------------------------------------------------------------
		//------------------------------------------------------------
		std::vector<std::string> FileSystem::GetPossibleAbsoluteDirectoryPaths(ChilliSource::StorageLocation in_storageLocation, const std::string& in_path) const
		{
			std::vector<std::string> output;

			switch (in_storageLocation)
			{
				case ChilliSource::StorageLocation::k_DLC:
				{
					output.push_back(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_package) + GetPackageDLCPath() + in_path);
					output.push_back(GetAbsolutePathToStorageLocation(ChilliSource::StorageLocation::k_DLC) + in_path);
					break;
				}
				default:
				{
					output.push_back(GetAbsolutePathToStorageLocation(in_storageLocation) + in_path);
					break;
				}
			}

			return output;
		}
	}
}

#endif
