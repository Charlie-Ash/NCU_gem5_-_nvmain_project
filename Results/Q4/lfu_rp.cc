/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Daniel Carvalho
 */

#include "mem/cache/replacement_policies/lfu_rp.hh"

#include <cassert>
#include <memory>

#include "params/LFURP.hh"

// Define global timestamp static member
uint64_t LFURP::globalTimestamp = 0;

LFURP::LFURP(const Params *p)
    : BaseReplacementPolicy(p)
{
}

void
LFURP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    // Reset reference count and timestamp
    std::static_pointer_cast<LFUReplData>(replacement_data)->refCount = 0;
    std::static_pointer_cast<LFUReplData>(replacement_data)->timestamp = 0;
}

void
LFURP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update reference count and timestamp
    std::static_pointer_cast<LFUReplData>(replacement_data)->refCount++;
    std::static_pointer_cast<LFUReplData>(replacement_data)->timestamp = ++globalTimestamp;
}

void
LFURP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Reset reference count and set timestamp
    std::static_pointer_cast<LFUReplData>(replacement_data)->refCount = 1;
    std::static_pointer_cast<LFUReplData>(replacement_data)->timestamp = ++globalTimestamp;
}

ReplaceableEntry*
LFURP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Initialize with the first candidate
    ReplaceableEntry* victim = candidates[0];
    auto victim_data = std::static_pointer_cast<LFUReplData>(victim->replacementData);
    unsigned min_ref_count = victim_data->refCount;
    uint64_t oldest_timestamp = victim_data->timestamp;

    // Find entry with minimum reference count and oldest timestamp through all candidates
    for (const auto& candidate : candidates){
        
        auto data = std::static_pointer_cast<LFUReplData>(candidate->replacementData);
        if (data->refCount < min_ref_count){

            // Lower reference count found, update victim
            victim = candidate;
            min_ref_count = data->refCount;
            oldest_timestamp = data->timestamp;

        }else if (data->refCount == min_ref_count && data->timestamp < oldest_timestamp){

            // Same reference count, but older timestamp, update victim
            victim = candidate;
            oldest_timestamp = data->timestamp;

        }
        
    }

    return victim;
}

std::shared_ptr<ReplacementData>
LFURP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new LFUReplData());
}

LFURP*
LFURPParams::create()
{
    return new LFURP(this);
}
