/*
CHANGES MADE:
Changed all the character ids to integer values.
Created two pointer to patient arrays:
Mandatory_patients: stores pointers to mandatory patients
Optional_patients: stores pointers to optional patients
Patient is still an struct array.
*/
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "cJSON.h"
#include "cJSON.c"


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
int is_admitted[500] = {0};
int *current_size_dm_nurse;

// Enums for the mapped values
typedef enum {A, B} gender;
//typedef enum {infant, adult, elderly} age_groups;
typedef enum {early , late , night } shift_types;
typedef enum {optional, mandatory} PatientType;


const char* gender_to_string(gender g) {
    switch (g) {
        case A: return "Male";
        case B: return "Female";
        default: return "Unknown";
    }
}


// const char* age_group_to_string(age_groups a) {
//     switch (a) {
//         case infant: return "infant";
//         case adult: return "Adult";
//         case elderly: return "Elderly";
//         default: return "Unknown";
//     }
// }


// Enum conversion functions
gender string_to_gender(const char *str) {
    if (strcmp(str, "A") == 0) return A;
    if (strcmp(str, "B") == 0) return B;
    return -1; // Invalid value
}


// age_groups string_to_age_group(const char *str) {
//     if (strcmp(str, "infant") == 0) return infant;
//     if (strcmp(str, "adult") == 0) return adult;
//     if (strcmp(str, "elderly") == 0) return elderly;
//     return -1; // Invalid value
// }


shift_types string_to_shift_types(const char *str){
    if(strcmp(str, "early") == 0) return early;
    if(strcmp(str, "late") == 0) return late;
    if(strcmp(str, "night") == 0) return night;
    return -1; // Invalid value
}


int str2int(char *a)
{   /* ex. We'll get a string like p01 or n010.
    We have to find the number in the string and return that number in integer form.
    */
    int number=0, i, n, num;
    char *p = a;
    p++;
    n = strlen(p);
    for (i=0; i<n; ++i){
        num = p[i] - '0';
        number += (num * pow(10, n-i-1));
    }
    return number;
}
// Struct definitions
typedef struct {
    int day;
    shift_types shift_time;
    int max_load;
} Shifts;


typedef struct {
    int id;
    int skill_level;
    Shifts *shift;
    int num_shifts;
    int remaining_load;
} Nurses;


Nurses *nurses;
Nurses ***dm_nurses_availability;
int **max_load_updated;

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


Weights *weights = NULL; // Global pointer for weights
typedef struct {
    int id;
    int length_of_stay;
    int room_id;
    char *age;
    gender gen;
    int *workload_produced;
    int *skill_level_required;
} Occupants;


Occupants *occupants = NULL; // Global pointer for occupants
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
    int *incompatible_room_ids;
    int num_incompatible_rooms;
    int *workload_produced;
    int *skill_level_required;
    int assigned_room_no;
    int assigned_ot;
    int admission_day;
} Patient;


Patient *patients;
Patient **mandatory_patients = NULL;
Patient **optional_patients = NULL;
typedef struct {
    int id;
    int *max_surgery_time;
    int *patients_assigned;
    int *time_left; // default: max_surgery_time
    int num_assigned;
} Surgeon;


Surgeon *surgeon;
typedef struct {
    int id;
    int *max_ot_time;
    int *time_left; // default: max_ot_time
} op_theatres;


op_theatres *ot;
typedef struct {
    int id;
    int cap;
    int occupied_cap;
    int *nurses_alloted;
    int length_of_nurses_alloted;
} Rooms;


Rooms *room;

typedef struct{
    int load_sum;
    int max_skill_req;
} Rooms_req;
Rooms_req **rooms_requirement;

// Function to parse occupants
void parse_occupants(cJSON *occupants_array) {
    num_occupants = cJSON_GetArraySize(occupants_array);
    occupants = (Occupants *)malloc(num_occupants * sizeof(Occupants));
    if (!occupants) {
        printf("Memory allocation failed for occupants.\n");
        exit(1);
    }
    for (int i = 0; i < num_occupants; i++) {
        cJSON *item = cJSON_GetArrayItem(occupants_array, i);
        if (!item) continue;
        // Parse individual fields
        cJSON *id_json = cJSON_GetObjectItem(item, "id");
        cJSON *gender_json = cJSON_GetObjectItem(item, "gender");
        cJSON *age_group_json = cJSON_GetObjectItem(item, "age_group");
        cJSON *length_of_stay_json = cJSON_GetObjectItem(item, "length_of_stay");
        cJSON *workload_produced_json = cJSON_GetObjectItem(item, "workload_produced");
        cJSON *skill_level_required_json = cJSON_GetObjectItem(item, "skill_level_required");
        cJSON *room_id_json = cJSON_GetObjectItem(item, "room_id");
        // Parse ID
        if (id_json && cJSON_IsString(id_json)) {
            int id_int = str2int(id_json->valuestring);
            occupants[i].id = id_int;
        } else {
            printf("Error parsing occupant ID for index %d\n", i);
            occupants[i].id = -1; // Set to invalid value
        }
        // Parse Gender
        if (gender_json && cJSON_IsString(gender_json)) {
            occupants[i].gen = string_to_gender(gender_json->valuestring);
        } else {
            occupants[i].gen = -1; // Default invalid gender
        }
        // Parse Age Group
        if (age_group_json && cJSON_IsString(age_group_json)) {
            occupants[i].age = strdup(age_group_json->valuestring);
        } else {
            occupants[i].age = NULL;
        }
        // Parse Length of Stay
        if (length_of_stay_json && cJSON_IsNumber(length_of_stay_json)) {
            occupants[i].length_of_stay = length_of_stay_json->valueint;
        } else {
            occupants[i].length_of_stay = 0; // Default invalid value
        }
        // Parse Room ID
        if (room_id_json && cJSON_IsString(room_id_json)) {
            int room_id_int = str2int(room_id_json->valuestring);
            occupants[i].room_id = room_id_int;
        } else {
            occupants[i].room_id = -1; // Default invalid room ID
        }
        // Parse Workload Produced
        if (workload_produced_json && cJSON_IsArray(workload_produced_json)) {
            int size = cJSON_GetArraySize(workload_produced_json);
            occupants[i].workload_produced = (int *)malloc(size * sizeof(int));
            if (!occupants[i].workload_produced) {
                printf("Memory allocation failed for workload_produced of occupant %d\n", i);
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                occupants[i].workload_produced[j] = cJSON_GetArrayItem(workload_produced_json, j)->valueint;
            }
        } else {
            occupants[i].workload_produced = NULL;
        }
        // Parse Skill Level Required
        if (skill_level_required_json && cJSON_IsArray(skill_level_required_json)) {
            int size = cJSON_GetArraySize(skill_level_required_json);
            occupants[i].skill_level_required = (int *)malloc(size * sizeof(int));
            if (!occupants[i].skill_level_required) {
                printf("Memory allocation failed for skill_level_required of occupant %d\n", i);
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                occupants[i].skill_level_required[j] = cJSON_GetArrayItem(skill_level_required_json, j)->valueint;
            }
        } else {
            occupants[i].skill_level_required = NULL;
        }
    }
}


void free_occupants() {
    for (int i = 0; i < num_occupants; i++) {
       // free(occupants[i].id);
        free(occupants[i].age);
        free(occupants[i].workload_produced);
        free(occupants[i].skill_level_required);
    }
    free(occupants);
}


// Function to parse weights
void parse_weights(cJSON *weights_json) {
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


void parse_patients(cJSON *patients_array) {
    num_patients = cJSON_GetArraySize(patients_array);
   // printf("Number of patients: %d\n", num_patients);
    patients = (Patient*)malloc(num_patients * sizeof(Patient));
    if (!patients) {
        printf("Memory allocation failed for patients.\n");
        exit(1);
    }
    // Allocate memory for mandatory and optional patient pointers
    mandatory_patients = (Patient**)malloc(num_patients * sizeof(Patient*));
    optional_patients = (Patient**)malloc(num_patients * sizeof(Patient*));
    if (!mandatory_patients || !optional_patients) {
        printf("Memory allocation failed for mandatory or optional patient arrays.\n");
        exit(1);
    }
    for (int i = 0; i < num_patients; i++) {
        // int prev_id=0;
        cJSON *item = cJSON_GetArrayItem(patients_array, i);
        if (!item) continue;
        patients[i].assigned_room_no = -1;
        patients[i].admission_day = -1;
        patients[i].assigned_ot = -1;
        // Parse individual fields of each patient
        cJSON *id_json = cJSON_GetObjectItem(item, "id");
        int id_int = str2int(id_json->valuestring);
        if (id_json && cJSON_IsString(id_json)) {
            patients[i].id = id_int;
        }
        cJSON *mandatory_json = cJSON_GetObjectItem(item, "mandatory");
        if (mandatory_json && cJSON_IsBool(mandatory_json)) {
            patients[i].mandatory = cJSON_IsTrue(mandatory_json) ? 1 : 0;
        } else {
            patients[i].mandatory = 0;
            printf("Missing or invalid mandatory status for patient.\n");
        }
        // Assign to mandatory or optional array based on the "mandatory" field
        if (patients[i].mandatory) {
            mandatory_patients[mandatory_count++] = patients+i;//.................................................................
        } else {
            optional_patients[optional_count++] = patients+i;
        }
        cJSON *gender_json = cJSON_GetObjectItem(item, "gender");
        if (gender_json && cJSON_IsString(gender_json)) {
            patients[i].gen = string_to_gender(gender_json->valuestring);
        } else {
            patients[i].gen = A;
        }
        cJSON *age_groups_json = cJSON_GetObjectItem(item, "age_group");
        if (age_groups_json && cJSON_IsString(age_groups_json)) {
            patients[i].age_group = strdup(age_groups_json->valuestring); // Allocate and copy the string
            if (!patients[i].age_group) {
                printf("Memory allocation failed for age_group of patient at index %d.\n", i);
                exit(1);
        }
    } else {
        patients[i].age_group = strdup("adult"); // Default value
        if (!patients[i].age_group) {
            printf("Memory allocation failed for default age_group of patient at index %d.\n", i);
            exit(1);
    }
        printf("Missing or invalid age group for the patient at index %d.\n", i);
}
        cJSON *length_of_stay_json = cJSON_GetObjectItem(item, "length_of_stay");
        if (length_of_stay_json && cJSON_IsNumber(length_of_stay_json)) {
            patients[i].length_of_stay = length_of_stay_json->valueint;
        } else {
            patients[i].length_of_stay = 0;
            printf("Missing or invalid length_of_stay for patient at index %d.\n", i);
        }
        cJSON *surgery_release_day_json = cJSON_GetObjectItem(item, "surgery_release_day");
        if (surgery_release_day_json && cJSON_IsNumber(surgery_release_day_json)) {
            patients[i].surgery_release_day = surgery_release_day_json->valueint;
        } else {
            patients[i].surgery_release_day = 0;
            printf("Missing or invalid surgery_release_day for patient at index %d.\n", i);
        }
        cJSON *surgery_duration_json = cJSON_GetObjectItem(item, "surgery_duration");
        if (surgery_duration_json && cJSON_IsNumber(surgery_duration_json)) {
            patients[i].surgery_duration = surgery_duration_json->valueint;
        } else {
            patients[i].surgery_duration = 0;
            printf("Missing or invalid surgery_duration for patient at index %d.\n", i);
        }
        if (patients[i].mandatory) {
            cJSON *surgery_due_day_json = cJSON_GetObjectItem(item, "surgery_due_day");
            if (surgery_due_day_json && cJSON_IsNumber(surgery_due_day_json)) {
                patients[i].surgery_due_day = surgery_due_day_json->valueint;
            } else {
                patients[i].surgery_due_day = -1;
                printf("Missing or invalid surgery_due_day for mandatory patient at index %d.\n", i);
            }
        } else {
            patients[i].surgery_due_day = -1;
        }
       cJSON *surgeon_id_json = cJSON_GetObjectItem(item, "surgeon_id");
        if (surgeon_id_json && cJSON_IsString(surgeon_id_json)) {
            int surgeon_id_int = str2int(surgeon_id_json->valuestring);
            patients[i].surgeon_id = surgeon_id_int;
        // printf("%d\n" , surgeon_id_int);
        }    
        cJSON *incompatible_rooms_json = cJSON_GetObjectItem(item, "incompatible_room_ids");
        if (incompatible_rooms_json && cJSON_IsArray(incompatible_rooms_json)) {
            patients[i].num_incompatible_rooms = cJSON_GetArraySize(incompatible_rooms_json);
            patients[i].incompatible_room_ids = (int*)malloc(patients[i].num_incompatible_rooms * sizeof(int));
            if (!patients[i].incompatible_room_ids) {
                printf("Memory allocation failed for incompatible_room_ids at index %d.\n", i);
                exit(1);
            }
            for (int j = 0; j < patients[i].num_incompatible_rooms; j++) {
                cJSON *room_id_json = cJSON_GetArrayItem(incompatible_rooms_json, j);
                if (cJSON_IsString(room_id_json)) {
                    patients[i].incompatible_room_ids[j] = str2int(room_id_json->valuestring);
                } else {
                    printf("Invalid room ID format for patient at index %d, room %d.\n", i, j);
                    patients[i].incompatible_room_ids[j] = -1;
                }
            }
        } else {
            patients[i].incompatible_room_ids = NULL;
            patients[i].num_incompatible_rooms = 0;
            printf("Missing or invalid incompatible_room_ids for patient at index %d.\n", i);
        }
        cJSON *workload_json = cJSON_GetObjectItem(item, "workload_produced");
        if (workload_json && cJSON_IsArray(workload_json)) {
            int size = cJSON_GetArraySize(workload_json);
            patients[i].workload_produced = (int*)malloc(size * sizeof(int));
            if (!patients[i].workload_produced) {
                printf("Memory allocation failed for workload_produced at index %d.\n", i);
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                patients[i].workload_produced[j] = cJSON_GetArrayItem(workload_json, j)->valueint;
            }
        } else {
            patients[i].workload_produced = NULL;
            printf("Missing or invalid workload_produced for patient at index %d.\n", i);
        }
        cJSON *skills_json = cJSON_GetObjectItem(item, "skill_level_required");
        if (skills_json && cJSON_IsArray(skills_json)) {
            int size = cJSON_GetArraySize(skills_json);
            patients[i].skill_level_required = (int*)malloc(size * sizeof(int));
            if (!patients[i].skill_level_required) {
                printf("Memory allocation failed for skill_level_required at index %d.\n", i);
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                patients[i].skill_level_required[j] = cJSON_GetArrayItem(skills_json, j)->valueint;
            }
        } else {
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
        if(patients[i].age_group){
            free(patients[i].age_group);
        }
        // Free incompatible room IDs array
        if (patients[i].incompatible_room_ids) {
            free(patients[i].incompatible_room_ids);
        }
        // Free workload array
        if (patients[i].workload_produced) {
            free(patients[i].workload_produced);
        }
        // Free skill level array
        if (patients[i].skill_level_required) {
            free(patients[i].skill_level_required);
        }
    }
    // Free the main patients array
    free(patients);
    patients = NULL; // Set to NULL to avoid dangling pointer issues
}
// function to parse the surgeons
void parse_surgeons(cJSON *surgeons_array) {
    num_surgeons = cJSON_GetArraySize(surgeons_array);
   // printf("%d\n" , num_surgeons);
    surgeon = (Surgeon *)malloc(num_surgeons * sizeof(Surgeon));
    if (!surgeon) {
        printf("Memory allocation failed for surgeons.\n");
        exit(1);
    }
    for (int i = 0; i < num_surgeons; i++) {
        cJSON *item = cJSON_GetArrayItem(surgeons_array, i);
        if (!item) continue;
        cJSON *id_json = cJSON_GetObjectItem(item, "id");
        if (id_json && cJSON_IsString(id_json)) {
            (surgeon)[i].id = str2int(id_json->valuestring); // Dynamically allocate and copy the string
        } else {
            (surgeon)[i].id = -1;
            printf("Missing or invalid ID for surgeon at index %d.\n", i);
        }
        cJSON *max_surgery_time_json = cJSON_GetObjectItem(item, "max_surgery_time");
        if (max_surgery_time_json && cJSON_IsArray(max_surgery_time_json)) {
            int size = cJSON_GetArraySize(max_surgery_time_json);
            (surgeon)[i].max_surgery_time = (int *)malloc(size * sizeof(int));
            if (!(surgeon)[i].max_surgery_time) {
                printf("Memory allocation failed for max_surgery_time at index %d.\n", i);
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                (surgeon)[i].max_surgery_time[j] = cJSON_GetArrayItem(max_surgery_time_json, j)->valueint;
            }
        } else {
            (surgeon)[i].max_surgery_time = NULL;
            printf("Missing or invalid max_surgery_time for surgeon at index %d.\n", i);
        }
        surgeon[i].patients_assigned = NULL;
    }
    }


void assign_patients_to_surgeon(Surgeon *surgeons, int num_surgeons, Patient *patients, int num_patients) {
    // Clear previously assigned patients (if needed, to prevent incorrect assignments)
    for (int i = 0; i < num_surgeons; i++) {
        if (surgeons[i].patients_assigned != NULL) {
            free(surgeons[i].patients_assigned);
        }
        // Initialize the patients_assigned array for each surgeon
        surgeons[i].patients_assigned = (int *)malloc(num_patients * sizeof(int));
        if (surgeons[i].patients_assigned == NULL) {
            fprintf(stderr, "Memory allocation for patients_assigned failed for surgeon %d.\n", surgeons[i].id);
            return;
        }
        surgeons[i].num_assigned = 0;  // Reset the assigned count for each surgeon
    }
    // Assign patients to surgeons based on surgeon_id
    for (int i = 0; i < num_patients; i++) {
        int surgeon_index = patients[i].surgeon_id;  // Get the surgeon_id from the patient      
        if (surgeon_index >= 0 && surgeon_index < num_surgeons) { // Check if valid surgeon_id
            int assigned_index = surgeons[surgeon_index].num_assigned;
            surgeons[surgeon_index].patients_assigned[assigned_index] = patients[i].id;
            surgeons[surgeon_index].num_assigned++;  // Increment the assigned count
        } else {
            printf("Invalid surgeon_id %d for patient %d\n", surgeon_index, patients[i].id);
        }
    }
}
    // Print out the result
    // for (int i = 0; i < num_surgeons; i++) {
    //     printf("Surgeon %d has been assigned %d patients.\n", surgeons[i].id, surgeons[i].num_assigned);
    //     printf("Assigned patient IDs: ");
    //     for (int j = 0; j < surgeons[i].num_assigned; j++) {
    //         printf("%d ", surgeons[i].patients_assigned[j]);
    //     }
    //     printf("\n");
    // }
//}


void free_surgeons() {
    if (!surgeon) return;
    for (int i = 0; i < num_surgeons; i++) {
       // free(surgeon[i].id);  
     // Free the dynamically allocated ID
        if (surgeon[i].patients_assigned != NULL) {
        free(surgeon[i].patients_assigned); // Free dynamically allocated array
        //surgeon[i].patients_assigned = NULL;
        }
        free(surgeon[i].max_surgery_time);  // Free the dynamically allocated max_surgery_time array
    }
    free(surgeon); // Free the array of Surgeon structs
}


// Function to parse the operating theatres
void parse_ots(cJSON *ot_array) {
    num_ots = cJSON_GetArraySize(ot_array);
    //printf("Number of Operating Theatres: %d\n", num_ots);
    ot = (op_theatres *)malloc(num_ots * sizeof(op_theatres));
    if (!ot) {
        printf("Memory allocation failed for operating theatres.\n");
        exit(1);
    }
    for (int i = 0; i < num_ots; i++) {
        cJSON *item = cJSON_GetArrayItem(ot_array, i);
        if (!item) continue;
        // Parse 'id'
         cJSON *id_Json = cJSON_GetObjectItem(item, "id");
        if (id_Json && cJSON_IsString(id_Json)) {
            ot[i].id = str2int(id_Json->valuestring);
        } else {
            ot[i].id = -1;
            printf("Missing or invalid ID for operating theatre at index %d.\n", i);
            continue;  // Skip further processing for this OT
        }
        // Parse 'max_ot_time'
        cJSON *max_ot_time_json = cJSON_GetObjectItem(item, "availability");
        if (max_ot_time_json && cJSON_IsArray(max_ot_time_json)) {
            int size = cJSON_GetArraySize(max_ot_time_json);
            ot[i].max_ot_time = (int *)malloc(size * sizeof(int));
            if (!ot[i].max_ot_time) {
                printf("Memory allocation failed for max_ot_time.\n");
                //free_ots(i);  // Free already allocated memory
                exit(1);
            }
            for (int j = 0; j < size; j++) {
                cJSON *time_item = cJSON_GetArrayItem(max_ot_time_json, j);
                if (time_item && cJSON_IsNumber(time_item)) {
                    ot[i].max_ot_time[j] = time_item->valueint;
                } else {
                    ot[i].max_ot_time[j] = 0;  // Default value if invalid
                    printf("Invalid max_ot_time value at index %d, day %d. Defaulting to 0.\n", i, j);
                }
            }
        } else {
            ot[i].max_ot_time = NULL;
            printf("Missing or invalid max_ot_time for operating theatre at index %d.\n", i);
        }
    }
}


void free_ots(){
    if(!ot) return;
    for(int i = 0 ; i < num_ots ; i++){
        //free(ot[i].id);
        free(ot[i].max_ot_time);
    }
    free(ot);
}


// Function to parse the rooms
void parse_rooms(cJSON *room_array)
{
    num_rooms = cJSON_GetArraySize(room_array);
    room = (Rooms*)malloc(num_rooms * sizeof(Rooms));
    if(!room){
        printf("Memory allocation failed for room array");
        exit(1);
    }
    for(int i = 0 ; i < num_rooms ; i++){
        cJSON *item = cJSON_GetArrayItem(room_array , i);
        if(!item) continue;
        room[i].nurses_alloted = NULL;
        cJSON *id_json = cJSON_GetObjectItem(item , "id");
        room[i].occupied_cap = -1;
        if(id_json && cJSON_IsString(id_json)){
            (room)[i].id = str2int(id_json->valuestring);
        }
        else{
            (room)[i].id = -1;
            printf("Missing or invalid ID for room id.");
        }
        cJSON *capacity_json = cJSON_GetObjectItem(item, "capacity");
        if (capacity_json && cJSON_IsNumber(capacity_json)) {
            room[i].cap = capacity_json->valueint;
        } else {
            room[i].cap = 0;  // Default capacity if invalid
            printf("Missing or invalid capacity for room at index %d. Defaulting to 0.\n", i);
        }

        
    }
}


void free_rooms(void){
    if(!room) return;
    // for(int i = 0 ; i < num_rooms ; i++)
    //     free(room[i].id);
    free(room);
}


void parse_nurse(cJSON *nurses_array) {
    // Get the number of nurses from the JSON array
    num_nurses = cJSON_GetArraySize(nurses_array);
    nurses = (Nurses *)malloc(num_nurses * sizeof(Nurses)); // Allocate memory for nurses
    if (!nurses) {
        printf("Memory allocation failed for nurses.\n");
        exit(1);
    }
    for (int i = 0; i < num_nurses; i++) {
        cJSON *item = cJSON_GetArrayItem(nurses_array, i);
        if (!item) continue;
        // Parse 'id'
        cJSON *id_json = cJSON_GetObjectItem(item, "id");
        if (id_json && cJSON_IsString(id_json)) {
            nurses[i].id = str2int(id_json->valuestring); // Convert string ID to integer
        } else {
            nurses[i].id = -1;
            printf("Missing or invalid ID for nurse at index %d.\n", i);
        }
        // Parse 'skill_level'
        cJSON *skill_level_json = cJSON_GetObjectItem(item, "skill_level");
        if (skill_level_json && cJSON_IsNumber(skill_level_json)) {
            nurses[i].skill_level = skill_level_json->valueint;
        } else {
            nurses[i].skill_level = -1; // Default error value
            printf("Missing or invalid skill level for nurse at index %d.\n", i);
        }
        // Parse 'working_shifts'
        cJSON *shifts_json = cJSON_GetObjectItem(item, "working_shifts");
        if (shifts_json && cJSON_IsArray(shifts_json)) {
            int shift_count = cJSON_GetArraySize(shifts_json);
            nurses[i].shift = (Shifts *)malloc(shift_count * sizeof(Shifts)); // Allocate memory for shifts
            if (!nurses[i].shift) {
                printf("Memory allocation failed for shifts of nurse at index %d.\n", i);
                exit(1);
            }
            nurses[i].num_shifts = shift_count; // Store the number of shifts for the nurse
            for (int j = 0; j < shift_count; j++) {
                cJSON *shift_item = cJSON_GetArrayItem(shifts_json, j);
                if (shift_item) {
                    // Parse 'day'
                    cJSON *day_json = cJSON_GetObjectItem(shift_item, "day");
                    if (day_json && cJSON_IsNumber(day_json)) {
                        nurses[i].shift[j].day = day_json->valueint;
                    } else {
                        nurses[i].shift[j].day = -1; // Default error value
                        printf("Missing or invalid day for nurse %d, shift %d.\n", i, j);
                    }


                    // Parse 'shift'
                    cJSON *shift_json = cJSON_GetObjectItem(shift_item, "shift");
                    if (shift_json && cJSON_IsString(shift_json)) {
                        nurses[i].shift[j].shift_time = string_to_shift_types(shift_json->valuestring); // Convert string to enum
                    } else {
                        nurses[i].shift[j].shift_time = -1; // Default error value
                        printf("Missing or invalid shift time for nurse %d, shift %d.\n", i, j);
                    }

                    // Parse 'max_load'
                    cJSON *max_load_json = cJSON_GetObjectItem(shift_item, "max_load");
                    if (max_load_json && cJSON_IsNumber(max_load_json)) {
                        nurses[i].shift[j].max_load = max_load_json->valueint;
                    } else {
                        nurses[i].shift[j].max_load = -1; // Default error value
                        printf("Missing or invalid max load for nurse %d, shift %d.\n", i, j);
                    }
                }
            }
        } else {
            nurses[i].shift = NULL;
            nurses[i].num_shifts = 0; // Set number of shifts to zero
            printf("Missing or invalid working_shifts for nurse at index %d.\n", i);
        }

        nurses[i].remaining_load = -1;
    }
}


void free_nurses() {
    if (nurses) {
        for (int i = 0; i < num_nurses; i++) {
            if (nurses[i].shift) {
                free(nurses[i].shift); // Free the shifts array
            }
        }
        free(nurses); // Free the nurses array itself
    }
}


// Function to parse the JSON file
void parse_json(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Unable to open file.\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    fclose(file);
    json_data[file_size] = '\0';
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        printf("Error parsing JSON.\n");
        free(json_data);
        return;
    }
    days = cJSON_GetObjectItem(root, "days")->valueint;
    skill_levels = cJSON_GetObjectItem(root, "skill_levels")->valueint;
    // Parse weights
    cJSON *weights_json = cJSON_GetObjectItem(root, "weights");
    weights = (Weights *)malloc(sizeof(Weights));
    parse_weights(weights_json);
    // Parse occupants
    cJSON *occupants_array = cJSON_GetObjectItem(root, "occupants");
    parse_occupants(occupants_array);
    // parse patients
    cJSON *patients_array = cJSON_GetObjectItem(root , "patients");
    parse_patients(patients_array);
    // parse surgeons
    cJSON *surgery_array = cJSON_GetObjectItem(root , "surgeons");
    parse_surgeons(surgery_array);
    assign_patients_to_surgeon(surgeon,num_surgeons, patients, num_patients);
    // parse ots
    cJSON *ot_array = cJSON_GetObjectItem(root , "operating_theaters");
    parse_ots(ot_array);
    // parse rooms
    cJSON *room_array = cJSON_GetObjectItem(root , "rooms");
    parse_rooms(room_array);
    // parse nurses
    cJSON *nurse_array = cJSON_GetObjectItem(root , "nurses");
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
            for (int j = 0; j < occupants[i].length_of_stay*3; j++) {
                printf("%d ", occupants[i].workload_produced[j]);
            }
            printf("\n");
        }
        if (occupants[i].skill_level_required) {
            printf("  Skill Levels Required: ");
            for (int j = 0; j < occupants[i].length_of_stay*3; j++) {
                printf("%d ", occupants[i].skill_level_required[j]);
            }
            printf("\n");
        }
    }
}




void print_surgeons(Surgeon *surgeons) {
    if (!surgeons || num_surgeons <= 0) {
        printf("No surgeons to display.\n");
        return;
    }
    printf("List of Surgeons:\n");
    for (int i = 0; i < num_surgeons; i++) {
        printf("Surgeon ID: %d\n", surgeons[i].id);
        printf("Max Surgery Time (by day): ");
        if (surgeons[i].max_surgery_time) {
            for (int j = 0; j < 28; j++) { // Assuming 14 days
                printf("%d ", surgeons[i].max_surgery_time[j]);
            }
        } else {
            printf("No data available");
        }
        printf("\n");
        printf("-----------------------------------\n");
    }
}


// Printing the ots values
void print_ots(op_theatres *ots){
    if (!ots || num_ots <= 0){
        printf("No ots to display.\n");
    }
    for(int i=0 ; i < num_ots ; i++) {
        if(ot[i].max_ot_time){
            for(int j = 0 ; j < 28 ; j++ ){
                printf("%d " , ots[i].max_ot_time[j]);
            }
        }
        else{
            printf("No data available.");
        }
        printf("\n");
    }
}


// Printing the rooms
void print_rooms(){
    if(!room || num_rooms <= 0){
        printf("No rooms to display.\n");
    }
    for(int i = 0 ; i < num_rooms ; i++){
        printf("%d\t%d\t%d" , room[i].id , room[i].cap, room[i].occupied_cap);
        printf("\n");
    }
}


// Printing the nurses
void print_nurses() {
    if (!nurses || num_nurses <= 0) {
        printf("No nurses to display.\n");
        return;
    }
    for (int i = 0; i < num_nurses; i++) {
        printf("Nurse ID: %d\n", nurses[i].id);
        printf("Skill Level: %d\n", nurses[i].skill_level);
        // Print shifts for each nurse
        if (nurses[i].num_shifts > 0 && nurses[i].shift) {
            for (int j = 0; j < nurses[i].num_shifts; j++) {
                printf("  Day: %d, Shift: ", nurses[i].shift[j].day);
                // Convert shift_type enum to string
                switch (nurses[i].shift[j].shift_time) {
                    case early: printf("early"); break;
                    case late: printf("late"); break;
                    case night: printf("Night"); break;
                    default: printf("Unknown"); break;
                }
                printf(", Max Load: %d\n", nurses[i].shift[j].max_load);
            }
        } else {
            printf("  No shifts assigned.\n");
        }

        printf("Remaining_load : %d\n" , nurses[i].remaining_load);
    }
}


void print_patients(Patient *patient_struct_array) {
    int i;
    if (!patient_struct_array) return;
    for (i=0; i<num_patients; ++i) {
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
            for (int j = 0; j < 3*patient_struct_array[i].length_of_stay; j++) {
                printf("%d ", patient_struct_array[i].workload_produced[j]);
            }
            printf("\n");
        }
        // Print skill level required if it exists
        if (patient_struct_array[i].skill_level_required != NULL) {
            printf("Skill Level Required: ");
            for (int j = 0; j < 3*patient_struct_array[i].length_of_stay; j++) {
                printf("%d ", patient_struct_array[i].skill_level_required[j]);
            }
            printf("\n");
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
        if (optional_patients[i]->workload_produced != NULL) {
            printf("Workload Produced: ");
            for (int j = 0; j < 3*optional_patients[i]->length_of_stay; j++) {
                printf("%d ", optional_patients[i]->workload_produced[j]);
            }
            printf("\n");
        }
        // Print skill level required if it exists
        if (optional_patients[i]->skill_level_required != NULL) {
            printf("Skill Level Required: ");
            for (int j = 0; j < 3*optional_patients[i]->length_of_stay; j++) {
                printf("%d ", optional_patients[i]->skill_level_required[j]);
            }
            printf("\n");
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
            for (int j = 0; j < 3*mandatory_patients[i]->length_of_stay; j++) {
                printf("%d ", mandatory_patients[i]->workload_produced[j]);
            }
            printf("\n");
        }
        // Print skill level required if it exists
        if (mandatory_patients[i]->skill_level_required != NULL) {
            printf("Skill Level Required: ");
            for (int j = 0; j < 3*mandatory_patients[i]->length_of_stay; j++) {
                printf("%d ", mandatory_patients[i]->skill_level_required[j]);
            }
            printf("\n");
        }
    }
    printf("==================================================================\n");
}


// int greedy_heuristic(input_format)
// {
// }




// void swap(Patient **matrix, int a, int b) {
//     Patient *temp = matrix[a];
//     matrix[a] = matrix[b];
//     matrix[b] = temp;
// }




// int partition(Patient **patients_struct_array, int n, int pivot) {
//     int i, j;
//     for (i=0, j=n-1; i<=j; ){
//         while (i < n && patients_struct_array[i]->surgery_release_day <= pivot)
//             ++i;
//         while (j >= 0 && patients_struct_array[j]->surgery_release_day > pivot)
//             --j;
//         if (i<=j)
//             swap(patients_struct_array, i++, j--);
//     }
//     return i;
// }




// int get_pivot(Patient **patients_struct_array, int n) {
//     int small, big, i;
//     for (i=1, small=patients_struct_array[0]->surgery_release_day, big=patients_struct_array[n-1]->surgery_release_day; i<n; ++i){
//         if (patients_struct_array[i]->surgery_release_day > big)
//             big = patients_struct_array[i]->surgery_release_day;
//         if (patients_struct_array[i]->surgery_release_day < small)
//             small = patients_struct_array[i]->surgery_release_day;
//     }
//     return (small+big)/2;
//     // int first = patients_struct_array[0]->surgery_release_day;
//     // int middle = patients_struct_array[n / 2]->surgery_release_day;
//     // int last = patients_struct_array[n - 1]->surgery_release_day;


//     // // Find the median of first, middle, and last
//     // if ((first > middle) ^ (first > last))
//     //     return first;
//     // else if ((middle > first) ^ (middle > last))
//     //     return middle;
//     // else
//     //     return last;
// }


// void sort_on_release_day(Patient **patients_struct_array, int n) {
//     // using quick Sort technique
//     int n_left;
//     static prev_n_left;
//     printf("\nArray: ");
//     for (int i = 0; i < n; i++)
//         printf("%d, ", patients_struct_array[i]->surgery_release_day);
//     printf("\b\b ");
//     if (n==1) return;
//     if (n==2)
//         if (patients_struct_array[0]->surgery_release_day > patients_struct_array[1]->surgery_release_day) {
//             swap(patients_struct_array, 0, 1);
//             return;
//         }
//     int pivot = get_pivot(patients_struct_array, n);
//     printf("\nPivot: %d", pivot);
//     n_left = partition(patients_struct_array, n, pivot);
//     if (n_left == prev_n_left) return;
//     else prev_n_left = n_left;
//     printf("\nn_left: %d", n_left);
//     sort_on_release_day(patients_struct_array, n_left);
//     sort_on_release_day(patients_struct_array+n_left, n-n_left);
// }




// ..................FUNCTIONS BELOW ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON surgery_release_day...................




typedef struct LL {
    Patient *pointer;
    struct LL *next, *prev;
} Node;




Node *makeNode(Patient *struct_pointer) {
    Node *node = (Node *) calloc (1, sizeof(Node));
    node->pointer = struct_pointer;
    return node;
}




void appendNode(Node **sorted_array, Patient *struct_pointer, int index) {
    Node *node = makeNode(struct_pointer), *temp;
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




Node **sorted_mandatory_patients = NULL;
void sort_mandatory_patients_on_release_day(Patient **patient_array, int n) {
    sorted_mandatory_patients = (Node **) calloc (days, sizeof(Node *));
    for (int i=0; i<n; ++i)
        appendNode(sorted_mandatory_patients, patient_array[i], patient_array[i]->surgery_release_day);
}




Node **sorted_optional_patients = NULL;
void sort_optional_patients_on_release_day(Patient **patient_array, int n) {
    sorted_optional_patients = (Node **) calloc (days, sizeof(Node *));
    for (int i=0; i<n; ++i)
        appendNode(sorted_optional_patients, patient_array[i], patient_array[i]->surgery_release_day);
}




void parse_linked_list(Node *head) {
    putchar('\n');
    for (Node *self=head; self; self = self->next) {
        printf("%d, ", self->pointer->surgery_release_day);
    }
    printf("\b\b");
}




void print_sorted_mandatory_array(void) {
    int i;
    for(i=0; i<days; ++i) {
        Node *head = sorted_mandatory_patients[i];
        parse_linked_list(head);
    }
}




void print_sorted_optional_array(void) {
    int i;
    for(i=0; i<days; ++i) {
        Node *head = sorted_optional_patients[i];
        parse_linked_list(head);
    }
}




void free_linked_list(Node *head) {
    if (head->next)
        free_linked_list(head->next);
    free(head);
}




void free_patients_sorted_array(Node **sorted_array) {
    // free sorted_array
    int i;
    for(i=0; i<days; ++i)
        if (sorted_array[i])
            free_linked_list(sorted_array[i]);
    free(sorted_array);
}




void copy_paste_pointers_from_sorted_mandatory_array_to_mandatory_patients(void) {
    // Here we will copy all the pointer from the sorted_array to the patients array in the sorted order.
    int i, j=0;
    Node *self;
    for(i=0; i<days; ++i)
        for(self=sorted_mandatory_patients[i]; self; self=self->next)
            mandatory_patients[j++] = self->pointer;
}




void copy_paste_pointers_from_sorted_optional_array_to_optional_patients(void) {
    // Here we will copy all the pointer from the sorted_array to the patients array in the sorted order.
    int i, j=0;
    Node *self;
    for(i=0; i<days; ++i)
        for(self=sorted_optional_patients[i]; self; self=self->next)
            optional_patients[j++] = self->pointer;
}




// ..............FUNCTIONS ABOVE ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON surgery_release_day................




// .................FUNCTIONS BELOW ARE FOR SORTING mandatory_patients & optional_patients ARRAYS ON length_of_stay..................




Node *find_smallest_iter(Node *head) {
    Node *smallest=head, *self;
    for (self=head->next; self; self=self->next)
        if (self->pointer->length_of_stay < smallest->pointer->length_of_stay)
            smallest = self;
    return smallest;
}
/*
Node *find_smallest_rec(Node *head, Node* smallest) {
    if(!head) return smallest;
    if (head->pointer->length_of_stay < smallest->pointer->length_of_stay) {
        smallest = head;
        return find_smallest(head->next, smallest);
    }
}


void delete_node(Node *head, Node *node) {
    if (head->next == node) {
        head->next = node->next;
        return;
    }
    delete_node(head->next, node);
}


Node *insert_node(Node *head, Node *node, int index) {
    Node *temp;
    int j = 1;
    if (!index) {
        temp = head;
        head = node;
        head->next = temp;
        return head;
    }
    if (j == index) {
        temp = head->next;
        head->next = node;
        node->next = temp;
        return head;
    }
    return insert_node(head->next, node, index-1);
}
*/


Node *insertion_sort_for_linked_list(Node *head) {
    if (!head || !head->next) return head; // No sorting needed for 0 or 1 node.
    Node *sorted = NULL; // New head of the sorted list
    Node *current = head;
    while (current) {
        Node *next = current->next; // Save the next node
        // Insert current node into the sorted list
        if (!sorted || current->pointer->length_of_stay < sorted->pointer->length_of_stay) {
            // Insert at the head
            current->next = sorted;
            if (sorted) sorted->prev = current; // Update prev for old head
            sorted = current;
            sorted->prev = NULL; // New head has no prev
        } else {
            // Traverse sorted list to find the correct position
            Node *temp = sorted;
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
    for (i=0; i<days; ++i) {
        Node *head = sorted_mandatory_patients[i];
        head = insertion_sort_for_linked_list(head);
        sorted_mandatory_patients[i] = head;
    }
}




void sort_optional_patients_on_LOS(void) {
    // here we will sort the sorted_optional_patients array on LOS of the patient.
    // Approach: send each linked list to the insertion_sort_for_linked_list(Node *head) function.
    // It'll sort that list and return the head of the list.
    int i;
    for (i=0; i<days; ++i) {
        Node *head = sorted_optional_patients[i];
        head = insertion_sort_for_linked_list(head);
        sorted_optional_patients[i] = head;
    }
}
// .............................FUNCTION TO APPEND THE TWO LINKED LISTS....................................


void append_optional_to_mandatory(Node **sorted_mandatory_patients, Node **sorted_optional_patients, int days) {
    for (int i = 0; i < days; ++i) {
        //printf("i am here\n");
        Node *mandatory_head = sorted_mandatory_patients[i];
        Node *optional_head = sorted_optional_patients[i];


        if (!mandatory_head) {
            sorted_mandatory_patients[i] = optional_head;
        } else {
            Node *tail = mandatory_head;
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




// ...................FUNCTION BELOW ARE FOR THE ROOM_GENDER_MAP.....................//




// void initialize_room_gender_map(int *room_gender_map){
//     // for(int i = 0 ; i<num_rooms ; i++){
//     //     for(int j = 0 ; j<2 ; j++)
//     //         room_gender_map[i][j] = -1;
//     // }
//     memset(room_gender_map, 0, sizeof(room_gender_map));
// }


void populate_room_gender_map(int *room_gender_map){
    for(int i = 0 ; i<num_occupants ; i++){
        int x = occupants[i].room_id;
        int y = occupants[i].gen;
        if (y)
            room_gender_map[x] = 1;
        // if(y == 0){
        // room_gender_map[x][0] = 1;
        // }
        // else{
        // room_gender_map[x][1] = 1;
        // }
    }
}




void print_map(int *room_gender_map){
    printf("Gender A\t gender B\n");
    for(int i = 0 ; i<num_rooms ; i++)
            printf("%d\t", room_gender_map[i]);
}


// ...................FUNCTION ABOVE ARE FOR THE ROOM_GENDER_MAP.....................//


// ...................FUNCTION BELOW ARE FOR PATIENT ADMISSION.....................//


// int find_length(Node *head) {
//     static count;
//     if (!head)
//         return count;
//     ++count;
//     return find_length(head->next);
// }


int find_length_of_DLL(Node *head) {
    int count;
    Node *self;
    for(self=head, count=0; self; self=self->next) count++;
    return count;
}


void transfer_surgeon_ids_to_s_id_arr(Node *head, int *s_id_arr, int n) {
    int i;
    Node *self;
    for (self=head, i=0; i<n; ++i, self=self->next) s_id_arr[i] = head->pointer->surgeon_id;
    if (self) puts("\n\n-------------------------ERROR : transfer_surgeon_ids_to_s_id_arr(head, s_id_arr, n)--------------------------\n");}


void free_s_id_arr(int *s_id_arr) {
    free(s_id_arr);
}


void sort_s_id_arr(int *s_id_arr, int n, int *sorted_s_ids) {
    // sort this surgeon id array using counting sort algorithm
    int i, id;
    for (i=0; i<n; ++i) {
        id = s_id_arr[i];
        sorted_s_ids[id]++;
    }
}


// void admit_patients(int *roo_gender_map) {
//     /*
//         we'll go patient by patient.
//         when a patient comes, we'll check what's his release_day and due_day
//             if the due_day == release_day then admit him at any cost
//             if not check the resources...
           
//             - put patients with same surgeon in one OT, thereby controlling surgeon_transfer contraint
//             - do this for each linked-list in the mandatory_patients and optional_patients arrays
//             - for this, firstly create an array of size #(patients in the 1 linked-list) and put surgeon-id of each
//             patient as the element
//             - then sort it using counting sort (as we already know #surgeons OR we can find that also while parsing
//             the linked-list)
//             -
//     */
//     int d, i, j, *s_id_arr, length, sorted_s_ids[num_surgeons];
//     memset(sorted_s_ids, 0, sizeof(sorted_s_ids));
//     Node *self, *head;
//     for (d=0; d<days; ++d) {
//         head = mandatory_patients[d];
//         length = find_length_of_DLL(head);
//         s_id_arr = (int *) calloc (length, sizeof(int));
//         transfer_surgeon_ids_to_s_id_arr(head, s_id_arr, length);
//         sort_s_id_arr(s_id_arr, length, sorted_s_ids);
//         // first, write code for getting the ids of patients in the array sorted_s_ids
//         // then assign OTs sequentially
//         // and admission_date will be d
//     }
// }


// ...................FUNCTION ABOVE ARE FOR PATIENT ADMISSION.....................//


//...................FUNCTION FOR THE ROOMS REQUIREMENT MATRIX........//

void initialize_rooms_requirement(){
    rooms_requirement = (Rooms_req**)malloc(3*days * (sizeof (Rooms_req*)));
    for(int i = 0 ; i < 3*days ; i++){
        rooms_requirement[i] = (Rooms_req*)malloc(num_rooms * sizeof(Rooms_req));
    }
}

void create_rooms_requirement(){

}

//......................FUNCTIONS BELOW ARE FOR NURSE ASSIGNMENTS......................//

void swap_nurse_pointers(Nurses **nurse1, Nurses **nurse2) {
    Nurses *temp = *nurse1;
    *nurse1 = *nurse2;
    *nurse2 = temp;
}

int partition(Nurses **arr, int low, int high) {
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

void quicksort(Nurses **arr, int low, int high) {
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
    dm_nurses_availability = (Nurses ***)malloc(days * 3 * sizeof(Nurses **));
    max_load_updated = (int **) malloc (num_nurses *(sizeof(int *)));
    if (!dm_nurses_availability) {
        perror("Failed to allocate memory for main array");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < days * 3; ++i) {
        dm_nurses_availability[i] = NULL;  // Initialize each slot to NULL
        max_load_updated[i] = (int *)malloc (days*3 * sizeof(int));
        }

    if (!max_load_updated) {
        perror("Failed to allocate memory for max_load_updated");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_nurses; ++i) {
        max_load_updated[i] = (int *)malloc(days * 3 * sizeof(int));
        if (!max_load_updated[i]) {
            perror("Failed to allocate memory for max_load_updated row");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < days * 3; ++j) {
            max_load_updated[i][j] = -1;
        }
    }

}

void allocate_sub_array(Nurses ***main_array, int index, int sub_size) {
    main_array[index] = (Nurses **)malloc(sub_size * sizeof(Nurses *));
    if (!main_array[index]) {
        perror("Failed to allocate memory for sub-array");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < sub_size; ++i) {
        main_array[index][i] = NULL; // Initialize subarray elements to NULL
    }
}

void append_to_sub_array(Nurses ***main_array, int index, int *current_sizes, Nurses *nurse) {
    int current_size = current_sizes[index];
    int new_size = current_size + 1;

    main_array[index] = (Nurses **)realloc(main_array[index], new_size * sizeof(Nurses *));
    if (!main_array[index]) {
        perror("Failed to reallocate memory for sub-array");
        exit(EXIT_FAILURE);
    }
    main_array[index][current_size] = nurse; // Add pointer to Nurse structure
    current_sizes[index] = new_size;        // Update current size
}


void initialize_current_size_dm_nurse() {
    current_size_dm_nurse = (int *)calloc(days * 3, sizeof(int));
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

void free_all_dm_nurses(Nurses ***main_array, int main_size, int *current_sizes) {
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
                printf("id: %d\t max_load: %d\t", dm_nurses_availability[i][j]->id , dm_nurses_availability[i][j]->shift->max_load);
            } else {
                printf("NULL\t");
            }
        }
        printf("\n");
    }
}


void nurse_assignments() {
    for (int i = 0; i < days; i++) {
        for (int j = 0; j < num_rooms; j++) {
            for (int k = 0; k < 3; k++) {
                int m = 0;
                Nurses **arr = dm_nurses_availability[i * 3 + k];
                int arr_size = current_size_dm_nurse[i * 3 + k]; 
                
                if (room[j].nurses_alloted == NULL) {
                    room[j].nurses_alloted = (int *)malloc(sizeof(int));
                    room[j].length_of_nurses_alloted = 0;
                }
                
                Nurses *nurse_to_be_assigned = NULL;
                
                /*conditions check to assign the nurse
                1. if the max_load is not exhauted
                2. if the skill_level_required is matching the skill_level_required for jth room
                3. also remaining_load should be greater than or equal to the load_sum for that room in the shift 3*i + k.
                4. ignoring continuity of care 
                5. going for 0 scenario penalty for constraint s2 and s4.
                6. ignoring continuity of care
                */

                while (m < arr_size) {
                    nurse_to_be_assigned = arr[m];
                    int max_load_of_nurse_to_be_assigned = max_load_updated[nurse_to_be_assigned->id][3 * i + k];
                    
                    if (max_load_of_nurse_to_be_assigned != -1 &&
                        max_load_of_nurse_to_be_assigned >= rooms_requirement[j]->load_sum &&
                        nurse_to_be_assigned->skill_level >= rooms_requirement[j]->max_skill_req) {
                        break; 
                    }
                    m++;
                }
                
                // If a valid nurse is found, assign to the room
                if (m < arr_size && nurse_to_be_assigned != NULL) {
                    room[j].nurses_alloted = (int *)realloc(
                        room[j].nurses_alloted,
                        (room[j].length_of_nurses_alloted + 1) * sizeof(int)
                    );
                    
                    if (room[j].nurses_alloted == NULL) {
                        perror("Memory allocation failed for nurses_alloted");
                        exit(EXIT_FAILURE);
                    }
                    room[j].nurses_alloted = (int *)realloc(room[j].nurses_alloted, 2 * sizeof(int));
                    room[j].nurses_alloted[room[j].length_of_nurses_alloted] = nurse_to_be_assigned->id;
                    room[j].length_of_nurses_alloted++;
                    
                    max_load_updated[nurse_to_be_assigned->id][3 * i + k] -= rooms_requirement[j]->load_sum;
                } else {
                    fprintf(stderr, "No valid nurse available for Room %d, Shift %d\n", j, 3 * i + k);
                }
            }
        }
    }
}



void print_max_loadd_updated(){
    for(int i = 0 ; i < num_nurses ; i++){
        printf("Max_load of the nurse %d\n" , i);
        for(int j = 0 ; j < 3 * days ; j++){
            printf("%d\t", max_load_updated[i][j]);
        }
        printf("\n");
    }
}

int main(void) {
    parse_json("test10.json");
    // printf("%d\n" , num_rooms);
    int room_gender_map[num_rooms];
  //  int rooms_requirement[3*days][num_rooms];
    memset(room_gender_map, 0, sizeof(room_gender_map));
    // initialize_room_gender_map(room_gender_map);
    populate_room_gender_map(room_gender_map);
    //print_map(room_gender_map);
    sort_mandatory_patients_on_release_day(mandatory_patients, mandatory_count);
    sort_mandatory_patients_on_LOS();
    // print_sorted_mandatory_array();
    copy_paste_pointers_from_sorted_mandatory_array_to_mandatory_patients();
    sort_optional_patients_on_release_day(optional_patients, optional_count);
    sort_optional_patients_on_LOS();
    //print_sorted_optional_array();
    copy_paste_pointers_from_sorted_optional_array_to_optional_patients();
    append_optional_to_mandatory(sorted_mandatory_patients,sorted_optional_patients, days);
    //print_sorted_mandatory_array();

    create_dm_nurses_availability();
    sorting_nurse_id_max_load();
    //print_max_loadd_updated();

    //print_dm_nurses();



    // Use the parsed data in your algorithm
    // int *surgery_time[num_surgeons][days];
    // printf("Weights:\n");
    // printf("Room Mixed Age: %d\n", weights->room_mixed_age);
    // printf("Room Nurse Skill: %d\n", weights->room_nurse_skill);
    // printf("Continuity of Care: %d\n", weights->continuity_of_care);
    // printf("Nurse Excessive Workload: %d\n", weights->nurse_excessive_workload);
    // printf("Open Operating Theater: %d\n", weights->open_operating_theater);
    // printf("Surgeon Transfer: %d\n", weights->surgeon_transfer);
    // printf("Patient Delay: %d\n", weights->patient_delay);
    // printf("Unscheduled Optional: %d\n", weights->unscheduled_optional);


    // print_surgeons(surgeon);
    // print_ots(ot);
    // print_rooms();
    // print_nurses();
    // print_patients(patients);
   // print_occupants();
    //print_mandatory_patients();
    // print_optional_patients();




    //Free allocated memory
    free_patients_sorted_array(sorted_mandatory_patients);
   // free_patients_sorted_array(sorted_optional_patients);
    free_occupants();
    free_patients();
    free_surgeons();
    free_ots();
    free_rooms();
    free_nurses();
    free(weights);
      //free_all_dm_nurses(dm_nurses_availability, days * 3, current_size_dm_nurse);
}




/*
UPDATES SECTION: Here we will put all the updates related to this program. Whether it's about the heuristic or anything else.
1. Wrote code for admit_patients function and some utility functions
2. Working on: written in admit_patients function
*/

