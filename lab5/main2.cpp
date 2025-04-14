#include <iostream> // For standard input/output
#include <unistd.h> // For pipe(), fork(), read(), write(), close()
using namespace std;

int main()
{
    int pipefd1[2], pipefd2[2];            // Arrays to hold file descriptors for two pipes
    int returnStatus1, returnStatus2;      // Return values for pipe creation
    char pipe1WriteMessage[20] = "Hi!";    // Message from parent to child
    char pipe2WriteMessage[20] = "Hello!"; // Message from child to parent
    char readMessage[20];                  // Buffer to store received message
    pid_t pid;                             // Variable to store process ID

    // Create the first pipe
    returnStatus1 = pipe(pipefd1);
    if (returnStatus1 == -1)
    {
        cerr << "Error: Failed to create pipe1" << endl;
    }

    // Create the second pipe
    returnStatus2 = pipe(pipefd2);
    if (returnStatus2 == -1)
    {
        cerr << "Error: Failed to create pipe2" << endl;
    }

    // Create a child process
    pid = fork();

    // Parent process
    if (pid != 0)
    {
        close(pipefd1[0]); // Close read end of pipe1, parent writes
        close(pipefd2[1]); // Close write end of pipe2, parent reads

        cout << "Parent process writing to pipe1: " << pipe1WriteMessage << endl;
        write(pipefd1[1], pipe1WriteMessage, sizeof(pipe1WriteMessage)); // Send message to child

        read(pipefd2[0], readMessage, sizeof(readMessage)); // Read response from child
        cout << "Parent process read from pipe2: " << readMessage << endl;
    }
    // Child process
    else
    {
        close(pipefd1[1]); // Close write end of pipe1, child reads
        close(pipefd2[0]); // Close read end of pipe2, child writes

        read(pipefd1[0], readMessage, sizeof(readMessage)); // Read message from parent
        cout << "Child process read from pipe1: " << readMessage << endl;

        cout << "Child process writing to pipe2: " << pipe2WriteMessage << endl;
        write(pipefd2[1], pipe2WriteMessage, sizeof(pipe2WriteMessage)); // Send response to parent
    }

    return 0; // Exit process
}
