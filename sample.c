#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

// Define enums for Gender and Age Group
typedef enum {
    MALE = 'A',  // 'A' for male, adjust based on your data
    FEMALE = 'B' // 'B' for female, adjust accordingly
} Gender;

typedef enum {
    INFANT,
    CHILD,
    ADULT,
    ELDERLY
} AgeGroup;

// Define Patient structure
typedef struct {
    char *id;
    int mandatory;  // 1 for mandatory, 0 for not mandatory
    Gender gender;
    AgeGroup age_group;
    int length_of_stay;
    int surgery_release_day;
    int surgery_due_day;
    int surgery_duration;
    char *surgeon_id;
    int *incompatible_room_ids;  // Dynamically allocated array
    int incompatible_room_count;
    int *workload_produced;  // Dynamically allocated array
    int *skill_level_required;  // Dynamically allocated array
} Patient;

// Global variables
Patient *patients = NULL;
int num_patients = 0;

// Parse patients from the JSON array
void parse_patients(cJSON *patients_array) {
    num_patients = cJSON_GetArraySize(patients_array);  // Get the size of the patients array
    printf("Number of Patients: %d\n", num_patients);
    
    // Dynamically allocate memory for the patients array
    patients = (Patient *)malloc(num_patients * sizeof(Patient));
    if (!patients) {
        printf("Memory allocation failed for patients.\n");
        exit(1);
    }

    // Iterate over the patients array
    for (int i = 0; i < num_patients; i++) {
        cJSON *item = cJSON_GetArrayItem(patients_array, i);
        if (!item) continue;  // If the item is NULL, skip to the next
        
        // Parse the 'id' field
        cJSON *id_json = cJSON_GetObjectItem(item, "id");
        if (id_json && cJSON_IsString(id_json)) {
            patients[i].id = strdup(id_json->valuestring);  // Dynamically allocate and copy the string
        } else {
            patients[i].id = NULL;
            printf("Missing or invalid ID for patient at index %d.\n", i);
        }

        // Parse the 'mandatory' field
        cJSON *mandatory_json = cJSON_GetObjectItem(item, "mandatory");
        if (mandatory_json && cJSON_IsBool(mandatory_json)) {
            patients[i].mandatory = cJSON_IsTrue(mandatory_json) ? 1 : 0;
        } else {
            patients[i].mandatory = 0;  // Default to not mandatory if missing or invalid
            printf("Missing or invalid mandatory status for patient at index %d.\n", i);
        }

        // Parse the 'gender' field
        cJSON *gender_json = cJSON_GetObjectItem(item, "gender");
        if (gender_json && cJSON_IsString(gender_json)) {
            if (strcmp(gender_json->valuestring, "Male") == 0) {
                patients[i].gender = MALE;
            } else if (strcmp(gender_json->valuestring, "Female") == 0) {
                patients[i].gender = FEMALE;
            }
        } else {
            patients[i].gender = MALE;  // Default to Male if missing or invalid
            printf("Missing or invalid gender for patient at index %d.\n", i);
        }

        // Parse the 'age_group' field
        cJSON *age_group_json = cJSON_GetObjectItem(item, "age_group");
        if (age_group_json && cJSON_IsString(age_group_json)) {
            if (strcmp(age_group_json->valuestring, "Infant") == 0) {
                patients[i].age_group = INFANT;
            } else if (strcmp(age_group_json->valuestring, "Child") == 0) {
                patients[i].age_group = CHILD;
            } else if (strcmp(age_group_json->valuestring, "Adult") == 0) {
                patients[i].age_group = ADULT;
            } else if (strcmp(age_group_json->valuestring, "Elderly") == 0) {
                patients[i].age_group = ELDERLY;
            }
        } else {
            patients[i].age_group = ADULT;  // Default to Adult if missing or invalid
            printf("Missing or invalid age group for patient at index %d.\n", i);
        }

        // Parse the 'length_of_stay' field
        cJSON *length_of_stay_json = cJSON_GetObjectItem(item, "length_of_stay");
        if (length_of_stay_json && cJSON_IsNumber(length_of_stay_json)) {
            patients[i].length_of_stay = length_of_stay_json->valueint;
        } else {
            patients[i].length_of_stay = 0;  // Default to 0 if missing or invalid
            printf("Missing or invalid length_of_stay for patient at index %d.\n", i);
        }

        // Parse other fields similarly...
        // (Repeat for other fields like surgery_release_day, surgery_due_day, surgery_duration, etc.)
        
        // Example for parsing an array of incompatible_room_ids:
        cJSON *incompatible_rooms_json = cJSON_GetObjectItem(item, "incompatible_room_ids");
        if (incompatible_rooms_json && cJSON_IsArray(incompatible_rooms_json)) {
            patients[i].incompatible_room_count = cJSON_GetArraySize(incompatible_rooms_json);
            patients[i].incompatible_room_ids = (int *)malloc(patients[i].incompatible_room_count * sizeof(int));
            if (!patients[i].incompatible_room_ids) {
                printf("Memory allocation failed for incompatible_room_ids at index %d.\n", i);
                exit(1);
            }

            for (int j = 0; j < patients[i].incompatible_room_count; j++) {
                patients[i].incompatible_room_ids[j] = cJSON_GetArrayItem(incompatible_rooms_json, j)->valueint;
            }
        } else {
            patients[i].incompatible_room_ids = NULL;
            patients[i].incompatible_room_count = 0;
            printf("Missing or invalid incompatible_room_ids for patient at index %d.\n", i);
        }

        // Similar parsing can be done for workload_produced, skill_level_required, etc.
    }
}

int main() {
    // Example usage
    const char *json_file = "patients_data.json";  // Specify your JSON file path
    FILE *file = fopen(json_file, "r");
    if (!file) {
        printf("Could not open file %s\n", json_file);
        return 1;
    }

    // Read the file into a string
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json_data = malloc(file_size + 1);
    fread(json_data, 1, file_size, file);
    fclose(file);

    // Parse the JSON string
    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    cJSON *patients_array = cJSON_GetObjectItem(root, "patients");
    if (patients_array) {
        parse_patients(patients_array);  // Call the parsing function
    }

    // Clean up and free memory
    cJSON_Delete(root);
    free(json_data);

    return 0;
}
