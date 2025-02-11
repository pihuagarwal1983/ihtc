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
extern int sorting_day;

// int is_admitted[1000] = {0}; // we're not using this array anywhere so let's comment it out
int num_gender_A_rooms, num_gender_B_rooms, num_empty_rooms;
int* current_size_dm_nurse;
int* room_gender_map;

int** size_of_room_schedule;

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
    int is_admitted;
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

typedef struct {
    Rooms** data;
    int size;
    int capacity;
}RoomVector;

RoomVector* v_A, * v_B, * v_empty;

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
extern Node** sorted_mandatory_patients;
extern Node** sorted_optional_patients;
typedef struct {
    HeapNode** data;
    int size;
    int capacity;
} Mand_opt_PQ;

extern Nurses* nurses;
extern Weights* weights;
extern Occupants* occupants;
extern Patient* patients;
extern Patient** mandatory_patients;
extern Patient** optional_patients;
extern Patient**** room_schedule;
extern Surgeon* surgeon;
extern OTs* ot;
extern Rooms* room;
// one idea is that make the patients_assigned array a static array and remove num_patients_assigned field from the structure.


//..............FUNCTION FOR PARSING THE DATA..................//
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

//................FUNCTIONS FOR PRIORITY QUEUE......................//
extern void initPQ(PriorityQueue* pq, int initialCapacity);
extern void insertNodeInPQ(PriorityQueue* pq, HeapNode node);
extern HeapNode extractMaxFromPQ(PriorityQueue* pq);
extern HeapNode peekMaxFromPQ(PriorityQueue* pq);
extern int isEmptyPQ(PriorityQueue* pq);
extern void heapifyUp(PriorityQueue* pq, int index);
extern void heapifyDown(PriorityQueue* pq, int index);
extern int compareNodesForPQ(HeapNode a, HeapNode b);
extern void resizePQ(PriorityQueue* pq);
extern void freePQ(PriorityQueue* pq);
extern HeapNode makeHeapNode(int p_id, int mandatory, int due_day, int delay);

//...................FUNCTIONS FOR PATIENT ADMISSION..............//
extern void free_s_data_arr(Surgeon_data**, int);
extern void update_LOS_of_patients(int);
extern int findSuitableRoom(int, RoomVector*);
extern void sort_s_data_arr(Surgeon_data**, int, Node*);
extern void sort_ot_data_arr(OTs**, int);
extern int find_max_surgeon_id(Node*);
extern void init_Rooms(RoomVector* vector, int initialcap);
extern void pushback(RoomVector* vector, Rooms* room);
extern Rooms* removebyid(RoomVector* vector, int room_id);
extern void moveRoom(RoomVector* source, RoomVector* destination, int room_id);
extern void freevector(RoomVector* vector);


//.................FUNCTION FOR CHAINING OF SORTED MANDATORY AND SORTED OPTIONAL PATIENTS........//
extern Node* makeNode(Patient* struct_pointer);
extern void appendNode(Node** sorted_array, Patient* struct_pointer, int index);
extern void sort_mandatory_patients_on_release_day(Patient** patient_array, int n);
extern void sort_optional_patients_on_release_day(Patient** patient_array, int n);
extern void parse_linked_list(Node* head);
extern void print_sorted_mandatory_array(void);
extern void free_linked_list(Node* head);
extern void free_patients_sorted_array(Node** sorted_array);
extern void copy_paste_pointers_from_sorted_mandatory_array_to_mandatory_patients(void);
extern void copy_paste_pointers_from_sorted_optional_array_to_optional_patients(void);
extern Node* find_smallest_iter(Node* head);
extern Node* insertion_sort_for_linked_list(Node* head);
extern void sort_mandatory_patients_on_LOS(void);
extern void append_optional_to_mandatory(Node** sorted_mandatory_patients, Node** sorted_optional_patients);


//..............FUNCTIONS FOR ROOM GENDER MAP..........................//
extern void initialize_room_gender_map(int* room_gender_map);
extern void populate_room_gender_map(int* room_gender_map);


//...............FUNCTIONS FOR SORTING THE SURGEON AND OT DATA ARRAY..........//
extern int compare_surgeon(const void* a, const void* b);
extern void sort_s_data_arr(Surgeon_data** s_data_arr, int n, Node* head);
extern int compare_ots(const void* a, const void* b);
extern void sort_ot_data_arr(OTs** ot_data_arr, int d);

extern void init_Mand_opt_PQ(Mand_opt_PQ* vector, int initial_cap);
extern void resizevector(Mand_opt_PQ* vector);
extern void pushback_Mand_Opt_PQ(Mand_opt_PQ* vector, HeapNode* node);
extern HeapNode* pop_Mand_opt_PQ(Mand_opt_PQ* vector);
extern OTs* admitFromPQ(PriorityQueue* pq, int d, OTs** ot_data_arr, OTs* current_ot, int current_ot_index, int flag_opt);

//.............ADMISSION OF MANDATORY PATIENTS..................//
void admit_mandatory_patients(int* room_gender_map, PriorityQueue* pq) {
    int i, j, k, l, d, g, r_id, p_id, s_id, flag, flag_opt = 0, count_admit_PQ = 0, s_duration, current_ot_index, s_duration_sum, max_surgeon_id, len_surgeon_array, temp_patient_id;
    Surgeon_data** s_data_arr;
    HeapNode node;
    Node* head;
    OTs** ot_data_arr, * current_ot, * new_ot;

    v_A = (RoomVector*)malloc(sizeof(RoomVector));
    v_B = (RoomVector*)malloc(sizeof(RoomVector));
    v_empty = (RoomVector*)malloc(sizeof(RoomVector));

    //make_3_room_arrays(room_gender_map);
    make_3_vectors(&room_gender_map);
    printVector("A", v_A);
    printVector("B", v_B);
    printVector("Empty", v_empty);
    // first, make another OT array (array of pointers to structures)
   // copy all the OT struct pointers to the ot_data_array.
    ot_data_arr = (OTs**)calloc(num_ots, sizeof(OTs*));
    if (ot_data_arr == NULL) {
        printf("Memory not allocated.\n");
        exit(-1);
    }
    //print_ots(ot);

    for (i = 0; i < num_ots; ++i)
        ot_data_arr[i] = &ot[i];

    // ---------------------------------------------------------START ADMISSION------------------------------------------------------------
    for (d = 0; d < days; ++d) {
        count_admit_PQ = 0;
        // update LOS for all patients
        if (d) update_LOS_of_patients(d);

        sort_ot_data_arr(ot_data_arr, d);
        current_ot_index = 0;
        current_ot = ot_data_arr[0];
        flag = 0;

        Node* head = sorted_mandatory_patients[d];
        //printf("sorted_mandatory_patients[%d]: %p\n", d, (void*)sorted_mandatory_patients[d]);
        //printf("head:%p\n", head);
        if (head) {
            max_surgeon_id = find_max_surgeon_id(head);
            len_surgeon_array = max_surgeon_id + 1;
            // Added 1 to max_surgeon_id cuz we need space for the nth (max_is) also, so we'll have to make the total length (n+1)
            s_data_arr = (Surgeon_data**)calloc(len_surgeon_array, sizeof(Surgeon_data*));
            sort_s_data_arr(s_data_arr, len_surgeon_array, head);
        }
        else {
            continue;
        }

        //incrementing the current ot while we get to an ot which is available that day
        while (current_ot && !current_ot->time_left[d]) {
            if (current_ot_index >= num_ots) {
                flag = 1;
                break;
            }
            current_ot = ot_data_arr[current_ot_index++];
        }


        // extreme situation - when we come to a new day and find that no OT is available
        if (flag) {
            for (i = 0; i < len_surgeon_array; ++i) {
                if (!s_data_arr[i]->isNull) {
                    for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
                        temp_patient_id = s_data_arr[i]->assigned_patients[k];
                        insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0));
                        //printPriorityQueue(pq); printf("\n2241");
                    }
                }
            }
            continue; // go to the next day and then admit these patients
        }

        flag = 0;
        // admit the mandatory patients previously not admitted from the PQ
        if (d && pq->current_size > 0 && count_admit_PQ < 1) {
            current_ot = admitFromPQ(pq, d, ot_data_arr, current_ot, current_ot_index, flag_opt);
            count_admit_PQ++;
            flag_opt = 1;
        }
        if (!current_ot) {
            for (i = 0; i < len_surgeon_array; ++i)
                if (!s_data_arr[i]->isNull)
                    for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
                        temp_patient_id = s_data_arr[i]->assigned_patients[k];
                        insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0));
                        //printPriorityQueue(pq); printf("\n2261");
                    }
            continue; // go to the next day and then admit these patients
        }

        for (i = 0; i < len_surgeon_array; ++i)
            if (!s_data_arr[i]->isNull) {
                if (!surgeon[s_data_arr[i]->surgeon_id].time_left[d]) {
                    // the surgeon is not available and hence add all the patients to the PQ
                    for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
                        temp_patient_id = s_data_arr[i]->assigned_patients[k];
                        if (patients[temp_patient_id].mandatory && patients[temp_patient_id].surgery_due_day == d) {
                            //admitting the patient at any cost
                            s_duration = patients[temp_patient_id].surgery_duration;
                            if (s_duration <= current_ot->time_left[d] && surgeon[s_data_arr[i]->surgeon_id].time_left[d] >= s_duration) {
                                // no need to check now actually but check only to be safe
                                // assign OT--------------------------------------------------------------------------------------------------
                                patients[temp_patient_id].assigned_ot = current_ot->id;
                                // update the time left
                                current_ot->time_left[d] -= s_duration;
                                surgeon[s_data_arr[i]->surgeon_id].time_left[d] -= s_duration;
                            }
                            r_id = patients[temp_patient_id].gen ? findSuitableRoom(temp_patient_id, v_B) : findSuitableRoom(temp_patient_id, v_A);
                            patients[temp_patient_id].assigned_room_no = r_id;
                            room[r_id].num_patients_allocated += 1;
                            patients[temp_patient_id].admission_day = d;
                            patients[i].is_admitted = 1;
                            printVector("A", v_A);
                            printVector("B", v_B);
                            printVector("Empty", v_empty);
                        }
                        else
                            insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0));
                        //printPriorityQueue(pq); printf("\n2291");
                    }
                    continue;
                }
                // if the control comes here, that means the surgeon has time on that day to operate patients
                for (j = 0; j < s_data_arr[i]->num_assigned_patients; ++j) {
                    p_id = s_data_arr[i]->assigned_patients[j];
                    // s_id = s_data_arr[i]->surgeon_id;
                    s_duration = patients[p_id].surgery_duration;
                    // s_duration_sum = s_data_arr[i]->surgery_duration_sum;
                    if (patients[p_id].mandatory) {
                        // assign available OT
                        // first - select a suitable OT
                        while (current_ot->time_left[d] < s_duration) {
                            if (current_ot_index == num_ots) { flag = 1; break; }
                            current_ot = ot_data_arr[current_ot_index++];
                        }
                        if (flag) {
                            for (k = 0; k < s_data_arr[i]->num_assigned_patients; ++k) {
                                temp_patient_id = s_data_arr[i]->assigned_patients[k];
                                insertNodeInPQ(pq, makeHeapNode(temp_patient_id, patients[temp_patient_id].mandatory, patients[temp_patient_id].surgery_due_day, 0));
                                //printPriorityQueue(pq); printf("\n2312");
                            }
                            break;
                        }
                        if (s_duration <= current_ot->time_left[d]) {
                            // no need to check now actually but check only to be safe
                            // assign OT--------------------------------------------------------------------------------------------------
                            //patients[p_id].assigned_ot = current_ot->id;
                            //// update the time left
                            //current_ot->time_left[d] -= s_duration;
                         //}
                         // assign a room
                            r_id = patients[p_id].gen ? findSuitableRoom(p_id, v_B) : findSuitableRoom(p_id, v_A);
                            if (r_id == -1) {
                                // means there's no room in the gender_room_array and empty_array that is compatible with this patient
                                // so in this case - put the patient in the PQ and move on
                                insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0));
                                //printPriorityQueue(pq); printf("\n2329");
                                continue;
                            }
                            else {
                                if (surgeon[patients[p_id].surgeon_id].time_left[d] >= s_duration) {
                                    patients[p_id].assigned_room_no = r_id;
                                    room[r_id].num_patients_allocated += 1;
                                    // assign admission day
                                    patients[p_id].admission_day = d;
                                    patients[p_id].assigned_ot = current_ot->id;
                                    current_ot->time_left[d] -= s_duration;
                                    surgeon[patients[p_id].surgeon_id].time_left[d] -= s_duration;
                                    patients[i].is_admitted = 1;
                                    printVector("A", v_A);
                                    printVector("B", v_B);
                                    printVector("Empty", v_empty);
                                }
                                else {
                                    insertNodeInPQ(pq, makeHeapNode(p_id, patients[p_id].mandatory, patients[p_id].surgery_due_day, 0));
                                }
                            }
                        }
                    }
                }
                free_s_data_arr(s_data_arr, len_surgeon_array);
            }
    }
}

//.............FUNCTIONS FOR NURSE ADMISSION...............//
extern void initialize_3d_array();
extern void create_3d_array(void);
extern void swap_nurse_pointers(Nurses** nurse1, Nurses** nurse2);
extern int partition(Nurses** arr, int low, int high);
extern void quicksort(Nurses** arr, int low, int high);
extern void sorting_nurse_id_max_load();
extern void allocate_dm_nurses_availability();
extern void allocate_sub_array(Nurses*** main_array, int index, int sub_size);
extern void append_to_sub_array(Nurses*** main_array, int index, int* current_sizes, Nurses* nurse);
extern void initialize_current_size_dm_nurse();
extern void create_dm_nurses_availability();
extern void free_all_dm_nurses(Nurses*** main_array, int main_size, int* current_sizes);
extern void initialize_rooms_req(int num_rooms);
extern void create_rooms_req();
extern void cleanup_rooms_req(int num_rooms);
extern void nurse_assignments();


extern void create_json_file(Patient* patients, int num_patients, Nurses* nurse, int num_nurses, int num_rooms, const char* instance_name, const char* output_folder);


//int main(void){
//    prinf("running improved heusistic.\n");
//    parse_json("data/instances/i02.json");
//    PriorityQueue* pq;
//
//    initialize_room_gender_map(&room_gender_map);
//
//    pq = (PriorityQueue*)calloc(1, sizeof(PriorityQueue));
//    initPQ(pq, 20); // InitalCapacity = 20. It'll resizePQ itself if we try to insert more HeapNodes in it.
//    populate_room_gender_map(&room_gender_map);
//    sort_mandatory_patients_on_release_day(mandatory_patients, mandatory_count);
//    sort_mandatory_patients_on_LOS();
////print_sorted_mandatory_array();
//    sort_optional_patients_on_release_day(optional_patients, optional_count);
//    append_optional_to_mandatory(sorted_mandatory_patients, sorted_optional_patients);
//    admit_mandatory_patients(room_gender_map, pq);
//    create_dm_nurses_availability();
//    sorting_nurse_id_max_load();
//    create_3d_array();
////print_room_schedule();
//    initialize_rooms_req(num_rooms);
//    create_rooms_req();
////print_rooms_req();
//    nurse_assignments();
//
//// print_dm_nurses();
//    create_json_file(patients, num_patients, nurses, num_nurses, num_rooms, "i02", "D:/major_code/build/output");
//
//}
