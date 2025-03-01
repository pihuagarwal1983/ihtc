#include <definition.h>


// Global declarations of variables
int days;                      // Total number of days in the schedule
int skill_levels;              // Total number of skill levels
int num_occupants;             // Total number of occupants
int num_patients;              // Total number of patients
int num_surgeons;              // Total number of surgeons
int num_ots;                   // Total number of operating theaters (OTs)
int num_rooms;                 // Total number of rooms
int num_nurses;                // Total number of nurses
int mandatory_count = 0;       // Counter for mandatory patients
int optional_count = 0;        // Counter for optional patients
int current_ot_index = 0;      // Index to track current operating theater
int** room_shift_nurse;

//// int is_admitted[1000] = {0}; // we're not using this array anywhere so let's comment it out
int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
int* current_size_dm_nurse;
int* room_gender_map;
//
int** size_of_room_schedule;

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

Nurses* nurses;
Nurses*** dm_nurses_availability;  //nurse assignment
int** max_load_updated; //nurse assignment
Weights* weights = NULL; // Global pointer for weights
Occupants* occupants = NULL; // Global pointer for occupants
Patient* patients;
Patient** mandatory_patients = NULL;
Patient** optional_patients = NULL;
char**** room_schedule;
Surgeon* surgeon;
OTs* ot;
Rooms* room;
GenderRoom* gender_A_rooms = NULL, * gender_B_rooms = NULL, * empty_rooms = NULL;
RoomVector* v_A, * v_B, * v_empty;
Rooms_req** rooms_requirement;

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
        occupants[i].is_admitted = 1;
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
        //room[r_id].gen = occupants[i].gen;
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
        patients[i].is_admitted = 0;
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
            ASSERT(0, "no value present in the time_left array of ot.\n");
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
        room[i].nurses_alloted = (int*)calloc(1, sizeof(int));
        room[i].num_patients_info = (int*)calloc(days, sizeof(int));
        for (int m = 0; m < days; m++) room[i].num_patients_info[m] = 0;
        room[i].gender_days_info = (gender*)calloc(days, sizeof(gender));
        for (int m = 0; m < days; m++) room[i].gender_days_info[m] = -1;
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
        room[i].gen = -1;
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
                        nurses[i].shift[j].load_left = max_load_json->valueint;
                    }
                    else {
                        nurses[i].shift[j].max_load = -1; // Default error value
                        printf("Missing or invalid max load for nurse %d, shift %d.\n", i, j);
                    }
                    nurses[i].shift[j].rooms = (int*)calloc(1, sizeof(int));  // Initialize rooms as NULL
                    nurses[i].shift[j].rooms = NULL;
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


void free_nurses() {
   if (nurses) {
      for (int i = 0; i < num_nurses; i++)
         if (nurses[i].shift) {
            for (int j = 0; j < nurses[i].num_shifts; j++)
               if (nurses[i].shift[j].rooms)
                  free(nurses[i].shift[j].rooms); // Free the rooms array if allocated
            free(nurses[i].shift); // Free the shifts array for the nurse
         }
      free(nurses); // Free the nurses array itself
   }
}

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
    if (!head || !head->next) return head; // No sorting needed for 0 or 1 node.

    Node* sorted = NULL; // New head of the sorted list
    Node* current = head;

    while (current) {
        Node* next = current->next; // Save the next node

        // Extract the length of stay
        int length_of_stay = current->pointer->length_of_stay;

        // Insert current node into the sorted list
        if (!sorted || length_of_stay < sorted->pointer->length_of_stay) {
            // Insert at the head
            current->next = sorted;
            if (sorted) sorted->prev = current; // Update prev for old head
            sorted = current;
            sorted->prev = NULL; // New head has no prev
        }
        else {
            // Traverse sorted list to find the correct position
            Node* temp = sorted;
            while (temp->next && temp->next->pointer->length_of_stay < length_of_stay) {
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
    // Allocate memory for room_gender_map (array of pointers to rows)
    *room_gender_map = (int*)calloc(num_rooms, sizeof(int)); // Allocate memory for row pointers
    if (!*room_gender_map) {
        printf("Memory allocation failed for room_gender_map.\n");
        exit(1);
    }

    // Allocate memory for each row and initialize values to -1
    for (int i = 0; i < num_rooms; i++) {
        (*room_gender_map)[i] = -1;
    }
}

void populate_room_gender_map(int** room_gender_map) {
    // populates the room_gender_map
    if (!*room_gender_map) {
        printf("Error: room_gender_map is not initialized.\n");
        return;
    }
    for (int i = 0; i < num_rooms; ++i) {
        ((*room_gender_map)[i]) = -1;
    }

    // Populate room_gender_map based on occupants
    for (int i = 0; i < num_occupants; ++i) {
        int room_id = occupants[i].room_id;

        if (room_id >= 0 && room_id < num_rooms) {
            (*room_gender_map)[room_id] = occupants[i].gen; // Correct indexing
        }
        else {
            printf("Warning: Invalid room_id %d for occupant %d.\n", room_id, i);
        }
    }
}

void print_map(int** room_gender_map) {
    for (int i = 0; i < num_rooms; i++)
        printf("room number: %d\tgender: %d\n", i, (*room_gender_map)[i]);
}


// .......FUNCTIONs BELOW - PRIORITY QUEUE IMPLEMENTATION FOR KEEPING TRACK OF CARRY FORWARDED PATIENTS..........//

// Initialize the priority queue
void heapifyUp(PriorityQueue* pq, int index) {
    // printf("HeapifyUp - Starting at index: %d\n", index);
    while (index > 0) {
        int parent = (index - 1) / 2;
        //printf("Comparing node at index %d with parent at index %d\n", index, parent);
        if (compareNodesForPQ(pq->data[index], pq->data[parent]) > 0) {
            // Swap child with parent
          //  printf("Swapping node with parent: (%d, %d, %d, %d) <-> (%d, %d, %d, %d)\n",
            //    pq->data[index].patient_id, pq->data[index].mandatory, pq->data[index].due_day, pq->data[index].delay,
              //  pq->data[parent].patient_id, pq->data[parent].mandatory, pq->data[parent].due_day, pq->data[parent].delay);

            HeapNode temp = pq->data[index];
            pq->data[index] = pq->data[parent];
            pq->data[parent] = temp;
            index = parent;
        }
        else {
            //printf("No swap needed\n");
            break;
        }
    }
    //printf("HeapifyUp - Final index: %d\n", index);
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
    HeapNode* data = (HeapNode*)calloc(initialCapacity, sizeof(HeapNode));
    if (data) {
        pq->data = data;
    }
    else {
        ASSERT(!data, "No memory allocated\n");
    }
}

// Insert a new element into the priority queue
void insertNodeInPQ(PriorityQueue* pq, HeapNode node) {
    if (pq->current_size == pq->capacity) {
        resizePQ(pq); // Resize if full
    }
    pq->data[pq->current_size] = node; // Add node at the end
    heapifyUp(pq, pq->current_size);   // Restore heap property
    pq->current_size++;  // Increment AFTER heapifyUp
}

// Extract the element with the highest priority
HeapNode extractMaxFromPQ(PriorityQueue* pq) {
    if (isEmptyPQ(pq)) {
        printf("Priority Queue Underflow\n");
        exit(EXIT_FAILURE);
    }
    HeapNode maxNode = pq->data[0];
    pq->data[0] = pq->data[pq->current_size - 1]; // Fix: Assign last element
    pq->current_size--;  // Fix: Decrement AFTER swap
    heapifyDown(pq, 0);  // Fix: Maintain heap property
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
    // If the mandatory flag differs, prioritize the mandatory patient first
    if (a.mandatory != b.mandatory) {
        return a.mandatory - b.mandatory;  // Prioritize mandatory=1
    }

    // If both patients are mandatory, prioritize by due_day and then by length_of_stay
    if (a.mandatory == 1) {
        if (a.due_day != b.due_day) {
            return a.due_day - b.due_day;  // Patient with earlier due_day comes first
        }
        return a.los - b.los;  // If due_day is the same, sort by length_of_stay
    }

    // If both patients are not mandatory, prioritize by delay
    if (a.delay != b.delay) {
        return a.delay - b.delay;  // Higher delay has higher priority
    }

    // If all fields are equal, return -1 (default)
    return -1;
}



void resizePQ(PriorityQueue* pq) {
    // resizes PQ's capacity
    pq->capacity *= 2; // Double the capacity
    HeapNode* data = (HeapNode*)realloc(pq->data, pq->capacity * sizeof(HeapNode));
    if (data) {
        pq->data = data;
    }
    else {
        ASSERT(!data, "No memory allocated to pq.\n");
    }
}


void printPQTree(PriorityQueue* pq, int index, int space, int level, char branch) {
    if (index >= pq->current_size) return;

    space += 5;  // Increase spacing for better alignment

    // Print right child first for proper visualization
    printPQTree(pq, 2 * index + 2, space, level + 1, '/');

    // Print the current node with appropriate indentation
    printf("\n");
    for (int i = 5; i < space; i++) printf(" ");

    if (level > 0) {
        printf("|--");  // Use "|--" to indicate branching
    }

    printf("(%d)\n", pq->data[index].patient_id);

    // Print left child
    printPQTree(pq, 2 * index + 1, space, level + 1, '\\');
}

// Function to print tree with edges
void printPQWithEdges(PriorityQueue* pq) {
    if (pq->current_size == 0) {
        printf("Priority Queue is empty.\n");
        return;
    }

    printf("Priority Queue Tree Structure:\n");
    printPQTree(pq, 0, 0, 0, ' ');
}

void freePQ(PriorityQueue* pq) {
    free(pq->data);
    // pq->data = NULL;
    // pq->current_size = 0;
    // pq->capacity = 0;
    free(pq);
}



HeapNode makeHeapNode(int p_id, int mandatory, int due_day, int delay, int los) {
    // creates a heap_node for PQ
    HeapNode node = { p_id, mandatory, due_day, delay, los };
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

//-----------------------------------------ABOVE: QUICK SORT IMPLEMENTATION FOR THE OT ARRAY--------------------------------------

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
    //vector->size++;  // Missing increment
}


Rooms* removebyid(RoomVector* vector, int room_id) {
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
    Rooms* room = removebyid(source, room_id);
    if (room) {
        pushback(destination, room);
    }
    else {
        ASSERT(!room, "No room available.\n");
    }
}

void freevector(RoomVector* vector) {
    if (!vector) return;
    free(vector->data);
    free(vector);  // Free the struct itself
}


void make_3_vectors(int** room_gender_map) {
    init_Rooms(v_A, 2);
    init_Rooms(v_B, 2);
    init_Rooms(v_empty, 2);

    for (int i = 0; i < num_rooms; i++) {
        if (((*room_gender_map)[i]) == -1)
            pushback(v_empty, &room[i]);
        else {
            if (((*room_gender_map)[i]) == 0)
                pushback(v_A, &room[i]);
            else
                pushback(v_B, &room[i]);
        }
    }
}

void printVector(const char* label, RoomVector* vector) {
    printf("%s:\n", label);
    for (int i = 0; i < vector->size; i++) {
        printf("Room ID: %d, Capacity: %d, occupants: %d, patients: %d\n", vector->data[i]->id, vector->data[i]->cap, vector->data[i]->occupants_cap, vector->data[i]->num_patients_allocated);
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
    // Returns a room_id OR -1 if no suitable room is found
    int flag = 0, j, r_id;
    gender g = patients[p_id].gen; // Gender of patient
    char* age = patients[p_id].age_group; // Age group of patient

    // First pass: Look in associated gender-based vector
    for (int i = 0; i < vector->size; i++) {
        flag = 0;
        r_id = vector->data[i]->id;

        // Check if room is full
        if ((room[r_id].cap == (room[r_id].num_patients_allocated + room[r_id].occupants_cap)) || room[r_id].gen != g ) {
            continue;
        }

        // Check for incompatible rooms
        for (j = 0; j < patients[p_id].num_incompatible_rooms; j++) {
            if (r_id == patients[p_id].incompatible_room_ids[j]) {
                flag = 1; // Mark as incompatible
                break;
            }
        }

        // If incompatible, skip this room
        if (flag) continue;

        return r_id; // Found a suitable room
    }

    // Second pass: Look in empty rooms if no room was assigned
    if (patients[p_id].assigned_room_no == -1) {
        for (int i = 0; i < v_empty->size; i++) {
            flag = 0;
            r_id = v_empty->data[i]->id;

            // Check if room is full
            if (room[r_id].cap == (room[r_id].num_patients_allocated + room[r_id].occupants_cap)) {
                continue;
            }

            // Check for incompatible rooms
            for (j = 0; j < patients[p_id].num_incompatible_rooms; ++j) {
                if (r_id == patients[p_id].incompatible_room_ids[j]) {
                    flag = 1;
                    break;
                }
            }

            // If incompatible, skip this room
            if (flag) continue;

            // Move room from empty list to the main vector
            moveRoom(v_empty, vector, r_id);
            room[r_id].gen = g;

            return r_id; // Found a suitable room
        }
    }

    return -1; // No suitable room found
}

void update_LOS_of_patients(int d) {
    /* Check if any patient's length_of_stay is over and if yes then remove that patient from -
        1. the room he's been assigned to.
        2. any assignment in the NRA problem pertaining to this patient.
    */

    int i, g, r_id, admit_day, los, days_passed;
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
        if (days_passed > 0 && occupants[i].is_admitted == 1) {
            // throw the occupant out
             //if(room[r_id].occupants_cap > 0)
            room[r_id].occupants_cap--;
            occupants[i].is_admitted = 0;
            if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
                // remove the room from the associated gender array and append at the head of the empty_array linked list
                //g ? removeAndAppendGenderRoom(r_id, gender_B_rooms) : removeAndAppendGenderRoom(r_id, gender_A_rooms);
                g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
                room[r_id].gen = -1;
            }
        }
    }

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
                if (days_passed >= los && p->pointer->is_admitted) {
                    // release the patient
                    // remove this patient's data from everywhere, i.e. 
                    // from room (capacity, gender, age-mix)
                    // update the data in nurses array about this patient
                    room[r_id].num_patients_allocated--;
                    p->pointer->is_admitted = 0;
                    // ignore age-mix
                    // consider if the room is now empty then what will you do
                    if (!room[r_id].occupants_cap && !room[r_id].num_patients_allocated) {
                        // remove this room from the associated gender array and append it in empty_rooms array
                        g ? moveRoom(v_B, v_empty, r_id) : moveRoom(v_A, v_empty, r_id);
                        room[r_id].gen = -1;
                    }
                }
            }
        }
    }
}

void free_s_data_arr(Surgeon_data** s_data_arr, int n) {
    // frees s_data_arr array
    int i;
    for (i = 0; i < n; ++i)
        free(s_data_arr[i]->assigned_patients);
    free(s_data_arr);
}

// --------------------BELOW: Functions for admitting patients from the PQ (linked list functions as well)---------------------



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



OTs* admitFromPQ(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index) {
    Mand_opt_PQ* deferredPatients = (Mand_opt_PQ*)malloc(sizeof(Mand_opt_PQ));
    init_Mand_opt_PQ(deferredPatients, 20);

    while (pq->current_size) {
        if (!pq->data[0].mandatory) break;
        HeapNode node = extractMaxFromPQ(pq);
        int p_id = node.patient_id;

        if (!patients[p_id].is_admitted) {
            int s_duration = patients[p_id].surgery_duration;
            int admitted = 0;
            int is_due_today = (patients[p_id].surgery_due_day == d);

            // Emergency handling for due-day patients
            if (is_due_today) {
                // Force admission even if constraints are violated
                for (int i = 0; i < num_ots && !admitted; i++) {
                    OTs* ot = ot_data_arr[i];
                    int surgeon_id = patients[p_id].surgeon_id;
                    int r_id = -1;

                    // 1. Try normal room assignment
                    r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);

                    // 2. Try empty rooms if no gender-specific room found
                    //if (r_id == -1 && v_empty->size > 0) {
                    //    r_id = v_empty->data[0];
                    //   // removeElement(v_empty, 0);  // Remove from empty list
                    //}

                    //// 3. Force assignment to any available room
                    /*if (r_id == -1) {
                        for (int room_idx = 0; room_idx < num_rooms; room_idx++) {
                            if (room[room_idx].num_patients_allocated + room[room_idx].occupants_cap < room[room_idx].cap) {
                                r_id = room_idx;
                                break;
                            }
                        }
                    }*/

                    if (r_id != -1) {
                        // Forcefully deduct time (even if negative)
                        ot->time_left[d] -= s_duration;
                        surgeon[surgeon_id].time_left[d] -= s_duration;

                        patients[p_id].assigned_room_no = r_id;
                        patients[p_id].assigned_ot = ot->id;
                        patients[p_id].admission_day = d;
                        patients[p_id].is_admitted = 1;
                        room[r_id].num_patients_allocated++;
                        admitted = 1;
                        current_ot_index = i;
                    }
                }
            }
            else {
                // Original logic for non-due-day patients
                for (int i = current_ot_index; i < num_ots && !admitted; i++) {
                    OTs* ot = ot_data_arr[i];
                    int surgeon_id = patients[p_id].surgeon_id;

                    if (ot->time_left[d] >= s_duration &&
                        surgeon[surgeon_id].time_left[d] >= s_duration) {
                        int r_id = patients[p_id].gen ?
                            findSuitableRoom(p_id, v_B) :
                            findSuitableRoom(p_id, v_A);
                        if (r_id != -1) {
                            ot->time_left[d] -= s_duration;
                            surgeon[surgeon_id].time_left[d] -= s_duration;
                            patients[p_id].assigned_room_no = r_id;
                            patients[p_id].assigned_ot = ot->id;
                            patients[p_id].admission_day = d;
                            patients[p_id].is_admitted = 1;
                            room[r_id].num_patients_allocated++;
                            admitted = 1;
                            current_ot_index = i;
                        }
                    }
                }
            }

            if (!admitted) {
                HeapNode* temp = (HeapNode*)malloc(sizeof(HeapNode));
                *temp = node;
                temp->delay = d - patients[temp->patient_id].surgery_release_day;
                pushback_Mand_Opt_PQ(deferredPatients, temp);
            }
        }
    }

    // Reinsert deferred patients (filter expired due dates)
    for (int i = 0; i < deferredPatients->size; i++) {
        int p_id = deferredPatients->data[i]->patient_id;
        if (patients[p_id].surgery_due_day > d) {
            insertNodeInPQ(pq, *(deferredPatients->data[i]));
        }
        free(deferredPatients->data[i]);
    }
    free(deferredPatients->data);
    free(deferredPatients);

    return ot_data_arr[current_ot_index % num_ots];
}
// --------------------ABOVE: Functions for admitting optional patients from the PQ (linked list functions as well)---------------------


//void admit_patients(int** room_gender_map, PriorityQueue* pq) {
//   /*
//       we'll go patient by patient.
//       when a patient comes, we'll check what's his release_day and due_day
//           if the due_day == release_day then admit him at any cost
//           1. check his s_id - if the surgeon is not available then insert him into the PQ
//           2. if surgeon is available,
//
//           - put patients with same surgeon in one OT, thereby controlling surgeon_transfer contraint
//           - do this for each linked-list in the sorted_mandatory_patients and sorted_optional_patients arrays
//           - for this, firstly create an array of size #(patients in the 1 linked-list) and put surgeon-id of each
//           patient as the element
//           - then sort it using counting sort (as we already know #surgeons OR we can find that also while parsing
//           the linked-list)
//   */
//
//   int i, j, k, l, d, g, r_id, p_id, s_id, flag, flag_opt = 0, count_admit_PQ = 0, s_duration, current_ot_index, s_duration_sum, max_surgeon_id, len_surgeon_array, temp_patient_id;
//   Surgeon_data** s_data_arr;
//   HeapNode node;
//   Node* head;
//   OTs** ot_data_arr, * current_ot, * new_ot;
//
//   v_A = (RoomVector*)malloc(sizeof(RoomVector));
//   v_B = (RoomVector*)malloc(sizeof(RoomVector));
//   v_empty = (RoomVector*)malloc(sizeof(RoomVector));
//
//   //make_3_room_arrays(room_gender_map);
//   make_3_vectors(&room_gender_map);
//  /* printVector("A", v_A);
//   printVector("B", v_B);cd
//   printVector("Empty", v_empty);*/
//   // first, make another OT array (array of pointers to structures)
//  // copy all the OT struct pointers to the ot_data_array.
//   ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
//   if (ot_data_arr == NULL) {
//      printf("Memory not allocated.\n");
//      exit(-1);
//   }
//   //print_ots(ot);
//
//   for (i = 0; i < num_ots; ++i)
//      ot_data_arr[i] = &ot[i];
//
//   // ---------------------------------------------------------START ADMISSION------------------------------------------------------------
//   for (d = 0; d < days; ++d) {
//       count_admit_PQ = 0;
//      // update LOS for all patients
//      if (d) update_LOS_of_patients(d);
//
//      sort_ot_data_arr(ot_data_arr, d);
//      current_ot_index = 0;
//      current_ot = ot_data_arr[0];
//      flag = 0;
//
//      Node* head = sorted_mandatory_patients[d];
//      //printf("sorted_mandatory_patients[%d]: %p\n", d, (void*)sorted_mandatory_patients[d]);
//      //printf("head:%p\n", head);
//      if (head) {
//          max_surgeon_id = find_max_surgeon_id(head);
//          len_surgeon_array = max_surgeon_id + 1;
//          // Added 1 to max_surgeon_id cuz we need space for the nth (max_is) also, so we'll have to make the total length (n+1)
//          s_data_arr = (Surgeon_data**)calloc(len_surgeon_array, sizeof(Surgeon_data*));
//          sort_s_data_arr(s_data_arr, len_surgeon_array, head); 
//      }
//      else {
//          continue;
//      }
//
//      //incrementing the current ot while we get to an ot which is available that day
//      while (current_ot && !current_ot->time_left[d]) {
//          if (current_ot_index >= num_ots) {
//              flag = 1;
//              break;
//          }
//          current_ot = ot_data_arr[current_ot_index++];
//      }
//
//
//      // extreme situation - when we come to a new day and find that no OT is available
//      if (flag) {
//         for (i = 0; i < len_surgeon_array; ++i) {
//            if (!s_data_arr[i]->isNull) {
//               for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
//                  temp_patient_id = s_data_arr[i]->assigned_patients[k];
//                  insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day,0, patients[temp_patient_id].length_of_stay));
//                  //printPriorityQueue(pq); printf("\n2241");
//               }
//            }
//         }
//         continue; // go to the next day and then admit these patients
//      }
//
//      flag = 0;
//      // admit the mandatory patients previously not admitted from the PQ
//      if (d && pq->current_size > 0 && count_admit_PQ < 1) {
//          current_ot = admitFromPQ(pq, d, ot_data_arr, current_ot, current_ot_index);
//          count_admit_PQ++;
//          flag_opt = 1;
//      }
//      if (!current_ot) {
//         for (i = 0; i < len_surgeon_array; ++i)
//            if (!s_data_arr[i]->isNull)
//               for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
//                  temp_patient_id = s_data_arr[i]->assigned_patients[k];
//                  insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
//                  //printPriorityQueue(pq); printf("\n2261");
//               }
//         continue; // go to the next day and then admit these patients
//      }
//
//
//      for (i = 0; i < len_surgeon_array; ++i)
//         if (!s_data_arr[i]->isNull) {
//            if (!surgeon[s_data_arr[i]->surgeon_id].time_left[d]) {
//               // the surgeon is not available and hence add all the patients to the PQ
//               for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
//                  temp_patient_id = s_data_arr[i]->assigned_patients[k];
//                  if (patients[temp_patient_id].mandatory && patients[temp_patient_id].surgery_due_day == d) {
//                      //admitting the patient at any cost
//                      s_duration = patients[temp_patient_id].surgery_duration;
//                      if (s_duration <= current_ot->time_left[d] && surgeon[s_data_arr[i]->surgeon_id].time_left[d] >= s_duration) {
//                          // no need to check now actually but check only to be safe
//                          // assign OT--------------------------------------------------------------------------------------------------
//                          patients[temp_patient_id].assigned_ot = current_ot->id;
//                          // update the time left
//                          current_ot->time_left[d] -= s_duration;
//                          surgeon[s_data_arr[i]->surgeon_id].time_left[d] -= s_duration;
//                      }
//                      r_id = patients[temp_patient_id].gen ? findSuitableRoom(temp_patient_id, v_B) : findSuitableRoom(temp_patient_id, v_A);
//                      if (r_id != -1 ) {
//                          patients[temp_patient_id].assigned_room_no = r_id;
//                          room[r_id].num_patients_allocated += 1;
//                          patients[temp_patient_id].admission_day = d;
//                          patients[temp_patient_id].assigned_ot = current_ot->id;
//                          //surgeon[s_data_arr[k]->surgeon_id].time_left[d] -= s_duration;
//                          patients[temp_patient_id].is_admitted = 1;
//                      }
//                      
//                      /*printVector("A", v_A);
//                      printVector("B", v_B);
//                      printVector("Empty", v_empty);*/
//                  }
//                  else
//                  insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
//                  //printPriorityQueue(pq); printf("\n2291");
//               }
//               continue;
//            }
//            // if the control comes here, that means the surgeon has time on that day to operate patients
//            for (j = 0; j < s_data_arr[i]->num_assigned_patients; ++j) {
//               p_id = s_data_arr[i]->assigned_patients[j];
//               // s_id = s_data_arr[i]->surgeon_id;
//               s_duration = patients[p_id].surgery_duration;
//               // s_duration_sum = s_data_arr[i]->surgery_duration_sum;
//               if (patients[p_id].mandatory) {
//                   // assign available OT
//                   // first - select a suitable OT
//                   while (current_ot->time_left[d] < s_duration) {
//                       if (current_ot_index == num_ots) { flag = 1; break; }
//                       current_ot = ot_data_arr[current_ot_index++];
//                   }
//                   if (flag) {
//                       for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
//                           temp_patient_id = s_data_arr[i]->assigned_patients[k];
//                           insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
//                           //printPriorityQueue(pq); printf("\n2312");
//                       }
//                       break;
//                   }
//                   if (s_duration <= current_ot->time_left[d]) {
//                       // no need to check now actually but check only to be safe
//                       // assign OT--------------------------------------------------------------------------------------------------
//                       //patients[p_id].assigned_ot = current_ot->id;
//                       //// update the time left
//                       //current_ot->time_left[d] -= s_duration;
//                    //}
//                    // assign a room
//                       r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
//                       if (r_id == -1) {
//                           // means there's no room in the gender_room_array and empty_array that is compatible with this patient
//                           // so in this case - put the patient in the PQ and move on
//                           insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                           //printPriorityQueue(pq); printf("\n2329");
//                           continue;
//                       }
//                       else {
//                           if (surgeon[patients[p_id].surgeon_id].time_left[d] >= s_duration) {
//                               patients[p_id].assigned_room_no = r_id;
//                               room[r_id].num_patients_allocated += 1;
//                               // assign admission day
//                               patients[p_id].admission_day = d;
//                               patients[p_id].assigned_ot = current_ot->id;
//                               current_ot->time_left[d] -= s_duration;
//                               surgeon[patients[p_id].surgeon_id].time_left[d] -= s_duration;
//                               patients[i].is_admitted = 1;
//                               printVector("A", v_A);
//                               printVector("B", v_B);
//                               printVector("Empty", v_empty); 
//                           }
//                           else {
//                               insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                           }
//                       }
//                   }
//               }
//               // the patient is optional - 
//               /*
//               Bcz we know that in the sorted_mandatory_patients array, first there are mandatory patients in the linked list and then
//               the optional ones are there. So the control will only come to this else block when all the mandatory patients have been
//               dealt with.
//               Now we will try to admit these optional patients as well and if the conditions dont match then we'll put the into the PQ
//               for the next day.
//               The next day we'll try to
//               1. first admit all the mandatory patients in the PQ.
//               2. Then we'll admit today's mandatory patients
//               3. then yesterday's optional patients from the PQ and
//               4. then today's optional patients.
//               */
//               else {
//                  // continue;
//                   /*if (pq->current_size > 0 && count_admit_PQ < 2) {
//                       current_ot = admitFromPQ(pq, d, ot_data_arr, current_ot, current_ot_index);
//                       count_admit_PQ++;
//                   }*/
//                   //admit today's optionals
//                  // otherwise - put into the PQ
//                  if (current_ot) {
//                     // write code here for admitting today's optional patients
//                     r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
//                     if (r_id == -1) {
//                         insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                         //printPriorityQueue(pq); printf("\n2367");
//                         continue;
//                     }
//                     else {
//                         while (current_ot_index < num_ots && current_ot->time_left[d] < s_duration) {
//                             current_ot_index++;
//                             if (current_ot_index < num_ots) {  // Prevent accessing out of bounds
//                                 current_ot = ot_data_arr[current_ot_index];
//                             }
//
//                         }
//                     }
//
//                     if (current_ot_index == num_ots) {
//                         current_ot_index--;
//                         current_ot = ot_data_arr[current_ot_index];
//                     }
//                     if (current_ot->time_left[d] && current_ot->time_left[d] >= s_duration && surgeon[s_data_arr[i]->surgeon_id].time_left[d] >= patients[p_id].surgery_duration) {
//                         patients[p_id].admission_day = d;
//                         patients[p_id].assigned_room_no = r_id;
//                         patients[p_id].assigned_ot = current_ot->id;
//                         room[r_id].num_patients_allocated += 1;
//                         //int value = current_ot->time_left[d] - patients[p_id].surgery_duration;
//                         current_ot->time_left[d] = current_ot->time_left[d] - patients[p_id].surgery_duration;
//                         surgeon[s_data_arr[i]->surgeon_id].time_left[d] = surgeon[s_data_arr[i]->surgeon_id].time_left[d] - patients[p_id].surgery_duration;
//                         patients[i].is_admitted = 1;
//                     }
//                     else {
//                         insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
//                     }
//                  }
//                  else {
//                     for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
//                        temp_patient_id = s_data_arr[i]->assigned_patients[k];
//                        insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
//                       // printPriorityQueue(pq); printf("\n2394");
//                     }
//                  }
//               }
//            }
//         }
//      free_s_data_arr(s_data_arr, len_surgeon_array);
//   }
//   /*freevector(v_A);
//   freevector(v_B);
//   freevector(v_empty);*/
//}



void admit_patients_2(int** room_gender_map, PriorityQueue* pq) {
    int i, j, k, l, d, g, r_id, p_id, s_id, flag, flag_opt = 0, count_admit_PQ = 0, s_duration, current_ot_index, s_duration_sum, max_surgeon_id, len_surgeon_array, temp_patient_id;
    Surgeon_data** s_data_arr;
    HeapNode node;
    Node* head, * temp;
    OTs** ot_data_arr, * current_ot, * new_ot;

    /*v_A = (RoomVector*)malloc(sizeof(RoomVector));
    v_B = (RoomVector*)malloc(sizeof(RoomVector));
    v_empty = (RoomVector*)malloc(sizeof(RoomVector));

    make_3_vectors(&room_gender_map);*/
    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    //print_ots(ot);

    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = &ot[i];

    for (int d = 0; d < days; d++) {
        count_admit_PQ = 0;
        // update LOS for all patients
        if (d) update_LOS_of_patients(d);

        sort_ot_data_arr(ot_data_arr, d);
        current_ot_index = 0;
        current_ot = ot_data_arr[0];
        flag = 0;

        Node* head = sorted_mandatory_patients[d];
        temp = head;

        //incrementing the current ot while we get to an ot which is available that day
        while (current_ot && !current_ot->time_left[d]) {
            if (current_ot_index >= num_ots) {
                flag = 1;
                break;
            }
            current_ot = ot_data_arr[current_ot_index++];
        }

        //condition when no ot is available that day
        if (flag) {
            while (temp) {
                int temp_patient_id = temp->pointer->id;
                insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
                temp = temp->next;
            }
            continue;
        }

        flag = 0;
        // admit the mandatory patients previously not admitted from the PQ
        if (d && pq->current_size > 0 && count_admit_PQ < 1) {
            current_ot = admitFromPQ(pq, d, ot_data_arr, current_ot, current_ot_index);
            count_admit_PQ++;
            flag_opt = 1;
        }

        if (!current_ot) {
            while (temp) {
                int temp_patient_id = temp->pointer->id;
                insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0, patients[temp_patient_id].length_of_stay));
                temp = temp->next;
            }
            continue;
        }


        while (temp) {
            current_ot = ot_data_arr[0];
            current_ot_index = 0;
            /* case 1: if the patient is mandatory then admit him by checking the following conditions:
                        i. check surgeon availability : if surgeon not available put patient in pq
                        ii. check ot availability: if ot not available then put patient in pq
                        iii. extreme condition: if surgeon not available and mandatory patient and also due day
                              is today then admit at any cost.
                  */
                  // if (temp->pointer->mandatory) {
            p_id = temp->pointer->id;
            s_duration = temp->pointer->surgery_duration;
            //if(surgeon[temp->pointer->surgeon_id].time_left[d]) {
            if (current_ot->time_left[d] >= s_duration && surgeon[temp->pointer->surgeon_id].time_left[d] >= s_duration) {
                r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                if (r_id != -1) {
                    patients[p_id].admission_day = d;
                    patients[p_id].assigned_room_no = r_id;
                    patients[p_id].assigned_ot = current_ot->id;
                    room[r_id].num_patients_allocated += 1;
                    //int value = current_ot->time_left[d] - patients[p_id].surgery_duration;
                    current_ot->time_left[d] = current_ot->time_left[d] - patients[p_id].surgery_duration;
                    surgeon[temp->pointer->surgeon_id].time_left[d] = surgeon[temp->pointer->surgeon_id].time_left[d] - patients[p_id].surgery_duration;
                    patients[p_id].is_admitted = 1;
                }
                else {
                    insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                    temp = temp->next;
                    continue;
                }
            }
            else {
                if (current_ot->time_left[d] < s_duration) {
                    while (current_ot->time_left[d] < s_duration) {
                        if (current_ot_index == num_ots) { flag = 1; break; }
                        current_ot = ot_data_arr[current_ot_index++];
                    }
                    if (flag) {
                        insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                        temp = temp->next;
                        continue;
                    }
                    else {
                        if (surgeon[temp->pointer->surgeon_id].time_left[d] >= s_duration) {
                            r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                            if (r_id != -1) {
                                patients[p_id].admission_day = d;
                                patients[p_id].assigned_room_no = r_id;
                                patients[p_id].assigned_ot = current_ot->id;
                                room[r_id].num_patients_allocated += 1;
                                //int value = current_ot->time_left[d] - patients[p_id].surgery_duration;
                                current_ot->time_left[d] = current_ot->time_left[d] - patients[p_id].surgery_duration;
                                surgeon[temp->pointer->surgeon_id].time_left[d] = surgeon[temp->pointer->surgeon_id].time_left[d] - patients[p_id].surgery_duration;
                                patients[p_id].is_admitted = 1;
                            }
                            else {
                                insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                                temp = temp->next;
                                continue;
                            }
                        }
                        else {
                            insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                            temp = temp->next;
                            continue;
                        }
                    }
                }
            }
            //}

            temp = temp->next;
        }

    }
}


void admit_remaining_patients(PriorityQueue* pq) {
    HeapNode node;
    int p_counter;
    OTs curr_ot;
    Mand_opt_PQ* vector = (Mand_opt_PQ*)calloc(1, sizeof(Mand_opt_PQ));
    init_Mand_opt_PQ(vector, 20);

    for (int day = 0; day < days; day++) {
        for (p_counter = 0; p_counter < pq->current_size && pq->current_size; p_counter++) {
            if (pq->data[p_counter].mandatory) continue;
            node = extractMaxFromPQ(pq);
            int p_id = node.patient_id;
            if (!patients[p_id].is_admitted) {
                for (int i = 0; i < num_ots; i++) {
                    if (ot[i].time_left[day] && ot[i].time_left[day] >= patients[p_id].surgery_duration) {
                        curr_ot = ot[i];
                        break;
                    }
                }
                if (surgeon[patients[p_id].surgeon_id].time_left[day] >= patients[p_id].surgery_duration) {
                    int r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                    if (r_id != -1) {
                        curr_ot.time_left[day] -= patients[p_id].surgery_duration;
                        surgeon[patients[p_id].surgeon_id].time_left[day] -= patients[p_id].surgery_duration;
                        patients[p_id].assigned_room_no = r_id;
                        patients[p_id].assigned_ot = curr_ot.id;
                        patients[p_id].admission_day = day;
                        patients[p_id].is_admitted = 1;
                        room[r_id].num_patients_allocated++;
                    }
                }

                else {
                    //insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0, patients[p_id].length_of_stay));
                    pushback_Mand_Opt_PQ(vector, &node);

                }
            }
        }
        for (int i = 0; i < vector->size; i++) {
            insertNodeInPQ(pq, *(vector->data[i]));
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
// -----------------------------------------------FUNCTIONS BELOW ARE FOR NURSE ASSIGNMENTS--------------------------------------------------

//...............FUNCTION FOR CREATING THE 3-D ARRAY TO STORE THE PATIENTS IN THE ROOM ON A PARTICULAR DAY.........

void initialize_3d_array() {
    // Allocate memory for room_schedule: 3D array (rooms x days x patients)
    room_schedule = (char****)calloc(num_rooms, sizeof(char***));  // Allocate space for rooms
    size_of_room_schedule = (int**)calloc(num_rooms, sizeof(int*));  // Allocate space for size tracking by room

    for (int i = 0; i < num_rooms; i++) {
        room_schedule[i] = (char***)calloc(days, sizeof(char**));  // Allocate space for days per room
        size_of_room_schedule[i] = (int*)calloc(days, sizeof(int));  // Allocate space for size tracking per day

        //for (int j = 0; j < days; j++) {
            //room_schedule[i][j] = (char**)calloc(10, sizeof(char*));  // Pre-allocate space for 10 patients per day in each room
            //size_of_room_schedule[i][j] = 0;  // Initialize the patient count for each room/day to 0
    }
}


char* id_to_str(char prefix, int id) {
    char* result = (char*)malloc(12 * sizeof(char)); // Enough space for prefix + number + null terminator
    if (result == NULL) {
        return NULL; // Return NULL if memory allocation fails
    }
    snprintf(result, 12, "%c%d", prefix, id); // Format the string
    return result;
}

void put_occupants(void) {
    // here we'll add occupants in the room_schedule array
    int i, r_id, los,j;
    for (i = 0; i < num_occupants; ++i) {
        r_id = occupants[i].room_id;
        los = occupants[i].length_of_stay;

        for (j = 0; j < los; ++j) {
            if (j >= days) break;

            if (room_schedule[r_id][j]) {
                // Corrected: Use a temporary pointer before realloc
                int new_size = size_of_room_schedule[r_id][j] + 1;
                char** temp = (char**)realloc(room_schedule[r_id][j], new_size * sizeof(char*));

                if (!temp) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                room_schedule[r_id][j] = temp;
                room_schedule[r_id][j][size_of_room_schedule[r_id][j]] = id_to_str('a', occupants[i].id);
                size_of_room_schedule[r_id][j] = new_size;
            }
            else {
                // Corrected: Allocate memory properly
                room_schedule[r_id][j] = (char**)calloc(2,sizeof(char*));
                if (!room_schedule[r_id][j]) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                size_of_room_schedule[r_id][j] = 1;
                room_schedule[r_id][j][0] = id_to_str('a', occupants[i].id);
            }
        }
    }
}

void create_3d_array(void) {
    initialize_3d_array();
    int i, j, admission_day, r_id, los;

    put_occupants();
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
                char** temp = (char**)realloc(room_schedule[r_id][j], new_size * sizeof(char*));

                if (!temp) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                room_schedule[r_id][j] = temp;
                room_schedule[r_id][j][size_of_room_schedule[r_id][j]] = id_to_str('p', patients[i].id);
                size_of_room_schedule[r_id][j] = new_size;
            }
            else {
                // Corrected: Allocate memory properly
                room_schedule[r_id][j] = (char**)calloc(2,sizeof(char*));
                if (!room_schedule[r_id][j]) {
                    ASSERT(0, "Dynamic Memory Allocation Error.");
                    return;
                }

                size_of_room_schedule[r_id][j] = 1;
                room_schedule[r_id][j][0] = id_to_str('p', patients[i].id);
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
                    if (room_schedule[r][d][p]) { // Ensure pointer is valid before printing
                        char* id_str = room_schedule[r][d][p];
                        int id = str2int(id_str); // Extract numerical ID
                        printf("[%s (ID: %d)] ", id_str, id);
                    }
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

        if (arr[j]->shift->load_left > pivot) {
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
    Nurses** temp = (Nurses**)realloc(main_array[index], new_size * sizeof(Nurses*));
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
            if ((room_schedule[i][j] && !size_of_room_schedule[i][j]) || (!room_schedule[i][j] && size_of_room_schedule[i][j]))
                ASSERT(0, "Something wrong is happening.");
            if (!room_schedule[i][j]) {
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
                    int id = str2int(room_schedule[i][j][k]);

                    // Ensure valid index before accessing arrays
                    int workload_index = 3 * j + x;
                    int skill_index = 3 * j + x;
                    if (j >= patients[id].length_of_stay) continue;
                    sum += patients[id].workload_produced[workload_index];
                    max_skill = (max_skill > patients[id].skill_level_required[skill_index])
                        ? max_skill
                        : patients[id].skill_level_required[skill_index];
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



void initialize_room_shift_nurse() {
    room_shift_nurse = (int**)calloc(num_rooms, sizeof(int*));

    for (int i = 0; i < num_rooms; i++) {
        room_shift_nurse[i] = (int*)calloc(3 * days, sizeof(int));
    }
}


//void nurse_assignments() {
//    for (int i = 0; i < days; i++) {
//        for (int j = 0; j < num_rooms; j++) {
//            for (int k = 0; k < 3; k++) {
//                int m = 0;
//
//                // Skip if room has no load but requires a certain skill level
//                if (rooms_requirement[j][3 * i + k].load_sum == 0 && rooms_requirement[j][3 * i + k].max_skill_req)
//                    break;
//
//                Nurses** arr = dm_nurses_availability[i * 3 + k];
//                int arr_size = current_size_dm_nurse[i * 3 + k];
//
//                if (room[j].nurses_alloted == NULL) {
//                    room[j].nurses_alloted = NULL;
//                    room[j].length_of_nurses_alloted = 0;
//                }
//
//                Nurses* nurse_to_be_assigned = NULL;
//
//                // **Step 1: Check if any patient in the room has a previously assigned nurse**
//                for (int p = 0; p < size_of_room_schedule[j][i]; p++) {
//                    int p_id = str2int(room_schedule[j][i][p]);
//                    int prev_nurse_id = room[j].nurses_alloted[];
//
//                    if (prev_nurse_id != -1) { // If a nurse was assigned before
//                        for (m = 0; m < arr_size; m++) {
//                            if (arr[m]->id == prev_nurse_id && max_load_updated[arr[m]->id][3 * i + k] >= rooms_requirement[j][3 * i + k].load_sum) {
//                                nurse_to_be_assigned = arr[m]; // Prioritize the same nurse
//                                break;
//                            }
//                        }
//                    }
//                    if (nurse_to_be_assigned) break; // Found a suitable previous nurse, stop searching
//                }
//
//                // **Step 2: If no previously assigned nurse is available, find a new one**
//                if (nurse_to_be_assigned == NULL) {
//                    for (m = 0; m < arr_size; m++) {
//                        if (max_load_updated[arr[m]->id][3 * i + k] >= rooms_requirement[j][3 * i + k].load_sum) {
//                            nurse_to_be_assigned = arr[m]; // Found a suitable nurse
//                            break;
//                        }
//                    }
//                }
//
//                // **Step 3: If no nurse meets the requirement, find the one with the smallest deficit**
//                if (nurse_to_be_assigned == NULL) {
//                    int min_deficit = INT_MAX;
//                    for (m = 0; m < arr_size; m++) {
//                        int deficit = rooms_requirement[j][3 * i + k].load_sum - max_load_updated[arr[m]->id][3 * i + k];
//                        if (deficit < min_deficit) {
//                            min_deficit = deficit;
//                            nurse_to_be_assigned = arr[m];
//                        }
//                    }
//                }
//
//                // **Step 4: Assign the nurse and update nurse_alloted in patients**
//                if (nurse_to_be_assigned != NULL) {
//                    int* temp = (int*)realloc(room[j].nurses_alloted, (room[j].length_of_nurses_alloted + 1) * sizeof(int));
//                    if (temp == NULL) {
//                        perror("Memory allocation failed for nurses_alloted");
//                        exit(EXIT_FAILURE);
//                    }
//
//                    room[j].nurses_alloted = temp;
//                    room[j].nurses_alloted[room[j].length_of_nurses_alloted++] = nurse_to_be_assigned->id;
//                    max_load_updated[nurse_to_be_assigned->id][3 * i + k] -= rooms_requirement[j][3 * i + k].load_sum;
//                    room_shift_nurse[j][3 * i + k] = nurse_to_be_assigned->id;
//
//                    // **Update `nurse_alloted` for all patients in the room**
//                    for (int p = 0; p < room[j].num_patients; p++) {
//                        room[j].patients[p].nurse_alloted = nurse_to_be_assigned->id;
//                    }
//
//                    // **Step 5: Assign room to nurse’s shift (avoiding duplicates)**
//                    for (int z = 0; z < nurse_to_be_assigned->num_shifts; z++) {
//                        if (nurse_to_be_assigned->shift[z].day != i) continue;
//
//                        bool already_assigned = false;
//                        for (int p = 0; p < nurse_to_be_assigned->shift[z].num_rooms; p++) {
//                            if (nurse_to_be_assigned->shift[z].rooms[p] == room[j].id) {
//                                already_assigned = true;
//                                break;
//                            }
//                        }
//
//                        if (!already_assigned) {
//                            int* temp_room = NULL;
//                            int new_size = nurse_to_be_assigned->shift[z].num_rooms + 1;
//                            temp_room = (nurse_to_be_assigned->shift[z].rooms) ?
//                                (int*)realloc(nurse_to_be_assigned->shift[z].rooms, new_size * sizeof(int)) :
//                                (int*)calloc(new_size, sizeof(int));
//
//                            if (temp_room) {
//                                nurse_to_be_assigned->shift[z].rooms = temp_room;
//                                nurse_to_be_assigned->shift[z].rooms[nurse_to_be_assigned->shift[z].num_rooms] = room[j].id;
//                                nurse_to_be_assigned->shift[z].num_rooms++;
//                            }
//                            else {
//                                fprintf(stderr, "Memory allocation error for nurse shift rooms.\n");
//                                exit(EXIT_FAILURE);
//                            }
//                        }
//                    }
//                }
//                else {
//                    fprintf(stderr, "No valid nurse available for Room %d, Shift %d\n", j, 3 * i + k);
//                }
//            }
//        }
//    }
//}


//.........INCORPORATING EXCESSIVE NURSE LOAD......//
void nurse_assignments() {
    for (int i = 0; i < days; i++) {
        for (int j = 0; j < num_rooms; j++) {
            for (int k = 0; k < 3; k++) {
                sorting_nurse_id_max_load();
                // Skip if room has no load but requires a certain skill level
                if (rooms_requirement[j][3 * i + k].load_sum == 0 && rooms_requirement[j][3 * i + k].max_skill_req)
                    continue;

                Nurses** arr = dm_nurses_availability[i * 3 + k];
                int arr_size = current_size_dm_nurse[i * 3 + k];

                if (room[j].nurses_alloted == NULL) {
                    room[j].nurses_alloted = NULL;
                    room[j].length_of_nurses_alloted = 0;
                }

                Nurses* nurse_to_be_assigned = NULL;
                int max_remaining_capacity = -1;
                int best_nurse_index = -1;

                // Find a nurse with the most remaining capacity
                for (int m = 0; m < arr_size; m++) {
                    int nurse_id = arr[m]->id;
                    int remaining_capacity = max_load_updated[nurse_id][3 * i + k] - rooms_requirement[j][3 * i + k].load_sum;

                    if (remaining_capacity >= 0 && remaining_capacity > max_remaining_capacity) {
                        max_remaining_capacity = remaining_capacity;
                        best_nurse_index = m;
                    }
                }

                // If no nurse has enough capacity, find the one with the smallest overload
                if (best_nurse_index == -1) {
                    int min_overload = INT_MAX;
                    for (int m = 0; m < arr_size; m++) {
                        int nurse_id = arr[m]->id;
                        int overload = rooms_requirement[j][3 * i + k].load_sum - max_load_updated[nurse_id][3 * i + k];

                        if (overload < min_overload) {
                            min_overload = overload;
                            best_nurse_index = m;
                        }
                    }
                }

                // Assign the best nurse
                if (best_nurse_index != -1) {
                    nurse_to_be_assigned = arr[best_nurse_index];
                }

                // Ensure nurse_to_be_assigned is valid before accessing its data
                if (nurse_to_be_assigned != NULL) {
                    int nurse_id = nurse_to_be_assigned->id;
                    int max_load_of_nurse = max_load_updated[nurse_id][3 * i + k];

                    if (max_load_of_nurse > 0) {
                        // Assign nurse to room
                        int* temp = (int*)realloc(room[j].nurses_alloted, (room[j].length_of_nurses_alloted + 1) * sizeof(int));
                        if (temp == NULL) {
                            perror("Memory allocation failed for nurses_alloted");
                            exit(EXIT_FAILURE);
                        }

                        room[j].nurses_alloted = temp;
                        room[j].nurses_alloted[room[j].length_of_nurses_alloted++] = nurse_id;
                        max_load_updated[nurse_id][3 * i + k] -= rooms_requirement[j][3 * i + k].load_sum;
                        room_shift_nurse[j][3 * i + k] = nurse_id;

                        // Assign room to nurse's shift, ensuring no duplicates
                        for (int z = 0; z < nurse_to_be_assigned->num_shifts; z++) {
                            if (nurse_to_be_assigned->shift[z].day != i) {
                                continue;
                            }

                            // Check if the room is already assigned
                            bool already_assigned = false;
                            nurse_to_be_assigned->shift[z].load_left -= rooms_requirement[j][3 * i + k].load_sum;
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
                                    //nurse_to_be_assigned[nurse_id].shift[z].load_left -= rooms_requirement[j][3 * i + k].load_sum;
                                }
                                else {
                                    fprintf(stderr, "Memory allocation error for nurse shift rooms.\n");
                                    exit(EXIT_FAILURE);
                                }
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

//.....first logic......//
//void nurse_assignments() {
//    for (int i = 0; i < days; i++) {
//        for (int j = 0; j < num_rooms; j++) {
//            for (int k = 0; k < 3; k++) {
//                int m = 0;
//
//                // Skip if room has no load but requires a certain skill level
//                if (rooms_requirement[j][3 * i + k].load_sum == 0 && rooms_requirement[j][3 * i + k].max_skill_req)
//                    break;
//
//                Nurses** arr = dm_nurses_availability[i * 3 + k];
//                int arr_size = current_size_dm_nurse[i * 3 + k];
//
//                if (room[j].nurses_alloted == NULL) {
//                    room[j].nurses_alloted = NULL;
//                    room[j].length_of_nurses_alloted = 0;
//                }
//
//                Nurses* nurse_to_be_assigned = NULL;
//
//                // Find a suitable nurse
//                nurse_to_be_assigned = NULL; // Reset before searching
//                while (m < arr_size) {
//                    if (max_load_updated[arr[m]->id][3 * i + k] >= rooms_requirement[j][3 * i + k].load_sum) {
//                        nurse_to_be_assigned = arr[m]; // Found a suitable nurse
//                        break;
//                    }
//                    m++;
//                }
//
//                // If no suitable nurse is found, find the one with the smallest deficit
//                if (nurse_to_be_assigned == NULL) {
//                    int min_deficit = INT_MAX;
//                    for (m = 0; m < arr_size; m++) {
//                        int deficit = rooms_requirement[j][3 * i + k].load_sum - max_load_updated[arr[m]->id][3 * i + k];
//                        if (deficit < min_deficit) {
//                            min_deficit = deficit;
//                            nurse_to_be_assigned = arr[m];
//                        }
//                    }
//                }
//
//                // Ensure nurse_to_be_assigned is valid before accessing its data
//                if (nurse_to_be_assigned != NULL) {
//                    int max_load_of_nurse_to_be_assigned = max_load_updated[nurse_to_be_assigned->id][3 * i + k];
//
//                    if (max_load_of_nurse_to_be_assigned > 0) { // && (max_load_of_nurse_to_be_assigned - rooms_requirement[j][3 * i + k].load_sum) >= 3){
//                        // Nurse is suitable, proceed
//                    }
//                }
//
//                if (m < arr_size && nurse_to_be_assigned != NULL) {
//                    int* temp = (int*)realloc(room[j].nurses_alloted, (room[j].length_of_nurses_alloted + 1) * sizeof(int));
//                    if (temp == NULL) {
//                        perror("Memory allocation failed for nurses_alloted");
//                        exit(EXIT_FAILURE);
//                    }
//
//                    room[j].nurses_alloted = temp;
//                    room[j].nurses_alloted[room[j].length_of_nurses_alloted++] = nurse_to_be_assigned->id;
//                    max_load_updated[nurse_to_be_assigned->id][3 * i + k] -= rooms_requirement[j][3 * i + k].load_sum;
//                    room_shift_nurse[j][3 * i + k] = nurse_to_be_assigned->id;
//
//                    // Assign room to nurse's shift, ensuring no duplicates
//                    for (int z = 0; z < nurse_to_be_assigned->num_shifts; z++) {
//                        if (nurse_to_be_assigned->shift[z].day != i) {
//                            continue;
//                        }
//
//                        // Check if the room is already assigned
//                        bool already_assigned = false;
//                        for (int p = 0; p < nurse_to_be_assigned->shift[z].num_rooms; p++) {
//                            if (nurse_to_be_assigned->shift[z].rooms[p] == room[j].id) {
//                                already_assigned = true;
//                                break;
//                            }
//                        }
//
//                        if (!already_assigned) {
//                            int* temp_room = NULL;
//                            int new_size = nurse_to_be_assigned->shift[z].num_rooms + 1;
//                            if (!nurse_to_be_assigned->shift[z].rooms) {
//                                temp_room = (int*)calloc(new_size, sizeof(int));
//                            }
//                            else {
//                                temp_room = (int*)realloc(nurse_to_be_assigned->shift[z].rooms, new_size * sizeof(int));
//                            }
//
//
//                            if (temp_room) {
//                                nurse_to_be_assigned->shift[z].rooms = temp_room;
//                                nurse_to_be_assigned->shift[z].rooms[nurse_to_be_assigned->shift[z].num_rooms] = room[j].id;
//                                nurse_to_be_assigned->shift[z].num_rooms++;
//                            }
//                            else {
//                                fprintf(stderr, "Memory allocation error for nurse shift rooms.\n");
//                                exit(EXIT_FAILURE);
//                            }
//                        }
//                    }
//                }
//                else {
//
//                    fprintf(stderr, "No valid nurse available for Room %d, Shift %d\n", j, 3 * i + k);
//                }
//            }
//        }
//    }
//}


//void nurse_assignment_new() {
//    int i, n_id, j;
//    int** nurse_assigned_to_room = (int**)calloc(num_rooms, sizeof(int*));
//    for (i = 0; i < num_rooms; i++) nurse_assigned_to_room[i] = (int*)calloc(num_nurses, sizeof(int));
//
//
//    for (i = 0; i < num_rooms; i++) {
//        int* shifts_left = (int*)calloc(3 * days, sizeof(int));
//        do {
//            n_id = rand() % num_nurses;
//        } while (nurse_assigned_to_room[i][n_id] != 1);
//        for (j = 0; j < nurses[n_id].num_shifts; j++) {
//            int day = nurses[n_id].shift[j].day;
//            int shift = nurses[n_id].shift[j].shift_time;
//            if (!shifts_left[3 * day + shift]) {
//
//
//                int new_size = nurses[n_id].shift[j].num_rooms + 1;
//                int* temp_room = (int*)realloc(nurses[n_id].shift[j].rooms, new_size * sizeof(int));
//
//                if (temp_room) {
//                    nurses[n_id].shift[j].rooms = temp_room;
//                    nurses[n_id].shift[j].rooms[nurses[n_id].shift[j].num_rooms] = room[j].id;
//                    nurses[n_id].shift[j].num_rooms++;
//                }
//                nurses[n_id].shift[j].load_left -= rooms_requirement[j][3 * day + shift].load_sum;
//            }
//            shifts_left[3 * day + shift] = 1;
//        }
//
//    }
//}

void nurse_assignment_new() {
    int i, j, n_id;

    // Define a load threshold to prevent excessive work
    const int LOAD_THRESHOLD = 10;  // Example threshold for minimum remaining load

    // Allocate memory for nurse-room assignment tracking
    int** nurse_assigned_to_room = (int**)calloc(num_rooms, sizeof(int*));
    for (i = 0; i < num_rooms; i++) {
        nurse_assigned_to_room[i] = (int*)calloc(num_nurses, sizeof(int));
    }

    for (i = 0; i < num_rooms; i++) {
        int* shifts_left = (int*)calloc(3 * days, sizeof(int));

        // **Step 1: Try to find an already assigned nurse for continuity of care**
        int found_existing_nurse = 0;
        for (n_id = 0; n_id < num_nurses; n_id++) {
            if (nurse_assigned_to_room[i][n_id] == 1) {
                found_existing_nurse = 1;
                break;
            }
        }

        // **Step 2: If no assigned nurse, pick a nurse based on workload**
        if (!found_existing_nurse) {
            int best_nurse = -1;
            int max_load_left = -1;

            for (int k = 0; k < num_nurses; k++) {
                // Check if this nurse has at least one shift with available workload
                int has_capacity = 0;
                for (int s = 0; s < nurses[k].num_shifts; s++) {
                    if (nurses[k].shift[s].load_left >= LOAD_THRESHOLD) {
                        has_capacity = 1;
                        break;
                    }
                }

                // Pick the nurse with the most remaining load
                if (has_capacity && (best_nurse == -1 || nurses[k].shift[0].load_left > max_load_left)) {
                    best_nurse = k;
                    max_load_left = nurses[k].shift[0].load_left;
                }
            }

            // Assign the best available nurse
            if (best_nurse != -1) {
                n_id = best_nurse;
            }
            else {
                // If no nurse has enough load, pick the one with the *least* overload
                int min_overloaded_nurse = -1;
                int min_load = INT_MAX;
                for (int k = 0; k < num_nurses; k++) {
                    for (int s = 0; s < nurses[k].num_shifts; s++) {
                        if (nurses[k].shift[s].load_left < min_load) {
                            min_load = nurses[k].shift[s].load_left;
                            min_overloaded_nurse = k;
                        }
                    }
                }
                n_id = min_overloaded_nurse;
            }
        }

        // **Step 3: Assign nurse to shifts if workload allows**
        for (j = 0; j < nurses[n_id].num_shifts; j++) {
            int day = nurses[n_id].shift[j].day;
            int shift = nurses[n_id].shift[j].shift_time;

            if (!shifts_left[3 * day + shift] && nurses[n_id].shift[j].load_left >= LOAD_THRESHOLD) {
                int new_size = nurses[n_id].shift[j].num_rooms + 1;
                int* temp_room = (int*)realloc(nurses[n_id].shift[j].rooms, new_size * sizeof(int));

                if (temp_room) {
                    nurses[n_id].shift[j].rooms = temp_room;
                    nurses[n_id].shift[j].rooms[nurses[n_id].shift[j].num_rooms] = room[i].id;
                    nurses[n_id].shift[j].num_rooms++;
                }

                // **Reduce nurse's available load**
                nurses[n_id].shift[j].load_left -= rooms_requirement[i][3 * day + shift].load_sum;
                shifts_left[3 * day + shift] = 1;
            }
        }

        // **Step 4: Mark the nurse as assigned to the room**
        nurse_assigned_to_room[i][n_id] = 1;

        free(shifts_left);
    }

    // Free allocated memory
    for (i = 0; i < num_rooms; i++) {
        free(nurse_assigned_to_room[i]);
    }
    free(nurse_assigned_to_room);
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

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
    int room_digits = (num_rooms > 1) ? ((int)log10(num_rooms) + 1) : 1;
    int ot_digits = (num_ots > 1) ? ((int)log10(num_ots) + 0) : 1;

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
            fprintf(file, "      \"operating_theater\": \"t%0*d\"\n", ot_digits, patients[i].assigned_ot);
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


//int main(void) {
//
//   parse_json("data/instances/i06.json");
//   PriorityQueue* pq;
//
//   initialize_room_gender_map(&room_gender_map);
//
//   pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
//   initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it. 
//   populate_room_gender_map(&room_gender_map);
//   v_A = (RoomVector*)malloc(sizeof(RoomVector));
//   v_B = (RoomVector*)malloc(sizeof(RoomVector));
//   v_empty = (RoomVector*)malloc(sizeof(RoomVector));
//   initialize_room_shift_nurse();
// 
//   make_3_vectors(&room_gender_map);
//   sort_mandatory_patients_on_release_day(mandatory_patients, mandatory_count);
//   sort_optional_patients_on_release_day(optional_patients, optional_count);
//   append_optional_to_mandatory(sorted_mandatory_patients, sorted_optional_patients);
//   print_rooms();
//   admit_patients_2(room_gender_map, pq);
//   admit_remaining_patients(pq);
//   create_dm_nurses_availability();
//   sorting_nurse_id_max_load();
//   create_3d_array();
//   initialize_rooms_req(num_rooms);
//   create_rooms_req();
//  nurse_assignments();
//   create_json_file(patients , num_patients , nurses , num_nurses,num_rooms, "i06","D:/major_code/build/output");
////   // print_surgeons(surgeon);
////   //print_ots(ot);
////   // print_rooms();
////   //Free allocated memory
//   free_occupants();
//   free_patients();
//   free_surgeons();
//   free_ots();
//   free_rooms();
//   //free_nurses();
//   free(weights);
//   return 0;
//}