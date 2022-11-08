#ifndef __SERVER_HPP_
#define __SERVER_HPP_
#include <iostream>
#include <algorithm>
#include <zmq.hpp>
#include <string>
#include <random>
#include <vector>
#include <ctime>
// Header file for OMAT 
#include "OMAT.hpp"

using namespace std;

const int SERVER_PORT           = 8080;

random_device rd;
mt19937       mt(rd());

uint8_t       **in;
uint8_t       **out;

class Server
{
    public:
        zmq::context_t        *context;
        zmq::socket_t         *socket;

        OMAT                  *omat_sample;
        int                   party;

        Server(char **server_info);
        ~Server();
        
        void initialize_ORAM(int num_and_gates, int num_xor_gates);
        void process();
};

Server::Server(char **args)
{
    int party, port;
    parse_party_and_port(args, &party, &port);   
    this->party = party;

    cout << "Party: " << party << " Port: " << port << endl;

    omat_sample = new OMAT(party, port);
    
    context = new zmq::context_t(1);
    socket  = new zmq::socket_t(*context, ZMQ_REP);
    
    socket->bind("tcp://*:" + to_string(SERVER_PORT+2*(party-1)*nP+(party-1)));
}

Server::~Server()
{
    // delete omat_sample;
}

void Server::initialize_ORAM(int num_and_gates, int num_xor_gates)
{   
    cout << "#AND Gates: " << num_and_gates << endl;
    cout << "#XOR Gates: " << num_xor_gates << endl;

    omat_sample->initialize(num_and_gates, num_xor_gates);
    
    cout << "INITIALIZING DATA SAMPLE HAS BEEN DONE!!!" << endl;
        
    // Buffer for MPC
    in   = new uint8_t*[MAX_NUM_CIRCUITS];
    out  = new uint8_t*[MAX_NUM_CIRCUITS];

    for(int i = 0; i < MAX_NUM_CIRCUITS; ++i)
    {
        in[i]   = new uint8_t[omat_sample->data_size_per_thread>>3];
        out[i]  = new uint8_t[omat_sample->data_size_per_thread>>4];
    }
}

void Server::process()
{
    PRG prg;

    for(int i = 0; i < 10; ++i)
    {
        // Generate random data
        for(int j = 0; j < MAX_NUM_CIRCUITS; ++j)
            prg.random_data(in[j], omat_sample->data_size_per_thread>>3);

        // Run circuit Evaluation
        omat_sample->test_eval_circuit(in, out); 
    }
}

#endif
