#include "student.h"

int main()
{
    key_t syncKey = ftok("student.h", 1);
    key_t dataKey = ftok("student.h", 2);
    key_t topStudentsKey = ftok("student.h", 3);

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

    cout << "Process 3: Waiting for Process 2 to finish..." << endl;
    while (syncMemory[1] != 1) usleep(1000); // Small delay to prevent busy waiting

    cout << "Process 3: Reading top 50 students from shared memory..." << endl;

    int topStudentsMemoryId = shmget(topStudentsKey, TOP_STUDENTS_COUNT * sizeof(Student), 0);
    if (topStudentsMemoryId == -1)
    {
        perror("shmget top");
        exit(-1);
    }

    Student *topStudentsMemory = (Student *)shmat(topStudentsMemoryId, nullptr, 0);
    if (topStudentsMemory == (Student *)-1)
    {
        perror("shmat top");
        exit(-1);
    }

    cout << "Process 3: Analyzing grades above 90..." << endl;

    int maxExcellentGrades = 0;
    int studentsWithMaxCount = 0;

    // Find maximum number of grades > 90
    for (size_t i = 0; i < TOP_STUDENTS_COUNT; i++)
    {
        int count = topStudentsMemory[i].countSubjectsWithGradesAbove90();
        if (count > maxExcellentGrades) maxExcellentGrades = count;
    }

    // Count students with maximum number of grades > 90
    for (size_t i = 0; i < TOP_STUDENTS_COUNT; i++)
        if (topStudentsMemory[i].countSubjectsWithGradesAbove90() == maxExcellentGrades)
            studentsWithMaxCount++;

    // Sort by GPA descending
    sort(topStudentsMemory, topStudentsMemory + TOP_STUDENTS_COUNT,
        [](const Student &a, const Student &b)
        { return a > b; });

    cout << "\n=== ANALYSIS RESULTS ===" << endl;
    cout << "Maximum number of subjects with grades > 90: " << maxExcellentGrades << endl;
    cout << "Number of students with " << maxExcellentGrades << " subjects > 90: " << studentsWithMaxCount << endl;
    cout << "\nFirst 5 students with the most grades > 90:" << endl;

    int displayedStudents = 0;
    for (size_t i = 0; i < TOP_STUDENTS_COUNT && displayedStudents < 5; i++)
    {
        if (topStudentsMemory[i].countSubjectsWithGradesAbove90() == maxExcellentGrades)
        {
            cout << (displayedStudents + 1) << ". " << topStudentsMemory[i] << endl;
            cout << "   Grades > 90: " << topStudentsMemory[i].countSubjectsWithGradesAbove90() << " subjects" << "\n\n";
            displayedStudents++;
        }
    }

    cout << "Process 3: Analysis complete!" << endl;

    shmdt(syncMemory);
    shmdt(topStudentsMemory);

    // Clean up shared memory segments
    shmctl(syncMemoryId, IPC_RMID, nullptr);

    int dataMemoryId = shmget(dataKey, TOTAL_STUDENTS * sizeof(Student), 0);
    if (dataMemoryId != -1) shmctl(dataMemoryId, IPC_RMID, nullptr);

    shmctl(topStudentsMemoryId, IPC_RMID, nullptr);

    return 0;
}