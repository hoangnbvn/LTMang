#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dllist.h"
#include "fields.h"

typedef struct {
  char *name;
  char *pass;
  int status;
  int sign; // 1: sign in; 0: not sign in
} Account;

void Menu()
{
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Register\n");
    printf("2. Sign in\n");
    printf("3. Search\n");
    printf("4. Sign out\n"); 
    printf("Your choice (1-4, other to quit):\n");
}

void Print(Dllist l)
{
    Dllist ptr; 
    FILE *f = fopen("account.txt","w+");
    dll_traverse(ptr,l) {
        Account *tmp = (Account*)jval_v(dll_val(ptr)); 
        fprintf(f,"%s %s %d\n",tmp->name,tmp->pass,tmp->status);
    }
    fclose(f);
}

void Option1(Dllist l, Account *a)
{
    Dllist ptr;
    Account *tmp;
    char name[50];
    char pass[50];
    printf("Input username: "); scanf("%s",name);

    // check if username existed
    int check = 0;
    dll_traverse(ptr, l)
    {
        tmp = (Account*)jval_v(dll_val(ptr));
        if(strcmp(tmp->name,name) == 0){
            check = 1;
            break;
        }
    }

    if(check == 1) {
        printf("Account existed.\n");
    }
    else {
        // add to link-list
        strcpy(a->name, name);
        printf("Input password: "); scanf("%s",pass);
        strcpy(a->pass, pass);
        a->status = 1;
        dll_append(l,new_jval_v(a));
        
        // add to account.txt
        FILE *f1;
        f1 = fopen("account.txt", "a+");
        fprintf(f1,"%s %s 1\n",name,pass);
        fclose(f1);
        printf("Successful registration.\n");
    }
}

void Option2(Dllist l, Account *a){
    char name[50];
    char pass[50];
    Dllist ptr; Account *tmp;
    int check = 0;

    printf("Input username: "); scanf("%s", name);

    dll_traverse(ptr, l)
    {
        tmp = (Account*)jval_v(dll_val(ptr));

        if(strcmp(tmp->name,name) == 0){
            printf("Input password: "); scanf("%s",pass);
            if(strcmp(tmp->pass,pass)==0) {
                printf("Hello %s",name);
                tmp->sign = 1;
            }
            else {
                // Input password 3 times - check
                for(int i=0; i<3; i++)
                {
                    printf("Password is incorrect. Please try again: ");
                    scanf("%s",pass);
                    if(strcmp(tmp->pass,pass)==0) {
                        printf("Hello %s",name);
                        tmp->sign = 1;
                        break;
                    }
                }
                // Change status
                if(strcmp(tmp->pass,pass)!=0) {
                    tmp->status = 0;
                    Print(l);
                    printf("Password is incorrect. Account is blocked.");
                }
            }
            check = 1;
            break;
        }
    }
    if(check==0) {
        printf("Cannot find account.");
    }
}

void Option3(Dllist l, Account *a){
    char name[50];
    Dllist ptr; Account *tmp;
    int check = 0;

    printf("Input username: ");
    scanf("%s",name);
    
    dll_traverse(ptr,l)
    {
        tmp = (Account*)jval_v(dll_val(ptr));
        if(strcmp(tmp->name,name)==0) {
            if(tmp->status==0) printf("Account is blocked.");
            else printf("Account is active");
            check = 1;
            break;
        }
    }
    if(check==0) printf("Cannot find account.");
}

void Option4(Dllist l , Account *a){
    Dllist ptr; Account *tmp;
    char name[50];
    int check = 0;

    printf("Input username: "); scanf("%s", name);
    dll_traverse(ptr,l){
        tmp = (Account*)jval_v(dll_val(ptr));
        if(strcmp(tmp->name,name)==0) {
            check = 1;
            if(tmp->sign==1) {
                printf("Goodbye %s",tmp->name);
            }
            else{
                printf("Account is not sign in.");
            }
            break;
        }
    }
    if(check==0) printf("Cannot find account.");
}

int main()
{
    IS is; Dllist l,ptr;
    int choice;
    Account *a;
    
    is = new_inputstruct("account.txt");
    l = new_dllist();

 // Read from account.txt and add to link-list   
    while (get_line(is) >= 0) {
        if (is->NF > 1) {
            a = malloc(sizeof(Account));
            a->name = (char*)malloc(sizeof(char)*20);
            strcpy(a->name, is->fields[0]);
            a->pass = (char*)malloc(sizeof(char)*20);
            strcpy(a->pass, is->fields[1]);
            a->status = atoi(is->fields[2]);
            a->sign = 0; 
            dll_append(l,new_jval_v(a));
        }
    }
  
    //Print menu
    Menu();
    do
    {
        scanf("%d",&choice);
        switch (choice)
        {
        case 1:
            Option1(l,a);
            break;
        case 2:
            Option2(l,a);
            break;
        case 3:
            Option3(l,a);
            break;
        case 4:
            Option4(l,a);
            break;
        default: exit(1);
            break;
        }
        Menu();
    } while (choice>=1 && choice<=4);

    free(a); free_dllist(l);  
    jettison_inputstruct(is);
    return 0;
}