#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <iostream>
#include <pthread.h>
using namespace std;
#define IPS "127.0.0.1"
#define IPC "127.0.0.1"
#define PORTNOS 12348
#define PORTNOC 12358
#define BUFFSIZE 512
#define MSGSIZE 1024*32
#define COL ":"

typedef struct clientinfo{   // info of one client
    
    string cliIPaddr;
    int  cliport;
    string username;
    string password;
    int loggedinflag;

}clientinfo;

typedef struct filesinfo{ 
    
    string filesha;
    string filename;
    long long filesize;
    //string filepath;
    //set <int> grouphasthefile;  // set of groups that have the file

}filesinfo;

typedef struct filesupload{ 
    
    string filesha;
    string filename;
    int groupid;
    string filepath;
    string username;
    
}filesupload;

typedef struct clientgroup{

    vector <string> clientsingroup;   // username of all the clients in a group
    clientinfo owner; // owner of the group // that client must be sent request to join the group
    //vector <filesinfo> filesinagroup; // all files that can be shared within a group
    int groupid;

}clientgroup;

vector<clientgroup> cligroup;   // contains all groups
vector<clientinfo> clients;  // contains all clients
unordered_map<string, clientinfo> unamemap;  // username and clientinfo map check if the user exists
unordered_map<int, clientgroup> groupmap;  // gropid and group map check if the groupid exists
unordered_map<string, filesinfo> filemap; // filesha and fileinfo
unordered_map<string, string> fileidtosha;
//ds group pending requests 
//pendingreq; 
// // requests pending to be accepted by owner -> first string for user -> second string along with groupid

unordered_map<string, vector< filesupload > > fileswithgroupsandclients;
unordered_map<int, set<string> > grouptofilemap;
// filesha/ filepath with groupid and username

//  uploadfile(path, groupid, username); 
string uploadfile(string path, string fname, string sha, int grid, int filesize, string username){
	string msg;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	else if(find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username) == groupmap[grid].clientsingroup.end() ){
		msg = "user does not belong to the group.";
		return msg;
	}
	else{  // good to go as group is present and user belongs to group
		filesinfo f1;
		f1.filesha = sha;
    	f1.filename = fname; // strtok name constant id for a file
    	f1.filesize = filesize;
    	//f1.filepath = path;
    	if(filemap.find(f1.filesha) == filemap.end()){  // file already not present .. hence add to file map
    		filemap[sha] = f1; // put in filesinfo
    		fileidtosha[f1.filename] = sha;
    		grouptofilemap[grid].insert(f1.filename);
    	}	
    		filesupload fu1;
    		fu1.filesha = sha;
    		fu1.filename = path;// strtok not needed
    		fu1.groupid = grid;
    		fu1.filepath = path;
    		fu1.username = username;
    		//fileswithgroupsandclients[sha].insert(fu1); // this file is with 
    		fileswithgroupsandclients[sha].push_back(fu1);
    
    	msg = "file info uploaded for sharing.";
	} /// user u1 of group g1 has file. now the file just belongs to the group
}

string getsenderinfo(set <pair <string, string> > userswiththeirpathrequired){
	string msg;
	string col = COL;
	for (auto const &x : userswiththeirpathrequired) { 
       		msg = msg + x.first + col + unamemap[x.first].cliIPaddr + col + to_string(unamemap[x.first].cliport) + col + x.second + col;
    } 
    return msg;
}

					// can have file sha
string downloadfile(string filename, string dpath, int grid, string username){
	string msg;
	set <pair <string, string> > userswiththeirpathrequired;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	else if(find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username) == groupmap[grid].clientsingroup.end() ){
		msg = "user does not belong to the group.";
		return msg;
	}
	else{
		string sha = fileidtosha[filename];
		if(fileswithgroupsandclients.find(sha) == fileswithgroupsandclients.end())
			msg = "file not present.";
		else{
			for (auto const &x : fileswithgroupsandclients[sha]) { 
        			if(x.groupid == grid){    // got a vector with file... check if same grid
        				userswiththeirpathrequired.insert(make_pair(x.username, x.filepath));
        			}
     		} 
     		msg = "down:" + to_string(filemap[sha].filesize)+ COL +getsenderinfo(userswiththeirpathrequired);
		}
	}
		
	return msg; /// user u1 of group g1 has file. now the file just belongs to the group
}

//creategroup(groupid, username);
bool creategroup(int grid, string username){
    
    clientgroup g1;
    clientinfo ownerofgroup = unamemap[username];
    g1.owner = ownerofgroup;
    g1.groupid = grid;
    g1.clientsingroup.push_back(username); // add even the owner to the clients in a group vector
    if( groupmap.find(grid) != groupmap.end() ){   // if group with groupid exists
        string msg = "group name already exists";
        cout<<msg<<endl;
        return false;
    }
    else{
        cligroup.push_back(g1); 
        groupmap[grid] = g1;
        return true;
    }
    return false;

}

string joingroup(int grid, string username){
	string msg;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	else if(find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username) == groupmap[grid].clientsingroup.end() ){
		groupmap[grid].clientsingroup.push_back(username);
		msg = "join request pending with owner";
		return msg;
	}
	else 
		msg = "user already exists.";
	return msg;

}

string listrequests(int grid, string username){
	string msg;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	else if(groupmap[grid].owner.username == username){   // the user is the owner of the group then only display the reqs
		//msg = ".";list the pending reqests with the owner : corresponding groupids and users as well
	}
	else 
		msg = "user already exists.";
	return msg;

}

string leavegroup(int grid, string username){
	string msg;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	else if(find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username) == groupmap[grid].clientsingroup.end() ){
		msg = "user does not exist in the group";
		return msg;
	}
	// owner cannot exit the group
	else if(groupmap[grid].owner.username == username){
		msg = "owner cannot exit the group.";
		return msg;
	}
	else {
		auto it = find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username);
		groupmap[grid].clientsingroup.erase(it); // erase the entry from the group of a particular client
		msg = "user removed from group successfully.";
	}
	return msg;

}

string login(string IP, int port, string uname, string passd){

	string msg = "";
	if(unamemap.find(uname) == unamemap.end()){   // user not present
		// first signup
		msg = msg+"user not present. first create user.";
	}
    else if(passd.compare(unamemap[uname].password) == 0){
        unamemap[uname].loggedinflag = 1;
        unamemap[uname].cliIPaddr = IP;
        unamemap[uname].cliport = port;
        msg = msg+"successfully logged in.";
    }
    else{
        // do not let the user login
        msg = msg+"incorrect password.";
    }
    return msg;
}
 
string listgroups(){
	string msg;
	int len = cligroup.size();
	for(int i = 0 ; i < len; i++){
		msg = msg + to_string(cligroup[i].groupid) + COL;
	}
	return msg;
}

string listfiles(int grid){
	string msg;
	string col = COL;
	if(groupmap.find(grid) == groupmap.end()){
		msg = "group does not exist.";
		return msg;
	}
	// else if(find(groupmap[grid].clientsingroup.begin(), groupmap[grid].clientsingroup.end(), username) == groupmap[grid].clientsingroup.end() ){
	// 	msg = "user does not belong to the group.";
	// 	return msg;
	// }
	else if(grouptofilemap.find(grid) == grouptofilemap.end()){
		msg = "no files in the group.";
	}
	else{
		msg = "list" + col;
		for (auto const &x : grouptofilemap[grid]) { 
        		msg = msg + x + col;
     		} 
	}
	return msg;
}

string logout(string uname){
    string msg = "";
    unamemap[uname].loggedinflag = 0;
    msg = msg+"successfully logged out.";
    return msg;
}

bool signup(string IP, int port, string uname, string passd){   // for create
    clientinfo user;
    user.username = uname;
    user.password = passd;
    user.loggedinflag = 0;
    user.cliport = port;
    user.cliIPaddr = IP;
    
    if( unamemap.find(uname) != unamemap.end() ){
        string msg = "user already exists";
        cout<<msg<<endl;
        return false;
    }

    else{

        clients.push_back(user);
        unamemap[uname] = user;
        return true;
    }

    return false;
    
}


string check_option(int option, string params[20], int params_num){
    string returnmsg;

    switch(option){
       

			case 1 :{ //cout<<msg<<" "<<option<<" ";
				string ip = params[1];
				int port = atoi(params[2].c_str());
				string username = params[3];
				string password = params[4];
				if(signup(ip, port, username, password)){
					// send msg signed up
					returnmsg = "signed up.";
				}
				else{
					// unable to signup, username already exists
					returnmsg = "user already exists.";
				}
                //send_create_user(option, ip, port, username, password);
                break;
            }
            case 2 :{ //cout<<msg<<" "<<option<<" ";
                string ip = params[1];
				int port = atoi(params[2].c_str());
				string username = params[3];
				string password = params[4];
				returnmsg = login(ip, port, username, password);
                break;
            }
            case 3 :{ //cout<<msg<<" "<<option<<" ";
                string ip = params[1];
                int port = atoi(params[2].c_str());
                int groupid = atoi(params[3].c_str());
                string username = params[4];
                //send_group_create_info(option, ip, port, groupid);
                if(creategroup(groupid, username)){
                	returnmsg = "group created successfully.";
                }
                else{
                	returnmsg = "group already exists. use another group id.";
                }
                break;
            }
            case 4 :{// cout<<msg<<" "<<option<<" ";
                string ip = params[1];
                int port = atoi(params[2].c_str());
                int groupid = atoi(params[3].c_str());
                string username = params[4];
                returnmsg = joingroup(groupid, username);
                //send_group_join_info(option, ip, port, groupid);
                break;
            }
            case 5 : {//cout<<msg<<" "<<option<<" ";
            	string ip = params[1];
                int port = atoi(params[2].c_str());
                int groupid = atoi(params[3].c_str());
                string username = params[4]; 
                returnmsg = leavegroup(groupid, username);
                //send_leave_group_info(option, ip, port, groupid);
                break;
            }
            // case 6 :{ //cout<<msg<<" "<<option<<" ";
            //     string ip = params[1];
            //     int port = atoi(params[2].c_str());
            //     int groupid = atoi(params[3].c_str());
            //     string username = params[4]; 
            // 	returnmsg = listrequests(groupid, username);
            //     //send_list_request_group_info(option, ip, port, groupid);
            //     break;
            // }
            // case 7 : {// cout<<msg<<" "<<option<<" ";
            //     //accept_request <group_id> <user_id>
            //     string ip = params[1];
            //     int port = atoi(params[2].c_str());
            //     int groupid = atoi(params[3].c_str());
            //     string username_c = params[4]; 
            //     string username_o = params[4]; 
            // 	returnmsg = listrequests(groupid, username_o, username_u);
            //     //send_accept_request_group_info(option, ip, port, groupid, userid);
            //     break;
            // }
            case 8 :{ //cout<<msg<<" "<<option<<" ";
                // list_groups
            	returnmsg = listgroups();
                //send_list_all_group_info(option, ip, port);
                break;
            }
            case 9 :{ //cout<<msg<<" "<<option<<" ";
                //list_files <group_id>
            	int groupid = atoi(params[3].c_str());
            	returnmsg = listfiles(groupid);
                //send_groupid_for_listing_files_info(option, ip, port, groupid);
                break;
            }
            case 10 :{ //cout<<msg<<" "<<option<<" ";
                //upload_file <file_path> <group_id>
            	string ip = params[1];
                int port = atoi(params[2].c_str());
                string path = params[3];
                string fname = params[4];
                string sha = params[4];
                int groupid = atoi(params[6].c_str());
                int filesize = atoi(params[7].c_str());
                string username = params[8]; 
                returnmsg = uploadfile(path, fname, sha ,groupid, filesize, username);
                //
                break;
            }
            case 11 : {//cout<<msg<<" "<<option<<" ";
                //download_file <group_id> <file_name> <destination_path>
            //params = params + option_s + col + ip + col + port + col + spath + col + dpath + col + groupid_s + col + u_name_g + col; 
				string ip = params[1];
                int port = atoi(params[2].c_str());
                string spath = params[3];
                string dpath = params[4];
                int groupid = atoi(params[5].c_str());
                string username = params[6]; 
                returnmsg = downloadfile(spath, dpath ,groupid, username);
                break;
            }
            case 12 :{// cout<<msg<<" "<<option<<" ";
                //logout
            	string ip = params[1];
            	int port = atoi(params[2].c_str());
            	string username = params[3];
            	returnmsg = logout(username);
                break;
             }
            // case 13 : {//cout<<msg<<" "<<option<<" ";
            //     //Show_downloads
            //     send_show_downloads_info(option, ip, port);
            //     break;
            // }
            // case 14 : {cout<<msg<<" "<<option<<" ";
            //     //stop_share <group_id> <file_name

            //     break;
            // }
            default : break; 

        }

        return returnmsg;

}



