#include <stdio.h>
#include <stdlib.h>

// Function to sort an array using bucket sort
void bucket_sort(float arr[], int n) {
    if (n <= 0) return;

    // Create n empty buckets
    float **buckets = (float **)malloc(n * sizeof(float *));
    int *bucket_sizes = (int *)calloc(n, sizeof(int)); // To track size of each bucket
    int *bucket_capacities = (int *)malloc(n * sizeof(int)); // To track capacities of each bucket

    for (int i = 0; i < n; i++) {
        bucket_capacities[i] = 2; // Initial capacity for each bucket
        buckets[i] = (float *)malloc(bucket_capacities[i] * sizeof(float));
    }

    // Put elements into respective buckets
    for (int i = 0; i < n; i++) {
        int bucket_index = n * arr[i]; // Index based on value
        if (bucket_index >= n) bucket_index = n - 1; // Handle edge cases

        // Resize the bucket if needed
        if (bucket_sizes[bucket_index] == bucket_capacities[bucket_index]) {
            bucket_capacities[bucket_index] *= 2;
            buckets[bucket_index] = (float *)realloc(buckets[bucket_index], bucket_capacities[bucket_index] * sizeof(float));
        }

        buckets[bucket_index][bucket_sizes[bucket_index]++] = arr[i];
    }

    // Sort individual buckets (using insertion sort here)
    for (int i = 0; i < n; i++) {
        for (int j = 1; j < bucket_sizes[i]; j++) {
            float key = buckets[i][j];
            int k = j - 1;

            // Move elements that are greater than key
            while (k >= 0 && buckets[i][k] > key) {
                buckets[i][k + 1] = buckets[i][k];
                k--;
            }
            buckets[i][k + 1] = key;
        }
    }

    // Concatenate all buckets into the original array
    int index = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < bucket_sizes[i]; j++) {
            arr[index++] = buckets[i][j];
        }
    }

    // Free memory
    for (int i = 0; i < n; i++) {
        free(buckets[i]);
    }
    free(buckets);
    free(bucket_sizes);
    free(bucket_capacities);
}

// Utility function to print an array
void print_array(float arr[], int n) {
    for (int i = 0; i < n; i++) {
        printf("%.2f ", arr[i]);
    }
    printf("\n");
}

// Driver code
int main() {
    float arr[] = {0.78, 0.17, 0.39, 0.26, 0.72, 0.94, 0.21, 0.12, 0.23, 0.68};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("Original array:\n");
    print_array(arr, n);

    bucket_sort(arr, n);

    printf("\nSorted array:\n");
    print_array(arr, n);

    return 0;
}
