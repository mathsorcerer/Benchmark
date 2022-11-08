#include "Server.hpp"

int main(int argc, char **argv)
{
    Server Server(argv);
    
    int num_and_gates = 0;
    int num_xor_gates = 0;

    if(argc < 2) {
        cout << "Lack parameters!\n";
    }
    else {
        for(int i = 3; i < argc; ++i) {
            if(strcmp(argv[i], "-a") == 0) {
                num_and_gates = atoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-x") == 0) {
                num_xor_gates = atoi(argv[i+1]);
            }
            ++i;
        }
    }
    
    Server.initialize_ORAM(num_and_gates, num_xor_gates);
    
    Server.process();
    
    return 0;
} 


