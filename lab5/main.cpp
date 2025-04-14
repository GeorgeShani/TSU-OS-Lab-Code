#include <iostream>   // For standard input/output
#include <unistd.h>   // For pipe(), fork(), read(), write(), close()
#include <cstring>    // For strlen()
#include <sys/wait.h> // For waitpid()
using namespace std;

int main()
{
    int pipefd[2];  // Array to hold file descriptors: pipefd[0] for reading, pipefd[1] for writing
    char buff[100]; // Buffer to hold the message read from the pipe
    pid_t pid;      // Variable to store process ID

    // Create the pipe
    if (pipe(pipefd) == -1)
    {
        cerr << "Error: Unable to create a communication pipe." << endl;
        return 1;
    }

    // Create a child process
    pid = fork();
    if (pid == -1)
    {
        cerr << "Error: Failed to fork the process." << endl;
        return 1;
    }

    // Child process
    if (pid == 0)
    {
        close(pipefd[1]);                    // Close write end, child only reads
        read(pipefd[0], buff, sizeof(buff)); // Read data from pipe
        cout << "[Child] Received message from parent: " << buff << endl;
        close(pipefd[0]); // Close read end after reading
        return 0;         // Exit child process
    }

    // Parent process
    close(pipefd[0]);                                        // Close read end, parent only writes
    const char *message = "გამარჯობა, მე ვარ შენი მშობელი!"; // Message to send to child
    write(pipefd[1], message, strlen(message));              // Write message to pipe
    cout << "[Parent] Message sent to child." << endl;
    close(pipefd[1]); // Close write end after writing

    waitpid(pid, nullptr, 0); // Wait for child process to finish
    cout << "[Parent] Child process has finished execution." << endl;

    return 0; // Exit parent process
}
