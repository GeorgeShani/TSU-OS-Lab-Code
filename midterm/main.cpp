#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
using namespace std;

/**
 * - - - - - - - - - - - - - - - - - -  2.0 points - - - - - - - - - - - - - - - - - - - - - - -
 * Let's say lakes.txt file contains information about deep lakes in different countries
 * with records for 65 countries in total: country name, lake name, lake
 * depth (for example, Argentina Viedma 900). Each record is placed on a new line.
 * Create a Lake class that describes a lake. Overload the input (>>) and output (<<) operators for the class.
 */

// Lake class to store lake information
class Lake
{
private:
    string country;
    string name;
    int depth;

public:
    Lake() : depth(0) {}
    Lake(const string &c, const string &n, int d) : country(c), name(n), depth(d) {}

    bool isDeep() const
    {
        return depth > 500;
    }

    // Overloaded input operator
    friend istream &operator>>(istream &is, Lake &lake)
    {
        return is >> lake.country >> lake.name >> lake.depth;
    }

    // Overloaded output operator
    friend ostream &operator<<(ostream &os, const Lake &lake)
    {
        return os << lake.country << " " << lake.name << " " << lake.depth;
    }
};

/**
 * - - - - - - - - - - - - - - - - - -  1.0 point - - - - - - - - - - - - - - - - - - - - - - -
 * Add data reading and writing functions to the myPipe class below.
 */

// Pipe class for handling pipe communication
class Pipe
{
private:
    int pipefd[2];
    bool isOpen;

public:
    Pipe() : isOpen(false) {}

    bool create()
    {
        isOpen = (pipe(pipefd) == 0);
        return isOpen;
    }

    void close()
    {
        if (isOpen)
        {
            ::close(pipefd[0]);
            ::close(pipefd[1]);
            isOpen = false;
        }
    }

    void closeReadEnd()
    {
        if (isOpen)
        {
            ::close(pipefd[0]);
        }
    }

    void closeWriteEnd()
    {
        if (isOpen)
        {
            ::close(pipefd[1]);
        }
    }

    int getReadFd() const
    {
        return pipefd[0];
    }

    int getWriteFd() const
    {
        return pipefd[1];
    }

    // Function to read data from the pipe
    ssize_t readData(void *buf, size_t count)
    {
        return read(pipefd[0], buf, count);
    }

    // Function to write data to the pipe
    ssize_t writeData(const void *buf, size_t count)
    {
        return write(pipefd[1], buf, count);
    }

    ~Pipe()
    {
        close();
    }
};

/**
 * - - - - - - - - - - - - - - - - - -  2.0 points - - - - - - - - - - - - - - - - - - - - - - -
 * Create a process hierarchy corresponding to the image in the program
 *
 * - - - - - - - - - - - - - - - - - -  2.0 points - - - - - - - - - - - - - - - - - - - - - - -
 * Create two pipe channels in the program and use them to ensure data transfer
 * P2 -> P4 and P4 -> P3 in sequence (according to the image). Specifically, P2 process should
 * use the open system call to read data from lakes.txt and pass it to P4 (N bytes)
 * through one pipe, and the P4 process should use a second pipe to pass received data
 * to P3 process. P3 process should create a file (data.in) with 0640 access rights
 * (umask must be used before creation) and write the received data into it.
 *
 * - - - - - - - - - - - - - - - - - - 1.0 point - - - - - - - - - - - - - - - - - - - - - - -
 * P5 process should read data from data.in file and use it to create class objects
 * (using >> operator)
 *
 * - - - - - - - - - - - - - - - - - - 1.0 point - - - - - - - - - - - - - - - - - - - - - - -
 * P5 process should find and print information about lakes whose depth exceeds 500
 *
 * - - - - - - - - - - - - - - - - - - 1.0 point - - - - - - - - - - - - - - - - - - - - - - -
 * Logically correct code
 */

// Main function
int main()
{
    // Create pipe instances for process communication
    Pipe pipe_P2_P4;
    Pipe pipe_P4_P3;

    // Create the pipes
    if (!pipe_P2_P4.create() || !pipe_P4_P3.create())
    {
        cerr << "Failed to create pipes" << endl;
        return 1;
    }

    // Create P1 (parent) process
    pid_t p2_pid = fork();
    if (p2_pid == -1)
    {
        cerr << "Failed to fork P2" << endl;
        return 1;
    }

    if (p2_pid == 0)
    {
        // This is P2 process
        // Close unused pipe ends
        pipe_P2_P4.closeReadEnd();
        pipe_P4_P3.close();

        // Open lakes.txt file
        int fd = open("lakes.txt", O_RDONLY);
        if (fd == -1)
        {
            cerr << "P2: Failed to open lakes.txt" << endl;
            return 1;
        }

        // Read data from the file
        char buffer[4096];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0)
        {
            // Write data to pipe
            pipe_P2_P4.writeData(buffer, bytes_read);
        }

        close(fd);
        pipe_P2_P4.closeWriteEnd();
        exit(0);
    }
    else
    {
        // Parent process continues
        pid_t p3_pid = fork();
        if (p3_pid == -1)
        {
            cerr << "Failed to fork P3" << endl;
            return 1;
        }

        if (p3_pid == 0)
        {
            // This is P3 process
            // Close unused pipe ends
            pipe_P2_P4.close();
            pipe_P4_P3.closeWriteEnd();

            // Read data from pipe
            char buffer[4096] = {0};
            ssize_t bytes_read = pipe_P4_P3.readData(buffer, sizeof(buffer) - 1);

            if (bytes_read > 0)
            {
                // Set file permissions with umask before creating file
                mode_t original_mask = umask(0137); // 0640 permission = 0777 - 0137

                // Create and write to data.in file
                int fd = open("data.in", O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (fd != -1)
                {
                    write(fd, buffer, bytes_read);
                    close(fd);
                }
                else
                {
                    cerr << "P3: Failed to create data.in" << endl;
                }

                // Restore original umask
                umask(original_mask);
            }

            pipe_P4_P3.closeReadEnd();
            exit(0);
        }
        else
        {
            // Parent process continues
            pid_t p4_pid = fork();
            if (p4_pid == -1)
            {
                cerr << "Failed to fork P4" << endl;
                return 1;
            }

            if (p4_pid == 0)
            {
                // This is P4 process
                // Close unused pipe ends
                pipe_P2_P4.closeWriteEnd();
                pipe_P4_P3.closeReadEnd();

                // Read data from P2
                char buffer[4096] = {0};
                ssize_t bytes_read = pipe_P2_P4.readData(buffer, sizeof(buffer) - 1);

                if (bytes_read > 0)
                {
                    // Write data to P3
                    pipe_P4_P3.writeData(buffer, bytes_read);
                }

                pipe_P2_P4.closeReadEnd();
                pipe_P4_P3.closeWriteEnd();
                exit(0);
            }
            else
            {
                // Parent process continues
                pid_t p5_pid = fork();
                if (p5_pid == -1)
                {
                    cerr << "Failed to fork P5" << endl;
                    return 1;
                }

                if (p5_pid == 0)
                {
                    // This is P5 process
                    // Close all pipes as P5 uses file
                    pipe_P2_P4.close();
                    pipe_P4_P3.close();

                    // Wait a bit to ensure file is created by P3
                    sleep(1);

                    // Read data from data.in
                    ifstream file("data.in");
                    if (!file)
                    {
                        cerr << "P5: Failed to open data.in" << endl;
                        exit(1);
                    }

                    // Create lake objects and filter those deeper than 500
                    vector<Lake> lakes;
                    Lake lake;

                    // Read lakes using the overloaded >> operator
                    while (file >> lake)
                    {
                        lakes.push_back(lake);
                    }

                    // Find and print lakes deeper than 500
                    cout << "Lakes with depth greater than 500:" << endl;
                    for (const auto &l : lakes)
                    {
                        if (l.isDeep())
                        {
                            cout << l << endl;
                        }
                    }

                    exit(0);
                }
                else
                {
                    // Parent process (P1) - close all pipes and wait for children
                    pipe_P2_P4.close();
                    pipe_P4_P3.close();

                    // Wait for all children to complete
                    waitpid(p2_pid, NULL, 0);
                    waitpid(p3_pid, NULL, 0);
                    waitpid(p4_pid, NULL, 0);
                    waitpid(p5_pid, NULL, 0);

                    return 0;
                }
            }
        }
    }
}