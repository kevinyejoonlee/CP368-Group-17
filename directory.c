#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

int main() {

    char UserInput;

    do {
        printf("Select the option(s) appropriately by entering the number:\n"
        "1. Create a directory\n"
        "2. Remove directory\n"
        "3. Print current working directory\n"
        "4. Change directory one level up\n"
        "5. Read the contents of directory\n"
        "6. Close the current directory\n"
        "q. Exit the program\n");

        scanf(" %c", &UserInput);

        // make switch case for various input values
        switch (UserInput) {
            // creating directory
            case '1':
            {
                printf("Enter Directory name:\n");
                char directName[256];
                scanf("%s", directName);
                int output = mkdir(directName, 0777); // use 0777 to signify rwxrwxrwx mode

                if (output == 0) {
                    printf("Directory successfully created");
                } else {
                    printf("ERROR: unable to create directory");
                }
            } break;

            // removing a directory
            case '2':
            {
                printf("Enter name of Directory to be removed:\n");
                char directName[256];
                scanf("%s", directName);
                int output = rmdir(directName);

                if (output == 0){
                    printf("Directory is removed Successfully.\n");
                } else {
                    printf("Directory does not exist. Please try again.\n");
                }
            } break;

            // print current working directory
            case '3':
            {
                printf("Current working directory: %s\n",getcwd(NULL,256));
            } break;

            // change the directory one level up
            case '4':
            {
                printf("Current working Directory before leveling up: %s\n", getcwd(NULL, 256));
                int value = chdir("..");

                if (value == 0) {
                    printf("Directory Changed Successfully.\n");
                    printf("Working Directory After leveling up: %s\n",getcwd(NULL,256));
                }
            } break;

            // reading the contents of the directory
            case '5':
            {
                struct dirent *direct;
                DIR *dest = opendir(".");
                while ( (direct = readdir(dest)) != NULL){
                    printf("%s\n",direct->d_name);
                }
            } break;

            // exiting program
            case '6':
            {
               DIR *dest = opendir(".");
                if (closedir(dest)==0){
                    printf("Directory Closed Successfully.\n");
                }
            } break;
        }

    } while (UserInput != 'q');

    return 0;
}