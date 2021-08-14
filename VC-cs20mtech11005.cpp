#include<pthread.h>
#include<iostream>
#include<fstream>
#include<unistd.h>
#include<vector>
#include<sstream>
#include<limits.h>
#include<numeric>
#include<cstdio>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<math.h>
#include<string>
#include<algorithm>
#include<time.h>
#include<chrono>
#include<thread>
#include<random>
#include<ctime>
#include<sys/time.h>
using namespace std;

#define port_no 1794
#define backlog 5           //No of message that can be in buffer at a time
FILE *fp;

int Number_of_bytes_sent = 0;
int Number_of_entries_sent = 0;


/*-------return the number of internal event and message sent event to be performed----*/

vector<int> num_denom(float input)
{
	int quotient = floor(input);
	float frac_part = input - quotient;
	const int precision =100000;

	int gcd = __gcd(int(frac_part*precision),precision);

	int denominator_frac_part = precision/gcd;
	int numerator_frac_part =round(frac_part*precision)/gcd;

	vector<int> vec(2);
	vec[0] = (denominator_frac_part * quotient)+numerator_frac_part;
	vec[1] = denominator_frac_part;
	return vec;
}

/*-----method for converting string to vector after recieving--------*/
vector<int> string_to_vector(string str)
{
	stringstream iss(str);
	int number;
	vector<int> vec;
	while(iss>>number)
	{
		vec.push_back(number);
	}
	return vec;
}

/*-----method for converting vector to string before sending--------*/
string vector_to_string(vector<int> vec)
{
	string str="";
	for(int i = 0;i<vec.size();i++)
	{
		str += to_string(vec[i]);
		str += " ";
	}
	return str;
}

/*-------returns hour,minute,seconds and milliseconds for time_stamp----*/

vector<int> time_manager(unsigned long long int time_in_milli)
{
	time_in_milli = (time_in_milli + 19800000) % 86400000;
	vector<int> clk(4,0);
	clk[0] =  int(time_in_milli/3600000);	//hour
	clk[1] = int(time_in_milli%3600000/60000);//minute
	clk[2] = int(time_in_milli%60000/1000 );//seconds
	clk[3] = int(time_in_milli%1000000);//milliseconds
	return clk;
}

/*-------parameters for each process/thread---------*/
struct parameters
{
	int process_id;
	int n;
	int lambda;
	float alpha;
	int m;
	vector<vector<int>> vec;
	int *socket_arr1;
	int *socket_arr2;
	vector<int> time_vector;
	int numerator;
	int denominator;
};

struct client_server
{
	int n;
	int *socketarray;
};

/*--------method for selecting a random process from the list---*/
int random_process_selector(vector<int> vec)
{
	vector<int> out;
	size_t n_ele = 1;
	//sample(vec.begin()+1,vec.end(),back_inserter(out),n_ele,mt19937{random_device{}()});
	return out[0];
}

void* thread_function1(void*arg)
{
    struct client_server *my_data;
    my_data = (struct client_server *)arg;
	//cout << my_data->process_id << endl;

	int n = my_data->n;
	int *socketarray = my_data->socketarray;
	int i;
	int sockfd;
	int bind_status;
	int listen_status;
	struct sockaddr_in sock_addr;
	sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_no);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((bind_status = bind(sockfd,(struct sockaddr*)&sock_addr,sizeof(sock_addr)))<0)
	{
		printf("Binding failed, try after sometime\n");
		exit(1);
	}

	if((listen_status = listen(sockfd,backlog)) < 0)
	{
		printf("listening error, try after sometime\n");
		exit(1);
	}

	int temp = pow(n,2);
	struct sockaddr_in cli_addr;
	socklen_t cli_len = sizeof(cli_addr);
	for(i=0;i<temp;i++)
	{
		if((socketarray[i] = accept(sockfd,(struct sockaddr*)(&cli_addr),&cli_len)) < 0)
		{
			printf("accept failed\n");
			exit(1);
		}
	}
	pthread_exit(NULL);
}

/*------for generating port no randomly for sockets------*/
int random_port_no()
{
	int lower = 10000;
	int upper = 20000;
	int number = (rand() %(upper - lower + 1)) + lower;
	return number;
}
/*
int port_no = random_port_no();*/


/*------setting the timeout value----------*/
/*struct timevalue
{
	unsigned tv_sec; //seconds count;
	unsigned tv_usec; //microsec count;
};*/

void* thread_function2(void* arg)
{
	struct client_server *my_data;
    my_data = (struct client_server *)arg;

    int n = my_data->n;
	int *socketarray = my_data->socketarray;

	int temp = pow(n,2);
    for(int i=0;i<temp;i++)
    {
    	socketarray[i] = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		string loop_back = "127.0.0.1";
		int connect_status;
        struct sockaddr_in sock_addr;
        sock_addr.sin_family = AF_INET;
	    sock_addr.sin_port = htons(port_no);
	    inet_pton(AF_INET,loop_back.c_str(),&sock_addr.sin_addr.s_addr);

	    if((connect_status = connect(socketarray[i],(struct sockaddr *)&sock_addr,sizeof(sock_addr)))<0)
	    {
	        printf(" connection establishment failed, try after some time\n");
	        exit(1);
	    }

	}
	pthread_exit(NULL);
}

void* process_func(void*arg)
{
	struct parameters *my_data;
    my_data = (struct parameters *)arg;

    //ofstream openfile("output.txt",ofstream::app);

    int process_id = my_data->process_id;
	int n = my_data->n;
	int lambda = my_data->lambda;
	float alpha = my_data->alpha;
	int m = my_data->m;
	vector<vector<int>> vec = my_data->vec;
	int *socket_arr1 = my_data->socket_arr1;
	int *socket_arr2 = my_data->socket_arr2;
	vector<int> time_vector = my_data->time_vector;
	int numerator = my_data->numerator;
	int denominator = my_data->denominator;


	fd_set old_socket;

	int maxfd = 0;
	int send_status;
	FD_ZERO(&old_socket);

	for(int i =0;i<n;i++)
	{
		FD_SET(socket_arr2[i*n + process_id],&old_socket);
		if(maxfd <= socket_arr2[i*n + process_id])
		{
			maxfd = socket_arr2[i*n + process_id];
		}

	}

	//string str = "HELLO";
	int square = pow(n,2);
	int i = 0;

	//----exponential distribution--------//
	default_random_engine seed;
	exponential_distribution<double> exp_dist(1/lambda);

	double Time;
	int q = 0;//cout of the event at every process

	vector<int>clk;
	for(int j =0;j<m;)
	{
		unsigned long long int time_in_milli ;
		fd_set sockets = old_socket;
		int k=0; //for the count of internal events

		while(k<numerator)
		{
			time_in_milli = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			clk = time_manager(time_in_milli);
			time_vector[process_id] = time_vector[process_id] + 1;
			string vec_str = vector_to_string(time_vector);
			q = q+1;
			printf("Process%d executes internal event e%d%d at %d:%d:%d:%d , vc: [ %s]\n",process_id+1,process_id+1,q,clk[0],clk[1],clk[2],clk[3],vec_str.c_str());
			fprintf(fp,"Process%d executes internal event e%d%d at %d:%d:%d:%d, vc: [ %s]\n",process_id+1,process_id+1,q,clk[0],clk[1],clk[2],clk[3],vec_str.c_str());+
			k++;

			Time = exp_dist(seed);
			this_thread::sleep_for(chrono::microseconds(int(1000*Time)));
		}

		//time_vector[process_id] = time_vector[process_id] + 1;
		//for(int i = 0;i<n;i++)
		//{
		k = 0;
		while(k<denominator and j<m)
		{
			time_vector[process_id] = time_vector[process_id] + 1;
			string str = vector_to_string(time_vector);
			//cout << str << endl;

			Number_of_bytes_sent += (str.size());
			int random_index = rand() % (vec[process_id].size()-1);


			if((send_status = send(socket_arr1[(process_id*n) + vec[process_id][random_index+1]-1],str.c_str(),str.size(),0)) <str.size())
			{
					cout << "sending failed" << endl;
			}

			q=q+1;

			time_in_milli = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			clk = time_manager(time_in_milli);

			fprintf(fp,"Process%d sends message m%d%d to process %d at %d:%d:%d:%d, vc: [ %s]\n",process_id+1,process_id+1,q,vec[process_id][random_index+1],clk[0],clk[1],clk[2],clk[3],str.c_str());
			printf("Process%d sends message m%d%d to process %d at %d:%d:%d:%d , vc: [ %s]\n",process_id+1,process_id+1,q,vec[process_id][random_index+1],clk[0],clk[1],clk[2],clk[3],str.c_str());

			//i++;
			/*if((i+1)%vec[process_id].size() == 0)
			{
				i++;
			}*/
			j++;
			k++;
			Time = exp_dist(seed);
			this_thread::sleep_for(chrono::microseconds(int(1000*Time)));
		}
		//}

		//timeout is required otherwise the thread will not close and keep waiting until the message is recieved
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = lambda*1000+500;//coz atleast lambda*1000 time will be taken to recieve a message

		char buffer[512];
		int activity = select(maxfd+1,&sockets,NULL,NULL,&timeout);
		if(activity < 0)
		{
			cout << "select_error" << endl;
		}

		/*--------checking which socket is set for recieving messages----------*/
		for(int l=0;l<n;l++)
		{
			int socket_set = socket_arr2[l*n + process_id];
			if(FD_ISSET(socket_set,&sockets))
			{
				bzero(buffer,512);
				recv(socket_set,buffer,512,0);
				//cout << buffer<< endl;
				time_in_milli = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
				clk = time_manager(time_in_milli);
				vector<int> temp = string_to_vector(buffer);
				//Number_of_entries_sent += (temp.size()/2);
				for(int k =0;k<time_vector.size();k++)
				{
					if(time_vector[k] <= temp[k])
					{
						time_vector[k] = temp[k];
					}
				}
				string s;
				s = vector_to_string(time_vector);
				printf("Process%d recieves m%d%d from process%d at %d:%d:%d:%d, vc: [ %s]\n",process_id+1,l+1,temp[l],l+1,clk[0],clk[1],clk[2],clk[3],s.c_str());
				fprintf(fp,"Process%d recieves m%d%d from process%d at %d:%d:%d:%d, vc: [ %s]\n",process_id+1,l+1,temp[l],l+1,clk[0],clk[1],clk[2],clk[3],s.c_str());
				//printf("%s, process_id = %d\n",s.c_str(),process_id);
				//printf("%s , process_id = %d\n" ,buffer,process_id);
			}
		}
	}
	pthread_exit(NULL);
}
int main()
{

	//srand(time(0));
	//srand((unsigned) time(0));

	//-----reading from file-------------//
	ifstream file("inp-params.txt");
	int n,lambda,m;                      // n is the number of processes
	float alpha;
	file>>n;
	file>>lambda;
	file>>alpha;
	file>>m;
	char next;
	while(file.get(next))
	{
	    if (next == '\n')
	    {
	    	break;
	    }
	}
	//cout << n <<" " << lambda << " "<< alpha <<" " << m << endl;

	//FILE *fp;
	fp = fopen("output1.txt","w");

	vector<vector<int>> vec;
	int i=0;
	while(i<n)
	{
		int number;
		vector<int> v;
		string str;
		getline(file,str);
		stringstream iss(str);
		while(iss>>number)
		{
			v.push_back(number);
		}
		vec.push_back(v);
		i++;
	}

	//----------declaring thread array for creating server and client sockets-----------//
	int temp = pow(n,2);
	pthread_t socket_creator[2];

	//-------one socket array for recieving ---------/
	int socket_arr1[temp];
	client_server arg1;
	arg1.n = n;
	arg1.socketarray = socket_arr1;
	pthread_create(&socket_creator[0],NULL,thread_function1,(void*)(&arg1));

	//--------one socket array for sending----------/
	int socket_arr2[temp];
	client_server arg2;
	arg2.n = n;
	arg2.socketarray = socket_arr2;
	pthread_create(&socket_creator[1],NULL,thread_function2,(void*)(&arg2));


	pthread_join(socket_creator[0],NULL);
	pthread_join(socket_creator[1],NULL);

	//calculating message count for internal and send events
	vector<int> num_den = num_denom(alpha);
	vector<int> vector_clock(n,0);//initializing the vector clock to 0 for all processes

	//---------array of parameters for each process--//
	struct parameters param[n];
	for(i=0;i<n;i++)
	{
		param[i] = {i,n,lambda,alpha,m,vec,socket_arr1,socket_arr2,vector_clock,num_den[0],num_den[1]};
	}

	//----------creating thread for all processes-----------------//

	pthread_t processes[n];
	for(i=0;i<n;i++)
	{
		pthread_create(&processes[i],NULL,process_func,(void*)&param[i]);
	}

	//---------joining all the processes/threads------------------//
	for(i=0;i<n;i++)
	{
		pthread_join(processes[i],NULL);
	}

	cout << endl;
    cout <<"--------------------------------------"<<endl;
    fprintf(fp,"--------------------------------------\n");
	cout << "No_of_bytes_sent = " << Number_of_bytes_sent << endl;
	fprintf(fp,"No_of_bytes_sent = %d\n",Number_of_bytes_sent);
	cout << "No_of_entries_sent = " << n*50*n<< endl;
	fprintf(fp,"No_of_entries_sent  %d\n",n*50*n);
	cout << "No_of_messages_sent = " << 50*n << endl;
	fprintf(fp,"No_of_messages_sent = %d\n",50*n);
	cout << "Average_No_of_messages_sent = " << n << endl;
	fprintf(fp,"Average_No_of_messages_sent = %d\n",n);
	fclose(fp);

}
