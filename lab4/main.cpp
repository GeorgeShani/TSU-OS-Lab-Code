#include <iostream> // For standard input/output
#include <cstring>  // For strlen()
#include <cstdlib>  // For general utilities (not used here but included)
#include <unistd.h> // For read(), write(), close()
#include <fcntl.h>  // For open(), file control options
using namespace std;

int main()
{
    const char *filename = "program.txt";        // Name of the file to write to and read from
    const char *text = "მონაცემების გადაცემა\n"; // Text content to write into the file

    // Open the file for writing (create it if it doesn't exist, fail if it does)
    int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0664);
    if (fd == -1)
    {
        cerr << "The file already exists" << endl; // Handle case where file already exists
        return 1;
    }

    // Write text data to the file
    ssize_t n = write(fd, text, strlen(text));
    if (n == -1)
    {
        cerr << "Failed to write data to the file" << endl; // Handle write error
        close(fd);                                          // Close file descriptor before exiting
        return 1;
    }

    cout << "Data successfully written: " << n << " bytes\n" << text << endl;

    // Reopen the file in read-only mode
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        cerr << "Failed to open the file for reading" << endl; // Handle read open error
        close(fd);                                             // Close file descriptor before exiting
        return 1;
    }

    char buff[100];                   // Buffer to store data read from file
    n = read(fd, buff, sizeof(buff)); // Read data into buffer
    if (n == -1)
    {
        cerr << "Error reading the data from the file" << endl; // Handle read error
        close(fd);                                              // Close file descriptor before exiting
        return 1;
    }

    cout << "Data successfully read: " << buff << "Total bytes: " << n << endl;

    close(fd); // Close file descriptor after reading
    return 0;  // Exit program
}
