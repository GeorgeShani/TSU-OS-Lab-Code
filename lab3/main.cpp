#include <iostream>   // For standard input/output
#include <unistd.h>   // For fork(), sleep(), getpid(), getppid()
#include <sys/wait.h> // For wait()
using namespace std;

int main()
{
    pid_t pid = fork(); // Create a new child process

    // Error handling for failed fork
    if (pid < 0)
    {
        cerr << "Error creating a process..." << endl;
        return 1;
    }
    // Child process
    else if (pid == 0)
    {
        sleep(5); // Simulate some delay in the child process
        cout << "Child process created. Child PID: " << getpid()
             << ". Parent PID: " << getppid() << endl;
    }
    // Parent process
    else
    {
        wait(NULL); // Wait for the child process to finish
        cout << "Parent process initialized. PID: " << getpid() << endl;
    }

    return 0; // Exit process
}
