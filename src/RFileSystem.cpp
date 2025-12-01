
#include <iostream>
#include <filesystem>
#include "Debug.h"
#include "RFileSystem.h"

namespace fs = std::filesystem;

namespace packagemanager
{
    namespace RFileSystem
    {
    bool CheckDirectoryAndFile(const std::string& directoryPath, const std::string& srcPath) 
    {
       std::filesystem::path p(srcPath);
      // Get the filename with extension
       std::string fileName = p.filename().string();
    fs::path dir_path(directoryPath);
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
       // std::cerr << "Error: Directory '" << directoryPath << "' does not exist or is not a directory." << std::endl;
        return false;
    }
    fs::path file_path = dir_path / fileName;
    if (!fs::exists(file_path)) {
        //std::cerr << "Error: File '" << fileName << "' not found in directory '" << directoryPath << "'." << std::endl;
        return false;
    }

    return true;
}
  bool CreateDirectoryr(const std::string& path) 
  {
    std::error_code ec; // Declare an error_code object to store potential errors

    // Attempt to create directories recursively
    std::filesystem::create_directories(path, ec);

    // Check if an error occurred
    if (ec) {
        std::cerr << "Error creating directory '" << path << "': " << ec.message() << std::endl;
        return false; 
    } else {
        // If the directory already exists, create_directories might return false
        // but not set an error_code. We only care if an actual error prevented creation.
        // If the directory exists and no error occurred, we consider it a success.
        std::cout << "Directory '" << path << "' created successfully or already exists." << std::endl;
        return true;
    }
  }
    //Copy file name as <PackageID>.bolt, it will be easier to retrieve 
    bool  CopyFile(const std::string& src, const std::string& dst,const std::string& id) 
    {
        bool result = true;
       std::error_code ec; // Default-constructed error_code indicates no error
       fs::path srcPath(src);
       fs::path dstPath(dst);
       fs::path idPath(id + ".bolt"); //TODO
       fs::path targetFile = dstPath / idPath;
       // Attempt to copy the file, overwriting if it exists
       fs::copy_file(srcPath, targetFile, fs::copy_options::overwrite_existing, ec);
      if(ec)
       {
        std::cerr << "Error Copy file: "  << src<< "Erro:" << ec.message() <<  '\n';
        result = false;
       }
      // ec will contain error information if the copy operation failed
      return result;
   }

  bool FileExist(const std::string& filePath)
  {
    std::filesystem::path fl(filePath);

    // Check if the path exists and refers to a regular file
    return std::filesystem::is_regular_file(fl);
   }
 
    } // namespace Filesystem
} // namespace packagemanager

