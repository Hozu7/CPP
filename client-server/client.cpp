//For Linux OS

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <fstream>
#include <arpa/inet.h>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <ctime>


using namespace std;

#define DEFAULT_PORT 8080

//Global vars
bool manual;
int time_out = 300000;

// Function prototypes
char* toLower(char* str);
bool readConfigFile(const char* filePath, string& ip, int& port, int& timeout);

int main(int argc, char* argv[]) {

    // Creating client socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;// Defining variable for client
    serverAddress.sin_family = AF_INET;// IP type IPv4

    if (argc == 2) {// CASE: manual
        if (strcmp(toLower(argv[1]), "manual") == 0) {
            serverAddress.sin_port = htons(DEFAULT_PORT);// Set default port number
            serverAddress.sin_addr.s_addr = INADDR_ANY;// Set default IP address (any)
	    manual = true;// Manual mode
        } else {
            cout << endl << "Mode does not exist." << endl;
            return 1;
        }

    } else if (argc == 3) {// CASE: automatic
        if (strcmp(toLower(argv[1]), "automatic") == 0) {

            // Open and parse the configuration file passed as argv[2]
            string ip;
            int port, timeout;

            if (!readConfigFile(argv[2], ip, port, timeout)) {
                cout << endl << "Error reading configuration file." << endl;
                return 1;
            }

            // Convert IP address string to binary form
            if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr.s_addr) < 0) {
                cout << endl << "Invalid IP address format: " << ip.c_str() << endl;
                return 1;
            }

            serverAddress.sin_port = htons(port);// Set port from file
	    time_out = timeout;// Set time_out from file
	    manual = false;// Automatic mode

        } else {
            cout << endl << "Mode does not exist." << endl;
            return 1;
        }

    } else {
        cout << endl << "Too many arguments." << endl;
        return 1;
    }

    // Sending connection request
    if (connect(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        return 1;
    }

    // Coordinates buffer
    int buffer[3] = { 0 };
    // Code buffer
    char code;

    int x, y;

    if(manual){// manual mode

	buffer[0] = 0;
	cout << "Input coordinates, or a negative integer to quit: ";

	while(true){

	    if(cin >> x >> y){
	        if(x < 0 || y < 0){
		    cout << endl << "Severing connection. \n";
		    close(serverSocket);
		    return 0;
	        }else{
		    buffer[1] = x;
		    buffer[2] = y;
		    cout << "Sending" << endl;

		    if(send(serverSocket, buffer, sizeof(buffer), 0) < 0){
			cout << endl << "Sold out.";
			return 0;
		    }

		    if(recv(serverSocket, &code, sizeof(char), 0) < 1){
			cout << endl << "Sold out.";
			return 0;
		    }

		    if(code == 0){
			cout << endl << "The seat is yours. Input new coordinates, for another ticket or a negative integer to quit: ";
		    }else if(code == 1){
			cout << endl << "The seat is already taken. Input new coordinates, for another ticket or a negative integer to quit: ";
		    }else if(code == 2){
		        cout << endl << "Seat out of bounds. Input new coordinates, for another ticket or a negative integer to quit: ";
		    }
		}
	    }else{
		cout << "Only integers." << endl;
		cin.clear();
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
	    }



	}
    }else if(!manual){// automatic mode

	cout << "AUTOMATIC MODE" << endl;
	buffer[0] = 1;// Automatic mode
	buffer[1] = 0;
	buffer[2] = 0;
	if(send(serverSocket, buffer, sizeof(buffer), 0) < 0){// Dimensions request
	    return 0;
	}
	int dimBuffer[2] = { 0 };// Dimensions buffer
	if(recv(serverSocket, dimBuffer, sizeof(dimBuffer), 0) < 1){// Receiving dimensions
	    cout << "Sold out." << endl;
	    return 0;
	}


	while(true){

	    //generate random coordinates
	    srand(time(0));
	    x = rand() % dimBuffer[0];
	    y = rand() % dimBuffer[1];

	    buffer[1] = x;
	    buffer[2] = y;

	    usleep(time_out);

	    if(send(serverSocket, buffer, sizeof(buffer), 0) < 0){
	        cout << endl << "Sold out. \n";
	        return 0;
	    }

            if(recv(serverSocket, &code, sizeof(char), 0) < 1){
	        cout << endl << "Sold out. \n";
	        return 0;
	    }

	    if(code == 0){
	        cout << endl << "The seat is yours.";
	    }else if(code == 1){
	        cout << endl << "The seat is already taken.";
	    }else if(code == 2){
	        cout << endl << "Seat out of bounds.";
	    }
        }
    }

    //Closing socket
    close(serverSocket);

    return 0;
}

// Convert string to lower to help input validation
char* toLower(char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        str[i] = tolower(static_cast<unsigned char>(str[i]));
    }
    return str;
}

// Function to read configuration file and parse IP, Port, and Timeout
bool readConfigFile(const char* filePath, string& ip, int& port, int& timeout) {

    ifstream configFile(filePath);

    if (!configFile.is_open()) {
        return false;
    }

    string line;
    bool foundIP = false, foundPort = false, foundTimeout = false;

    // Skip the first line "[Connection]"
    getline(configFile, line);

    // Parse the rest of the lines for IP, Port, and Timeout
    while (getline(configFile, line)) {

        // Trim leading/trailing whitespaces
        line = line.substr(line.find_first_not_of(" \t"), line.find_last_not_of(" \t") + 1);

        if (line.find("IP") == 0) {// IP = x.x.x.x
            size_t pos = line.find("=");
            if (pos != string::npos) {
                ip = line.substr(pos + 1);
		//cout << endl << ip << endl;
                foundIP = true;
            }
        } else if (line.find("Port") == 0) {// Port = z
            size_t pos = line.find("=");
            if (pos != string::npos) {
                string portStr = line.substr(pos + 1);
                port = stoi(portStr);// Convert to int
                foundPort = true;
            }
        } else if (line.find("Timeout") == 0) {// Timeout = y
            size_t pos = line.find("=");
            if (pos != string::npos) {
                string timeoutStr = line.substr(pos + 1);
                timeout = stoi(timeoutStr);// Convert to int
                foundTimeout = true;
            }
        }
    }

    // Return true if all fields were found and parsed
    return foundIP && foundPort && foundTimeout;
}
