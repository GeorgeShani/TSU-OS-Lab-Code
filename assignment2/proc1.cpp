#include "student.h"

int main()
{
    key_t syncKey = ftok("student.h", 1);
    key_t dataKey = ftok("student.h", 2);

    if (syncKey == -1 || dataKey == -1)
    {
        perror("ftok");
        exit(-1);
    }

    // Create synchronization shared memory
    int syncMemoryId = shmget(syncKey, 2 * sizeof(int), IPC_CREAT | 0666);
    if (syncMemoryId == -1)
    {
        perror("shmget sync");
        exit(-1);
    }

    int *syncMemory = (int *)shmat(syncMemoryId, nullptr, 0);
    if (syncMemory == (int *)-1)
    {
        perror("shmat sync");
        exit(-1);
    }

    // Initialize synchronization variables
    syncMemory[0] = 0; // Process 1 completion flag
    syncMemory[1] = 0; // Process 2 completion flag

    cout << "Process 1: Reading students from data.in..." << endl;

    // Create data shared memory - try with smaller size first if needed
    size_t dataMemorySize = TOTAL_STUDENTS * sizeof(Student);
    int dataMemoryId = shmget(dataKey, dataMemorySize, IPC_CREAT | 0666);
    if (dataMemoryId == -1)
    {
        cout << "Process 1: Failed to create shared memory of size " << dataMemorySize << " bytes" << endl;
        perror("shmget data");

        // Try to get system limits
        system("ipcs -l | grep -E 'max seg size|max total shared memory'");
        exit(-1);
    }

    Student *studentsData = (Student *)shmat(dataMemoryId, nullptr, 0);
    if (studentsData == (Student *)-1)
    {
        perror("shmat data");
        exit(-1);
    }

    // Initialize all students
    for (size_t i = 0; i < TOTAL_STUDENTS; i++)
    {
        studentsData[i] = Student();
    }

    ifstream inputFile("data.in");
    if (!inputFile.is_open())
    {
        cerr << "Error: Cannot open data.in file" << endl;
        exit(-1);
    }

    size_t studentsReadCount = 0;
    for (size_t i = 0; i < TOTAL_STUDENTS && inputFile >> studentsData[i]; i++)
        studentsReadCount++;
    
    inputFile.close();

    cout << "Process 1: Successfully read " << studentsReadCount << " students from file" << endl;

    syncMemory[0] = 1; // Signal that data reading is complete
    cout << "Process 1: Finished writing students to shared memory" << endl;

    shmdt(syncMemory);
    shmdt(studentsData);
    
    return 0;
}