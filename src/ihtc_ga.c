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


// Global declarations of variables
int days;
int skill_levels;
int num_occupants;
int num_patients;
int num_surgeons;
int num_ots;
int num_rooms;
int num_nurses;
int mandatory_count = 0;
int optional_count = 0;
int current_ot_index = 0;

// int is_admitted[1000] = {0}; // we're not using this array anywhere so let's comment it out
int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
int* current_size_dm_nurse;
int* room_gender_map;

int **size_of_room_schedule;

// Enums for the mapped values
typedef enum { A, B } gender;
typedef enum { early, late, night } shift_types;
typedef enum { optional, mandatory } PatientType;

const char* gender_to_string(gender g) {
   switch (g) {
   case A: return "Male";
   case B: return "Female";
   default: return "Unknown";
   }
}

const char* shift_types_to_string(shift_types shift) {
   switch (shift) {
   case early: return "early";
   case late: return "late";
   case night: return "night";
   default: return "unknown";
   }
}

// Enum conversion functions
gender string_to_gender(const char* str) {
   if (strcmp(str, "A") == 0) return A;
   if (strcmp(str, "B") == 0) return B;
   return A; // Invalid value
}

shift_types string_to_shift_types(const char* str) {
   if (strcmp(str, "early") == 0) return early;
   if (strcmp(str, "late") == 0) return late;
   if (strcmp(str, "night") == 0) return night;
   return early; // Invalid value
}

int str2int(char* a)
{   /* ex. We'll get a string like p01 or n010.
    We have to find the number in the string and return that number in integer form.
    */
   int number = 0, i, n, num;
   char* p = a;
   p++;
   n = strlen(p);
   for (i = 0; i < n; ++i) {
      num = p[i] - '0';
      number += (num * pow(10, n - i - 1));
   }
   return number;
}
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

Nurses* nurses;
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


Weights* weights = NULL; // Global pointer for weights
typedef struct {
   int id;
   int length_of_stay;
   int room_id;
   char* age;
   gender gen;
   int* workload_produced;
   int* skill_level_required;
} Occupants;


Occupants* occupants = NULL; // Global pointer for occupants
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


Patient* patients;
Patient** mandatory_patients = NULL;
Patient** optional_patients = NULL;
Patient**** room_schedule;

typedef struct {
   int id;
   int* max_surgery_time;
   int* time_left; // default: max_surgery_time
   int* patients_assigned;
   int num_assigned_patients; // a fixed number - how many patients are assigned to this surgeon
   int current_size; // dynamic size of the patients_assigned array
} Surgeon;


Surgeon* surgeon;
typedef struct {
   int id;
   int* max_ot_time;
   int* time_left; // default: max_ot_time
} OTs;


OTs* ot;
typedef struct {
   int id;
   int cap;
   int num_patients_allocated;
   // int *patients_allocated;
   int occupants_cap;
   int* nurses_alloted;
   int length_of_nurses_alloted;
} Rooms;
// one idea is that make the patients_assigned array a static array and remove num_patients_assigned field from the structure.

Rooms* room;

typedef struct RoomsList {
   Rooms* pointer;
   struct RoomsList* next;
   struct RoomsList* prev;
} GenderRoom;
GenderRoom* gender_A_rooms = NULL, * gender_B_rooms = NULL, * empty_rooms = NULL;

typedef struct {
    Rooms** data;
    int size;
    int capacity;
}RoomVector;

RoomVector *v_A, *v_B, *v_empty;

typedef struct {
   int load_sum;
   int max_skill_req;
} Rooms_req;
Rooms_req** rooms_requirement;

typedef struct {
   int surgeon_id;
   int* assigned_patients;
   int num_assigned_patients;
   int surgery_duration_sum;
   int isNull;
   /* added isNull in the struct cuz s_data_arr has null values as well and we need to sort that array on the basis of
   surgery_duration_sum. if that cell is null then we cant really sort the array as that may contrive problems while selecting the pivot
   so for those null pointers we'll create this same structure and put 1 in the isNull field. this way we'll know that this pointer is
   a null one and we can deal with it accordingly.
   */
} Surgeon_data;


// Define the HeapNode and PriorityQueue structures
typedef struct {
    int patient_id;  // Unique ID for patient
    int mandatory;   // 1 if mandatory, 0 otherwise
    int due_day;     // The day when the patient is due
    int delay;      // Lower is better
} HeapNode;

typedef struct {
   HeapNode* data; // Dynamically allocated array for heap nodes
   int current_size;       // Current number of elements in the priority queue
   int capacity;   // Maximum capacity of the priority queue
} PriorityQueue;


typedef struct LL {
   Patient* pointer;
   struct LL* next, * prev;
} Node;

// ----------------------------------------------BELOW: FUNCTION PROTOTYPES FOR PAS AND SCP--------------------------------------------

GenderRoom* appendGenderRoomNode(GenderRoom*, GenderRoom*);
GenderRoom* makeGenderRoomNode(Rooms*);
void free_s_data_arr(Surgeon_data**, int);
void make_3_room_arrays(int*);
void update_LOS_of_patients(int);
int findSuitableRoom(int, RoomVector *);
void sort_s_data_arr(Surgeon_data**, int, Node*);
void sort_ot_data_arr(OTs**, int);
int find_max_surgeon_id(Node*);
OTs* admitOptionalFromPQ(PriorityQueue*, int, OTs**, OTs*);
OTs* admitMandatoryFromPQ(PriorityQueue*, int, OTs**, OTs*, int);
GenderRoom* deleteGenderRoomNode(GenderRoom*, GenderRoom*);
void free_3_room_arrays(void);
GenderRoom* removeAndAppendGenderRoom(int, GenderRoom*);
void update_LOS_of_patients(int);
void admit_patients(int*, PriorityQueue*);
void init_Rooms(RoomVector* vector, int initialcap);
void pushback(RoomVector* vector, Rooms* room);
Rooms* removebyid(RoomVector* vector, int room_id);
void moveRoom(RoomVector* source, RoomVector* destination, int room_id);
void freevector(RoomVector* vector);

// ----------------------------------------------ABOVE: FUNCTION PROTOTYPES FOR PAS AND SCP--------------------------------------------


// Function prototypes
void initPQ(PriorityQueue* pq, int initialCapacity);
void insertNodeInPQ(PriorityQueue* pq, HeapNode node);
HeapNode extractMaxFromPQ(PriorityQueue* pq);
HeapNode peekMaxFromPQ(PriorityQueue* pq);
int isEmptyPQ(PriorityQueue* pq);
void heapifyUp(PriorityQueue* pq, int index);
void heapifyDown(PriorityQueue* pq, int index);
int compareNodesForPQ(HeapNode a, HeapNode b);
void resizePQ(PriorityQueue* pq);
void freePQ(PriorityQueue* pq);

// Function to parse occupants
void parse_occupants(cJSON* occupants_array) {
   num_occupants = cJSON_GetArraySize(occupants_array);
   occupants = (Occupants*)calloc(num_occupants, sizeof(Occupants));
   if (!occupants) {
      printf("Memory allocation failed for occupants.\n");
      exit(1);
   }
   for (int i = 0; i < num_occupants; i++) {
      cJSON* item = cJSON_GetArrayItem(occupants_array, i);
      if (!item) continue;
      // Parse individual fields
      cJSON* id_json = cJSON_GetObjectItem(item, "id");
      cJSON* gender_json = cJSON_GetObjectItem(item, "gender");
      cJSON* age_group_json = cJSON_GetObjectItem(item, "age_group");
      cJSON* length_of_stay_json = cJSON_GetObjectItem(item, "length_of_stay");
      cJSON* workload_produced_json = cJSON_GetObjectItem(item, "workload_produced");
      cJSON* skill_level_required_json = cJSON_GetObjectItem(item, "skill_level_required");
      cJSON* room_id_json = cJSON_GetObjectItem(item, "room_id");
      // Parse ID
      if (id_json && cJSON_IsString(id_json)) {
         int id_int = str2int(id_json->valuestring);
         occupants[i].id = id_int;
      }
      else {
         printf("Error parsing occupant ID for index %d\n", i);
         occupants[i].id = -1; // Set to invalid value
      }
      // Parse Gender
      if (gender_json && cJSON_IsString(gender_json)) occupants[i].gen = string_to_gender(gender_json->valuestring);
      else occupants[i].gen = -1; // Default invalid gender
      // Parse Age Group
      if (age_group_json && cJSON_IsString(age_group_json)) occupants[i].age = strdup(age_group_json->valuestring);
      else occupants[i].age = NULL;
      // Parse Length of Stay
      if (length_of_stay_json && cJSON_IsNumber(length_of_stay_json)) occupants[i].length_of_stay = length_of_stay_json->valueint;
      else occupants[i].length_of_stay = 0; // Default invalid value
      // Parse Room ID
      if (room_id_json && cJSON_IsString(room_id_json)) {
         int room_id_int = str2int(room_id_json->valuestring);
         occupants[i].room_id = room_id_int;
      }
      else {
         occupants[i].room_id = -1; // Default invalid room ID
      }
      // Parse Workload Produced
      if (workload_produced_json && cJSON_IsArray(workload_produced_json)) {
         int size = cJSON_GetArraySize(workload_produced_json);
         occupants[i].workload_produced = (int*)calloc(size, sizeof(int));
         if (!occupants[i].workload_produced) {
            printf("Memory allocation failed for workload_produced of occupant %d\n", i);
            exit(1);
         }
         for (int j = 0; j < size; j++) {
            occupants[i].workload_produced[j] = cJSON_GetArrayItem(workload_produced_json, j)->valueint;
         }
      }
      else {
         occupants[i].workload_produced = NULL;
      }
      // Parse Skill Level Required
      if (skill_level_required_json && cJSON_IsArray(skill_level_required_json)) {
         int size = cJSON_GetArraySize(skill_level_required_json);
         occupants[i].skill_level_required = (int*)calloc(size, sizeof(int));
         if (!occupants[i].skill_level_required) {
            printf("Memory allocation failed for skill_level_required of occupant %d\n", i);
            exit(1);
         }
         for (int j = 0; j < size; j++) {
            occupants[i].skill_level_required[j] = cJSON_GetArrayItem(skill_level_required_json, j)->valueint;
         }
      }
      else {
         occupants[i].skill_level_required = NULL;
      }
   }
}

void free_occupants() {
   for (int i = 0; i < num_occupants; i++) {
      free(occupants[i].workload_produced);
      free(occupants[i].skill_level_required);
   }
   free(occupants);
}

void assign_occupants_to_rooms(void) {
   // here we'll update the occupants_Cap filed of each room
   // method - parse the occupants array (arr of structs) and find out what's its room id and then increase the 
   // occupants_cap filed of that room
   int i, r_id;
   for (i = 0; i < num_occupants; ++i) {
      r_id = occupants[i].room_id;
      room[r_id].occupants_cap++;
   }
}

// Function to parse weights
void parse_weights(cJSON* weights_json) {
   if (!weights_json) {
      printf("No weights data found.\n");
      return;
   }
   weights->room_mixed_age = cJSON_GetObjectItem(weights_json, "room_mixed_age") ? cJSON_GetObjectItem(weights_json, "room_mixed_age")->valueint : 0;
   weights->room_nurse_skill = cJSON_GetObjectItem(weights_json, "room_nurse_skill") ? cJSON_GetObjectItem(weights_json, "room_nurse_skill")->valueint : 0;
   weights->continuity_of_care = cJSON_GetObjectItem(weights_json, "continuity_of_care") ? cJSON_GetObjectItem(weights_json, "continuity_of_care")->valueint : 0;
   weights->nurse_excessive_workload = cJSON_GetObjectItem(weights_json, "nurse_excessive_workload") ? cJSON_GetObjectItem(weights_json, "nurse_excessive_workload")->valueint : 0;
   weights->open_operating_theater = cJSON_GetObjectItem(weights_json, "open_operating_theater") ? cJSON_GetObjectItem(weights_json, "open_operating_theater")->valueint : 0;
   weights->surgeon_transfer = cJSON_GetObjectItem(weights_json, "surgeon_transfer") ? cJSON_GetObjectItem(weights_json, "surgeon_transfer")->valueint : 0;
   weights->patient_delay = cJSON_GetObjectItem(weights_json, "patient_delay") ? cJSON_GetObjectItem(weights_json, "patient_delay")->valueint : 0;
   weights->unscheduled_optional = cJSON_GetObjectItem(weights_json, "unscheduled_optional") ? cJSON_GetObjectItem(weights_json, "unscheduled_optional")->valueint : 0;
}


void parse_patients(cJSON* patients_array) {
   num_patients = cJSON_GetArraySize(patients_array);
   // printf("Number of patients: %d\n", num_patients);
   patients = (Patient*)calloc(num_patients, sizeof(Patient));
   if (!patients) {
      printf("Memory allocation failed for patients.\n");
      exit(1);
   }
   // Allocate memory for mandatory and optional patient pointers
   mandatory_patients = (Patient**)calloc(num_patients, sizeof(Patient*));
   optional_patients = (Patient**)calloc(num_patients, sizeof(Patient*));
   if (!mandatory_patients || !optional_patients) {
      printf("Memory allocation failed for mandatory or optional patient arrays.\n");
      exit(1);
   }
   for (int i = 0; i < num_patients; i++) {
      // int prev_id=0;
      cJSON* item = cJSON_GetArrayItem(patients_array, i);
      if (!item) continue;
      patients[i].assigned_room_no = -1;
      patients[i].admission_day = -1;
      patients[i].assigned_ot = -1;
      // Parse individual fields of each patient
      cJSON* id_json = cJSON_GetObjectItem(item, "id");
      int id_int = str2int(id_json->valuestring);
      if (id_json && cJSON_IsString(id_json)) {
         patients[i].id = id_int;
      }
      cJSON* mandatory_json = cJSON_GetObjectItem(item, "mandatory");
      if (mandatory_json && cJSON_IsBool(mandatory_json)) {
         patients[i].mandatory = cJSON_IsTrue(mandatory_json) ? 1 : 0;
      }
      else {
         patients[i].mandatory = 0;
         printf("Missing or invalid mandatory status for patient.\n");
      }
      // Assign to mandatory or optional array based on the "mandatory" field
      if (patients[i].mandatory) {
         mandatory_patients[mandatory_count++] = patients + i;//.................................................................
      }
      else {
         optional_patients[optional_count++] = patients + i;
      }
      cJSON* gender_json = cJSON_GetObjectItem(item, "gender");
      if (gender_json && cJSON_IsString(gender_json)) {
         patients[i].gen = string_to_gender(gender_json->valuestring);
      }
      else
         patients[i].gen = A;

      cJSON* age_groups_json = cJSON_GetObjectItem(item, "age_group");
      if (age_groups_json && cJSON_IsString(age_groups_json)) {
         patients[i].age_group = strdup(age_groups_json->valuestring); // Allocate and copy the string
         if (!patients[i].age_group) {
            printf("Memory allocation failed for age_group of patient at index %d.\n", i);
            exit(1);
         }
      }
      else {
         patients[i].age_group = strdup("adult"); // Default value
         if (!patients[i].age_group) {
            printf("Memory allocation failed for default age_group of patient at index %d.\n", i);
            exit(1);
         }
         printf("Missing or invalid age group for the patient at index %d.\n", i);
      }
      cJSON* length_of_stay_json = cJSON_GetObjectItem(item, "length_of_stay");
      if (length_of_stay_json && cJSON_IsNumber(length_of_stay_json)) {
         patients[i].length_of_stay = length_of_stay_json->valueint;
      }
      else {
         patients[i].length_of_stay = 0;
         printf("Missing or invalid length_of_stay for patient at index %d.\n", i);
      }
      cJSON* surgery_release_day_json = cJSON_GetObjectItem(item, "surgery_release_day");
      if (surgery_release_day_json && cJSON_IsNumber(surgery_release_day_json)) {
         patients[i].surgery_release_day = surgery_release_day_json->valueint;
      }
      else {
         patients[i].surgery_release_day = 0;
         printf("Missing or invalid surgery_release_day for patient at index %d.\n", i);
      }
      cJSON* surgery_duration_json = cJSON_GetObjectItem(item, "surgery_duration");
      if (surgery_duration_json && cJSON_IsNumber(surgery_duration_json)) {
         patients[i].surgery_duration = surgery_duration_json->valueint;
      }
      else {
         patients[i].surgery_duration = 0;
         printf("Missing or invalid surgery_duration for patient at index %d.\n", i);
      }
      if (patients[i].mandatory) {
         cJSON* surgery_due_day_json = cJSON_GetObjectItem(item, "surgery_due_day");
         if (surgery_due_day_json && cJSON_IsNumber(surgery_due_day_json)) {
            patients[i].surgery_due_day = surgery_due_day_json->valueint;
         }
         else {
            patients[i].surgery_due_day = -1;
            printf("Missing or invalid surgery_due_day for mandatory patient at index %d.\n", i);
         }
      }
      else {
         patients[i].surgery_due_day = -1;
      }
      cJSON* surgeon_id_json = cJSON_GetObjectItem(item, "surgeon_id");
      if (surgeon_id_json && cJSON_IsString(surgeon_id_json)) {
         int surgeon_id_int = str2int(surgeon_id_json->valuestring);
         patients[i].surgeon_id = surgeon_id_int;
         // printf("%d\n" , surgeon_id_int);
      }
      cJSON* incompatible_rooms_json = cJSON_GetObjectItem(item, "incompatible_room_ids");
      if (incompatible_rooms_json && cJSON_IsArray(incompatible_rooms_json)) {
         patients[i].num_incompatible_rooms = cJSON_GetArraySize(incompatible_rooms_json);
         patients[i].incompatible_room_ids = (int*)calloc(patients[i].num_incompatible_rooms, sizeof(int));
         if (!patients[i].incompatible_room_ids) {
            printf("Memory allocation failed for incompatible_room_ids at index %d.\n", i);
            exit(1);
         }
         for (int j = 0; j < patients[i].num_incompatible_rooms; j++) {
            cJSON* room_id_json = cJSON_GetArrayItem(incompatible_rooms_json, j);
            if (cJSON_IsString(room_id_json)) {
               patients[i].incompatible_room_ids[j] = str2int(room_id_json->valuestring);
            }
            else {
               printf("Invalid room ID format for patient at index %d, room %d.\n", i, j);
               patients[i].incompatible_room_ids[j] = -1;
            }
         }
      }
      else {
         patients[i].incompatible_room_ids = NULL;
         patients[i].num_incompatible_rooms = 0;
         printf("Missing or invalid incompatible_room_ids for patient at index %d.\n", i);
      }
      cJSON* workload_json = cJSON_GetObjectItem(item, "workload_produced");
      if (workload_json && cJSON_IsArray(workload_json)) {
         int size = cJSON_GetArraySize(workload_json);
         patients[i].workload_produced = (int*)calloc(size, sizeof(int));
         if (!patients[i].workload_produced) {
            printf("Memory allocation failed for workload_produced at index %d.\n", i);
            exit(1);
         }
         for (int j = 0; j < size; j++) {
            patients[i].workload_produced[j] = cJSON_GetArrayItem(workload_json, j)->valueint;
         }
      }
      else {
         patients[i].workload_produced = NULL;
         printf("Missing or invalid workload_produced for patient at index %d.\n", i);
      }
      cJSON* skills_json = cJSON_GetObjectItem(item, "skill_level_required");
      if (skills_json && cJSON_IsArray(skills_json)) {
         int size = cJSON_GetArraySize(skills_json);
         patients[i].skill_level_required = (int*)calloc(size, sizeof(int));
         if (!patients[i].skill_level_required) {
            printf("Memory allocation failed for skill_level_required at index %d.\n", i);
            exit(1);
         }
         for (int j = 0; j < size; j++) {
            patients[i].skill_level_required[j] = cJSON_GetArrayItem(skills_json, j)->valueint;
         }
      }
      else {
         patients[i].skill_level_required = NULL;
         printf("Missing or invalid skill_level_required for patient at index %d.\n", i);
      }
   }
   // Print counts for verification
   // printf("Number of mandatory patients: %d\n", mandatory_count);
   // printf("Number of optional patients: %d\n", optional_count);
}


void free_patients() {
   if (!patients) {
      return; // No need to free if patients is NULL
   }
   for (int i = 0; i < num_patients; i++) {
      // Free incompatible room IDs array
      if (patients[i].incompatible_room_ids)
         free(patients[i].incompatible_room_ids);

      // Free workload array
      if (patients[i].workload_produced)
         free(patients[i].workload_produced);

      // Free skill level array
      if (patients[i].skill_level_required)
         free(patients[i].skill_level_required);

   }
   // Free the main patients array
   free(patients);
   patients = NULL; // Set to NULL to avoid dangling pointer issues
}

// function to parse the surgeons
void parse_surgeons(cJSON* surgeons_array) {
   num_surgeons = cJSON_GetArraySize(surgeons_array);
   // printf("%d\n" , num_surgeons);
   surgeon = (Surgeon*)calloc(num_surgeons, sizeof(Surgeon));
   if (!surgeon) {
      printf("Memory allocation failed for surgeons.\n");
      exit(1);
   }
   for (int i = 0; i < num_surgeons; i++) {
      cJSON* item = cJSON_GetArrayItem(surgeons_array, i);
      if (!item) continue;
      cJSON* id_json = cJSON_GetObjectItem(item, "id");
      if (id_json && cJSON_IsString(id_json)) {
         (surgeon)[i].id = str2int(id_json->valuestring); // Dynamically allocate and copy the string
      }
      else {
         (surgeon)[i].id = -1;
         printf("Missing or invalid ID for surgeon at index %d.\n", i);
      }
      cJSON* max_surgery_time_json = cJSON_GetObjectItem(item, "max_surgery_time");
      if (max_surgery_time_json && cJSON_IsArray(max_surgery_time_json)) {
         int size = cJSON_GetArraySize(max_surgery_time_json);
         (surgeon)[i].max_surgery_time = (int*)calloc(size, sizeof(int));
         (surgeon)[i].time_left = (int*)calloc(size, sizeof(int));
         if (!(surgeon)[i].max_surgery_time) {
            printf("Memory allocation failed for max_surgery_time at index %d.\n", i);
            exit(1);
         }
         for (int j = 0; j < size; j++) {
            (surgeon)[i].max_surgery_time[j] = cJSON_GetArrayItem(max_surgery_time_json, j)->valueint;
            (surgeon)[i].time_left[j] = cJSON_GetArrayItem(max_surgery_time_json, j)->valueint;
         }
      }
      else {
         (surgeon)[i].max_surgery_time = NULL;
         (surgeon)[i].time_left = NULL;
         printf("Missing or invalid max_surgery_time for surgeon at index %d.\n", i);
      }
      surgeon[i].patients_assigned = NULL;
   }
}


void assign_patients_to_surgeon(void) {
   // Clear previously assigned patients (if needed, to prevent incorrect assignments)
   // for (int i = 0; i < num_surgeons; i++) {
   //     if (surgeons[i].patients_assigned != NULL) {
   //         free(surgeons[i].patients_assigned);
   //     }
   //     // Initialize the patients_assigned array for each surgeon
   //     surgeons[i].patients_assigned = (int *)calloc(num_patients, sizeof(int));
   //     if (surgeons[i].patients_assigned == NULL) {
   //         fprintf(stderr, "Memory allocation for patients_assigned failed for surgeon %d.\n", surgeons[i].id);
   //         return;
   //     }
   //     surgeons[i].num_assigned = 0;  // Reset the assigned count for each surgeon
   // }
   // Assign patients to surgeons based on surgeon_id
   for (int i = 0; i < num_patients; i++) {
      int surgeon_index = patients[i].surgeon_id;  // Get the surgeon_id from the patient 
      if (surgeon[surgeon_index].patients_assigned == NULL) {
         surgeon[surgeon_index].patients_assigned = (int*)calloc(100, sizeof(int)); // create a block for 100 patients
         surgeon[surgeon_index].current_size = 100;
      }
      else
         if (surgeon[surgeon_index].num_assigned_patients == surgeon[surgeon_index].current_size) {// increase the current_size by 100
            surgeon[surgeon_index].current_size += 100;
            surgeon[surgeon_index].patients_assigned = (int*)realloc(surgeon[surgeon_index].patients_assigned, (surgeon[surgeon_index].current_size) * sizeof(int));
         }
      surgeon[surgeon_index].patients_assigned[surgeon[surgeon_index].num_assigned_patients] = patients[i].id;
      surgeon[surgeon_index].num_assigned_patients++;  // Increment the assigned count
   }
}



void free_surgeons() {
    if (!surgeon) return; // Check if surgeon array exists

    for (int i = 0; i < num_surgeons; i++) {
        if (surgeon[i].patients_assigned) {
            free(surgeon[i].patients_assigned);
            surgeon[i].patients_assigned = NULL; // Avoid dangling pointer
        }
        if (surgeon[i].max_surgery_time) {
            free(surgeon[i].max_surgery_time);
            surgeon[i].max_surgery_time = NULL; // Avoid dangling pointer
        }
    }

    free(surgeon); // Free the array of Surgeon structs
    surgeon = NULL; // Avoid dangling pointer
}


// Function to parse the operating theatres
void parse_ots(cJSON* ot_array) {
   num_ots = cJSON_GetArraySize(ot_array);
   //printf("Number of Operating Theatres: %d\n", num_ots);
   ot = (OTs*)calloc(num_ots, sizeof(OTs));
   if (!ot) {
      printf("Memory allocation failed for operating theatres.\n");
      exit(1);
   }
   for (int i = 0; i < num_ots; i++) {
      cJSON* item = cJSON_GetArrayItem(ot_array, i);
      if (!item) continue;
      // Parse 'id'
      cJSON* id_Json = cJSON_GetObjectItem(item, "id");
      if (id_Json && cJSON_IsString(id_Json)) {
         ot[i].id = str2int(id_Json->valuestring);
      }
      else {
         ot[i].id = -1;
         printf("Missing or invalid ID for operating theatre at index %d.\n", i);
         continue;  // Skip further processing for this OT
      }
      // Parse 'max_ot_time'
      cJSON* max_ot_time_json = cJSON_GetObjectItem(item, "availability");
      if (max_ot_time_json && cJSON_IsArray(max_ot_time_json)) {
         int num_days = cJSON_GetArraySize(max_ot_time_json);
         ot[i].max_ot_time = (int*)calloc(num_days, sizeof(int));
         ot[i].time_left = (int*)calloc(num_days, sizeof(int));
         if (!ot[i].max_ot_time) {
            printf("Memory allocation failed for max_ot_time.\n");
            exit(1);
         }
         for (int n_day = 0; n_day < num_days; n_day++) {
            cJSON* time_item = cJSON_GetArrayItem(max_ot_time_json, n_day);
            int time = time_item->valueint;
            if (time_item && cJSON_IsNumber(time_item)) {
                ot[i].max_ot_time[n_day] = time_item->valueint;
                ot[i].time_left[n_day] = time_item->valueint;
            }
            else printf("Invalid max_ot_time value at index. Defaulting to 0.\n");
            
         }
      }
   }
}

void free_ots() {
    if (!ot) return; // Check if 'ot' is already NULL

    for (int i = 0; i < num_ots; i++) {
        if (ot[i].max_ot_time) {
            free(ot[i].max_ot_time);
            ot[i].max_ot_time = NULL; // Avoid dangling pointer
        }
        else {
            ASSERT(0, "no value present in the max_ot_time array of ot.\n");
        }
        
        if (ot[i].time_left) {
            free(ot[i].time_left);
            ot[i].time_left = NULL; // Avoid dangling pointer
        }
        else {
            ASSERT(0 , "no value present in the time_left array of ot.\n");
        }
    }

    free(ot); // Free the array of OT structs
    ot = NULL; // Avoid dangling pointer
}

void parse_rooms(cJSON* room_array) {
   num_rooms = cJSON_GetArraySize(room_array);
   room = (Rooms*)calloc(num_rooms, sizeof(Rooms));
   if (!room) {
      printf("Memory allocation failed for room array");
      exit(1);
   }
   for (int i = 0; i < num_rooms; i++) {
      cJSON* item = cJSON_GetArrayItem(room_array, i);
      if (!item) continue;
      room[i].nurses_alloted = NULL;
      cJSON* id_json = cJSON_GetObjectItem(item, "id");
      // room[i].occupied_cap = -1;
      if (id_json && cJSON_IsString(id_json)) room[i].id = str2int(id_json->valuestring);
      else {
         room[i].id = -1;
         printf("Missing or invalid ID for room id.");
      }
      cJSON* capacity_json = cJSON_GetObjectItem(item, "capacity");
      if (capacity_json && cJSON_IsNumber(capacity_json)) room[i].cap = capacity_json->valueint;
      else printf("Missing or invalid capacity for room at index %d. Defaulting to 0.\n", i);
   }
}

void free_rooms(void) {
   if (!room) return;
   int i;
   for (i = 0; i < num_rooms; ++i)
      free(room[i].nurses_alloted);
   free(room);
}

void parse_nurse(cJSON* nurses_array) {
   // Get the number of nurses from the JSON array
   num_nurses = cJSON_GetArraySize(nurses_array);
   nurses = (Nurses*)calloc(num_nurses, sizeof(Nurses)); // Allocate memory for nurses
   if (!nurses) {
      printf("Memory allocation failed for nurses.\n");
      exit(1);
   }
   for (int i = 0; i < num_nurses; i++) {
      cJSON* item = cJSON_GetArrayItem(nurses_array, i);
      if (!item) continue;
      // Parse 'id'
      cJSON* id_json = cJSON_GetObjectItem(item, "id");
      if (id_json && cJSON_IsString(id_json)) {
         nurses[i].id = str2int(id_json->valuestring); // Convert string ID to integer
      }
      else {
         nurses[i].id = -1;
         printf("Missing or invalid ID for nurse at index %d.\n", i);
      }
      // Parse 'skill_level'
      cJSON* skill_level_json = cJSON_GetObjectItem(item, "skill_level");
      if (skill_level_json && cJSON_IsNumber(skill_level_json)) {
         nurses[i].skill_level = skill_level_json->valueint;
      }
      else {
         nurses[i].skill_level = -1; // Default error value
         printf("Missing or invalid skill level for nurse at index %d.\n", i);
      }
      // Parse 'working_shifts'
      cJSON* shifts_json = cJSON_GetObjectItem(item, "working_shifts");
      if (shifts_json && cJSON_IsArray(shifts_json)) {
         int shift_count = cJSON_GetArraySize(shifts_json);
         nurses[i].shift = (Shifts*)calloc(shift_count, sizeof(Shifts)); // Allocate memory for shifts
         if (!nurses[i].shift) {
            printf("Memory allocation failed for shifts of nurse at index %d.\n", i);
            exit(1);
         }
         nurses[i].num_shifts = shift_count; // Store the number of shifts for the nurse
         for (int j = 0; j < shift_count; j++) {
            cJSON* shift_item = cJSON_GetArrayItem(shifts_json, j);
            if (shift_item) {
               cJSON* day_json = cJSON_GetObjectItem(shift_item, "day");
               if (day_json && cJSON_IsNumber(day_json)) {
                  nurses[i].shift[j].day = day_json->valueint;
               }
               else {
                  nurses[i].shift[j].day = -1; // Default error value
                  printf("Missing or invalid day for nurse %d, shift %d.\n", i, j);
               }

               // Parse 'shift'
               cJSON* shift_json = cJSON_GetObjectItem(shift_item, "shift");
               if (shift_json && cJSON_IsString(shift_json)) {
                  nurses[i].shift[j].shift_time = string_to_shift_types(shift_json->valuestring); // Convert string to enum
               }
               else {
                  nurses[i].shift[j].shift_time = early; // Default error value
                  printf("Missing or invalid shift time for nurse %d, shift %d.\n", i, j);
               }

               // Parse 'max_load'
               cJSON* max_load_json = cJSON_GetObjectItem(shift_item, "max_load");
               if (max_load_json && cJSON_IsNumber(max_load_json)) {
                  nurses[i].shift[j].max_load = max_load_json->valueint;
               }
               else {
                  nurses[i].shift[j].max_load = -1; // Default error value
                  printf("Missing or invalid max load for nurse %d, shift %d.\n", i, j);
               }
               nurses[i].shift[j].rooms = (int*)calloc(1,sizeof(int));  // Initialize rooms as NULL
               nurses[i].shift[j].num_rooms = 0; // Initialize num_rooms to 0
            }
         }
      }
      else {
         nurses[i].shift = NULL;
         nurses[i].num_shifts = 0; // Set number of shifts to zero
         printf("Missing or invalid working_shifts for nurse at index %d.\n", i);
      }
   }
}


//void free_nurses() {
//   if (nurses) {
//      for (int i = 0; i < num_nurses; i++)
//         if (nurses[i].shift) {
//            for (int j = 0; j < nurses[i].num_shifts; j++)
//               if (nurses[i].shift[j].rooms)
//                  free(nurses[i].shift[j].rooms); // Free the rooms array if allocated
//            free(nurses[i].shift); // Free the shifts array for the nurse
//         }
//      free(nurses); // Free the nurses array itself
//   }
//}

// Function to parse the JSON file
void parse_json(const char* filename) {
   FILE* file = fopen(filename, "r");
   if (!file) {
      printf("Unable to open file.\n");
      return;
   }
   fseek(file, 0, SEEK_END);
   long file_size = ftell(file);
   fseek(file, 0, SEEK_SET);
   char* json_data = (char*)calloc(1, file_size + 1);
   fread(json_data, 1, file_size, file);
   fclose(file);
   json_data[file_size] = '\0';
   cJSON* root = cJSON_Parse(json_data);
   if (!root) {
      printf("Error parsing JSON.\n");
      free(json_data);
      return;
   }
   days = cJSON_GetObjectItem(root, "days")->valueint;
   skill_levels = cJSON_GetObjectItem(root, "skill_levels")->valueint;
   // Parse weights
   cJSON* weights_json = cJSON_GetObjectItem(root, "weights");
   weights = (Weights*)calloc(1, sizeof(Weights));
   parse_weights(weights_json);
   // Parse occupants
   cJSON* occupants_array = cJSON_GetObjectItem(root, "occupants");
   parse_occupants(occupants_array);
   // parse patients
   cJSON* patients_array = cJSON_GetObjectItem(root, "patients");
   parse_patients(patients_array);
   // parse surgeons
   cJSON* surgery_array = cJSON_GetObjectItem(root, "surgeons");
   parse_surgeons(surgery_array);
   assign_patients_to_surgeon();
   // parse ots
   cJSON* ot_array = cJSON_GetObjectItem(root, "operating_theaters");
   parse_ots(ot_array);
   // parse rooms
   cJSON* room_array = cJSON_GetObjectItem(root, "rooms");
   parse_rooms(room_array);
   assign_occupants_to_rooms();
   // parse nurses
   cJSON* nurse_array = cJSON_GetObjectItem(root, "nurses");
   parse_nurse(nurse_array);
   free(json_data);
   cJSON_Delete(root);
}




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

    memset(POPULATION, 0, sizeof(POPULATION));
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
        for (j=0; j<CHROMOSOME_SIZE; ++j)
            printf("%d ", POPULATION[i][j]);
        putch('\n');
    }
}

//-------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------

int main(void) {
    parse_json("C:/sant_saran/C in One/IHTC/VS_Code/EA/test09.json");
    CHROMOSOME_SIZE = mandatory_count;
    generatePopulation();
    printPopulation();


}