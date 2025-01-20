#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

// Prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

// Prototypes for functions to handle required functionality
int  count_words(char *,int, int);
//add additional prototypes here
void reverse_string(char *, int);
void print_words(char *, int);
// Prototype for the replace function
int replace_first_occurrence(char *buff, const char *target, const char *replacement, int buffer_sz, int original_length);

void handle_replace(int argc, char *argv[], char *buff, int buffer_sz, int original_length);

int setup_buff(char *buff, char *user_str, int len) {
     //TODO: #4:  Implement the setup buff as per the directions
    char *src = user_str;
    char *dst = buff;
    int str_len = 0;           
    int valid_str_len = 0;     

    // Skip leading spaces and tabs
    while (*src == ' ' || *src == '\t') {
        src++;
    }

    while (*src != '\0') {
        // Return -1 if the buffer length limit is exceeded
        if (str_len >= len) {
            return -1; 
        }

        if (*src == ' ' || *src == '\t') {
            if (dst > buff && *(dst - 1) != ' ') {
                *dst++ = ' ';
                str_len++;
                valid_str_len++;
            }
        } else {
            // Copy non-space character to the buffer
            *dst++ = *src; 
            str_len++;
            valid_str_len++;
        }
        src++;
    }

    // Remove trailing space if present
    if (valid_str_len > 0 && *(dst - 1) == ' ') {
        dst--;
        str_len--;
        valid_str_len--;
    }

    // Pad the buffer with dots until it reaches the desired length
    while (str_len < len) {
        *dst++ = '.';
        str_len++;
    }

    return valid_str_len; 
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len) {
    int word_count = 0;
    int is_word = 0; 
    int max_len = (str_len < len) ? str_len : len;

    for (int i = 0; i < max_len; i++) {
        if (buff[i] != ' ') { // Non-space character
            if (!is_word) {
                word_count++;
                is_word = 1; // Start of a word
            }
        } else {
            is_word = 0; // End of a word
        }
    }
    return word_count;
}


//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int replace_first_occurrence(char *buff, const char *target, const char *replacement, int buffer_sz, int original_length) {
    int target_len = 0, replacement_len = 0, i, j; // Calculate target string length
    while (target[target_len] != '\0') target_len++; // Calculate replacement string length

    while (replacement[replacement_len] != '\0') replacement_len++;

    int match_index = -1;
    for (i = 0; i <= original_length - target_len; i++) {
        int found = 1;
        for (j = 0; j < target_len; j++) {
            if (buff[i + j] != target[j]) {
                found = 0;
                break;
            }
        }
        if (found) {
            match_index = i;
            break;
        }
    }

    if (match_index == -1) {
        printf("Not Implemented!\n"); // Indicates that no occurrence was found
        exit(3);
    }

    int new_length = original_length - target_len + replacement_len;
    if (new_length > buffer_sz) {
        new_length = buffer_sz; // Ensure new length does not exceed buffer size
    } 

    if (replacement_len > target_len) {
        int shift = replacement_len - target_len;
        for (i = original_length - 1; i >= match_index + target_len; i--) {
            if ((i + shift) < buffer_sz) {
                buff[i + shift] = buff[i]; // Shift characters to make space for longer replacement
            }
        }
    }
    else if (replacement_len < target_len) {
        int shift = target_len - replacement_len;
        for (i = match_index + replacement_len; i < original_length; i++) {
            if ((i + shift) < buffer_sz) {
                buff[i] = buff[i + shift];
            }
        }
        new_length = original_length - shift;
    }

    for (i = 0; i < replacement_len && (match_index + i) < buffer_sz; i++) {
        buff[match_index + i] = replacement[i];
    }

    new_length = original_length - target_len + replacement_len;
    if (new_length > buffer_sz) new_length = buffer_sz;

    for (i = new_length; i < buffer_sz; i++) {
        buff[i] = '.';
    }

    return (new_length <= buffer_sz) ? new_length : buffer_sz;
}

void handle_replace(int argc, char *argv[], char *buff, int buffer_sz, int original_length) {
    if (argc != 5) {
        printf("Error: -x flag requires exactly 3 arguments.\n");
        exit(1);
    }

    char *target_str = argv[3];
    char *replacement_str = argv[4];

    replace_first_occurrence(buff, target_str, replacement_str, buffer_sz, original_length);

    printf("Buffer:  [");
    for(int i = 0; i < buffer_sz; i++) {
        printf("%c", buff[i]);
    }
    printf("]\n");
    exit(0);
}



void reverse_string(char *buff, int str_len) {
    if (str_len == 0) return;

    char *first = buff;
    char *last = buff + str_len - 1; // Initialize pointer to the last character
    char temp;

    while (first < last) {
        temp = *first;
        *first = *last;
        *last = temp;

        first++;
        last--;
    }
}

void print_words(char *buff, int str_len) {
    char *start = buff;
    char *end = buff;
    int word_index = 1; 

    printf("Word Print\n----------\n");
    while (end < buff + str_len) {
        if (*end == ' ' || (end == buff + str_len - 1)) {  // Check for space or end of string
            if (end == buff + str_len - 1 && *end != ' ') {
                end++;  // Include the last character if it's not a space
            }
            if (end > start) {
                printf("%d. ", word_index);
                for (char *ptr = start; ptr < end; ptr++) {
                    putchar(*ptr);
                }
                printf("(%ld)\n", end - start); // Print word length
                word_index++;
            }
            start = end + 1; // Move to the start of the next word
        }
        end++;
    }
    printf("\nNumber of words returned: %d\n", word_index - 1); // Print total word count
}

int main(int argc, char *argv[]){
    char *buff;             // Placeholder for the internal buffer
    char *input_string;     // Holds the string provided by the user on cmd line
    char opt;               // Used to capture user option from cmd line
    int  rc;                // Used for return codes
    int  user_str_len;      // Length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?

    // This condition is safe due to short-circuit evaluation in C.
    // If argc < 2 is true, the second condition (*argv[1] != '-') is not evaluated,
    // which prevents accessing argv[1] when it does not exist.

    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = *(argv[1]+1);   // get the option flag

    // handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    // The purpose of the if statements below checks if the number 
    // of command-line arguments is less than 3
    // If so, display usage information and exit the program

    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; // capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ);
    if(buff == NULL) {
        printf("Error: Failed to allocate memory\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            print_buff(buff, BUFFER_SZ);
            break; 

            //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
            //       the case statement options

        case 'r': {
            reverse_string(buff, user_str_len);
            for (int i = user_str_len; i < BUFFER_SZ; i++) {
                buff[i] = '.';
            }

            print_buff(buff, BUFFER_SZ);
            break;
        }

        case 'w': 
            print_words(buff, user_str_len);
            print_buff(buff, BUFFER_SZ);
            break;
            
        case 'x':
            handle_replace(argc, argv, buff, BUFFER_SZ, user_str_len);
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?

//          ANSWER: Providing both the buffer pointer and its length  
//          ensures that functions can handle buffers of varying sizes safely 
//          and efficiently. It prevents buffer overruns by knowing 
//          the exact size, enhancing code reusability and clarity.