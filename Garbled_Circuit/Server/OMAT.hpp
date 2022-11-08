#ifndef __OMAT_HPP_
#define __OMAT_HPP_
#include <iostream>
#include <algorithm>
#include <zmq.hpp>
#include <string>
#include <random>
#include <vector>
#include <ctime>
// Header file for Authenticated Garbling Circuit
#include <emp-tool/emp-tool.h>
#include <emp-agmpc/emp-agmpc.h>

// Header file for building Bristol Fashion circuits
#include "Build_Circuit.hpp"

using namespace std;

const int nP                      = 3;
const int MAX_NUM_CIRCUITS        = 8;
const int MAX_CIRCUITS_PER_THREAD = 1;

block (*GTM[MAX_NUM_CIRCUITS])[4][nP+1];
block (*GTK[MAX_NUM_CIRCUITS])[4][nP+1];
bool  (*GTv[MAX_NUM_CIRCUITS])[4];
block (*GT[MAX_NUM_CIRCUITS])[nP+1][4][nP+1];
block *labels[MAX_NUM_CIRCUITS];
bool  *value[MAX_NUM_CIRCUITS];
block *(eval_labels[MAX_NUM_CIRCUITS])[nP+1];
int   value_temp;

class OMAT
{
    public:
        NetIOMP<nP>           *(ios[MAX_NUM_CIRCUITS])[2];

        ThreadPool            *pool[MAX_NUM_CIRCUITS/MAX_CIRCUITS_PER_THREAD];

        zmq::context_t        **context_req;
        zmq::socket_t         **socket_req;
        
        zmq::context_t        **context_rep;
        zmq::socket_t         **socket_rep;

        // These circuits are used for processing keywords (rows)
        BristolFormat         *cf_circuit;
        CMPC<nP>*             mpc_evaluate_circuit[MAX_NUM_CIRCUITS]; 

        int                   party;
        int                   time_step_word;
        int                   time_step_file;

        int                   *deepest_blocks;
        int                   *deepest_levels;
        int                   *target;
        int                   *full_path;
        int                   *deepest;
        char                  *available;

        int                   num_ands;
        double                online_time;
        double                offline_time;

        size_t                data_size_per_thread;

        OMAT(int party, int port);
        ~OMAT();

        void initialize(int word_block_size, int file_block_size);
        void test_eval_circuit(uint8_t **in, uint8_t **out);        
};

OMAT::OMAT(int party, int port)
{
    this->party = party;
    
    // Initialize timestep for eviction
    time_step_word = 0;
    time_step_file = 0;

    // ios for circuits
    for(int i = 0; i < MAX_NUM_CIRCUITS/MAX_CIRCUITS_PER_THREAD; ++i)
    {
        ios[i][0] = new NetIOMP<nP>(party, port);
        ios[i][1] = new NetIOMP<nP>(party, port + 2*(nP+1)*(nP+1)+1);
        
        ios[i][0]->flush();
        ios[i][1]->flush();

        // Initialize thread pool 
        pool[i] = new ThreadPool(nP+1);
    }
}

void OMAT::initialize(int num_and_gates, int num_xor_gates)
{        
    // Circuits
    data_size_per_thread = ((num_and_gates + num_xor_gates)<<1)/MAX_NUM_CIRCUITS;

    cout << "Building circuit..." << endl;
    uint64_t max_num_wires = build_sample_circuit(num_and_gates/MAX_NUM_CIRCUITS, num_xor_gates/MAX_NUM_CIRCUITS, "circuit.txt");
    cf_circuit = new BristolFormat("circuit.txt");
    
    uint64_t max_num_and_gates = 0;
    for(int i = 0; i < cf_circuit->num_gate; ++i) 
    {
        if (cf_circuit->gates[4*i+3] == AND_GATE)
            ++max_num_and_gates;
    }
    
    if(party == 1) 
    {
        for(int t = 0; t < MAX_NUM_CIRCUITS; ++t) 
            GTM[t] = new block[max_num_and_gates][4][nP+1];

        for(int t = 0; t < MAX_NUM_CIRCUITS; ++t) 
            GTK[t] = new block[max_num_and_gates][4][nP+1];

        for(int t = 0; t < MAX_NUM_CIRCUITS; ++t) 
            GTv[t] = new bool[max_num_and_gates][4];

        for(int t = 0; t < MAX_NUM_CIRCUITS; ++t) 
            GT[t] = new block[max_num_and_gates][nP+1][4][nP+1];
    }

    for(int t = 0; t < MAX_NUM_CIRCUITS; ++t)
        labels[t] = new block[max_num_wires];
    
    for(int t = 0; t < MAX_NUM_CIRCUITS; ++t)
        value[t] = new bool[max_num_wires];

    for(int t = 0; t < MAX_NUM_CIRCUITS; ++t)
        for(int i = 1; i <= nP; ++i)
            eval_labels[t][i] = new block[max_num_wires];
}

OMAT::~OMAT()
{
    // delete [] pool;
    // delete [] ios;

    // delete cf_circuit;
    delete mpc_evaluate_circuit; 
}

void OMAT::test_eval_circuit(uint8_t **in, uint8_t **out)
{
    cout << "Preprocessing sample circuit..." << endl;
    
    for(int t = 0; t < MAX_NUM_CIRCUITS; ++t)
    {
        int p = t/MAX_CIRCUITS_PER_THREAD;
        mpc_evaluate_circuit[t] = new CMPC<nP>(ios[p], pool[p], party, cf_circuit, &num_ands, GTM[t], 
                                               GTK[t], GTv[t], GT[t], labels[t], value[t], eval_labels[t]); 
        mpc_evaluate_circuit[t]->function_independent();
        ios[p][0]->flush();
        ios[p][1]->flush();
        mpc_evaluate_circuit[t]->function_dependent();
        ios[p][0]->flush();
        ios[p][1]->flush(); 
    }

    vector<future<void>> res;
    ThreadPool pool(MAX_NUM_CIRCUITS/MAX_CIRCUITS_PER_THREAD);

    auto start = clock_start();
    
	for(int t = 0; t < MAX_NUM_CIRCUITS; t += MAX_CIRCUITS_PER_THREAD) 
    {
        res.push_back(pool.enqueue([this, in, out, t]() 
		{
            auto start = clock_start();
            for(int v = t; v < t + MAX_CIRCUITS_PER_THREAD; ++v) 
            {
                mpc_evaluate_circuit[v]->online(in[v], out[v], num_ands);
                ios[v/MAX_CIRCUITS_PER_THREAD][0]->flush();
                ios[v/MAX_CIRCUITS_PER_THREAD][1]->flush();
            }
            if(party == 1)
                cout << "Time thread #" << (t/MAX_CIRCUITS_PER_THREAD) << ": " << time_from(start) << endl;
        }));
    }
    
    joinNclean(res);

    if(party == 1)
        cout << "Online time: " << time_from(start) << endl;

    for(int t = 0; t < MAX_NUM_CIRCUITS; ++t)
        delete mpc_evaluate_circuit[t];
}

#endif 
