#include "student.h"

void *calculateGpaThread(void *studentPtr)
{
    Student *currentStudent = (Student *)studentPtr;
    currentStudent->calculateGradePointAverage();
    return nullptr;
}

int main()
{
    key_t syncKey = ftok("student.h", 1);
    key_t dataKey = ftok("student.h", 2);
    key_t topStudentsKey = ftok("student.h", 3);

    if (syncKey == -1 || dataKey == -1 || topStudentsKey == -1)
    {
        perror("ftok");
        exit(-1);
    }

    // Access synchronization shared memory
    int syncMemoryId = shmget(syncKey, 2 * sizeof(int), 0);
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

    cout << "Process 2: Waiting for Process 1 to finish..." << endl;
    
    int waitCounter = 0;
    while (syncMemory[0] != 1)
    {
        usleep(10000); // 10ms delay
        
        waitCounter++;

        if (waitCounter % 1000 == 0) // Print every 10 seconds
            cout << "Process 2: Still waiting for Process 1... (sync1 = " << syncMemory[0] << ")" << endl;
    }

    cout << "Process 2: Reading students from shared memory..." << endl;

    // Access data shared memory
    size_t dataMemorySize = TOTAL_STUDENTS * sizeof(Student);
    int dataMemoryId = shmget(dataKey, dataMemorySize, 0);
    if (dataMemoryId == -1)
    {
        perror("shmget data");
        exit(-1);
    }

    Student *studentsData = (Student *)shmat(dataMemoryId, nullptr, 0);
    if (studentsData == (Student *)-1)
    {
        perror("shmat data");
        exit(-1);
    }

    cout << "Process 2: Calculating GPA using threads..." << endl;

    pthread_t threadArray[TOTAL_STUDENTS];
    for (size_t i = 0; i < TOTAL_STUDENTS; i++)
    {
        if (pthread_create(&threadArray[i], nullptr, calculateGpaThread, (void *)&studentsData[i]) != 0)
        {
            perror("pthread_create");
            exit(-1);
        }
    }

    for (size_t i = 0; i < TOTAL_STUDENTS; i++)
        pthread_join(threadArray[i], nullptr);

    cout << "Process 2: Sorting students by GPA..." << endl;
    sort(studentsData, studentsData + TOTAL_STUDENTS, 
        [](const Student &a, const Student &b)
        { return a < b; });

    cout << "Process 2: Writing top 50 students to shared memory..." << endl;

    // Create shared memory for top 50 students
    size_t topStudentsMemorySize = TOP_STUDENTS_COUNT * sizeof(Student);
    int topStudentsMemoryId = shmget(topStudentsKey, topStudentsMemorySize, IPC_CREAT | 0666);
    if (topStudentsMemoryId == -1)
    {
        perror("shmget top");
        exit(-1);
    }

    Student *topStudents = (Student *)shmat(topStudentsMemoryId, nullptr, 0);
    if (topStudents == (Student *)-1)
    {
        perror("shmat top");
        exit(-1);
    }

    // Copy top 50 students (highest GPA)
    for (size_t i = 0; i < TOP_STUDENTS_COUNT; i++)
        topStudents[i] = studentsData[TOTAL_STUDENTS - TOP_STUDENTS_COUNT + i];

    syncMemory[1] = 1; // Signal that processing is done
    cout << "Process 2: Finished processing. Top 50 students written to shared memory." << endl;

    shmdt(syncMemory);
    shmdt(studentsData);
    shmdt(topStudents);
    
    return 0;
}