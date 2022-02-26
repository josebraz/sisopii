
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>

#include "types.hpp"
#include "persistence.hpp"

using namespace std;

void write_string(const string &s, FILE *fp);

void write_vector_string(const vector<string> *vector, FILE *fp);

void read_string(string &s, FILE *fp);

void read_vector_string(vector<string>* vector, FILE *fp);

int read_users(user *users[])
{
    int total;
    FILE *fp;

    fp = fopen("db/users.bin", "rb");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
        return 0;
    }

    if (fread(&total, sizeof(int), 1, fp) != 1)
    {
        // arquivo vazio
        return 0;
    }

    cerr << "total " << total << endl;

    users = (user **) calloc(total, sizeof(user *));
    for (int i = 0; i < total; i++)
    {
        string username;
        vector<string>* follows = new vector<string>();

        read_string(username, fp);
        read_vector_string(follows, fp);

        user user = {username, follows};
        users[i] = &user;
    }

    for (int i = 0; i < total; i++)
    {
        if (users[i]->follows->size() > 0)
        {
            cerr << "Username " << users[i]->username << " first follower " << users[i]->follows->front() << endl;
        }
        else
        {
            cerr << "Username " << users[i]->username << endl;
        }
    }

    fclose(fp);

    return total;
}

void write_users(const user users[], const int total)
{
    FILE *fp;

    fp = fopen("db/users.bin", "wb");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    
    fwrite(&total, sizeof(int), 1, fp);
    
    for (int i = 0; i < total; i++)
    {
        const user user = users[i];

        write_string(user.username, fp);
        write_vector_string(user.follows, fp);
    }

    fclose(fp);
}

void write_vector_string(const vector<string> *vector, FILE *fp) {
    size_t size = vector->size();
    fwrite(&size, sizeof(size_t), 1, fp);
    for (int j = 0; j < size; j++)
    {
        write_string(vector->at(j), fp);
    }
}

void write_string(const string &s, FILE *fp) {
    size_t size = s.size();
    fwrite((void*) &size, sizeof(size_t), 1, fp);
    fwrite((void*) s.c_str(), size, 1, fp);
}

void read_vector_string(vector<string>* vector, FILE *fp) {
    size_t size;
    fread(&size, sizeof(size_t), 1, fp);
    for (int j = 0; j < size; j++)
    {
        string s;
        read_string(s, fp);
        vector->push_back(s);
    }
}

void read_string(string &s, FILE *fp) {
    size_t size;
    fread(&size, sizeof(size_t), 1, fp);

    char* array = (char *) calloc(size, sizeof(char));
    fread(array, size, 1, fp);

    s.assign(array);
}