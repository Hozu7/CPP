//For Linux OS

#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <pthread.h>
#include <mutex>

using namespace std;

// Global vars
int rows, cols, serverSocket;
atomic<int>tickets;
int** seats;
mutex mtx;

// Function prototypes
void printMatrix(int** matrix, int rows, int cols);
void* threader(void* arg);

int main(int argc, char* argv[]){

    bool invalidIn = false;

    // INPUT SANITATION
    if(argc == 1){// CASE: ./server.cpp

	rows = 10;
	cols = 10;

    }else if(argc == 3){// CASE: ./server.cpp x x

	for(int i = 1; i < 3; i++){// Check x's are integers

	    char* end;
	    long val = strtol(argv[i], &end, 10);// Convert argument to long

	    if(*end == '\0' && val >= 1 && val <= 30){// Checking x's are both integers & under 30; can change 30 too

		if(i == 1) rows = val;// Passing long value to int; val can't be greater than 30
		else if(i == 2) cols = val;

	    }else{// Not valid

		invalidIn = true;
		break;

	    }
    	}
    }else{// Any other case is to be considered invalid

	invalidIn = true;

    }

    if(invalidIn){

	cout << "Invalid input for rows and columns; retry: " << endl;

	while(!(cin >> rows >> cols) || rows < 1 || rows > 30 || cols < 1 || cols > 30){

	    cout << "Invalid input. \n";
	    cin.clear();
	    cin.ignore(numeric_limits<streamsize>::max(), '\n');
	}
    }

    //cout << rows << " || " << cols << endl;

    seats = new int* [rows];// Matrix of pointers
    for(int i = 0; i < rows; i++){

	seats[i] = new int[cols];// Allocating memory space for each column

    }

    for(int i = 0; i < rows; i++){// Initializing matrix
	for(int j = 0; j < cols; j++){
	    seats[i][j] = 0;// Seat = 0 means it can be bought; 1 it's been bought
	}
    }

    tickets = rows * cols;
    printMatrix(seats, rows, cols);// Print initial matrix

    // Creating socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding socket
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Listening to the assigned socket
    listen(serverSocket, 5);

    // Accepting connection request
    int* clientSocket = new int();

    while((*clientSocket = accept(serverSocket, nullptr, nullptr)) > -1){
	pthread_t th;
        pthread_create(&th, nullptr, threader, static_cast<void*>(clientSocket));
    }
}

// Prints matrix
void printMatrix(int** matrix, int rows, int cols){

    cout << "Available tickets: " << tickets << endl;

    for(int i = 0; i < rows; i++){
	for(int j = 0; j < cols; j++){
	    cout << "| " << matrix[i][j] << " |";
	}
	cout << endl;
    }
}

void* threader(void* arg){

    int* clientSocketpointer = static_cast<int*>(arg);
    int clientSocket = *clientSocketpointer;

    // Receive data buffer
    int buffer[3] = { 0 };
    // Send data buffer
    char code;


    while(tickets > 0){

        if(recv(clientSocket, buffer, sizeof(buffer), 0) < 1){
	    break;
	}else if(buffer[0] == 1){// Automatic mode
	    int dimBuffer[2] = { rows, cols };
	    if(send(clientSocket, dimBuffer, sizeof(dimBuffer), 0) < 0){// Send rows and columns
		cout << "Couldn't connect with AUTO client." << endl;
		break;
	    }
	    if(recv(clientSocket, buffer, sizeof(buffer), 0) < 1){
	        cout << "No random coordinates generated exiting";
	        break;
	    }
	}

	//here

	if(buffer[1] < rows && buffer[2] < cols && buffer[1] >= 0 && buffer[2] >= 0){// Seat is in bounds
	    if(seats[buffer[1]][buffer[2]] == 0){// Seat is not taken

		mtx.lock();
		if(tickets < 1){
		    break;
		}else{

		    code = 0;// Seat not taken, and in bounds; successfully purchased;
		    seats[buffer[1]][buffer[2]] = 1;// Edit matrix
		    tickets--;// Update number of tickets left
		    printMatrix(seats, rows, cols);

		}
		mtx.unlock();

	    }else{
	        code = 1;// Seat is taken
	    }
	}else{
	    code = 2;// Seat is not in bounds
	}
	if(send(clientSocket, &code, sizeof(char), 0) < 0){
    	    break;
        }
    }

    if(tickets < 1){
        mtx.unlock();
	shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
    }

    close(clientSocket);
    delete clientSocketpointer;
    return nullptr;

}
