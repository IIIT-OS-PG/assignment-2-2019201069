#ifndef _SHA_
#define _SHA_ 

#define IPS "127.0.0.1"
#define IPC "127.0.0.1"
#define PORTNOS 12348
#define PORTNOC 12358
#define BUFFSIZE 512
#define MSGSIZE 1024*32
#define COL ":"
#define SHASIZE 20
#define MAXCLIENTS 50
#define CHUNKSIZE 512  // must be 512 kb

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <iostream>
#include <pthread.h>
using namespace std;


// for peer that needs to download the file
// different for different file download
unordered_map <string, set<long long>  > userchunkmap;
unordered_map <string, set<long long>  > afterroundrobin;
// user : chunks respective for a given file


// for peer that is used to upload the file
unordered_map <string, set<long long> > chunkinfo;    // updated while uploading the file
// path and respective info


string sha256_hash_string(unsigned char hash[SHA256_DIGEST_LENGTH])
{
   stringstream ss;
   for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string sha256_file(FILE *file)//,int file_size)//,int noOfChunks)
{
    if(!file) return NULL;
    string finalHash="";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    const int bufSize = BUFFSIZE;
    unsigned char *buffer =(unsigned char*) malloc(bufSize+1);
    int bytesRead = 0;
    if(!buffer) return NULL;
    int i=0;
    while((bytesRead = fread(buffer, sizeof(char), bufSize, file))){
    SHA256_Update(&sha256, buffer, bytesRead);
    SHA256_Final(hash, &sha256);
    string outputBuffer = sha256_hash_string(hash);
    string finalAnswer = outputBuffer.substr(0, SHASIZE);
    finalHash += finalAnswer;
    memset ( buffer , '\0', BUFFSIZE);
}

    fclose(file);
    free(buffer);
    return finalHash;
}

#endif