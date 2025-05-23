#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <array>
#include <filesystem>
using namespace std;

namespace fs = filesystem;

// Check if file exists
inline bool exists(const string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// Base pipe class
class tpmClass  {
protected:
    array<int, 2> parentToChild; // Parent writes, child reads
    array<int, 2> childToParent; // Child writes, parent reads

public:
    tpmClass()
    {
        if (pipe(parentToChild.data()) < 0 || pipe(childToParent.data()) < 0)
        {
            cerr << "Can't create pipe!\n";
            exit(-1);
        }
    }

    ~tpmClass()
    {
        close(parentToChild[0]);
        close(parentToChild[1]);
        close(childToParent[0]);
        close(childToParent[1]);
    }

    // For child to read from parent
    string readFromParent()
    {
        array<char, 1024> buff{};
        size_t bytes = read(parentToChild[0], buff.data(), buff.size());
        return (bytes ? string(buff.data(), bytes) : "");
    }

    // For parent to write to child
    void writeToChild(const string &msg)
    {
        write(parentToChild[1], msg.data(), msg.size());
    }

    // For parent to read from child
    string readFromChild()
    {
        array<char, 1024> buff{};
        size_t bytes = read(childToParent[0], buff.data(), buff.size());
        return (bytes ? string(buff.data(), bytes) : "");
    }

    // For child to write to parent
    void writeToParent(const string &msg)
    {
        write(childToParent[1], msg.data(), msg.size());
    }

    // Close unused pipe ends based on process type
    void setupForChild()
    {
        close(parentToChild[1]); // Child doesn't write to parentToChild
        close(childToParent[0]); // Child doesn't read from childToParent
    }

    void setupForParent()
    {
        close(parentToChild[0]); // Parent doesn't read from parentToChild
        close(childToParent[1]); // Parent doesn't write to childToParent
    }
};

// myPipe class
class myPipe : public tpmClass {
private:
    size_t bufferSize;
    char *buffer;

public:
    myPipe(size_t bufferSize = 1024) // Default buffer size in bytes
    {
        this->bufferSize = bufferSize;
        this->buffer = new char[bufferSize];
    }

    // Destructor to free allocated buffer
    ~myPipe()
    {
        delete[] buffer;
    }

    // Copy constructor
    myPipe(const myPipe &other)
    {
        bufferSize = other.bufferSize;
        buffer = new char[bufferSize];
        memcpy(buffer, other.buffer, bufferSize);
    }

    // Assignment operator
    myPipe &operator=(const myPipe &other)
    {
        if (this != &other)
        {
            delete[] buffer;
            bufferSize = other.bufferSize;
            buffer = new char[bufferSize];
            memcpy(buffer, other.buffer, bufferSize);
        }
        return *this;
    }

    // Set buffer size
    void setBufferSize(size_t size)
    {
        delete[] buffer;
        bufferSize = size;
        buffer = new char[bufferSize];
    }

    // Get buffer size
    size_t getBufferSize() const
    {
        return bufferSize;
    }

    // Read from console with buffer
    string readFromConsole()
    {
        string input;
        char *tempBuffer = new char[bufferSize + 1]; // +1 for null terminator

        cin.getline(tempBuffer, bufferSize);
        input = tempBuffer;

        delete[] tempBuffer;
        return input;
    }

    // Write to console with buffer
    void writeToConsole(const string &data)
    {
        size_t remaining = data.length();
        size_t position = 0;

        while (remaining > 0)
        {
            size_t chunk = (remaining > bufferSize) ? bufferSize : remaining;
            memcpy(buffer, data.c_str() + position, chunk);

            // Ensure null termination for output
            if (chunk < bufferSize)
                buffer[chunk] = '\0';

            cout.write(buffer, chunk);

            position += chunk;
            remaining -= chunk;
        }
        cout << endl;
    }

    // Read from file with buffer
    string readFromFile(const string &filename)
    {
        ifstream file(filename, ios::binary);
        if (!file.is_open())
            return "File could not be opened.";

        string content;
        while (file)
        {
            file.read(buffer, bufferSize);
            content.append(buffer, file.gcount());
        }

        return content;
    }

    // Write to file with buffer
    void writeToFile(const string &filename, const string &data)
    {
        if (!exists(filename))
        {
            // File doesn't exist, create with specific permissions
            int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0640);
            if (fd == -1)
            {
                perror("open");
                return;
            }

            size_t remaining = data.length();
            size_t position = 0;

            while (remaining > 0)
            {
                size_t chunk = (remaining > bufferSize) ? bufferSize : remaining;
                memcpy(buffer, data.c_str() + position, chunk);
                write(fd, buffer, chunk);

                position += chunk;
                remaining -= chunk;
            }

            close(fd);
        }
        else
        {
            // File exists, use standard ofstream with buffer
            ofstream file(filename, ios::binary);

            size_t remaining = data.length();
            size_t position = 0;

            while (remaining > 0)
            {
                size_t chunk = (remaining > bufferSize) ? bufferSize : remaining;
                memcpy(buffer, data.c_str() + position, chunk);
                file.write(buffer, chunk);

                position += chunk;
                remaining -= chunk;
            }
        }
    }
};

// Show directory contents
void listDir(const string &path, bool showHidden, bool longList)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        cout << "Cannot open directory\n";
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        string name = entry->d_name;
        if (!showHidden && name[0] == '.')
            continue;

        if (longList)
        {
            struct stat st{};
            stat((path + "/" + name).c_str(), &st);
            cout << ((S_ISDIR(st.st_mode)) ? "d" : "-");
            cout << ((st.st_mode & S_IRUSR) ? "r" : "-");
            cout << ((st.st_mode & S_IWUSR) ? "w" : "-");
            cout << ((st.st_mode & S_IXUSR) ? "x" : "-");
            cout << ((st.st_mode & S_IRGRP) ? "r" : "-");
            cout << ((st.st_mode & S_IWGRP) ? "w" : "-");
            cout << ((st.st_mode & S_IXGRP) ? "x" : "-");
            cout << ((st.st_mode & S_IROTH) ? "r" : "-");
            cout << ((st.st_mode & S_IWOTH) ? "w" : "-");
            cout << ((st.st_mode & S_IXOTH) ? "x" : "-");
            cout << " " << st.st_size << " " << name << endl;
        }
        else
        {
            cout << name << "  ";
        }
    }

    if (!longList)
        cout << endl;

    closedir(dir);
}

// Create directories hierarchy
void createDirHierarchy(const string &path)
{
    error_code ec;
    if (fs::create_directories(path, ec))
    {
        cout << "Directory hierarchy created.\n";
    }
    else
    {
        cerr << "Error creating directory hierarchy: " << ec.message() << endl;
    }
}

// Change file permissions
void changeFilePermissions(const string &filename, const string &mode)
{
    if (!exists(filename))
    {
        cout << "File does not exist.\n";
        return;
    }

    struct stat st{};
    stat(filename.c_str(), &st);
    mode_t newMode = st.st_mode;

    if (mode == "+x")
    {
        newMode |= S_IXUSR;
    }
    else if (mode == "-w")
    {
        newMode &= ~S_IWGRP;
    }
    else if (mode == "-rw")
    {
        newMode &= ~(S_IROTH | S_IWOTH);
    }

    if (chmod(filename.c_str(), newMode) == 0)
    {
        cout << "Permissions changed successfully.\n";
    }
    else
    {
        perror("chmod");
    }
}

// Change owner/group
void changeOwnership(const string &filename, bool isGroup)
{
    if (!exists(filename))
    {
        cout << "File does not exist.\n";
        return;
    }

    string name;
    cout << "Enter " << (isGroup ? "group" : "owner") << " ID: ";
    getline(cin, name);

    try
    {
        int id = stoi(name);
        struct stat st{};

        if (stat(filename.c_str(), &st) != 0)
        {
            perror("stat");
            return;
        }

        if (isGroup)
        {
            // For group change, use chown with existing user ID and new group ID
            if (chown(filename.c_str(), st.st_uid, id) != 0)
            {
                perror("chown (group change)");
            }
            else
            {
                cout << "Group changed successfully.\n";
            }
        }
        else
        {
            // For owner change, use existing group ID
            if (chown(filename.c_str(), id, st.st_gid) != 0)
            {
                perror("chown");
            }
            else
            {
                cout << "Owner changed successfully.\n";
            }
        }
    }
    catch (const invalid_argument &e)
    {
        cout << "Invalid ID. Please enter a numeric ID.\n";
    }
}

// Copy file
void copyFile(const string &source, const string &destination)
{
    if (!exists(source))
    {
        cout << "Source file does not exist.\n";
        return;
    }

    ifstream src(source, ios::binary);
    ofstream dst(destination, ios::binary);

    if (!src.is_open() || !dst.is_open())
    {
        cout << "Error opening files.\n";
        return;
    }

    dst << src.rdbuf();
    cout << "File copied successfully.\n";
}

// Move/rename file
void moveFile(const string &source, const string &destination)
{
    if (!exists(source))
    {
        cout << "Source file does not exist.\n";
        return;
    }

    if (rename(source.c_str(), destination.c_str()) != 0)
    {
        perror("move/rename");
    }
    else
    {
        cout << "File moved/renamed successfully.\n";
    }
}

// Remove file
void removeFile(const string &filename)
{
    if (!exists(filename))
    {
        cout << "File does not exist.\n";
        return;
    }

    if (remove(filename.c_str()) != 0)
    {
        perror("remove");
    }
    else
    {
        cout << "File removed successfully.\n";
    }
}

// Remove directory
void removeDir(const string &dirname)
{
    if (!exists(dirname))
    {
        cout << "Directory does not exist.\n";
        return;
    }

    if (rmdir(dirname.c_str()) != 0)
    {
        perror("rmdir");
    }
    else
    {
        cout << "Directory removed successfully.\n";
    }
}

// Change directory with number navigation
void changeDirectory(fs::path &currentDir, const fs::path &rootDir)
{
    vector<fs::directory_entry> entries;
    int i = 1;

    cout << "\nAvailable directories:\n";
    for (auto &entry : fs::directory_iterator(currentDir))
    {
        if (entry.is_directory())
        {
            cout << i << ". " << entry.path().filename().string() << endl;
            entries.push_back(entry);
            i++;
        }
    }

    cout << "0. .. (go up)\nChoose directory number: ";
    int choice;
    cin >> choice;
    cin.ignore();

    if (choice == 0)
    {
        // Go up one level, but don't go beyond root
        if (currentDir != rootDir)
        {
            currentDir = currentDir.parent_path();
        }
        else
        {
            cout << "Already at root directory.\n";
        }
    }
    else if (choice > 0 && choice <= static_cast<int>(entries.size()))
    {
        currentDir = entries[choice - 1].path();
    }
    else
    {
        cout << "Invalid choice.\n";
    }
}

// Process commands
void processCommand(const string &cmd, fs::path &currentDir, const fs::path &rootDir, myPipe &pipe)
{
    if (cmd == "pwd")
    {
        cout << "Current directory: " << currentDir << endl;
    }
    else if (cmd == "ls")
    {
        listDir(currentDir, false, false);
    }
    else if (cmd == "ls -a")
    {
        listDir(currentDir, true, false);
    }
    else if (cmd == "ls -l")
    {
        listDir(currentDir, false, true);
    }
    else if (cmd == "ls -al" || cmd == "ls -la")
    {
        listDir(currentDir, true, true);
    }
    else if (cmd == "cd")
    {
        changeDirectory(currentDir, rootDir);
    }
    else if (cmd == "mkdir")
    {
        string name;
        cout << "Directory name: ";
        getline(cin, name);

        fs::path newDir = currentDir / name;
        if (mkdir(newDir.c_str(), 0755) == 0)
        {
            cout << "Directory created.\n";
        }
        else
        {
            perror("mkdir");
        }
    }
    else if (cmd == "mkdir -p")
    {
        string path;
        cout << "Directory hierarchy (e.g., dir1/dir2/dir3): ";
        getline(cin, path);

        createDirHierarchy((currentDir / path).string());
    }
    else if (cmd == "touch")
    {
        string name;
        cout << "File name: ";
        getline(cin, name);

        ofstream file((currentDir / name).string());
        if (file.is_open())
        {
            cout << "File created.\n";
        }
        else
        {
            cout << "Error creating file.\n";
        }
    }
    else if (cmd == "chmod +x")
    {
        string filename;
        cout << "File name: ";
        getline(cin, filename);

        changeFilePermissions((currentDir / filename).string(), "+x");
    }
    else if (cmd == "chmod -w")
    {
        string filename;
        cout << "File name: ";
        getline(cin, filename);

        changeFilePermissions((currentDir / filename).string(), "-w");
    }
    else if (cmd == "chmod -rw")
    {
        string filename;
        cout << "File name: ";
        getline(cin, filename);

        changeFilePermissions((currentDir / filename).string(), "-rw");
    }
    else if (cmd == "chgrp")
    {
        string filename;
        cout << "File name: ";
        getline(cin, filename);

        changeOwnership((currentDir / filename).string(), true);
    }
    else if (cmd == "chown")
    {
        string filename;
        cout << "File name: ";
        getline(cin, filename);

        changeOwnership((currentDir / filename).string(), false);
    }
    else if (cmd == "cp")
    {
        string source, dest;
        cout << "Source file: ";
        getline(cin, source);
        cout << "Destination file: ";
        getline(cin, dest);

        copyFile((currentDir / source).string(), (currentDir / dest).string());
    }
    else if (cmd == "mv")
    {
        cout << "1. Rename file\n2. Move file\nChoose option: ";
        int choice;
        cin >> choice;
        cin.ignore();

        string source, dest;
        cout << "Source file: ";
        getline(cin, source);
        cout << "Destination " << (choice == 1 ? "name" : "path") << ": ";
        getline(cin, dest);

        if (choice == 1)
        {
            moveFile((currentDir / source).string(), (currentDir / dest).string());
        }
        else if (choice == 2)
        {
            string destPath = dest;
            if (dest[0] != '/')
            {
                destPath = (currentDir / dest).string();
            }
            
            moveFile((currentDir / source).string(), destPath + "/" + source);
        }
    }
    else if (cmd == "rm")
    {
        string filename;
        cout << "File to remove: ";
        getline(cin, filename);

        removeFile((currentDir / filename).string());
    }
    else if (cmd == "rmdir")
    {
        string dirname;
        cout << "Directory to remove: ";
        getline(cin, dirname);

        removeDir((currentDir / dirname).string());
    }
    else if (cmd == "read_console")
    {
        cout << "Enter text to read from console: ";
        string input = pipe.readFromConsole();
        cout << "Text read from console: " << input << endl;
    }
    else if (cmd == "write_console")
    {
        cout << "Enter text to write to console: ";
        string text = pipe.readFromConsole();
        cout << "Writing to console: ";
        pipe.writeToConsole(text);
    }
    else if (cmd == "read_file")
    {
        string filename;
        cout << "Enter filename to read: ";
        getline(cin, filename);

        string content = pipe.readFromFile((currentDir / filename).string());
        cout << "File content:\n" << content << endl;
    }
    else if (cmd == "write_file")
    {
        string filename;
        cout << "Enter filename to write to: ";
        getline(cin, filename);

        cout << "Enter content to write: ";
        string content = pipe.readFromConsole();

        pipe.writeToFile((currentDir / filename).string(), content);
        cout << "Content written to file." << endl;
    }
    else
    {
        cout << "Unknown command: " << cmd << endl;
    }
}

int main()
{
    myPipe pipe;
    fs::path rootDir = getenv("HOME"); // $HOME = your_root_dir
    fs::path currentDir = rootDir;

    pid_t pid = fork();

    if (pid < 0)
    {
        cerr << "Fork failed.\n";
        exit(1);
    }
    else if (pid == 0)
    {
        // Child process - handles command execution
        pipe.setupForChild();

        while (true)
        {
            string cmd = pipe.readFromParent();
            if (cmd == "exit")
                break;

            processCommand(cmd, currentDir, rootDir, pipe);

            // Signal to parent that we're done with this command
            pipe.writeToParent("DONE");
        }
    }
    else
    {
        // Parent process - handles user interface
        pipe.setupForParent();

        while (true)
        {
            system("clear");

            cout << "============== FILE MANAGER ==============\n";
            cout << "1. View Operations:\n";
            cout << "   1. Show current directory (pwd)\n";
            cout << "   2. List files (ls)\n";
            cout << "   3. List all files (ls -a)\n";
            cout << "   4. List files with details (ls -l)\n";
            cout << "   5. List all files with details (ls -al)\n";
            cout << "   6. Change directory (cd)\n";
            cout << "2. Create Operations:\n";
            cout << "   7. Create directory (mkdir)\n";
            cout << "   8. Create directory hierarchy (mkdir -p)\n";
            cout << "   9. Create file (touch)\n";
            cout << "3. Modify Operations:\n";
            cout << "   10. Change file permissions (chmod +x)\n";
            cout << "   11. Change group permissions (chmod -w)\n";
            cout << "   12. Change other permissions (chmod -rw)\n";
            cout << "   13. Change group ownership (chgrp)\n";
            cout << "   14. Change file ownership (chown)\n";
            cout << "4. File Operations:\n";
            cout << "   15. Copy file (cp)\n";
            cout << "   16. Move/rename file (mv)\n";
            cout << "   17. Remove file (rm)\n";
            cout << "   18. Remove directory (rmdir)\n";
            cout << "5. myPipe Class Operations:\n";
            cout << "   19. Read from console\n";
            cout << "   20. Write to console\n";
            cout << "   21. Read from file\n";
            cout << "   22. Write to file\n";
            cout << "0. Exit\n";
            cout << "=============================================\n";
            cout << "Enter your choice: ";

            int choice;
            cin >> choice;
            cin.ignore(); // Clear newline

            string cmd;
            switch (choice)
            {
                case 1:
                    cmd = "pwd";
                    break;
                case 2:
                    cmd = "ls";
                    break;
                case 3:
                    cmd = "ls -a";
                    break;
                case 4:
                    cmd = "ls -l";
                    break;
                case 5:
                    cmd = "ls -al";
                    break;
                case 6:
                    cmd = "cd";
                    break;
                case 7:
                    cmd = "mkdir";
                    break;
                case 8:
                    cmd = "mkdir -p";
                    break;
                case 9:
                    cmd = "touch";
                    break;
                case 10:
                    cmd = "chmod +x";
                    break;
                case 11:
                    cmd = "chmod -w";
                    break;
                case 12:
                    cmd = "chmod -rw";
                    break;
                case 13:
                    cmd = "chgrp";
                    break;
                case 14:
                    cmd = "chown";
                    break;
                case 15:
                    cmd = "cp";
                    break;
                case 16:
                    cmd = "mv";
                    break;
                case 17:
                    cmd = "rm";
                    break;
                case 18:
                    cmd = "rmdir";
                    break;
                case 19:
                    cmd = "read_console";
                    break;
                case 20:
                    cmd = "write_console";
                    break;
                case 21:
                    cmd = "read_file";
                    break;
                case 22:
                    cmd = "write_file";
                    break;
                case 0:
                    cmd = "exit";
                    break;
                default:
                    cout << "Invalid choice. Press Enter to continue...";
                    cin.get();
                    continue;
            }

            pipe.writeToChild(cmd);
            if (cmd == "exit")
                break;

            // Wait for child to finish processing the command
            string response;
            do
            {
                response = pipe.readFromChild();
            } while (response != "DONE" && !response.empty());

            cout << "\nPress Enter to continue...";
            cin.get();
        }

        wait(nullptr); // Wait for child process to exit
    }

    return 0;
}