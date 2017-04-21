/*
 * perf_sim.cpp - mips performance simulator
 * Copyright 2015 MIPT-MIPS
 */

#ifndef PERF_SIM_H
#define PERF_SIM_H

#include <iostream>
#include <sstream>
#include <iomanip>

#include <common/log.h>
#include <common/ports/ports.h>

#include <func_sim/func_instr/func_instr.h>
#include <func_sim/func_memory/func_memory.h>
#include <func_sim/func_sim.h>

#include "perf_sim_rf.h"

class PerfMIPS : protected Log
{
private:
    cycles_t executed_instrs = 0;
    cycles_t last_writeback_cycle = 0; // to handle possible deadlocks

    /* the struture of data sent from fetch to decode stage */
    struct IfIdData {
        bool predicted_taken;    // Predicted direction
        addr_t predicted_target; // PC, predicted by BPU
        addr_t PC;               // current PC
        uint32 raw;              // fetched instruction code
    };

    /* decode stage variables */
    IfIdData decode_data;
    bool is_anything_to_decode = false;


    /* simulator units */
    RF rf;
    addr_t PC = NO_VAL32;
    FuncMemory* mem = nullptr;

    /* MIPS functional simulator for internal checks */
    MIPS checker;
    void check( const FuncInstr& instr);

    /* all ports */
    std::unique_ptr<WritePort<uint32>> wp_fetch_2_decode = nullptr;
    std::unique_ptr<ReadPort<uint32>> rp_fetch_2_decode = nullptr;
    std::unique_ptr<WritePort<bool>> wp_decode_2_fetch_stall = nullptr;
    std::unique_ptr<ReadPort<bool>> rp_decode_2_fetch_stall = nullptr;

    std::unique_ptr<WritePort<FuncInstr>> wp_decode_2_execute = nullptr;
    std::unique_ptr<ReadPort<FuncInstr>> rp_decode_2_execute = nullptr;

    std::unique_ptr<WritePort<FuncInstr>> wp_execute_2_memory = nullptr;
    std::unique_ptr<ReadPort<FuncInstr>> rp_execute_2_memory = nullptr;


    std::unique_ptr<WritePort<FuncInstr>> wp_memory_2_writeback = nullptr;
    std::unique_ptr<ReadPort<FuncInstr>> rp_memory_2_writeback = nullptr;

    std::unique_ptr<WritePort<bool>> wp_memory_2_all_flush = nullptr;
    std::unique_ptr<ReadPort<bool>> rp_memory_2_all_flush = nullptr;

    std::unique_ptr<WritePort<addr_t>> wp_memory_2_fetch_target = nullptr;
    std::unique_ptr<ReadPort<addr_t>> rp_memory_2_fetch_target = nullptr;

    /* main stages functions */
    void clock_fetch( int cycle);
    void clock_decode( int cycle);
    void clock_execute( int cycle);
    void clock_memory( int cycle);
    void clock_writeback( int cycle);

    /* forbid copies */ 
    PerfMIPS& operator=( const PerfMIPS&) = delete;
    PerfMIPS( const PerfMIPS&) = delete;

public:
    PerfMIPS( bool log);
    void run( const std::string& trace,
              uint64 instrs_to_run);
    ~PerfMIPS();
};

#endif

