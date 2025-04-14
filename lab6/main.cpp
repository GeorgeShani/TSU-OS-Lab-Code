#include <iostream>    // For standard input/output
#include <unistd.h>    // For fork(), read(), write(), close()
#include <fcntl.h>     // For open() and O_* constants
#include <sys/stat.h>  // For mkfifo(), S_IFIFO, and permission constants
#include <sys/types.h> // For pid_t
#include <cstring>     // For strlen()
using namespace std;

int main()
{
    const char *parentToChildFifo = "parent_to_child.fifo"; // FIFO for parent-to-child communication
    const char *childToParentFifo = "child_to_parent.fifo"; // FIFO for child-to-parent communication

    // Create FIFO for parent-to-child communication
    if (mknod(parentToChildFifo, S_IFIFO | 0644, 0) == -1)
    {
        cerr << "Error: Failed to create FIFO '" << parentToChildFifo << "'" << endl;
        return 1;
    }

    // Create FIFO for child-to-parent communication
    if (mknod(childToParentFifo, S_IFIFO | 0644, 0) == -1)
    {
        cerr << "Error: Failed to create FIFO '" << childToParentFifo << "'" << endl;
        return 1;
    }

    pid_t pid = fork(); // Create a child process

    if (pid == -1)
    {
        cerr << "Error: Failed to fork the process." << endl;
        return 1;
    }

    // Child process
    if (pid == 0)
    {
        char receivedMessage[100];                                     // Buffer to hold message from parent
        const char *responseMessage = "გამარჯობა, მე ვარ შენი შვილი!"; // Message to send to parent

        int readFd = open(parentToChildFifo, O_RDONLY);         // Open FIFO to read message from parent
        read(readFd, receivedMessage, sizeof(receivedMessage)); // Read data from FIFO
        close(readFd);                                          // Close FIFO after reading

        cout << "[Child] Received from parent: " << receivedMessage << endl;

        int writeFd = open(childToParentFifo, O_WRONLY);          // Open FIFO to send message to parent
        write(writeFd, responseMessage, strlen(responseMessage)); // Write message to FIFO
        close(writeFd);                                           // Close FIFO after writing

        cout << "[Child] Sent message to parent." << endl;
        return 0; // Exit child process
    }

    // Parent process
    char receivedMessage[100];                                 // Buffer to hold message from child
    const char *messageToChild = "გამარჯობა, მე ვარ მშობელი!"; // Message to send to child

    int writeFd = open(parentToChildFifo, O_WRONLY);        // Open FIFO to send message to child
    write(writeFd, messageToChild, strlen(messageToChild)); // Write message to FIFO
    close(writeFd);                                         // Close FIFO after writing

    cout << "[Parent] Sent message to child." << endl;

    int readFd = open(childToParentFifo, O_RDONLY);         // Open FIFO to read message from child
    read(readFd, receivedMessage, sizeof(receivedMessage)); // Read data from FIFO
    close(readFd);                                          // Close FIFO after reading

    cout << "[Parent] Received from child: " << receivedMessage << endl;

    return 0; // Exit parent process
}
