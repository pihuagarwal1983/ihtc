#include <stdio.h>
//#include <ncurses.h>
//#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"
#include <direct.h>    
#include <stdbool.h>


//#include <cassert>

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\n", message, __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// Extern declarations (telling the compiler these are defined elsewhere)
extern int days;
extern int skill_levels;
extern int num_occupants;
extern int num_patients;
extern int num_surgeons;
extern int num_ots;
extern int num_rooms;
extern int num_nurses;
extern int mandatory_count;
extern int optional_count;
extern int current_ot_index;

// int is_admitted[1000] = {0}; // we're not using this array anywhere so let's comment it out
int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
int* current_size_dm_nurse;
int* room_gender_map;

int **size_of_room_schedule;

// Enums for the mapped values
typedef enum { A, B } gender;
typedef enum { early, late, night } shift_types;
typedef enum { optional, mandatory } PatientType;

const char* gender_to_string(gender g);

const char* shift_types_to_string(shift_types shift);

// Enum conversion functions
gender string_to_gender(const char* str);

shift_types string_to_shift_types(const char* str);

int str2int(char* a);

// Struct definitions
typedef struct {
   int day;
   shift_types shift_time;
   int max_load;
   int* rooms;
   int num_rooms;
} Shifts;

typedef struct {
   int id;
   int skill_level;
   Shifts* shift;
   int num_shifts;
} Nurses;

//Nurses* nurses;
Nurses*** dm_nurses_availability;  //nurse assignment
int** max_load_updated; //nurse assignment

typedef struct {
   int room_mixed_age;
   int room_nurse_skill;
   int continuity_of_care;
   int nurse_excessive_workload;
   int open_operating_theater;
   int surgeon_transfer;
   int patient_delay;
   int unscheduled_optional;
} Weights;


//Weights* weights = NULL; // Global pointer for weights
typedef struct {
   int id;
   int length_of_stay;
   int room_id;
   char* age;
   gender gen;
   int* workload_produced;
   int* skill_level_required;
} Occupants;


//Occupants* occupants = NULL; // Global pointer for occupants
typedef struct {
   int id;
   int mandatory;
   gender gen;
   char* age_group;
   int length_of_stay;
   int surgery_release_day;
   int surgery_due_day;
   int surgery_duration;
   int surgeon_id;
   int* incompatible_room_ids;
   int num_incompatible_rooms;
   int* workload_produced;
   int* skill_level_required;
   int assigned_room_no;
   int assigned_ot;
   int admission_day;
} Patient;


//Patient* patients;
//Patient** mandatory_patients = NULL;
//Patient** optional_patients = NULL;
//Patient**** room_schedule;

typedef struct {
   int id;
   int* max_surgery_time;
   int* time_left; // default: max_surgery_time
   int* patients_assigned;
   int num_assigned_patients; // a fixed number - how many patients are assigned to this surgeon
   int current_size; // dynamic size of the patients_assigned array
} Surgeon;


//Surgeon* surgeon;
typedef struct {
   int id;
   int* max_ot_time;
   int* time_left; // default: max_ot_time
} OTs;


//OTs* ot;
typedef struct {
   int id;
   int cap;
   int num_patients_allocated;
   // int *patients_allocated;
   int occupants_cap;
   int* nurses_alloted;
   int length_of_nurses_alloted;
} Rooms;

extern Nurses* nurses;
extern Weights* weights;
extern Occupants* occupants;
extern Patient* patients;
extern Patient** mandatory_patients;
extern Patient** optional_patients;
extern Patient**** room_schedule;
extern Surgeon* surgeon;
extern OTs* ot;
extern Rooms* rooms;
// one idea is that make the patients_assigned array a static array and remove num_patients_assigned field from the structure.



extern void parse_occupants(cJSON* occupants_array);
extern void free_occupants();
extern void assign_occupants_to_rooms(void);
extern void parse_weights(cJSON* weights_json);
extern void parse_patients(cJSON* patients_array);
extern void free_patients();
extern void assign_patients_to_surgeon(void);
extern void free_surgeons();
extern void parse_ots(cJSON* ot_array);
extern void free_ots();
extern void parse_rooms(cJSON* room_array);
extern void free_rooms(void);
extern void parse_nurse(cJSON* nurses_array);
extern void print_occupants();
extern void parse_json(const char* filename); 

//-------------------------------------------BELOW: GENETIC ALGORITHM-------------------------------------------------------

const int POPULATION_SIZE = 100;
int **POPULATION, CHROMOSOME_SIZE;
// Added 1 for storing fitness score

void swapGenes(int chromo_num, int r1, int r2)
{   POPULATION[chromo_num][r1] += POPULATION[chromo_num][r2];
    POPULATION[chromo_num][r2] = POPULATION[chromo_num][r1] - POPULATION[chromo_num][r2];
    POPULATION[chromo_num][r1] -= POPULATION[chromo_num][r2];
}

void generateNewChromosome(int chromo_num)
{   int j, r1, r2;

    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j=0; j<CHROMOSOME_SIZE; ++j)
        POPULATION[chromo_num][j] = POPULATION[chromo_num-1][j];

    srand(time(0));
    for (j=0; j<CHROMOSOME_SIZE; ++j) {
        r1 = rand() % (CHROMOSOME_SIZE);
        r2 = rand() % (CHROMOSOME_SIZE);
        if (r1 == r2)
            r2 = rand() % (CHROMOSOME_SIZE);
        swapGenes(chromo_num, r1, r2);
    }
}

void generatePopulation(void)
{   int i, j;
    POPULATION = (int **) calloc (POPULATION_SIZE, sizeof(int *));
    if (!POPULATION)
        ASSERT (0, "DYNAMIC MEMORY ALLOCATION FOR POPULATION");
    for (i=0; i<POPULATION_SIZE; ++i) {
        POPULATION[i] = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
        if (!POPULATION[i])
            ASSERT (0, "DYNAMIC MEMORY ALLOCATION FOR A CHROMOSOME");
    }

    memset(POPULATION[0], 0, POPULATION_SIZE * (CHROMOSOME_SIZE + 1) * sizeof(POPULATION));
    // last column -1
    for (i=0; i<POPULATION_SIZE; ++i)
        POPULATION[i][CHROMOSOME_SIZE] = -1;

    // transfer mandatory_patients ids to the population as first chromosome
    for (j=0; j<CHROMOSOME_SIZE; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;

    for (i=1; i<POPULATION_SIZE; ++i)
        generateNewChromosome(i);
}


void freePopulation(void)
{   int i;
    for (i=0; i<POPULATION_SIZE; ++i)
        free(POPULATION[i]);
}

void printPopulation(void)
{   int i, j;
    for (i=0; i<POPULATION_SIZE; ++i) {
        printf("Population:%d\n", i);
        for (j=0; j<CHROMOSOME_SIZE; ++j)
            printf("%d ", POPULATION[i][j]);
        putch('\n');
    }
}

//-------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------

//int main(void) {
//    parse_json("data/instances/i02.json");
//    print_occupants();
//    CHROMOSOME_SIZE = mandatory_count;
//    generatePopulation();
//    printPopulation();
//
//
//}
