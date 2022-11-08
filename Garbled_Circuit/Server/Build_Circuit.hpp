#include <fstream>

using    namespace std; 

uint64_t build_sample_circuit(int num_and_gates, int num_xor_gates, string file_name)
{
    ofstream ofs;
    ofs.open(file_name);
    
    int num_gates = num_and_gates + num_xor_gates;
    int num_wires = (num_gates<<1) + num_gates;
    
    int start_p1  = 0;
    int start_p2  = num_and_gates + num_xor_gates;
    int start_out = num_gates<<1;
    
    ofs << num_gates << " " << num_wires << endl;
    ofs << num_gates << " " << num_gates << " " << num_gates << endl;

    for(int i = 0; i < num_and_gates; ++i)
        ofs << 2 << " " << 1 << " " << start_p1++ << " " << start_p2++ << " " << start_out++ << " AND" << endl;

    for(int i = 0; i < num_xor_gates; ++i)
        ofs << 2 << " " << 1 << " " << start_p1++ << " " << start_p2++ << " " << start_out++ << " XOR" << endl;

    ofs.close();
    return num_wires;    
}
