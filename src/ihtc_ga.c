#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"
#include <direct.h>
#include <stdbool.h>
#include <sys/stat.h>

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


// Function to print occupants data
void print_occupants() {
   printf("\nOccupants Data:\n");
   for (int i = 0; i < num_occupants; i++) {
      printf("Occupant id %d:\n", occupants[i].id);
      printf("  Gender: %d\n", occupants[i].gen);
      printf("  Age Group: %s\n", occupants[i].age);
      printf("  Length of Stay: %d\n", occupants[i].length_of_stay);
      printf("  Room ID: %d\n", occupants[i].room_id);
      if (occupants[i].workload_produced) {
         printf("  Workload Produced: ");
         for (int j = 0; j < occupants[i].length_of_stay * 3; j++) {
            printf("%d ", occupants[i].workload_produced[j]);
         }
         printf("\n");
      }
      if (occupants[i].skill_level_required) {
         printf("  Skill Levels Required: ");
         for (int j = 0; j < occupants[i].length_of_stay * 3; j++) {
            printf("%d ", occupants[i].skill_level_required[j]);
         }
         printf("\n");
      }
   }
}

void print_surgeons(Surgeon* surgeons) {
   if (!surgeons || num_surgeons <= 0) {
      printf("No surgeons to display.\n");
      return;
   }
   printf("List of Surgeons:\n");
   for (int i = 0; i < num_surgeons; i++) {
      printf("Surgeon ID: %d\n", surgeons[i].id);
      printf("Max Surgery Time (by day): ");
      if (surgeons[i].max_surgery_time) for (int j = 0; j < days; j++) printf("%d ", surgeons[i].max_surgery_time[j]);
      else printf("No data available");

      printf("Surgeon Time Left (by day): ");
      if (surgeons[i].time_left) for (int j = 0; j < days; j++) printf("%d ", surgeons[i].time_left[j]);
      else printf("No data available");

      printf("\n-----------------------------------\n");
   }
}

// Printing the ots values
void print_ots(OTs* ots) {
   if (!ots || num_ots <= 0) {
      printf("No ots to display.\n");
      return;
   }
   for (int i = 0; i < num_ots; i++) {
      printf("Max OT Time (by day): ");
      if (ot[i].max_ot_time) for (int j = 0; j < days; j++) printf("%d ", ot[i].max_ot_time[j]);
      else printf("No data available.");

      printf("OT Time left (by day): ");
      if (ot[i].time_left) for (int j = 0; j < days; j++) printf("%d ", ot[i].time_left[j]);
      else printf("No data available.");

      printf("\n-----------------------------------\n");
   }
}

// Printing the rooms
void print_rooms() {
   if (!room || num_rooms <= 0) {
      printf("No rooms to display.\n");
      return;
   }
   for (int i = 0; i < num_rooms; i++)
      printf("%d\t%d\t%d\n", room[i].id, room[i].cap, room[i].num_patients_allocated);
   printf("\n-----------------------------------\n");
}


// Printing the nurses
#include <stdio.h>

void print_nurses() {
    if (nurses == NULL || num_nurses == 0) {
        printf("No nurse data available.\n");
        return;
    }

    for (int i = 0; i < num_nurses; i++) {
        printf("Nurse ID: %d\n", nurses[i].id);
        printf("Skill Level: %d\n", nurses[i].skill_level);
        printf("Number of Shifts: %d\n", nurses[i].num_shifts);

        for (int j = 0; j < nurses[i].num_shifts; j++) {
            printf("  Shift %d:\n", j + 1);
            printf("    Day: %d\n", nurses[i].shift[j].day);
            printf("    Shift Time: %d\n", nurses[i].shift[j].shift_time);
            printf("    Max Load: %d\n", nurses[i].shift[j].max_load);
            printf("    Number of Rooms: %d\n", nurses[i].shift[j].num_rooms);

            printf("    Assigned Rooms: ");
            for (int k = 0; k < nurses[i].shift[j].num_rooms; k++) {
                printf("r%d ", nurses[i].shift[j].rooms[k]);
            }
            printf("\n");
        }
        printf("\n");
    }
}


void print_patients(Patient* patient_struct_array) {
   int i;
   if (!patient_struct_array) return;
   for (i = 0; i < num_patients; ++i) {
      printf("\nPatient ID: %d\n", patient_struct_array[i].id);
      printf("Mandatory: %d\n", patient_struct_array[i].mandatory);
      printf("Gender: %d\n", patient_struct_array[i].gen);
      printf("Age Group: %s\n", patient_struct_array[i].age_group);
      printf("Length of Stay: %d\n", patient_struct_array[i].length_of_stay);
      printf("Surgery Release Day: %d\n", patient_struct_array[i].surgery_release_day);
      printf("Surgery Duration: %d\n", patient_struct_array[i].surgery_duration);
      printf("Surgery Due Day: %d\n", patient_struct_array[i].surgery_due_day);
      printf("Surgeon ID: %d\n", patient_struct_array[i].surgeon_id);
      printf("Assigned room no:%d \n", patient_struct_array[i].assigned_room_no);
      // Print incompatible rooms if they exist
      if (patient_struct_array[i].num_incompatible_rooms > 0) {
         printf("Incompatible Rooms: ");
         for (int j = 0; j < patient_struct_array[i].num_incompatible_rooms; j++) {
            printf("%d ", patient_struct_array[i].incompatible_room_ids[j]);
         }
         printf("\n");
      }
      // Print workload produced if it exists
      if (patient_struct_array[i].workload_produced != NULL) {
         printf("Workload Produced: ");
         for (int j = 0; j < 3 * patient_struct_array[i].length_of_stay; j++) {
            printf("%d ", patient_struct_array[i].workload_produced[j]);
         }
         printf("\n");
      }
      // Print skill level required if it exists
      if (patient_struct_array[i].skill_level_required != NULL) {
         printf("Skill Level Required: ");
         for (int j = 0; j < 3 * patient_struct_array[i].length_of_stay; j++) {
            printf("%d ", patient_struct_array[i].skill_level_required[j]);
         }
         putchar('\n');
      }
   }
}


void print_optional_patients() {
   printf("==== Optional Patients ====\n");
   for (int i = 0; i < optional_count; i++) {
      printf("\n----- Patient %d -----\n", i);
      printf("Patient ID: %d\n", optional_patients[i]->id);
      printf("Mandatory: %d\n", optional_patients[i]->mandatory);
      printf("Gender: %d\n", optional_patients[i]->gen);
      printf("Age Group: %s\n", optional_patients[i]->age_group);
      printf("Length of Stay: %d\n", optional_patients[i]->length_of_stay);
      printf("Surgery Release Day: %d\n", optional_patients[i]->surgery_release_day);
      printf("Surgery Duration: %d\n", optional_patients[i]->surgery_duration);
      printf("Surgery Due Day: %d\n", optional_patients[i]->surgery_due_day);
      printf("Surgeon ID: %d\n", optional_patients[i]->surgeon_id);
      // Print incompatible rooms if they exist
      if (optional_patients[i]->num_incompatible_rooms > 0) {
         printf("Incompatible Rooms: ");
         for (int j = 0; j < optional_patients[i]->num_incompatible_rooms; j++) {
            printf("%d ", optional_patients[i]->incompatible_room_ids[j]);
         }
         printf("\n");
      }
      // Print workload produced if it exists
      if (optional_patients[i]->workload_produced) {
         printf("Workload Produced: ");
         for (int j = 0; j < 3 * optional_patients[i]->length_of_stay; j++) {
            printf("%d ", optional_patients[i]->workload_produced[j]);
         }
         printf("\n");
      }
      // Print skill level required if it exists
      if (optional_patients[i]->skill_level_required) {
         printf("Skill Level Required: ");
         for (int j = 0; j < 3 * optional_patients[i]->length_of_stay; j++)
            printf("%d ", optional_patients[i]->skill_level_required[j]);
         putchar('\n');
      }
   }
   printf("==================================================================\n");
}


void print_mandatory_patients() {
   printf("==== Mandatory Patients ====\n");
   for (int i = 0; i < mandatory_count; i++) {
      printf("\n----- Patient %d -----\n", i);
      printf("Patient ID: %d\n", mandatory_patients[i]->id);
      printf("Mandatory: %d\n", mandatory_patients[i]->mandatory);
      printf("Gender: %d\n", mandatory_patients[i]->gen);
      printf("Age Group: %s\n", mandatory_patients[i]->age_group);
      printf("Length of Stay: %d\n", mandatory_patients[i]->length_of_stay);
      printf("Surgery Release Day: %d\n", mandatory_patients[i]->surgery_release_day);
      printf("Surgery Duration: %d\n", mandatory_patients[i]->surgery_duration);
      printf("Surgery Due Day: %d\n", mandatory_patients[i]->surgery_due_day);
      printf("Surgeon ID: %d\n", mandatory_patients[i]->surgeon_id);
      // Print incompatible rooms if they exist
      if (mandatory_patients[i]->num_incompatible_rooms > 0) {
         printf("Incompatible Rooms: ");
         for (int j = 0; j < mandatory_patients[i]->num_incompatible_rooms; j++) {
            printf("%d ", mandatory_patients[i]->incompatible_room_ids[j]);
         }
         printf("\n");
      }
      // Print workload produced if it exists
      if (mandatory_patients[i]->workload_produced != NULL) {
         printf("Workload Produced: ");
         for (int j = 0; j < 3 * mandatory_patients[i]->length_of_stay; j++) {
            printf("%d ", mandatory_patients[i]->workload_produced[j]);
         }
         printf("\n");
      }
      // Print skill level required if it exists
      if (mandatory_patients[i]->skill_level_required != NULL) {
         printf("Skill Level Required: ");
         for (int j = 0; j < 3 * mandatory_patients[i]->length_of_stay; j++) {
            printf("%d ", mandatory_patients[i]->skill_level_required[j]);
         }
         printf("\n");
      }
   }
   printf("==================================================================\n");
}

//..................FUNCTIONS BELOW ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON surgery_release_day...................

Node* makeNode(Patient* struct_pointer) {
   Node* node = (Node*)calloc(1, sizeof(Node));
   node->pointer = struct_pointer;
   return node;
}

void appendNode(Node** sorted_array, Patient* struct_pointer, int index) {
   Node* node = makeNode(struct_pointer), * temp;
   if (sorted_array[index] == NULL) {
      sorted_array[index] = node;
   }
   else {
      temp = sorted_array[index];
      sorted_array[index] = node;
      node->next = temp;
      node->next->prev = node;
   }
}

Node** sorted_mandatory_patients = NULL;
void sort_mandatory_patients_on_release_day(Patient** patient_array, int n) {
   sorted_mandatory_patients = (Node**)calloc(days + 1, sizeof(Node*));
   for (int i = 0; i < n; ++i)
      appendNode(sorted_mandatory_patients, patient_array[i], patient_array[i]->surgery_release_day);
}

Node** sorted_optional_patients = NULL;
void sort_optional_patients_on_release_day(Patient** patient_array, int n) {
   sorted_optional_patients = (Node**)calloc(days + 1, sizeof(Node*));
   for (int i = 0; i < n; ++i)
      appendNode(sorted_optional_patients, patient_array[i], patient_array[i]->surgery_release_day);
}

void parse_linked_list(Node* head) {
   // prints the linked list
   putchar('\n');
   for (Node* self = head; self; self = self->next) {
      printf("%d, ", self->pointer->surgery_release_day);
   }
   printf("\b\b");
}

void print_sorted_mandatory_array(void) {
   int i;
   for (i = 0; i < days; ++i) {
      Node* head = sorted_mandatory_patients[i];
      parse_linked_list(head);
   }
}

void print_sorted_optional_array(void) {
   int i;
   for (i = 0; i < days; ++i) {
      Node* head = sorted_optional_patients[i];
      parse_linked_list(head);
   }
}

void free_linked_list(Node* head) {
   if (head->next)
      free_linked_list(head->next);
   free(head);
}

void free_patients_sorted_array(Node** sorted_array) {
   // free sorted_array
   int i;
   for (i = 0; i <= days; ++i)
      if (sorted_array[i])
         free_linked_list(sorted_array[i]);
   free(sorted_array);
}

void copy_paste_pointers_from_sorted_mandatory_array_to_mandatory_patients(void) {
   // Here we will copy all the pointer from the sorted_array to the patients array in the sorted order.
   int i, j = 0;
   Node* self;
   for (i = 0; i <= days; ++i)
      for (self = sorted_mandatory_patients[i]; self; self = self->next)
         mandatory_patients[j++] = self->pointer;
}

void copy_paste_pointers_from_sorted_optional_array_to_optional_patients(void) {
   // Here we will copy all the pointer from the sorted_array to the patients array in the sorted order.
   int i, j = 0;
   Node* self;
   for (i = 0; i <= days; ++i)
      for (self = sorted_optional_patients[i]; self; self = self->next)
         optional_patients[j++] = self->pointer;
}

// ..............FUNCTIONS ABOVE ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON surgery_release_day................

// .................FUNCTIONS BELOW ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON length_of_stay..................


Node* find_smallest_iter(Node* head) {
   // finds the smallest node in a linked list
   Node* smallest = head, * self;
   for (self = head->next; self; self = self->next)
      if (self->pointer->length_of_stay < smallest->pointer->length_of_stay)
         smallest = self;
   return smallest;
}

Node* insertion_sort_for_linked_list(Node* head) {
   // runs insertion sort algorithm on the patients array (mandatory adn optional).
   if (!head || !head->next) return head; // No sorting needed for 0 or 1 node.
   Node* sorted = NULL; // New head of the sorted list
   Node* current = head;
   while (current) {
      Node* next = current->next; // Save the next node
      // Insert current node into the sorted list
      if (!sorted || current->pointer->length_of_stay < sorted->pointer->length_of_stay) {
         // Insert at the head
         current->next = sorted;
         if (sorted) sorted->prev = current; // Update prev for old head
         sorted = current;
         sorted->prev = NULL; // New head has no prev
      }
      else {
         // Traverse sorted list to find the correct position
         Node* temp = sorted;
         while (temp->next && temp->next->pointer->length_of_stay < current->pointer->length_of_stay) {
            temp = temp->next;
         }
         // Insert current after temp
         current->next = temp->next;
         if (temp->next) temp->next->prev = current; // Update prev for next node
         temp->next = current;
         current->prev = temp; // Update prev for current node
      }
      current = next; // Move to the next node in the original list
   }
   return sorted;
}

void sort_mandatory_patients_on_LOS(void) {
   // here we will sort the sorted_mandatory_patients array on LOS of the patient.
   // Approach: send each linked list to the insertion_sort_for_linked_list(Node *head) function.
   // It'll sort that list and return the head of the list.
   int i;
   for (i = 0; i <= days; ++i) {
      Node* head = sorted_mandatory_patients[i];
      head = insertion_sort_for_linked_list(head);
      sorted_mandatory_patients[i] = head;
   }
   copy_paste_pointers_from_sorted_mandatory_array_to_mandatory_patients();
}

void append_optional_to_mandatory(Node** sorted_mandatory_patients, Node** sorted_optional_patients) {
   // combines the 2 data structures - appends the optional_patients array (pointer array) at the tail of the mandatory_patients array
   for (int i = 0; i <= days; ++i) {
      //printf("i am here\n");
      Node* mandatory_head = sorted_mandatory_patients[i];
      Node* optional_head = sorted_optional_patients[i];

      if (!mandatory_head) {
         sorted_mandatory_patients[i] = optional_head;
      }
      else {
         Node* tail = mandatory_head;
         while (tail->next) {
            tail = tail->next;
         }
         tail->next = optional_head;
         if (optional_head) {
            optional_head->prev = tail;
         }
      }
   }
}


void print_sorted_mandatory_patients() {
   for (int i = 0; i < days; i++) {
      printf("\n.........DAY %d.......\n", i + 1);
      Node* mandatory_head = sorted_mandatory_patients[i];
      Node* tail = sorted_mandatory_patients[i];
      while (tail) {
         printf("%d\t", tail->pointer->id);
         tail = tail->next;
      }
      printf("\n");
   }
}

// ...................FUNCTION BELOW ARE FOR THE ROOM_GENDER_MAP.....................//


void initialize_room_gender_map(int** room_gender_map) {
   // initializes the room_gender_map
   *room_gender_map = (int*)calloc(num_rooms, sizeof(int)); // Allocate memory for room_gender_map
   if (!*room_gender_map) {
      printf("Memory allocation failed for room_gender_map.\n");
      exit(1);
   }
   memset(*room_gender_map, -1, num_rooms * sizeof(int)); // Initialize values to -1
}

void populate_room_gender_map(int* room_gender_map) {
   // populates the room_gender_map
   if (!room_gender_map) {
      printf("Error: room_gender_map is not initialized.\n");
      return;
   }
   for (int i = 0; i < num_rooms; ++i) {
      room_gender_map[i] = -1;
   }
   for (int i = 0; i < num_occupants; ++i) {
      if (occupants[i].room_id >= 0 && occupants[i].room_id < num_rooms) {
         room_gender_map[occupants[i].room_id] = occupants[i].gen;
      }
      else {
         printf("Warning: Invalid room_id %d for occupant %d.\n", occupants[i].room_id, i);
      }
   }
}

void print_map(int* room_gender_map) {
   for (int i = 0; i < num_rooms; i++)
      printf("room number: %d\tgender: %d\n", i, room_gender_map[i]);
}


// .......FUNCTIONs BELOW - PRIORITY QUEUE IMPLEMENTATION FOR KEEPING TRACK OF CARRY FORWARDED PATIENTS..........//

// Initialize the priority queue
void heapifyUp(PriorityQueue* pq, int index) {
   // runs the max/min heapify from leaf to the root, i.e. O (log n) time
   while (index > 0) {
      int parent = (index - 1) / 2;
      if (compareNodesForPQ(pq->data[index], pq->data[parent]) > 0) {
         // Swap child with parent
         HeapNode temp = pq->data[index];
         pq->data[index] = pq->data[parent];
         pq->data[parent] = temp;
         index = parent;
      }
      else {
          break;
      }
   }
}

void heapifyDown(PriorityQueue* pq, int index) {
   // runs the max/min heapify from root to leaf, i.e. O (log n) time
   while (2 * index + 1 < pq->current_size) {
      int leftChild = 2 * index + 1;
      int rightChild = 2 * index + 2;
      int largest = index;

      if (leftChild < pq->current_size && compareNodesForPQ(pq->data[leftChild], pq->data[largest]) > 0) {
         largest = leftChild;
      }
      if (rightChild < pq->current_size && compareNodesForPQ(pq->data[rightChild], pq->data[largest]) > 0) {
         largest = rightChild;
      }
      if (largest != index) {
         // Swap parent with the largest child
         HeapNode temp = pq->data[index];
         pq->data[index] = pq->data[largest];
         pq->data[largest] = temp;
         index = largest;
      }
      else
         break;
   }
}

void initPQ(PriorityQueue* pq, int initialCapacity) {
   // initialCapacity is 20
   pq->current_size = 0;
   pq->capacity = initialCapacity;
   HeapNode *data = (HeapNode*)calloc(initialCapacity, sizeof(HeapNode));
   if (data) {
       pq->data = data;
   }
   else {
       ASSERT(!data, "No memory allocated\n");
   }
}

// Insert a new element into the priority queue
void insertNodeInPQ(PriorityQueue* pq, HeapNode node) {
   // inserts a node in the PQ
   if (pq->current_size == pq->capacity) {
      resizePQ(pq); // Resize the array if it's full
   }
   pq->data[pq->current_size] = node; // Add the new node at the end
   heapifyUp(pq, pq->current_size);   // Restore the heap property
   pq->current_size++;
}

// Extract the element with the highest priority
HeapNode extractMaxFromPQ(PriorityQueue* pq) {
   if (isEmptyPQ(pq)) {
      printf("Priority Queue Underflow\n");
      exit(EXIT_FAILURE);
   }
   HeapNode maxNode = pq->data[0];
   pq->data[0] = pq->data[pq->current_size--];
   heapifyDown(pq, 0);
   return maxNode;
}

// Peek at the element with the highest priority without removing it
HeapNode peekMaxFromPQ(PriorityQueue* pq) {
   // peeks the current root of heap (Or of PQ)
   if (isEmptyPQ(pq)) {
      printf("Priority Queue is empty\n");
      exit(EXIT_FAILURE);
   }
   return pq->data[0];
}

// Check if the priority queue is empty
int isEmptyPQ(PriorityQueue* pq) {
   // checks if the PQ is empty
   return pq->current_size == 0;
}

// Compare two HeapNodes based on priority
int compareNodesForPQ(HeapNode a, HeapNode b) {
   // compares 2 nodes and returns the difference of either p_id, mandatory or delay
    if (a.mandatory != b.mandatory) {
        return a.mandatory - b.mandatory;  // Mandatory patients first
    }
    if (a.due_day != b.due_day) {
        return b.due_day - a.due_day;  // Patients with nearer due days first
    }
    if (a.delay != b.delay) {
        return a.delay - b.delay;  // Patients with higher delay first
    }
    return b.patient_id - a.patient_id;
}

void resizePQ(PriorityQueue* pq) {
   // resizes PQ's capacity
   pq->capacity *= 2; // Double the capacity
   HeapNode * data = (HeapNode*)realloc(pq->data, pq->capacity * sizeof(HeapNode));
   if (data) {
       pq->data = data;
   }
   else {
       ASSERT(!data, "No memory allocated to pq.\n");
   }
}


void printPriorityQueue(PriorityQueue* pq) {
    if (isEmptyPQ(pq)) {
        printf("Priority Queue is empty\n");
        return;
    }

    printf("Priority Queue Elements:\n");
    for (int i = 0; i < pq->current_size; i++) {
        printf("Patient ID: %d, Mandatory: %d, Due Day: %d, Delay: %d\n",
            pq->data[i].patient_id, pq->data[i].mandatory,
            pq->data[i].due_day, pq->data[i].delay);
    }
}


void freePQ(PriorityQueue* pq) {
   free(pq->data);
   // pq->data = NULL;
   // pq->current_size = 0;
   // pq->capacity = 0;
   free(pq);
}

HeapNode makeHeapNode(int p_id, int mandatory, int due_day , int delay) {
   // creates a heap_node for PQ
   HeapNode node = { p_id, mandatory, due_day, delay };
   return node;
}


//-----------------------------------------BELOW: QUICK SORT IMPLEMENTATION FOR THE SURGEONS' ARRAY--------------------------------------

int sorting_day;

void print_s_data_arr(Surgeon_data** s_data_arr, int n, int d) {
   printf("\nDay: %d", d);
   for (int i = 0; i < n; i++) {
      printf("\nid: %d -- duration_Sum: %d \n  ", s_data_arr[i]->surgeon_id, s_data_arr[i]->surgery_duration_sum);
      // for(int j = 0 ; j<s_data_arr[i]->num_assigned_patients ; j++){
      //     printf("%d \t" , s_data_arr[i]->assigned_patients[j]);
      // }
      printf("\n");
   }
}

int compare_surgeon(const void* a, const void* b) {
   Surgeon_data* s_a = *(Surgeon_data**)a;
   Surgeon_data* s_b = *(Surgeon_data**)b;

   // Compare max_ot_time for the global sorting day
   if (s_b->surgery_duration_sum != s_a->surgery_duration_sum) {
      return s_b->surgery_duration_sum - s_a->surgery_duration_sum; // Descending order
   }
   return s_b->surgeon_id - s_a->surgeon_id; // Secondary sort by id (ascending order)
}

void sort_s_data_arr(Surgeon_data** s_data_arr, int n, Node* head) {
   // sort this array using quick sort algorithm on surgeon_id
   int i, p_id, s_id;
   Node* self;
   for (self = head; self; self = self->next) {
      s_id = self->pointer->surgeon_id;
      p_id = self->pointer->id;
      if (s_data_arr[s_id] == NULL) {
         s_data_arr[s_id] = (Surgeon_data*)calloc(1, sizeof(Surgeon_data));
         s_data_arr[s_id]->assigned_patients = (int*)calloc(++s_data_arr[s_id]->num_assigned_patients, sizeof(int));
      }
      else
         s_data_arr[s_id]->assigned_patients = (int*)realloc(s_data_arr[s_id]->assigned_patients, ++s_data_arr[s_id]->num_assigned_patients * sizeof(int));

      s_data_arr[s_id]->assigned_patients[s_data_arr[s_id]->num_assigned_patients - 1] = p_id;
      s_data_arr[s_id]->surgeon_id = s_id;
      s_data_arr[s_id]->surgery_duration_sum += self->pointer->surgery_duration;
      s_data_arr[s_id]->isNull = 0;
   }

   // for null pointers - we're creating a structure and putting:
   // 1 in isNull field    -1 in surgeon_id field  0 in num_assigned_patients  NULL in assigned_patients array.
   for (i = 0; i < n; ++i)
      if (s_data_arr[i] == NULL) {
         s_data_arr[i] = (Surgeon_data*)calloc(1, sizeof(Surgeon_data));
         s_data_arr[i]->isNull = 1;
         s_data_arr[i]->surgeon_id = -1;
      }
   // print_s_data_arr(s_data_arr, n);
//quick_sort_surgeons_on_s_duration_sum(s_data_arr, 0, n-1);
   qsort(s_data_arr, n, sizeof(Surgeon_data*), compare_surgeon);
  // print_s_data_arr(s_data_arr, n, sorting_day);
}



//-----------------------------------------ABOVE: QUICK SORT IMPLEMENTATION FOR THE SURGEON ARRAY--------------------------------------

//-----------------------------------------BELOW: QUICK SORT IMPLEMENTATION FOR THE OT ARRAY--------------------------------------



void print_ot_data_arr(OTs** ot_data_arr, int d) {
   printf("Day: %d\n", d);
   for (int i = 0; i < num_ots; i++) {
      printf("id: %d -- max_ot_time: %d\n", ot_data_arr[i]->id, ot_data_arr[i]->max_ot_time[d]);
   }
}



int compare_ots(const void* a, const void* b) {
   OTs* ot_a = *(OTs**)a;
   OTs* ot_b = *(OTs**)b;

   // Compare max_ot_time for the global sorting day
   if (ot_b->max_ot_time[sorting_day] != ot_a->max_ot_time[sorting_day]) {
      return ot_b->max_ot_time[sorting_day] - ot_a->max_ot_time[sorting_day]; // Descending order
   }
   return ot_a->id - ot_b->id; // Secondary sort by id (ascending order)
}

void sort_ot_data_arr(OTs** ot_data_arr, int d) {
   // sorts ot_data_arr using Quick sort algorithm
   if (ot_data_arr == NULL || num_ots <= 0) {
      fprintf(stderr, "Error: Invalid input array\n");
      return;
   }
   // print_ot_data_arr(ot_data_arr , d);
   // quick_sort_OTs_on_max_ot_time(ot_data_arr, 0, num_ots-1, d);
   sorting_day = d;
   qsort(ot_data_arr, num_ots, sizeof(OTs*), compare_ots);
   // print_ot_data_arr(ot_data_arr , d);
}


/////////////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------FUNCTIONS BELOW ARE FOR NURSE ASSIGNMENTS--------------------------------------------------

//...............FUNCTION FOR CREATING THE 3-D ARRAY TO STORE THE PATIENTS IN THE ROOM ON A PARTICULAR DAY.........

void initialize_3d_array() {
    // Allocate memory for room_schedule: 3D array (rooms x days x patients)
    room_schedule = (Patient***)calloc(num_rooms, sizeof(Patient**));  // Allocate space for rooms
    size_of_room_schedule = (int**)calloc(num_rooms, sizeof(int*));  // Allocate space for size tracking by room

    for (int i = 0; i < num_rooms; i++) {
        room_schedule[i] = (Patient**)calloc(days, sizeof(Patient*));  // Allocate space for days per room
        size_of_room_schedule[i] = (int*)calloc(days, sizeof(int));  // Allocate space for size tracking per day

        for (int j = 0; j < days; j++) {
            room_schedule[i][j] = (Patient*)calloc(1, sizeof(Patient));  // Pre-allocate space for 10 patients per day in each room
            //size_of_room_schedule[i][j] = 0;  // Initialize the patient count for each room/day to 0
        }
    }
}


void create_3d_array(void) {
    initialize_3d_array();
    int i, j, admission_day, r_id, los;

    for (i = 0; i < num_patients; ++i) {
        if (patients[i].admission_day == -1) continue;

        admission_day = patients[i].admission_day;
        r_id = patients[i].assigned_room_no;
        los = patients[i].length_of_stay;

        for (j = admission_day; j < admission_day + los; ++j) {
            if (j >= days) break;

            if (room_schedule[r_id][j]) {
                // Corrected: Use a temporary pointer before realloc
                int new_size = size_of_room_schedule[r_id][j] + 1;
                Patient** temp = (Patient**)realloc(room_schedule[r_id][j], new_size * sizeof(Patient*));

                if (!temp) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                room_schedule[r_id][j] = temp;
                room_schedule[r_id][j][size_of_room_schedule[r_id][j]] = &patients[i];
                size_of_room_schedule[r_id][j] = new_size;
            }
            else {
                // Corrected: Allocate memory properly
                room_schedule[r_id][j] = (Patient**)malloc(sizeof(Patient*));
                if (!room_schedule[r_id][j]) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                size_of_room_schedule[r_id][j] = 1;
                room_schedule[r_id][j][0] = &patients[i];
            }
        }
    }
}


void print_room_schedule(void) {
    printf("Room Schedule:\n");

    for (int r = 0; r < num_rooms; ++r) {
        printf("Room %d:\n", r);

        for (int d = 0; d < days; ++d) {
            if (room_schedule[r][d] && size_of_room_schedule[r][d] > 0) {
                printf("  Day %d: ", d);

                for (int p = 0; p < size_of_room_schedule[r][d]; ++p) {
                    printf("Patient %d ", room_schedule[r][d][p]->id);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
}




//void free_3d_array() {
//   if (room_schedule == NULL || size_of_room_schedule == NULL) {
//      return;
//   }
//   for (int i = 0; i < num_rooms; i++) {
//      for (int j = 0; j < days; j++) {
//         if (room_schedule[i][j] != NULL) {
//            free(room_schedule[i][j]);
//         }
//         if (size_of_room_schedule[i][j] != NULL) {
//            free(size_of_room_schedule[i][j]);
//         }
//      }
//      if (room_schedule[i] != NULL) {
//         free(room_schedule[i]);
//      }
//      if (size_of_room_schedule[i] != NULL) {
//         free(size_of_room_schedule[i]);
//      }
//   }
//   free(room_schedule);
//   free(size_of_room_schedule);
//}


void swap_nurse_pointers(Nurses** nurse1, Nurses** nurse2) {
   Nurses* temp = *nurse1;
   *nurse1 = *nurse2;
   *nurse2 = temp;
}

int partition(Nurses** arr, int low, int high) {
   int pivot = arr[high]->shift->max_load;
   int i = low - 1;
   for (int j = low; j <= high - 1; j++) {

      if (arr[j]->shift->max_load > pivot) {
         i++;
         swap_nurse_pointers(&arr[i], &arr[j]);
      }
   }
   swap_nurse_pointers(&arr[i + 1], &arr[high]);
   return i + 1;
}

void quicksort(Nurses** arr, int low, int high) {
   if (low < high) {
      int pi = partition(arr, low, high);
      quicksort(arr, low, pi - 1);
      quicksort(arr, pi + 1, high);
   }
}

void sorting_nurse_id_max_load() {
   for (int i = 0; i < 3 * days; i++) {
      if (current_size_dm_nurse[i] > 1) {
         quicksort(dm_nurses_availability[i], 0, current_size_dm_nurse[i] - 1);
      }
   }
}

void allocate_dm_nurses_availability() {
   dm_nurses_availability = (Nurses***)calloc(days * 3, sizeof(Nurses**));
   max_load_updated = (int**)calloc(num_nurses, (sizeof(int*)));
   if (!dm_nurses_availability) {
      perror("Failed to allocate memory for main array");
      exit(EXIT_FAILURE);
   }
   for (int i = 0; i < days * 3; ++i) {
      dm_nurses_availability[i] = NULL;  // Initialize each slot to NULL
     // max_load_updated[i] = (int*)calloc(days * 3, sizeof(int));
   }
   if (!max_load_updated) {
      perror("Failed to allocate memory for max_load_updated");
      exit(EXIT_FAILURE);
   }
   for (int i = 0; i < num_nurses; ++i) {
      max_load_updated[i] = (int*)calloc(days * 3, sizeof(int));
      if (!max_load_updated[i]) {
         perror("Failed to allocate memory for max_load_updated row");
         exit(EXIT_FAILURE);
      }
      for (int j = 0; j < days * 3; ++j) {
         max_load_updated[i][j] = -1;
      }
   }
}

void allocate_sub_array(Nurses*** main_array, int index, int sub_size) {
   main_array[index] = (Nurses**)calloc(sub_size, sizeof(Nurses*));
   if (!main_array[index]) {
      perror("Failed to allocate memory for sub-array");
      exit(EXIT_FAILURE);
   }
   for (int i = 0; i < sub_size; ++i) {
      main_array[index][i] = NULL; // Initialize subarray elements to NULL
   }
}

void append_to_sub_array(Nurses*** main_array, int index, int* current_sizes, Nurses* nurse) {
   int current_size = current_sizes[index];
   int new_size = current_size + 1;
   Nurses ** temp = (Nurses**)realloc(main_array[index], new_size * sizeof(Nurses*));
   if (temp) {
       main_array[index] = temp;
   }
   else {
       ASSERT(!temp, "Memory allocation error.\n");
   }
   main_array[index][current_size] = nurse; // Add pointer to Nurse structure
   current_sizes[index] = new_size;        // Update current size
}

void initialize_current_size_dm_nurse() {
   current_size_dm_nurse = (int*)calloc(days * 3, sizeof(int));
   if (!current_size_dm_nurse) {
      perror("Failed to allocate memory for current_size_dm_nurse");
      exit(EXIT_FAILURE);
   }
}

void create_dm_nurses_availability() {
   allocate_dm_nurses_availability();
   initialize_current_size_dm_nurse();
   for (int i = 0; i < 3 * days; i++) {
      allocate_sub_array(dm_nurses_availability, i, 2); // Initial subarray size
   }
   for (int i = 0; i < num_nurses; i++) {
      for (int j = 0; j < nurses[i].num_shifts; j++) {
         int index = 3 * nurses[i].shift[j].day + nurses[i].shift[j].shift_time;
         if (index < 0 || index >= 3 * days) {
            fprintf(stderr, "\nInvalid index: %d\n", index);
            continue;
         }
         append_to_sub_array(dm_nurses_availability, index, current_size_dm_nurse, &nurses[i]);
         max_load_updated[i][index] = nurses[i].shift[j].max_load;
         //printf("%d\n" , max_load_updated[i][index]);
      }
   }
}

void free_all_dm_nurses(Nurses*** main_array, int main_size, int* current_sizes) {
   for (int i = 0; i < main_size; ++i) {
      if (main_array[i]) {
         free(main_array[i]); // Free each subarray
      }
   }
   free(main_array);        // Free main array
   free(current_sizes);     // Free size tracking array
}

void print_dm_nurses() {
   for (int i = 0; i < 3 * days; i++) {
      printf("......Shift %d.......\n", i);
      for (int j = 0; j < current_size_dm_nurse[i]; j++) {
         if (dm_nurses_availability[i][j]) {
            printf("id: %d\t max_load: %d\t", dm_nurses_availability[i][j]->id, dm_nurses_availability[i][j]->shift->max_load);
         }
         else {
            printf("NULL\t");
         }
      }
      printf("\n");
   }
}

// ---------------------------------------------function for creating rooms_requirement---------------------------------------------------
void initialize_rooms_req(int num_rooms) {
   rooms_requirement = (Rooms_req**)calloc(num_rooms, sizeof(Rooms_req*));
   if (rooms_requirement == NULL) {
      perror("Failed to allocate memory for rooms_requirement");
      exit(EXIT_FAILURE);
   }
   for (int i = 0; i < num_rooms; i++) {
      rooms_requirement[i] = (Rooms_req*)calloc(3 * days, sizeof(Rooms_req));
      if (rooms_requirement[i] == NULL) {
         perror("Failed to allocate memory for a row");
         exit(EXIT_FAILURE);
      }
      for (int j = 0; j < 3 * days; j++) {
         rooms_requirement[i][j].load_sum = 0;
         rooms_requirement[i][j].max_skill_req = 0;
      }
   }
}

void create_rooms_req() {
    for (int i = 0; i < num_rooms; i++) {
        for (int j = 0; j < days; j++) {
            if (!room_schedule[i][j] || size_of_room_schedule[i][j] == 0) {
                // No patients assigned, set defaults
                for (int x = 0; x < 3; x++) {
                    rooms_requirement[i][3 * j + x].load_sum = 0;
                    rooms_requirement[i][3 * j + x].max_skill_req = 0;
                }
                continue;
            }

            for (int x = 0; x < 3; x++) {
                int sum = 0;
                int max_skill = 0;

                for (int k = 0; k < size_of_room_schedule[i][j]; k++) {
                    Patient* temp = room_schedule[i][j][k];

                    // Ensure valid index before accessing arrays
                    int workload_index = 3 * j + x;
                    int skill_index = 3 * j + x;
                    if (j >= temp->length_of_stay) continue;
                    sum += temp->workload_produced[workload_index];
                    max_skill = (max_skill > temp->skill_level_required[skill_index])
                        ? max_skill
                        : temp->skill_level_required[skill_index];
                }

                // Assign calculated values
                rooms_requirement[i][3 * j + x].load_sum = sum;
                rooms_requirement[i][3 * j + x].max_skill_req = max_skill;
            }
        }
    }
}

void print_rooms_req() {
    for (int i = 0; i < num_rooms; i++) {
        printf("\n==============================\n");
        printf("Room ID: %d\n", i);
        printf("==============================\n");

        for (int j = 0; j < days; j++) {
            printf("  Day %d:\n", j);
            for (int x = 0; x < 3; x++) {
                int index = 3 * j + x;
                printf("    Shift %d -> Load Sum: %d, Max Skill Required: %d\n",
                    x, rooms_requirement[i][index].load_sum,
                    rooms_requirement[i][index].max_skill_req);
            }
        }
    }
}


void cleanup_rooms_req(int num_rooms) {
   for (int i = 0; i < num_rooms; i++) {
      free(rooms_requirement[i]); // Free each row
   }
   free(rooms_requirement); // Free the array of rows
}

void nurse_assignments() {
    for (int i = 0; i < days; i++) {
        for (int j = 0; j < num_rooms; j++) {
            for (int k = 0; k < 3; k++) {
                int m = 0;

                // Skip if room has no load but requires a certain skill level
                if (rooms_requirement[j][3 * i + k].load_sum == 0 && rooms_requirement[j][3 * i + k].max_skill_req)
                    break;

                Nurses** arr = dm_nurses_availability[i * 3 + k];
                int arr_size = current_size_dm_nurse[i * 3 + k];

                if (room[j].nurses_alloted == NULL) {
                    room[j].nurses_alloted = NULL;
                    room[j].length_of_nurses_alloted = 0;
                }

                Nurses* nurse_to_be_assigned = NULL;

                // Find a suitable nurse
                while (m < arr_size) {
                    nurse_to_be_assigned = arr[m];
                    int max_load_of_nurse_to_be_assigned = max_load_updated[nurse_to_be_assigned->id][3 * i + k];

                    if (max_load_of_nurse_to_be_assigned > 0) {
                        break;
                    }
                    m++;
                }

                if (m < arr_size && nurse_to_be_assigned != NULL) {
                    int* temp = (int*)realloc(room[j].nurses_alloted, (room[j].length_of_nurses_alloted + 1) * sizeof(int));
                    if (temp == NULL) {
                        perror("Memory allocation failed for nurses_alloted");
                        exit(EXIT_FAILURE);
                    }

                    room[j].nurses_alloted = temp;
                    room[j].nurses_alloted[room[j].length_of_nurses_alloted++] = nurse_to_be_assigned->id;
                    max_load_updated[nurse_to_be_assigned->id][3 * i + k] -= rooms_requirement[j][3 * i + k].load_sum;

                    // Assign room to nurse's shift, ensuring no duplicates
                    for (int z = 0; z < nurse_to_be_assigned->num_shifts; z++) {
                        if (nurse_to_be_assigned->shift[z].day != i) {
                            continue;
                        }

                        // Check if the room is already assigned
                        bool already_assigned = false;
                        for (int p = 0; p < nurse_to_be_assigned->shift[z].num_rooms; p++) {
                            if (nurse_to_be_assigned->shift[z].rooms[p] == room[j].id) {
                                already_assigned = true;
                                break;
                            }
                        }

                        if (!already_assigned) {
                            int new_size = nurse_to_be_assigned->shift[z].num_rooms + 1;
                            int* temp_room = (int*)realloc(nurse_to_be_assigned->shift[z].rooms, new_size * sizeof(int));

                            if (temp_room) {
                                nurse_to_be_assigned->shift[z].rooms = temp_room;
                                nurse_to_be_assigned->shift[z].rooms[nurse_to_be_assigned->shift[z].num_rooms] = room[j].id;
                                nurse_to_be_assigned->shift[z].num_rooms++;
                            }
                            else {
                                fprintf(stderr, "Memory allocation error for nurse shift rooms.\n");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                }
                else {
                    fprintf(stderr, "No valid nurse available for Room %d, Shift %d\n", j, 3 * i + k);
                }
            }
        }
    }
}

void print_max_loadd_updated() {
   for (int i = 0; i < num_nurses; i++) {
      printf("Max_load of the nurse %d\n", i);
      for (int j = 0; j < 3 * days; j++) {
         printf("%d\t", max_load_updated[i][j]);
      }
      printf("\n");
   }
}

//..................................FUNCTION FOR OUTPUT JSON FILE....................

void create_json_file(Patient* patients, int num_patients, Nurses* nurse, int num_nurses, int num_rooms, const char* instance_name, const char* output_folder) {
    // Create output directory
#ifdef _WIN32
    if (_mkdir(output_folder) != 0) perror("Error creating folder");
#else
    if (mkdir(output_folder, 0777) != 0) perror("Error creating folder");
#endif

    char filepath[200];
    snprintf(filepath, sizeof(filepath), "%s/%s_solution.json", output_folder, instance_name);

    FILE* file = fopen(filepath, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Calculate ID padding sizes
    int patient_digits = (num_patients > 1) ? ((int)log10(num_patients) + 1) : 1;
    int nurse_digits = (num_nurses > 1) ? ((int)log10(num_nurses) + 1) : 1;
    int room_digits = (num_rooms > 1) ? ((int)log10(num_rooms) + 0) : 1;

    // Write JSON data
    fprintf(file, "{");

        // Patients array
        fprintf(file, "  \"patients\": [\n");
    for (int i = 0; i < num_patients; i++) {
        fprintf(file, "    {\n");
        fprintf(file, "      \"id\": \"p%0*d\",\n", patient_digits, patients[i].id);
        if (patients[i].admission_day != -1) {
            fprintf(file, "      \"admission_day\": %d,\n", patients[i].admission_day);
            fprintf(file, "      \"room\": \"r%0*d\",\n", room_digits, patients[i].assigned_room_no);
            fprintf(file, "      \"operating_theater\": \"t%d\"\n", patients[i].assigned_ot);
        }
        else {
            fprintf(file, "      \"admission_day\": \"none\"\n");
        }
        fprintf(file, "    }%s\n", (i < num_patients - 1) ? "," : "");
    }
    fprintf(file, "  ],\n");

    // Nurses array
    fprintf(file, "  \"nurses\": [\n");
    for (int i = 0; i < num_nurses; i++) {
        fprintf(file, "    {\n");
        fprintf(file, "      \"id\": \"n%0*d\",\n", nurse_digits, nurses[i].id);
        fprintf(file, "      \"assignments\": [\n");

        for (int j = 0; j < nurses[i].num_shifts; j++) {
            fprintf(file, "        {\n");
            fprintf(file, "          \"day\": %d,\n", nurses[i].shift[j].day);
            fprintf(file, "          \"shift\": \"%s\",\n", shift_types_to_string(nurses[i].shift[j].shift_time));
            fprintf(file, "          \"rooms\": [\n");

            for (int k = 0; k < nurses[i].shift[j].num_rooms; k++) {
                fprintf(file, "            \"r%0*d\"%s\n", room_digits, nurses[i].shift[j].rooms[k], (k < nurses[i].shift[j].num_rooms - 1) ? "," : "");
            }
            fprintf(file, "          ]\n");
            fprintf(file, "        }%s\n", (j < nurses[i].num_shifts - 1) ? "," : "");
        }

        fprintf(file, "      ]\n");
        fprintf(file, "    }%s\n", (i < num_nurses - 1) ? "," : "");
    }
    fprintf(file, "  ]\n");
    fprintf(file, "}\n");

    fclose(file);
    printf("JSON file saved at: %s/%s_solution.json\n", output_folder, instance_name);
}

// -----------------------------------------------------BELOW: PATIENT ADMISSION------------------------------------------------------

//..............................................FUNCTION BELOW ARE FOR PATIENT ADMISSION...........................................


void init_Rooms(RoomVector* vector, int initialcap) {
    vector->data = (Rooms**)calloc(initialcap, sizeof(Rooms*));
    if (!vector->data) {
        printf("\nMemory allocation error.");
        return;
    }
    vector->size = 0;
    vector->capacity = initialcap;
}

void pushback(RoomVector* vector, Rooms* room) {
    // add a room to a room vector
    if (vector->size == vector->capacity) {
        int new_capacity = vector->capacity * 2;
        Rooms** temp = realloc(vector->data, new_capacity * sizeof(Rooms*));
        if (temp) {
            vector->data = temp;
            vector->capacity = new_capacity;
        }
        else {
            fprintf(stderr, "Memory allocation failed in resize\n");
            return;
        }
    }
    vector->data[vector->size++] = room;
}

Rooms* removebyid(RoomVector* vector, int room_id) {
    // remove a room from a room vector given its room id
    for (int i = 0; i < vector->size; i++) {
        if (vector->data[i]->id == room_id) {
            Rooms* room = vector->data[i];

            // Shift elements left
            for (int j = i; j < vector->size - 1; j++) {
                vector->data[j] = vector->data[j + 1];
            }

            vector->size--;  // Reduce size
            return room;
        }
    }
    return NULL;
}

void moveRoom(RoomVector* source, RoomVector* destination, int room_id) {
    // take out a room (by room id) from a room vector and append it to another vector
    Rooms* room = removebyid(source, room_id);
    if (room)
        pushback(destination, room);
    else
        ASSERT(!room, "No room available.\n");
}

void freevector(RoomVector* vector) {
    if (vector) {
        free(vector->data);
        free(vector);  // Free the struct itself
    }
}

void make_3_vectors(int* room_gender_map) {
    init_Rooms(v_A, 5);
    init_Rooms(v_B, 5);
    init_Rooms(v_empty, 5);

    for (int i = 0; i < num_rooms; i++) {
        if (room_gender_map[i] == -1)
            pushback(v_empty, room+i);
        else
            if (room_gender_map[i] == 0)
                pushback(v_A, room+i);
        else
            pushback(v_B, room+i);
    }
}

void printVector(const char* label, RoomVector* vector) {
    // prints a room vector given its room vector pointer
    puts(label);
    for (int i = 0; i < vector->size; i++) {
        printf("Room ID: %d, Capacity: %d\n", vector->data[i]->id, vector->data[i]->cap);
    }
}

int find_max_surgeon_id(Node* head) {
   int max_s_id = -1, i;
   Node* self = head;
   while (self) {
      if (self->pointer->surgeon_id > max_s_id) {
         max_s_id = self->pointer->surgeon_id;
      }
      self = self->next;
   }

   return max_s_id;
}

int findSuitableRoom(int p_id, RoomVector* vector) {
   // return - room_id OR -1 in case no room is suitable
   // assign available Room and return the room_id
   // considerations - capacity, gender, compatibility & age-mix
   // do something so that the sum_workload_of_a_room does not exceed too much as that will create issues while assigning nurses
   int flag = 0, i, j, r_id, g = patients[p_id].gen, n;
   char* age = patients[p_id].age_group;
  // GenderRoom* self, * prev_ptr, * temp;

   for (int i = 0; i < vector->size; i++) {
       flag = 0;
       r_id = vector->data[i]->id;
       if (room[r_id].cap <= (room[r_id].num_patients_allocated + room[r_id].occupants_cap)) continue;
       for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
           if (r_id == patients[p_id].incompatible_room_ids[j]) {
               flag = 1; break;
           }
       }

       if (flag) continue;
       else return r_id;

   }
       //if no room was found in the associated gender array then look for a room in empty room array.

    if (patients[p_id].assigned_room_no == -1) {
        for (i = 0; i < v_empty->size; i++) {
            r_id = v_empty->data[i]->id;
            if (room[r_id].cap <= (room[r_id].num_patients_allocated + room[r_id].occupants_cap)) continue;
            for (j = 0; j < patients[p_id].num_incompatible_rooms; ++j)
                if (r_id == patients[p_id].incompatible_room_ids[j]) { flag = 1; break; }
            if (flag) continue;
            moveRoom(v_empty, vector, r_id);

            return r_id;
        }
    }
    return -1;
}


void update_LOS_of_patients(int d, int *chromosome) {
   /* Check if any patient's length_of_stay is over and if yes then remove that patient from -
       1. the room he's been assigned to.
       2. any assignment in the NRA problem pertaining to this patient.
   */

   int i, g, r_id, admit_day, los, days_passed, gene_p_id;
   Node* p, * self;
   GenderRoom* gen_array;
   Occupants occ;

   // first consider the occupants -
   // if an occupant's LOS is over then throw him out of the room and make space of a patient.
   for (i = 0; i < num_occupants; ++i) {
      occ = occupants[i];
      r_id = occ.room_id;
      g = occ.gen;
      days_passed = d - occ.length_of_stay;
      if (days_passed > 0) {
         // throw the occupant out
          if(room[r_id].occupants_cap > 0)
         room[r_id].occupants_cap--;
         if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
             // remove the room from the associated gender array and append at the head of the empty_array linked list
             //g ? removeAndAppendGenderRoom(r_id, gender_B_rooms) : removeAndAppendGenderRoom(r_id, gender_A_rooms);
             g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
         }
      }
   }

    for (i=0; i<CHROMOSOME_SIZE; ++i) {
        gene_p_id = chromosome[i];
        admit_day = patients[gene_p_id].admission_day;
        r_id = patients[gene_p_id].assigned_room_no;
        g = patients[gene_p_id].gen;
        los = patients[gene_p_id].length_of_stay;
        if (admit_day != -1) { // if true - means the patient has been admitted.
            days_passed = d - admit_day;
            if (days_passed >= los) {
                room[r_id].num_patients_allocated--;
                if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated)
                    g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
            }
        }
    }

    /*
   for (i = 1; i <= d; ++i) {
      p = sorted_mandatory_patients[i];
      // parse the sorted_mandatory_patients array (arr of linked lists pointers)
      for (self = p; self; self = self->next) {
         // extract the data about patient
         admit_day = self->pointer->admission_day;
         r_id = self->pointer->assigned_room_no;
         g = self->pointer->gen;
         los = self->pointer->length_of_stay;
         // release_day = self->pointer->surgery_release_day;
         // due_day = self->pointer->surgery_due_day;

         if (admit_day != -1) { // if true - means the patient has been admitted.
            days_passed = d - admit_day;
            if (days_passed >= los) {
                // release the patient
                // remove this patient's data from everywhere, i.e.
                // from room (capacity, gender, age-mix)
                // update the data in nurses array about this patient
                room[r_id].num_patients_allocated--;
                // ignore age-mix
                // consider if the room is now empty then what will you do
                if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
                    // remove this room from the associated gender array and append it in empty_rooms array
                    g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
                }
            }
         }
      }
   }
   */
}

typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;

void init_Mand_opt_PQ(Mand_opt_PQ* vector, int initial_cap) {
    vector->data = (HeapNode**)calloc(initial_cap, sizeof(HeapNode*));
    if (!vector->data) {
        ASSERT(!vector->data, "No memory allocated for Mand_opt_PQ\n");
    }
    vector->capacity = initial_cap;
    vector->size = 0;
}

void resizevector(Mand_opt_PQ* vector) {
    vector->capacity *= 2;
    HeapNode** data = (HeapNode**)realloc(vector->data, vector->capacity * sizeof(HeapNode*));
    if (!data) {
        ASSERT(!data, "No memory allocated for the Mand_opt_PQ.\n");
    }
    vector->data = data;
}

void pushback_Mand_Opt_PQ(Mand_opt_PQ* vector, HeapNode* node) {
    if (vector->size == vector->capacity) {
        resizevector(vector);
    }
    vector->data[vector->size++] = node;
}

HeapNode* pop_Mand_opt_PQ(Mand_opt_PQ* vector) {
    if (vector->size == 0) {
        return NULL;  // Avoid popping from an empty vector
    }
    return vector->data[--vector->size];  // Decrement size after access
}

OTs* admitFromPQ(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt) {
    HeapNode *node;
    int i, j, p_id, flag, r_id, s_duration, p_counter;

    Mand_opt_PQ *vector = (Mand_opt_PQ *) calloc (1, sizeof(Mand_opt_PQ));
    init_Mand_opt_PQ(vector, 20);

    /*
    if (flag_opt) {
        for (i = 0; pq->current_size > 0; i++) {
            node = (HeapNode *) calloc (1, sizeof(HeapNode));  // Allocate new node
            *node = extractMaxFromPQ(pq);  // Copy extracted node data
            pushback_Mand_Opt_PQ(vector, node);
        }
    }
    */

    for (p_counter = 0, flag = 0; p_counter < pq->current_size; ++p_counter) {
        if (!flag_opt && !(pq->data[p_counter].mandatory)) continue;
        else {
            node = (HeapNode *) calloc (1, sizeof(HeapNode));  // Allocate new node
            *node = extractMaxFromPQ(pq);  // Copy extracted node data
            p_id = node->patient_id;
            s_duration = patients[p_id].surgery_duration;

            while (!current_ot->time_left[d] || current_ot->time_left[d] < s_duration) {
                current_ot = ot_data_arr[current_ot_index];
                if (current_ot_index < num_ots)
                    current_ot_index++;
                if (current_ot_index == num_ots) {
                    flag = 1;
                    current_ot_index--;
                    break;
                }
            }

            if (flag) {
                pushback_Mand_Opt_PQ(vector, node);
                continue;
            }
            else {
                r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                if (r_id == -1) {
                    pushback_Mand_Opt_PQ(vector, node);
                    continue;
                }
                else {
                    if (surgeon[s_data_arr[i]->surgeon_id].time_left[d] >= patients[p_id].surgery_duration) {
                        patients[p_id].assigned_room_no = r_id;
                        patients[p_id].assigned_ot = current_ot->id;
                        room[r_id].num_patients_allocated++;
                        current_ot->time_left[d] -= s_duration;
                        patients[p_id].admission_day = d;
                    }
                    else
                        pushback_Mand_Opt_PQ(vector, node);
                }
            }
        }
    }

    for (int i = 0; i < vector->size; i++) {
        insertNodeInPQ(pq, *(vector->data[i]));
        free(vector->data[i]);  // Free dynamically allocated HeapNode
    }
    free(vector->data);  // Free vector data array
    free(vector);  // Free vector

    return current_ot;
}

// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------

//-------------------------------------------------BELOW: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

int admit_patients(int* room_gender_map, PriorityQueue* pq, int *chromosome)
{   int i, j, p_id, s_id, r_id, admitted_mandatory_count = 0, day, p_counter;
    int unscheduled_mandatory = 0, assigned_ot, flag;
    OTs **ot_data_arr, *current_ot;

    v_A = (RoomVector*) calloc (1, sizeof(RoomVector));
    v_B = (RoomVector*) calloc (1, sizeof(RoomVector));
    v_empty = (RoomVector*) calloc (1, sizeof(RoomVector));

    make_3_vectors(room_gender_map);
    printVector("A", v_A);
    printVector("B", v_B);
    printVector("Empty", v_empty);

    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    print_ots(ot);
    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = ot+i;

    //----------------------------------------------------------apply checks and ADMIT PATIENTS------------------------------------------------------

    for (day=0, p_counter=0; day < days; ++day) {
        current_ot_index = 0;
        sort_ot_data_arr(ot_data_arr, day);
        current_ot = ot_data_arr[current_ot_index];

        // try to admit the patients in PQ first
        current_ot = admitFromPQ(pq, dy, ot_data_arr, current_ot, current_ot_index, 0);
        if (!current_ot) {
            continue;
        }

        for (; p_counter < CHROMOSOME_SIZE; ++p_counter) {
            assigned_ot = -1;
            p_id = chromosome[p_counter];
            // if the release day is greater than current_day (day) - go to the next day and then try to admit this patient
            if (day < mandatory_patients[p_id]->surgery_release_day) {
                ++p_counter;
                break;
            }
            // if current_day (day) is greater than the due day of the patient - increase the unscheduled_mandatory count and go to the next patient
            if (day > mandatory_patients[p_id]->surgery_due_day) {
                ++unscheduled_mandatory;
                continue;
            }
            if (mandatory_patients[p_id]->surgery_release_day <= day && day <= mandatory_patients[p_id]->surgery_due_day) {
                s_id = mandatory_patients[p_id]->surgeon_id;
                // check if the surgeon is available
                if (surgeon[s_id].time_left[day] < mandatory_patients[p_id]->surgery_duration) {
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id].surgery_due_day, 0));
                    continue;
                }
                // check if the OT is available
                if (current_ot->time_left[day] < mandatory_patients[p_id]->surgery_duration) {
                    for (j=0; j<num_ots && assigned_ot == -1; ++j) {
                        if (ot_data_arr[j]->time_left[day] >= mandatory_patients[p_id]->surgery_duration)
                            assigned_ot = ot_data_arr[j]->id;
                    }
                    if (assigned_ot != -1) {
                        current_ot_index = j;
                        current_ot = ot_data_arr[current_ot_index];
                        ot_data_arr[current_ot_index]->time_left -= mandatory_patients[p_id]->surgery_duration;
                    }
                    else {
                        // if true - it means no ot could be assigned to this patient - put him in PQ
                        insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
                        continue;
                    }
                }
                else {
                    assigned_ot = current_ot->id;
                    ot_data_arr[current_ot_index]->time_left -= mandatory_patients[p_id]->surgery_duration;
                }

                // if the control comes here - that means surgeon and OT are available and only room remains
                // look for a suitable room
                r_id = (mandatory_patients[p_id].gen) ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                if (r_id != -1) {
                    room[r_id].num_patients_allocated++;
                    mandatory_patients[p_id]->admission_day = day;
                    mandatory_patients[p_id]->assigned_ot = assigned_ot;
                    mandatory_patients[p_id]->assigned_room_no = r_id;
                }
                else
                    // put the patient in the PQ
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id]->surgery_due_day, 0));
            }
            else
                if (day < mandatory_patients[p_id]->surgery_due_day) {
                    insertNodeInPQ(pq, makeHeapNode(p_id, 1, mandatory_patients[p_id].surgery_due_day, 0));
                    continue;
                }
        }
    }
    return unscheduled_mandatory;
}

//----------------------------------------------------ABOVE: FUNCTIONS FOR PATIENT ADMISSION----------------------------------------------------------

//-----------------------------------------------------------BELOW: GENETIC ALGORITHM-----------------------------------------------------------------

const int POPULATION_SIZE = 100, NUM_ITER = 10000, CONVERGENCE_STOPPING_CRITERION = 100;
int **POPULATION, *G_BEST, CHROMOSOME_SIZE;
int **CROSSOVER_OFFSPRING_STORAGE_PLACE, **CROSSOVER_PARENT_STORAGE_PLACE;
int *MUTATED_OFFSPRING_STORAGE_PLACE, *MUTATE_PARENT_STORAGE_PLACE;

//-------------------------------------------------------BELOW: FUNCTION PROTOTYPES FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(void);
int evaluateFitnessScore(int *chromosome);
void orderCrossover(void);
void swapMutation(void);
void generateNewChromosome(int chromo_num);
void generatePopulation(void);
void crossoverTournamentSelection(void);
void mutationTournamentSelection(void);
void crossoverElitism(void);
void mutationElitism(void);
void initDataStructures(void);
void freeDataStructures(void);
void printPopulation(void);

//-------------------------------------------------------ABOVE: FUNCTION PROTOTYPES FOR GA-----------------------------------------------------------

void applyGeneticAlgorithm(void)
{   int i, j, best_fitness, g_best;
    unsigned int same_fitness_iter;
    float p_c = 0.8;

    generatePopulation();
    for (i=0; i<POPULATION_SIZE; ++i)
        POPULATION[i][CHROMOSOME_SIZE] = evaluateFitnessScore(POPULATION[i]);
    memcpy(G_BEST, POPULATION[0], (CHROMOSOME_SIZE + 1) * sizeof(int));
    g_best = G_BEST[CHROMOSOME_SIZE];
    printPopulation();

    //-------------------------------------------------------START ITERATION-----------------------------------------------------------------

    for (i=0, same_fitness_iter=0; i != NUM_ITER && same_fitness_iter != CONVERGENCE_STOPPING_CRITERION; ++i) {
        if ((rand() / (float) RAND_MAX) <= p_c) {
            crossoverTournamentSelection();
            orderCrossover();
            // calculate the fitness of the 2 new offsprings
            CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE] = evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[0]);
            CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE] = evaluateFitnessScore(CROSSOVER_OFFSPRING_STORAGE_PLACE[1]);
            crossoverElitism();
            for (j=0; j<2; ++j)
                if (CROSSOVER_OFFSPRING_STORAGE_PLACE[j][CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                    memcpy(G_BEST, CROSSOVER_OFFSPRING_STORAGE_PLACE[j], sizeof(int) * (CHROMOSOME_SIZE + 1));
        }
        else {
            mutationTournamentSelection();
            swapMutation();
            // calculate the fitness of the new offspring
            MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] = evaluateFitnessScore(MUTATED_OFFSPRING_STORAGE_PLACE);
            mutationElitism();
            if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > G_BEST[CHROMOSOME_SIZE])
                memcpy(G_BEST, MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE +1));
        }
        // Checking for CONVERGENCE_STOPPING_CRITERION
        best_fitness = G_BEST[CHROMOSOME_SIZE];
        if (best_fitness == g_best)
            same_fitness_iter++;
        else
            same_fitness_iter = 0;
        g_best = G_BEST[CHROMOSOME_SIZE];
    }
    puts("\nChecking whether the iterations were over OR the algorithm converged: ");
    printf("Number of iterations: %d\tsame_fitness_iter: %d", i, same_fitness_iter);
    // either i will be NUM_ITER (10000) OR same_fitness_iter will be CONVERGENCE_STOPPING_CRITERION (100).
    // G_BEST is the best chromosome - use it for admitting patients.
}

int evaluateFitnessScore(int *chromosome, PriorityQueue *pq)
{   // fitness score is the number of mandatory patients who were admitted in the scheduling period.
    // The best fitness score is CHROMOSOME_SIZE
    `int total_unscheduled_mandatory;

    unscheduled_mandatory = admit_patients(room_gender_map, pq, chromosome);
    total_unscheduled_mandatory = unscheduled_mandatory + pq->current_size;

    // FITNESS = MANDATORY PATIENTS WHO WERE ADMITTED DURING THE SCHEDULING PERIOD
    return (mandatory_count - total_unscheduled_mandatory);
}

void orderCrossover(void)
{   // Apply Denver's Order Crossover on the two crossover_parents and generate two new offsprings.
    int r1, r2, i;

    // Select two random crossover points
    do {
        r1 = rand() % CHROMOSOME_SIZE;
        r2 = rand() % CHROMOSOME_SIZE;
    } while (r1 == r2);

    if (r1 > r2) {
        r1 += r2;
        r2 = r1 - r2;
        r1 -= r2;
    }

    // Copy parents to offspring storage directly
    memcpy(CROSSOVER_OFFSPRING_STORAGE_PLACE[0], CROSSOVER_PARENT_STORAGE_PLACE[0], (CHROMOSOME_SIZE + 1) * sizeof(int));
    memcpy(CROSSOVER_OFFSPRING_STORAGE_PLACE[1], CROSSOVER_PARENT_STORAGE_PLACE[1], (CHROMOSOME_SIZE + 1) * sizeof(int));

    // Swap genes outside the crossover segment
    for (i = 0; i < CHROMOSOME_SIZE; ++i) {
        if (i >= r1 && i <= r2)
            continue;
        CROSSOVER_OFFSPRING_STORAGE_PLACE[0][i] = CROSSOVER_PARENT_STORAGE_PLACE[1][i];
        CROSSOVER_OFFSPRING_STORAGE_PLACE[1][i] = CROSSOVER_PARENT_STORAGE_PLACE[0][i];
    }
}

void swapMutation(void)
{   // take the offspring from MUTATED_OFFSPRING_STORAGE_PLACE and mutate it using SWAP MUTATION method
    int r1, r2;

    do {
        r1 = rand() % CHROMOSOME_SIZE;
        r2 = rand() % CHROMOSOME_SIZE;
    } while (r1 == r2);

    MUTATED_OFFSPRING_STORAGE_PLACE[r1] += MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r2] = MUTATED_OFFSPRING_STORAGE_PLACE[r1] - MUTATED_OFFSPRING_STORAGE_PLACE[r2];
    MUTATED_OFFSPRING_STORAGE_PLACE[r1] -= MUTATED_OFFSPRING_STORAGE_PLACE[r2];
}

void generateNewChromosome(int chromo_num)
{   int j, r1, r2;

    void swapGenes(int chromo_num, int r1, int r2)
    {   POPULATION[chromo_num][r1] += POPULATION[chromo_num][r2];
        POPULATION[chromo_num][r2] = POPULATION[chromo_num][r1] - POPULATION[chromo_num][r2];
        POPULATION[chromo_num][r1] -= POPULATION[chromo_num][r2];
    }

    // copy all the genes from (chrmo_num-1)th chrmosome to (chromo_num)th chromosome
    for (j=0; j<CHROMOSOME_SIZE; ++j)
        POPULATION[chromo_num][j] = POPULATION[chromo_num-1][j];

    for (j=0; j<CHROMOSOME_SIZE; ++j) {
        do {
            r1 = rand() % (CHROMOSOME_SIZE);
            r2 = rand() % (CHROMOSOME_SIZE);
        } while (r1 == r2);

        swapGenes(chromo_num, r1, r2);
    }
}

void generatePopulation(void)
{   int i, j;

    // Copy mandatory patients' IDs into the first chromosome
    for (j=0; j<CHROMOSOME_SIZE; ++j)
        POPULATION[0][j] = mandatory_patients[j]->id;

    // Generate new chromosomes for the rest
    for (i = 1; i < POPULATION_SIZE; ++i)
        generateNewChromosome(i);
}

void crossoverTournamentSelection(void)
{   // select 2 parents using Tournament Selection method
    int r11, r12, r13, r21, r22, r23, f11, f12, f13, f21, f22, f23;
    int best_fitness, best_fitness_idx1, best_fitness_idx2;

    // select first parent
    do {
        r11 = rand() % (POPULATION_SIZE);
        r12 = rand() % (POPULATION_SIZE);
        r13 = rand() % (POPULATION_SIZE);
    } while (r11 == r12 || r12 == r13 || r11 == r13);

    // select the chromosome1 with the best fitness
    f11 = POPULATION[r11][CHROMOSOME_SIZE];
    f12 = POPULATION[r12][CHROMOSOME_SIZE];
    f13 = POPULATION[r13][CHROMOSOME_SIZE];
    best_fitness = f11;
    best_fitness_idx1 = r11;

    if (f12 > best_fitness) {
        if (f13 > f12) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx1 = r12;
        }
    }
    else
        if (f13 > best_fitness) {
            best_fitness = f13;
            best_fitness_idx1 = r13;
        }


    // select second parent
    do {
        r21 = rand() % (POPULATION_SIZE);
        r22 = rand() % (POPULATION_SIZE);
        r23 = rand() % (POPULATION_SIZE);
    } while (r21 == r22 || r22 == r23 || r21 == r23 ||
             r11 == r21 || r11 == r22 || r11 == r23 ||
             r12 == r21 || r12 == r22 || r12 == r23 ||
             r13 == r21 || r13 == r22 || r13 == r23);

    // select the chromosome2 with the best fitness
    f21 = POPULATION[r21][CHROMOSOME_SIZE];
    f22 = POPULATION[r22][CHROMOSOME_SIZE];
    f23 = POPULATION[r23][CHROMOSOME_SIZE];
    best_fitness = f21;
    best_fitness_idx2 = r21;

    if (f22 > best_fitness) {
        if (f23 > f22) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
        else {
            best_fitness = f22;
            best_fitness_idx2 = r22;
        }
    }
    else
        if (f23 > best_fitness) {
            best_fitness = f23;
            best_fitness_idx2 = r23;
        }
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[i], POPULATION[best_fitness_idx1], (CHROMOSOME_SIZE + 1) * sizeof(int));
    memcpy(CROSSOVER_PARENT_STORAGE_PLACE[1], POPULATION[best_fitness_idx2], (CHROMOSOME_SIZE + 1) * sizeof(int));
}

void mutationTournamentSelection(void)
{   // select 2 parents using Tournament Selection method
    int r11, r12, r13, f11, f12, f13;
    int best_fitness, best_fitness_idx;

    // select first parent
    do {
        r11 = rand() % (POPULATION_SIZE);
        r12 = rand() % (POPULATION_SIZE);
        r13 = rand() % (POPULATION_SIZE);
    } while (r11 == r12 || r12 == r13 || r11 == r13);

    // select the chromosome1 with the best fitness
    f11 = POPULATION[r11][CHROMOSOME_SIZE];
    f12 = POPULATION[r12][CHROMOSOME_SIZE];
    f13 = POPULATION[r13][CHROMOSOME_SIZE];
    best_fitness = f11;
    best_fitness_idx = r11;

    if (f12 > best_fitness) {
        if (f13 > f12) {
            best_fitness = f13;
            best_fitness_idx = r13;
        }
        else {
            best_fitness = f12;
            best_fitness_idx = r12;
        }
    }
    else
        if (f13 > best_fitness) {
            best_fitness = f13;
            best_fitness_idx = r13;
        }

    memcpy(MUTATE_PARENT_STORAGE_PLACE, POPULATION[best_fitness_idx], (CHROMOSOME_SIZE + 1) * sizeof(int));
}

void crossoverElitism(void)
{   int i, worst_fitness_chromosome_index = 0, second_worst_fitness_chromosome_index = 0;

    for (i=1; i<POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
            worst_fitness_chromosome_index = i;

    for (i=1; i<POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE] &&
            i != worst_fitness_chromosome_index)
            second_worst_fitness_chromosome_index = i;

    // replace the offsprings with the worst chromosomes
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[0][CHROMOSOME_SIZE] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[0], sizeof(int) * (CHROMOSOME_SIZE + 1));
    if (CROSSOVER_OFFSPRING_STORAGE_PLACE[1][CHROMOSOME_SIZE] > POPULATION[second_worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[second_worst_fitness_chromosome_index], CROSSOVER_OFFSPRING_STORAGE_PLACE[1], sizeof(int) * (CHROMOSOME_SIZE + 1));
}

void mutationElitism(void)
{   int i, worst_fitness_chromosome_index = 0;

    for (i=1; i<POPULATION_SIZE; ++i)
        if (POPULATION[i][CHROMOSOME_SIZE] < POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
            worst_fitness_chromosome_index = i;

    // replace the offspring with the worst chromosome
    if (MUTATED_OFFSPRING_STORAGE_PLACE[CHROMOSOME_SIZE] > POPULATION[worst_fitness_chromosome_index][CHROMOSOME_SIZE])
        memcpy(POPULATION[worst_fitness_chromosome_index], MUTATED_OFFSPRING_STORAGE_PLACE, sizeof(int) * (CHROMOSOME_SIZE + 1));
}

void initDataStructures(void)
{   MUTATED_OFFSPRING_STORAGE_PLACE = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    MUTATE_PARENT_STORAGE_PLACE = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    if (!MUTATED_OFFSPRING_STORAGE_PLACE || !MUTATE_PARENT_STORAGE_PLACE)
        ASSERT (0, "Dynamic Memory Allocation Error for MUTATED_STORAGE_PLACE");

    CROSSOVER_OFFSPRING_STORAGE_PLACE = (int **) calloc (2, sizeof(int *));
    CROSSOVER_OFFSPRING_STORAGE_PLACE[0]  = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    CROSSOVER_OFFSPRING_STORAGE_PLACE[1]  = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    if (!CROSSOVER_OFFSPRING_STORAGE_PLACE || !CROSSOVER_OFFSPRING_STORAGE_PLACE[0] || !CROSSOVER_OFFSPRING_STORAGE_PLACE[1])
        ASSERT (0, "Dynamic Memory Allocation Error for CROSSOVER_OFFSPRING_STORAGE_PLACE");

    CROSSOVER_PARENT_STORAGE_PLACE = (int **) calloc (2, sizeof(int *));
    CROSSOVER_PARENT_STORAGE_PLACE[0]  = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    CROSSOVER_PARENT_STORAGE_PLACE[1]  = (int *) calloc (CHROMOSOME_SIZE+1, sizeof(int));
    if (!CROSSOVER_PARENT_STORAGE_PLACE || !CROSSOVER_PARENT_STORAGE_PLACE[0] || !CROSSOVER_PARENT_STORAGE_PLACE[1])
        ASSERT (0, "Dynamic Memory Allocation Error for CROSSOVER_PARENT_STORAGE_PLACE");

    G_BEST = (int *) calloc(CHROMOSOME_SIZE + 1, sizeof(int));
    if (!G_BEST)
        ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR G_BEST");

    POPULATION = (int **) calloc(POPULATION_SIZE, sizeof(int *));
    if (!POPULATION)
        ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR POPULATION");

    for (i = 0; i < POPULATION_SIZE; ++i) {
        POPULATION[i] = (int *) calloc(CHROMOSOME_SIZE + 1, sizeof(int));
        if (!POPULATION[i])
            ASSERT(0, "DYNAMIC MEMORY ALLOCATION ERROR FOR A CHROMOSOME");

        // Set last column to -1
        POPULATION[i][CHROMOSOME_SIZE] = -1;
    }
}

void freeDataStructures(void)
{   free(MUTATED_OFFSPRING_STORAGE_PLACE);
    free(MUTATED_OFFSPRING_STORAGE_PLACE);

    free(CROSSOVER_OFFSPRING_STORAGE_PLACE[0]);
    free(CROSSOVER_OFFSPRING_STORAGE_PLACE[1]);
    free(CROSSOVER_OFFSPRING_STORAGE_PLACE);

    free(CROSSOVER_PARENT_STORAGE_PLACE[0]);
    free(CROSSOVER_PARENT_STORAGE_PLACE[1]);
    free(CROSSOVER_PARENT_STORAGE_PLACE);

    free(G_BEST);

    for (i=0; i<POPULATION_SIZE; ++i)
        free(POPULATION[i]);
    free(POPULATION);
}

void printPopulation(void)
{   int i, j;
    for (i=0; i<POPULATION_SIZE; ++i) {
        for (j=0; j<CHROMOSOME_SIZE; ++j)
            printf("%d ", POPULATION[i][j]);
        putch('\n');
    }
}

//---------------------------------------------------------ABOVE: GENETIC ALGORITHM-------------------------------------------------------------

int main(void) {
    parse_json("C:/sant_saran/C in One/IHTC/VS_Code/EA/test09.json");
    PriorityQueue* pq;
    srand(time(0));
    initDataStructures();
    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
    if (!pq)
        ASSERT(0, "Dynamic Memory Allocation Error for PQ");

    CHROMOSOME_SIZE = mandatory_count;
    initialize_room_gender_map(&room_gender_map);
    initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
    populate_room_gender_map(room_gender_map);
    // printf("\nroom: 6\tgender: %d", room_gender_map[6]);
    //print_map(room_gender_map);

    //sort_mandatory_patients_on_release_day(mandatory_patients, mandatory_count);
    //sort_mandatory_patients_on_LOS();
    //print_sorted_mandatory_array();
    //sort_optional_patients_on_release_day(optional_patients, optional_count);
    //append_optional_to_mandatory(sorted_mandatory_patients, sorted_optional_patients);
    //print_sorted_optional_array();
    //print_ots(ot);
    admit_patients(room_gender_map, pq);
    applyGeneticAlgorithm();
    freeDataStructures();

    /*
    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    create_3d_array();
    print_room_schedule();
    initialize_rooms_req(num_rooms);
    create_rooms_req();
    print_rooms_req();
    nurse_assignments();

    // print_dm_nurses();

    // print_mandatory_patients();
    //print_sorted_mandatory_array();
    //print_sorted_mandatory_patients();
    */
   create_json_file(patients , num_patients , nurses , num_nurses,num_rooms, "i05","D:/major_code/build/output");
   // Use the parsed data in your algorithm
    int *surgery_time[num_surgeons][days];
    printf("Weights:\n");
    printf("Room Mixed Age: %d\n", weights->room_mixed_age);
    printf("Room Nurse Skill: %d\n", weights->room_nurse_skill);
    printf("Continuity of Care: %d\n", weights->continuity_of_care);
    printf("Nurse Excessive Workload: %d\n", weights->nurse_excessive_workload);
    printf("Open Operating Theater: %d\n", weights->open_operating_theater);
    printf("Surgeon Transfer: %d\n", weights->surgeon_transfer);
    printf("Patient Delay: %d\n", weights->patient_delay);
    printf("Unscheduled Optional: %d\n", weights->unscheduled_optional);

    /*
    print_surgeons(surgeon);
    print_ots(ot);
    print_rooms();
    print_nurses();
    print_patients(patients);
    print_occupants();
    print_mandatory_patients();
    print_optional_patients();
    */

    //Free allocated memory
    // free_patients_sorted_array(sorted_mandatory_patients);
    //free_patients_sorted_array(sorted_optional_patients);
    free_occupants();
    free_patients();
    free_surgeons();
    free_ots();
    free_rooms();
    //free_nurses();
    free(weights);

    return 0;
}


/*
UPDATES SECTION: Here we will put all the updates related to this program. Whether it's about the heuristic or anything else.
1. Wrote code for admit_patients function and some utility functions
2. Working on: written in admit_patients function
3. Surgeons sorting function needs updation - we have to account for null values as well.
*/

