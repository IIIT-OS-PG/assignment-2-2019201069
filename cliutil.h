#ifndef _TRACKERCLI_
#define _TRACKERCLI_

#include "sha256.h"

bool loggedinflag = false;
string u_name_g ;  // global username for this client
string thisclientIP;
int thisclientPort;

string sendmsgdata(string params, int dataitems){

	string msg = "";
	int sockfd, portno, n;
    struct sockaddr_in ser_addr;

    char buffer[MSGSIZE],server_reply[MSGSIZE];
    portno = PORTNOS;    // server port number that will be supplied from a file
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(IPS); // server ip taken from a file
    ser_addr.sin_port = htons(portno);
    
    int status = connect(sockfd,(struct sockaddr *) &ser_addr,sizeof(ser_addr));

    if (status < 0)
    	perror("Error in connecting yedu\n");
    else
        printf("Connected...\n");

    params = params+";";  // at the end of string
    strcpy(buffer,params.c_str());
	
	send (sockfd , buffer, sizeof(buffer), 0 );

    recv(sockfd , server_reply , BUFFSIZE , 0);
    printf("Server reply : %s \n", server_reply);
    string server_reply_s = server_reply;

    if(server_reply_s == "successfully logged in."){
    	loggedinflag = true;
    	cout<<"abe sach me login ho gaya"<<endl; 
    }
    else if(server_reply_s == "successfully logged out."){
    	loggedinflag = false;
    	cout<<"out tata byebye"<<endl; 
    }
    else if(server_reply_s.substr(0,4) == "down"){
    	cout<<"gotta download"<<endl;
    	msg = server_reply_s; 
    }
    close(sockfd);
    return msg;
}

void send_create_user(int option, string ip, string port, string username, string password);
               
void send_login_user(int option, string ip, string port, string username, string password);

void send_group_create_info(int option, string ip, string port, int groupid);
             
void send_group_join_info(int option, string ip, string port, int groupid);
           
void send_leave_group_info(int option, string ip, string port, int groupid);
               
void send_list_request_group_info(int option, string ip, string port, int groupid);
                
void send_accept_request_group_info(int option, string ip, string port, int groupid, int userid);
                
void send_list_all_group_info(int option, string ip, string port);
                
void send_groupid_for_listing_files_info(int option, string ip, string port, int groupid, int userid);
               
void send_logout_info(int option, string ip, string port);
              
void send_show_downloads_info(int option, string ip, string port);

void send_upload_file_info(int option, string ip, int port, string path, int groupid);
               
string send_download_file_info(int option, string ip, int port, string spath, string dpath, int groupid);

void send_stop_share_info(int option, string ip, int port, int groupid, string filename);
 

///////////////////////////////// ***************************************  ////////////////////////////////////////////////// 


void send_create_user(int option, string ip, string port, string username, string password){

	string option_s;
	string col = COL;
	option_s = to_string(option);
	string params = "";
	params = params + option_s + col + ip + col + port + col + username + col + password + col;
	cout<<params<<endl;
	sendmsgdata(params, 5);
}
               
void send_login_user(int option, string ip, string port, string username, string password){
	
	string option_s;
	string col = COL;
	option_s = to_string(option);
	string params = "";
	params = params + option_s + col + ip + col + port + col + username + col + password + col;
	cout<<params<<endl;
	sendmsgdata(params, 5);
	if(loggedinflag){
		u_name_g = username;
	}
}

void send_group_create_info(int option, string ip, string port, int groupid){

	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col + u_name_g + col;
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 5);
	}
	else{
		cout<<"first login to use."<<endl;
	}
	
}
             
void send_group_join_info(int option, string ip, string port, int groupid){
	
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 5);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
           
void send_leave_group_info(int option, string ip, string port, int groupid){
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 5);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
               
void send_list_request_group_info(int option, string ip, string port, int groupid){
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 5);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
                
void send_accept_request_group_info(int option, string ip, string port, int groupid, int userid){
	string option_s, groupid_s, userid_s ;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	userid_s = to_string(userid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col + userid_s + col;
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 6);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
                
void send_list_all_group_info(int option, string ip, string port){
	string option_s;
	string col = COL;
	option_s = to_string(option);
	string params = "";
	params = params + option_s + col + ip + col + port + col;
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 3);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
                
void send_groupid_for_listing_files_info(int option, string ip, string port, int groupid){
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + groupid_s + col;
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 4);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
               
void send_logout_info(int option, string ip, string port){
	string option_s;
	string col = COL;
	option_s = to_string(option);
	string params = "";
	params = params + option_s + col + ip + col + port + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 4);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
              
void send_show_downloads_info(int option, string ip, string port){
	string option_s;
	string col = COL;
	option_s = to_string(option);
	string params = "";
	params = params + option_s + col + ip + col + port + col;
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 3);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}

void send_upload_file_info(int option, string ip, string port, string path, string fname, int groupid){
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	FILE *fp = fopen(path.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	long long filesize = ftell(fp);
	long long chunks = filesize/CHUNKSIZE + 1; 
	string filesize_s = to_string(filesize);
	rewind(fp);
	fclose(fp);
	string sha = sha256_file(fp);// = shalcalc();

	for(int i = 1; i <= chunks; i++){
		chunkinfo[path].insert(i);
	}

	//path, sha ,groupid, filesize, username    FILE *fp2 = fopen("kkk.txt", "rb");
    //string msg1 = sha256_file(fp1);
	params = params + option_s + col + ip + col + port + col + path + col + fname + col + fname + col + groupid_s + col + filesize_s + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 9);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}
               
string send_download_file_info(int option, string ip, string port, string spath, string dpath, int groupid){
	string option_s, groupid_s, msg;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col + spath + col + dpath + col + groupid_s + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		msg = sendmsgdata(params, 7);

	}
	else{
		msg="no:";
		cout<<"first login to use."<<endl;
	}
	return msg;
	//send_download_file_info(option, ip, port, spath, dpath, groupid);
}

void send_stop_share_info(int option, string ip, string port, int groupid, string filename){
	string option_s, groupid_s;
	string col = COL;
	option_s = to_string(option);
	groupid_s = to_string(groupid);
	string params = "";
	params = params + option_s + col + ip + col + port + col +  groupid_s + col + filename + col + u_name_g + col; 
	if(loggedinflag){
		cout<<params<<endl;
		sendmsgdata(params, 6);
	}
	else{
		cout<<"first login to use."<<endl;
	}
}


#endif