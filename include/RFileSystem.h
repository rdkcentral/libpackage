

#include <string>
namespace packagemanager
{
    namespace RFileSystem
    {
      bool CreateDirectoryr(const std::string& path) ;
      bool CheckDirectoryAndFile(const std::string& directoryPath, const std::string& srcPath);
      bool CopyFile(const std::string& src, const std::string& dst,const std::string& id) ;
     bool FileExist(const std::string& filePath);
    } // namespace RFileSystem
} // namespace packagemanager
