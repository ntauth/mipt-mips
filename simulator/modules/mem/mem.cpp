/**
 * mem.cpp - Simulation of memory stage
 * Copyright 2015-2018 MIPT-MIPS
 */


#include "mem.h"

static constexpr const uint32 FLUSHED_STAGES_NUM = 3;

template <typename ISA>
Mem<ISA>::Mem( bool log) : Log( log)
{
    wp_datapath = make_write_port<Instr>("MEMORY_2_WRITEBACK", PORT_BW, PORT_FANOUT);
    rp_datapath = make_read_port<Instr>("EXECUTE_2_MEMORY", PORT_LATENCY);

    wp_flush_all = make_write_port<bool>("MEMORY_2_ALL_FLUSH", PORT_BW, FLUSHED_STAGES_NUM);
    rp_flush = make_read_port<bool>("MEMORY_2_ALL_FLUSH", PORT_LATENCY);

    wp_flush_target = make_write_port<Addr>("MEMORY_2_FETCH_TARGET", PORT_BW, PORT_FANOUT);
    wp_bp_update = make_write_port<BPInterface>("MEMORY_2_FETCH", PORT_BW, PORT_FANOUT);

    wp_bypass = make_write_port<RegDstUInt>("MEMORY_2_EXECUTE_BYPASS", PORT_BW, SRC_REGISTERS_NUM);

    wp_bypassing_unit_flush_notify = make_write_port<Instr>("MEMORY_2_BYPASSING_UNIT_FLUSH_NOTIFY", 
                                                            PORT_BW, PORT_FANOUT);
}


template <typename ISA>
void Mem<ISA>::clock( Cycle cycle)
{
    sout << "memory  cycle " << std::dec << cycle << ": ";

    /* receieve flush signal */
    const bool is_flush = rp_flush->is_ready( cycle) && rp_flush->read( cycle);

    /* branch misprediction */
    if ( is_flush)
    {
        /* drop instruction as it is invalid */
        if ( rp_datapath->is_ready( cycle))
        {
            const auto& instr = rp_datapath->read( cycle);
            
            /* notifying bypassing unit about invalid instruction */
            wp_bypassing_unit_flush_notify->write( instr, cycle);
        }

        sout << "flush\n";
        return;
    }

    /* check if there is something to process */
    if ( !rp_datapath->is_ready( cycle))
    {
        sout << "bubble\n";
        return;
    }

    auto instr = rp_datapath->read( cycle);

    if ( instr.is_jump()) {
        /* acquiring real information for BPU */
        wp_bp_update->write( instr.get_bp_upd(), cycle);
        
        /* handle misprediction */
        if ( instr.is_misprediction())
        {
            /* flushing the pipeline */
            wp_flush_all->write( true, cycle);

            /* sending valid PC to fetch stage */
            wp_flush_target->write( instr.get_new_PC(), cycle);
            sout << "misprediction on ";
        }
    }

    /* perform required loads and stores */
    memory->load_store( &instr);
    
    /* bypass data */
    wp_bypass->write( instr.get_bypassing_data(), cycle);

    wp_datapath->write( instr, cycle);

    /* log */
    sout << instr << std::endl;
}


#include <mips/mips.h>
#include <risc_v/risc_v.h>

template class Mem<MIPS>;
template class Mem<RISCV32>;
template class Mem<RISCV64>;
template class Mem<RISCV128>;

