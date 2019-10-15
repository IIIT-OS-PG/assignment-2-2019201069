
//#include "sha256.h"
#include "cliutil.h"
int sockfd;

int clients;
string filepathtodownload;

typedef struct dwnloadinfo{
    string ipfordownload;
    string fromclient;
    long long filesize;
    int portfordownload;
    string pathfordownload; 
}dwnloadinfo;


// download the file in threads
void* finaldownload(void *threadarg){

    dwnloadinfo *downloadinfo  = (dwnloadinfo *)threadarg;
    char buffer[BUFFSIZE], okaysignal[BUFFSIZE] ;
    struct sockaddr_in cli_addr;
    int n;
    cout<<"here in thread finaldownload"<<endl;

    // form threads to download
    string tempmsg = "sendfile:" + downloadinfo->pathfordownload + COL + to_string(downloadinfo->filesize) +":";

    cout<<"finaldownload temp msg is "<<tempmsg<<endl;

    int cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr(downloadinfo->ipfordownload.c_str());  // taking just one client for now
    cli_addr.sin_port = htons(downloadinfo->portfordownload);           // taking just one client for now
    
    int status = connect(cli_sockfd,(struct sockaddr *) &cli_addr,sizeof(cli_addr));

    if (status < 0)
        perror("Error in connecting");
    else
        printf("Connected...\n");

    vector<int> acks;
    string chunksrequired;
    for(auto itr = afterroundrobin[downloadinfo->fromclient].begin(); itr != afterroundrobin[downloadinfo->fromclient].end(); ++itr){
        acks.push_back(*itr);
        chunksrequired = chunksrequired + to_string(*itr) + COL;
    }
    // actually required chunks
    chunksrequired = chunksrequired + ";";

    cout << "final chunks required in final : "<<chunksrequired<<endl;

    send(cli_sockfd, tempmsg.c_str(), BUFFSIZE, 0);

    // random signal saying ok;
    recv(cli_sockfd , okaysignal , BUFFSIZE, 0);

    // = chunkspresentdata;
    send(cli_sockfd, chunksrequired.c_str(), BUFFSIZE, 0);


    cout<<"file destination"<<filepathtodownload<<endl;
    FILE *fp = fopen (filepathtodownload.c_str()  , "r+" );
    int fd = fileno(fp); 
    int file_size = downloadinfo->filesize;

    int num_chunks = file_size / CHUNKSIZE + 1;
    cout<<"number of chunks :"<<num_chunks<<endl;
    
    int bytes_recv ;
    int chunks = acks.size();
    for(int i = 0 ; i < chunks ; ++i)
    {
        int ack = acks[i];
        int size_to_recv = (ack == num_chunks ? file_size - (CHUNKSIZE * (num_chunks-1)) : CHUNKSIZE) ;

        lseek(fd, CHUNKSIZE*(ack-1), SEEK_SET);
        //fseek(fp, CHUNKSIZE*(ack-1), SEEK_SET) ;

        char buf[512];
        //int chunk_sent = 0;
        while(size_to_recv > 0 && (bytes_recv = recv(cli_sockfd, buffer, sizeof buffer, 0)) > 0)
        {
            //printf("%d\n", ++chunk_sent);
            write(fd, buffer, bytes_recv);
            //cout<<buffer<<" **********************************"<<ack<<endl;
            //cout<<"****************************************************"<<endl;
            memset(buffer , '\0', bytes_recv);

            //fwrite(buf, sizeof(char), bytes_recv, fp);
            size_to_recv -= bytes_recv ;

            //printf("%d\n", size_to_recv) ;
        }
        printf("Received chunk %d\n", ack) ;
        //file_size -= CHUNK_SIZE ;
    }

    cout<<"file download successful";

    fclose(fp);
    close(fd);

    close(cli_sockfd);
    pthread_exit(NULL);
}


void executeroundrobin(dwnloadinfo *downloadinfo, long long clients_num,long long chunks_num){

    deque <string> clientq;

    for(long long i = 0; i < clients_num; i++){
        clientq.push_back(downloadinfo[i].fromclient);          // push all users to the queue
        afterroundrobin[downloadinfo[i].fromclient] = {};
    }

    for(long long i = 1; i <= chunks_num+1; i++){
        int flag = 0;

        while(flag != 1){

            string clienttobechoosen = clientq.front();
            clientq.pop_front();
            clientq.push_back(clienttobechoosen);
            if(userchunkmap[clienttobechoosen].count(i)) {  // if that chunk is present
                flag = 1;
                afterroundrobin[clienttobechoosen].insert(i);
            }  
            if(flag == 1)
                break;
        }
    }  

}



void * senddownloadsignal(void * threadarg){

    dwnloadinfo *downloadinfo  = (dwnloadinfo *)threadarg;
    char buffer[BUFFSIZE], chunkspresentdata[BUFFSIZE] ;
    struct sockaddr_in cli_addr;
    int n;
    cout<<"here in thread"<<endl;
    // form threads to download
    string tempmsg = "sendinfo:" + downloadinfo->pathfordownload + COL + to_string(downloadinfo->filesize) +":";

    cout<<"temp msg is "<<tempmsg<<endl;

    int cli_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr(downloadinfo->ipfordownload.c_str());  // taking just one client for now
    cli_addr.sin_port = htons(downloadinfo->portfordownload);           // taking just one client for now
    
    int status = connect(cli_sockfd,(struct sockaddr *) &cli_addr,sizeof(cli_addr));

    if (status < 0)
        perror("Error in connecting");
    else
        printf("Connected...\n");

    send(cli_sockfd, tempmsg.c_str(), BUFFSIZE, 0);

    // recieve the chunks present at the end
    recv(cli_sockfd , chunkspresentdata , BUFFSIZE, 0);

    string chunksrequired = chunkspresentdata;

    cout<<"received :"<<chunksrequired<<endl;

    vector<int> acks;
    string toks = strtok((char *)chunksrequired.c_str(), COL);
    while(toks != ";"){
        acks.push_back(atoi(toks.c_str()));
        toks = strtok(NULL, COL);
        //cout<<toks<<" ";
    }

    int ack_len = acks.size();
    for(int i = 0; i < ack_len; i++){
        userchunkmap[downloadinfo->fromclient].insert(acks[i]);
        //cout<<acks[i]<<", ";    // working fine
    }

    cout<<endl;
    // RR or selection technique after this
    pthread_exit(NULL);
}

void downloadactualfile(string downldmsg){  

    userchunkmap.clear();
    afterroundrobin.clear();

    // create thread for each client 
    dwnloadinfo *downloadinfo;
    char buffer[BUFFSIZE];
    // remove down and total filesize
    strtok((char *)downldmsg.c_str(), COL);  // remove down
    long long filesize = atol(strtok(NULL, COL));
    //cout<<"file size is :"<<filesize<<endl;
    long long i = 0, j = 0;
    string params[BUFFSIZE];
    params[0] = strtok(NULL, COL);
    int params_num = 0;
    while(params[params_num] != ";"){  // delim used at the end
        params_num++;
        params[params_num] = strtok(NULL,COL);
        cout<<params[params_num]<<endl;
        if(params[params_num] == ";")
            break;
    }

    filepathtodownload = strtok(NULL,";");

    FILE *fp = fopen (filepathtodownload.c_str()  , "wb" );
    int fd = fileno(fp); 

    long long filler = 1;
    memset (buffer , ' ', 1); 

    while (filler <= filesize){

                write(fd, buffer, 1);
                filler = filler + 1;

    }

    // file created with empty chars (space) and size = orig filesize
    fclose(fp);
    close(fd);


    downloadinfo = new dwnloadinfo[MSGSIZE];

    clients = params_num/4;
    pthread_t client_thread1[clients];

    for(i = 0, j = 0; i < clients && j < params_num; i++){
        
        downloadinfo[i].fromclient = params[j++];
        downloadinfo[i].ipfordownload = params[j++];
        downloadinfo[i].portfordownload = atoi(params[j++].c_str());
        downloadinfo[i].pathfordownload = params[j++];
        downloadinfo[i].filesize = filesize;
        pthread_create(&client_thread1[i], NULL, senddownloadsignal, (void *)&downloadinfo[i]);
    }

    clients = i;
    cout<<"no of clients with this file"<<endl;
    cout<<clients<<endl;
    for(int k = 0; k < clients; k++){
        pthread_join(client_thread1[k],NULL);
        //pthread_exit(NULL);
    }

    executeroundrobin(downloadinfo, clients, (filesize/CHUNKSIZE)); // downloadinfo, no of clients, no of chunks

    for(i = 0; i < clients; i++){         
        string chunksrequired;
        for(auto itr = afterroundrobin[downloadinfo[i].fromclient].begin(); itr != afterroundrobin[downloadinfo[i].fromclient].end(); ++itr){
            chunksrequired = chunksrequired + to_string(*itr) + COL;
        }
        
    }

    pthread_t client_thread2[clients];
    for(i = 0; i < clients; i++){
        pthread_create(&client_thread2[i], NULL, finaldownload, (void *)&downloadinfo[i]);
    }

    for(int k = 0; k < clients; k++){
        pthread_join(client_thread2[k],NULL);
    }

}

string getchunkinfo(string path){
   
    string msg;     
    if(chunkinfo[path].empty()){
        return "no:";
    }
    for (auto const &x : chunkinfo[path]){
        msg = msg + to_string(x) + COL;
    }

    msg = msg +";";
    return msg;
}

void * filegiver(void * threadarg){

    int newsockfd;
    struct sockaddr_in cli_addr,ser_addr;
    socklen_t clientlen = sizeof(cli_addr);

    int portno;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
    portno = thisclientPort;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(thisclientIP.c_str());
    ser_addr.sin_port = htons(portno);
     
    bind(sockfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));

    listen(sockfd,5);

    while(1){


        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientlen);
        char info[BUFFSIZE], tobesentchunkinfo[BUFFSIZE];
        recv(newsockfd, info, BUFFSIZE, 0);   // download dialoge with path and
        cout<<info<<endl;

        int n;
        string str = strtok(info,":");
        //cout<<str<<"is first token"<<endl;
        if(str == "sendinfo"){
            string path = strtok(NULL,":");
            int file_size = atoi(strtok(NULL,":"));
            cout<<path<<endl;
            FILE* fp ;
            fp = fopen(path.c_str(), "rb");
            if(fp == NULL){
                perror("error in opening file");
            }
            string sendchunkdata = getchunkinfo(path);
            //cout<<sendchunkdata<<endl;
            int sendchunkdata_len = sendchunkdata.size();
            send(newsockfd, sendchunkdata.c_str(), sendchunkdata_len, 0 );
            fclose(fp);

        }else if(str == "sendfile"){
            string path = strtok(NULL,":");
            int file_size = atoi(strtok(NULL,":"));
            cout<<path<<endl;
            FILE* fp ;
            fp = fopen(path.c_str(), "rb");
            if(fp == NULL){
                perror("error in opening file");
            }
            string okay = "ok;";
            int okay_len = okay.size();
            send(newsockfd, okay.c_str(), okay_len, 0 );
            // send okay signal

            recv(newsockfd, tobesentchunkinfo, BUFFSIZE, 0);

            vector<int> sendvector;
            string toks = strtok(tobesentchunkinfo,":");

            while(toks != ";"){
                sendvector.push_back(atoi(toks.c_str()));
                toks = strtok(NULL,":");
                cout<<sendvector.back()<<endl;
            }

            int num_chunks = file_size / CHUNKSIZE + 1;
            int chunks = sendvector.size();
            int bytes_read;

            for(int i = 0 ; i < chunks ; ++i)
            {
                int ack = sendvector[i];
                printf("Client asked for : %d\n", ack);
                char buff[512];
                int bytes_to_send = (ack == num_chunks ? file_size - (CHUNKSIZE * (num_chunks-1)) : CHUNKSIZE) ;
                fseek(fp, CHUNKSIZE*(ack-1), SEEK_SET);

                while( bytes_to_send > 0 && (bytes_read = fread(buff, sizeof(char), sizeof buff, fp)) > 0 )
                {
                    send(newsockfd, buff, bytes_read, 0);
                    //cout<<buff<<endl;
                    bytes_to_send -= bytes_read;
                }

                printf("Sent the chunk %d\n", ack);
        
            }

            fclose(fp);
        }
    }

    close(newsockfd);

}


int main(int argc, char * argv[])
{
    
    string downldmsg;
    char * ipport = new char[50];
    ipport = argv[1];
    string ip = strtok(ipport,":");
    string port = strtok(NULL," ");
    thisclientIP = ip;                    
    thisclientPort = atoi(port.c_str());                  // setting the port and client for binding on this client
    cout<<ip<<endl;
    cout<<port<<endl;
    int i = 0;
    int portno;

    pthread_t serverlistnerThread;
    pthread_create(&serverlistnerThread, NULL, filegiver, NULL);

    while(1){

        string msg;
        int option = 0;
        cin>>msg;

        if (msg.compare("create_user") == 0) option = 1;
        else if (msg.compare("login") == 0) option = 2;
        else if (msg.compare("create_group") == 0) option = 3;
        else if (msg.compare("join_group") == 0) option = 4;
        else if (msg.compare("leave_group") == 0) option = 5;
        else if (msg.compare("list_requests") == 0) option = 6;
        else if (msg.compare("accept_request") == 0) option = 7;
        else if (msg.compare("list_groups") == 0) option = 8;   // server reply to be parsed
        else if (msg.compare("list_files") == 0) option = 9;  // server reply to be parsed
        else if (msg.compare("upload_file") == 0) option = 10;    // upload file info
        else if (msg.compare("download_file") == 0) option = 11;
        else if (msg.compare("logout") == 0) option = 12;
        else if (msg.compare("show_downloads") == 0) option = 13;
        else if (msg.compare("stop_share") == 0) option = 14;
        else{
            cout<< "Enter correct options please"<<endl;
            continue;
        }

        switch(option){
            case 1 :{ //cout<<msg<<" "<<option<<" ";
                cout<<"Input username and password for signup"<<endl;
                string username;
                string password;
                cin>>username;
                cin>>password;
                send_create_user(option, ip, port, username, password);
                break;
            }
            case 2 :{ //cout<<msg<<" "<<option<<" ";
                cout<<"Input username and password for login"<<endl;
                string username;
                string password;
                cin>>username;
                cin>>password;
                send_login_user(option, ip, port, username, password);

                break;
            }
            case 3 :{ //cout<<msg<<" "<<option<<" ";
                cout<<"Input groupid for creating a group(enter an integer)"<<endl;
                int groupid;
                cin>>groupid;
                send_group_create_info(option, ip, port, groupid);
                break;
            }
            case 4 :{// cout<<msg<<" "<<option<<" ";
                cout<<"Input groupid for joining a group(enter an integer)"<<endl;
                int groupid;
                cin>>groupid;
                send_group_join_info(option, ip, port, groupid);
                break;
            }
            case 5 : {//cout<<msg<<" "<<option<<" ";
                cout<<"Input groupid for leaving a group(enter an integer)"<<endl;
                int groupid;
                cin>>groupid;
                send_leave_group_info(option, ip, port, groupid);
                break;
            }
            case 6 :{ //cout<<msg<<" "<<option<<" ";
                //list_requests <group_id>
                cout<<"Input groupid for listing requests for a group(enter an integer)"<<endl;
                int groupid;
                cin>>groupid;
                send_list_request_group_info(option, ip, port, groupid);
                break;
            }
            case 7 : {// cout<<msg<<" "<<option<<" ";
                //accept_request <group_id> <user_id>
                cout<<"Input groupid and userid for accepting requests for a group"<<endl;
                int groupid;
                int userid;
                cin>>groupid>>userid;
                send_accept_request_group_info(option, ip, port, groupid, userid);
                break;
            }
            case 8 :{ //cout<<msg<<" "<<option<<" ";
                // list_groups
                send_list_all_group_info(option, ip, port);
                break;
            }
            case 9 :{ //cout<<msg<<" "<<option<<" ";
                //list_files <group_id>
                cout<<"Input groupid for listing files"<<endl;
                int groupid;
                cin>>groupid;
                send_groupid_for_listing_files_info(option, ip, port, groupid);
                break;
            }
            case 10 :{ //cout<<msg<<" "<<option<<" ";
                //upload_file <file_path> <group_id>
                cout<<"Input the file path filename and group id for uploading the file"<<endl;
                int groupid;
                string path;
                string fname;
                cin>>path;
                cin>>fname;
                cin>>groupid;
                send_upload_file_info(option, ip, port, path, fname, groupid);
                break;
            }
            case 11 : {//cout<<msg<<" "<<option<<" ";
                //download_file <group_id> <file_name> <destination_path>
                cout<<"Input the groupid, file name and destination path for downloading the file"<<endl;
                int groupid;
                string spath, dpath;
                cin>>groupid;
                cin>>spath;
                cin>>dpath;
                downldmsg = send_download_file_info(option, ip, port, spath, dpath, groupid);
                if(downldmsg.substr(0,4) == "down"){
                    downldmsg = downldmsg + ";:" + dpath + ";" ; // add destn path
                    cout<<"here i am dear"<<downldmsg;
                    downloadactualfile(downldmsg);   
                    cout <<"resached" << endl;
                }
                
                // get some info and go to the thread for getting file from the info obtained
                break;
            }
            case 12 :{// cout<<msg<<" "<<option<<" ";
                //logout
                send_logout_info(option, ip, port);
                break;
            }
            case 13 : {//cout<<msg<<" "<<option<<" ";
                //Show_downloads
                send_show_downloads_info(option, ip, port);
                break;
            }
            case 14 : {cout<<msg<<" "<<option<<" ";
                //stop_share <group_id> <file_name
                int groupid;
                string filename;
                cin>>groupid;
                cin>>filename;
                send_stop_share_info(option, ip, port, groupid, filename);
                break;
            }
            default : break; 

        }

        if(option==11){
            if(downldmsg == "no:"){
                cout<<"tracker has no info for file.";
                continue;
            }
      
        }

    }

    pthread_join(serverlistnerThread, NULL);
    close(sockfd);
    
    return 0;
}

