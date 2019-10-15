#include "trackerutil.h"


int sockfd;

void * recievemsgdata(void * threadarg){

    int * newsockfd =  (int *)(threadarg);
    char buffer[BUFFSIZE], msg_data[MSGSIZE]; 
            string params[20];

            int option;
            recv(*newsockfd, msg_data, sizeof(msg_data), 0);  // get the option from client socket
            
            cout<<msg_data<<endl;
            params[0] = strtok(msg_data,COL);
            int params_num = 0;
            while(params[params_num] != ";"){  // delim used at the end
                params_num++;
                params[params_num] = strtok(NULL,COL);
                cout<<params[params_num]<<endl;
                if(params[params_num] == ";")
                    break;
            }

            option = atoi(params[0].c_str());

            cout<<"option is : "<<option<<endl;
            cout<<"no of params is : "<<params_num<<endl;

            string msg = check_option(option, params, params_num);
            //printf("Sent to client : %s\n",msg);
            cout<<"Sent to client"<<msg;
            char msg_c[100];
            strcpy(msg_c,msg.c_str());
            send(*newsockfd , msg_c , BUFFSIZE, 0);
}

void recievedata(){
    
     int newsockfd, portno;
     struct sockaddr_in ser_addr, cli_addr;
     socklen_t clientlen;
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
     portno = PORTNOS;   // serverport no
     ser_addr.sin_family = AF_INET;
     ser_addr.sin_addr.s_addr = inet_addr(IPS);  // server ip
     ser_addr.sin_port = htons(portno);
     
    bind(sockfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    
    printf("Tracker started... Waiting for connection...\n");
    
    listen(sockfd,5);

    
    int i = 0;
    clientlen = sizeof(cli_addr);
    while(1){
        
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientlen);   // accept from the client --> new sockid contains 
        if(newsockfd < 0){
            cout<<"unable to accept the socket";
            exit(EXIT_FAILURE);
        }
        
        pthread_t trackerlistnerThread[10];
        int thread_status = pthread_create(&trackerlistnerThread[i], NULL, recievemsgdata, (void *)&newsockfd);
        if(thread_status != 0){
            cout<<"thread creation failed"<<endl;
        } 
        if( i >= 4)
            {
                i = 0;
                while(i < 4)
                {
                        pthread_join(trackerlistnerThread[i++],NULL);
                }
                 i = 0;
            }

    }  // end while 1


        close(newsockfd);
        close(sockfd);

}




int main()
{

    recievedata();
    return 0;
}

