This is the (hopefully detailed enough) tutorial to the final project to NCU CSIE's Computer Organization final project.

## Q1 GEM5 + NVMAIN BUILD-UP
---
(basically just follow the tutorial)
* Ubuntu 18.04 
*  Others settings 
    *  Memory space:4GB
    * Disk space :20GB

Install build tools 
```
sudoapt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python3-dev python3-six python libboost-all-dev pkg-config
```
Install GEM 5 (sign in, and click `tgz` to download)
```
https://gem5.googlesource.com/public/gem5/+/525ce650e1a5bbe71c39d4b15598d6c003cc9f9e
```
Move the downloaded file to HOME
![alt text](image.png)
Compile GEM 5, cd into the downloaded (and extracted) GEM 5 file.
```
scons build/X86/gem5.opt -j 4  // This took a while to compile
```
Git install NVMain
* `git clone https://github.com/SEAL-UCSB/NVmain` into HOME
* Nvmainand gem5should be in the /Home together
 
Modify sconscript
* Enter the nvmain folder, click on SConscript, and comment out the 36 lines of `from gem5_scons import Transform` (remember to archive, (I guess they're telling me to make an extra copy of this file before modifying it))
```
charlie@charlie-VirtualBox:~$ cd NVmain/
charlie@charlie-VirtualBox:~/NVmain$ cp SConscript SConscript_original
```
Compile NVMain
* In ./Nvmain, Type: `scons --build-type=fast`

Modify GEM5 Options
* In `gem5/configs/common/Options.py` Add the next program in line 133 (remember to save it)
```
for arg in sys.argv:
    if arg[:9] == "--nvmain-":
        parser.add_option(arg, type="string", default="NULL", help="Set NVMain configuration value for a parameter")
```
![alt text](image-1.png)

Recover Sconscript
* Restore the previous instructions annotated by nvmain sconscript

Compile GEM5
* Compile GEM5 with mixed NVMAIN in the GEM5 directory
* `scons EXTRAS=../NVmain build/X86/gem5.opt`

Hello World
```
./build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --caches --l2cache --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

Output
* You can check the number of cache hits from stat.txt in gem5/m5out

Active Energy
* After running the program, you can see it in terminal.

## Q2 ENABLE L3 LAST LEVEL CACHE in GEM5 + NVMAIN
---
* The files that need to be modified are in the gem5 folder:
    * Options.py
    * Caches.py
    * Xbar.py
    * BaseCPU.py
    * CacheConfig.py
* The first four files only increase the parameters of the L3 cache, and you can imitate them according to the settings of the L2 cache.
* CacheConfig.py needs to connect the L3 cache to the entire Gem5 system. Pay attention to the relationship between the L2 and L3 caches. The system can only use the L3 cache when the L2 cache is already used. This condition is not met.
* For the details of the code, students can go online to find resource keywords like Gem5 L3 cache.

Reference: https://home.gamer.com.tw/artwork.php?sn=5874623

After changing (or mostly adding) the code in the reference above, configure again with this
`scons build/X86/gem5.opt -j 4`

Run a similar command to Q1
```
./build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

Check the difference in stats.txt from Q1, we'll see that an extra L3 cache now has its info as well.
## Q3 Config last level cache to 2-way and full-way associative cache and test performance
---
* You need to run the benchmark Quicksort in 2-way and full-way
	* Create a *benchmark* file that contains the quicksort code
* Quicksort code
```c
#define SWAP(x,y) {int t; t = x; x = y; y = t;} 

void quickSort(int number[], int left, int right) { 
    if(left < right) { 
        int s = number[(left+right)/2]; 
        int i = left - 1; 
        int j = right + 1; 

        while(1) { 
            while(number[++i] < s) ;   
            while(number[--j] > s) ;   
            if(i >= j) 
                break; 
            SWAP(number[i], number[j]);  
        } 

        quickSort(number, left, i-1);   
        quickSort(number, j+1, right);  
    } 
}

int main(){ 

	int arr[500000];
	int size = 500000;
	for (int i = 0; i < size; i++){
		arr[i] = rand() % size + 1;
    }
	quickSort(arr, 0, size-1); 

}
```
* Compile quicksort.c `gcc --static quicksort.c -o quicksort
* Use specific sizes for the caches in your command
```
L1i -> 32kB
L1d -> 32kB
L2 -> 128kB
L3 -> 1MB
```
#### 2-way associative L3 cache
(`--l3_assoc=2`)
```
./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --l3_assoc=2 --l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```
#### full-way associative L3 cache
(`--l3_assoc=0`, or  `--l3_assoc=16384` since 0 is illegal, and 1 MB L3 cache = 16384 cache lines, meaning `--l3_assoc=16384` makes it so that there's only 1 set)
```
./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --l3_assoc=16384 --l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

From results, we observe that the total miss rate (found in stats.txt) of the full-way associative cache is higher than that of 2-way associative cache. Usually, it should be the contrary, since having one large set would reduce conflict misses. 

But since the array size in quick sort is 500,000, being around 2MB, way larger than the L3 cache that was given, there would instead be MORE conflict misses for a full-way associative cache.

## Q4 Modify last level cache policy based on frequency based replacement policy
---
**REMEMBER: recompile files every time files in gem5 is edited**
`scons build/X86/gem5.opt -j 4`

Currently, the replacement policy is LRU (least-recently used). Our goal is to change it to LFU (least-frequently used)

We can see the replacement policies in `/src/mem/cache/replacement_policies/`

We change the replacement policy of the L3 cache in `/configs/common/Caches.py` to LFU
```python
class L3Cache(Cache):
    assoc = 64
    tag_latency = 32
    data_latency = 32
    response_latency = 32
    mshrs = 32
    tgts_per_mshr = 24
    write_buffers = 16
    replacement_policy = Param.BaseReplacementPolicy(LFURP(),"Replacement policy")
```

Change `/src/mem/cache/replacement_policies/lfu_rp.hh` header file
```c++
class LFURP : public BaseReplacementPolicy
{
protected:
/** LFU-specific implementation of replacement data. */
struct LFUReplData : ReplacementData
{
/** Number of references to this entry since it was reset. */
unsigned refCount;

/** Timestamp of the last access to this entry. */
uint64_t timestamp;  

/**
* Default constructor. Invalidate data.
*/
LFUReplData() : refCount(0), timestamp(0) {}
};

// Global counter to simulate time for timestamp assignment.
static uint64_t globalTimestamp;
......
```
Change `/src/mem/cache/replacement_policies/lfu_rp.cc`
```c++
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
```

A final comparison between the original (LRU) and LFU
* LRU:  `system.l3.replacements                          91436`
* LFU:  `system.l3.replacements                         113179`
This (is probably) because of quick sort tend to go through
- Recursive accesses
- Scanning partitions
- Temporary locality
- Shifting working sets
This indicates that data are used more recently, and less likely frequently. This is why policies such as LFU doesn't reward as much, and creates more cache replacements than LRU in this case.

## Q5 Test the performance of write back and write through policy based on 4-way associative cache with isscc_pcm
---
* You need to run the benchmark multiply in write through and write back
* You can go to the `mem/cache` folder to find two files, one is called `cache.cc` and the other is called `base.cc`. You can study how the underlying code of gem5 writes data into Memory
* Multiply code
```c

#define SIZE 300
int main()
{
 	int size = SIZE;

	int A[SIZE][SIZE],B[SIZE][SIZE],C[SIZE][SIZE];
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			A[i][j] = rand() % size + 1;
			B[i][j] = rand() % size + 1;
		}
    }
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			C[i][j] = 0;
			for (int k = 0; k< size ; k++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
    }
	
}
```
* Compile multiply.c `gcc --static multiply.c -o multiply`
* Write through VS Write back
	* **Write Through**: Every write updates memory immediately. Simple, consistent, but may cause many memory writes.
	* **Write Back**: Only updates the cache first, the memory will only be updated once the cache line is evicted. Fewer memory writes.
* gem5's default is write back!!

To change the code to write through, at around line `1049`, change part of the code as such:
```c
        // at this point either this is a writeback or a write-through
        // write clean operation and the block is already in this
        // cache, we need to update the data and the block flags
        assert(blk);
        // TODO: the coherent cache can assert(!blk->isDirty());
        if (!pkt->writeThrough()) {
            blk->status |= BlkDirty;
        }
        // nothing else to do; writeback doesn't expect response
        assert(!pkt->needsResponse());
        pkt->writeDataToBlock(blk->data, blkSize);
        DPRINTF(Cache, "%s new state is %s\n", __func__, blk->print());

        incHitCount(pkt);
        // populate the time when the block will be ready to access.
        blk->whenReady = clockEdge(fillLatency) + pkt->headerDelay +
            pkt->payloadDelay;
        // if this a write-through packet it will be sent to cache
        // below
        return !pkt->writeThrough();
    } else if (blk && (pkt->needsWritable() ? blk->isWritable() :
                       blk->isReadable())) {
        // OK to satisfy access
        incHitCount(pkt);
        satisfyRequest(pkt, blk);
        maintainClusivity(pkt->fromCache(), blk);
	// add this if statement to switch to write through
	// if (blk->isWritable()) {
           // PacketPtr writeclean_pkt = writecleanBlk(blk, pkt->req->getDest(), pkt->id);
           // writebacks.push_back(writeclean_pkt);
        // }
        return true;
    }
```

**REMEMBER: recompile files every time files in gem5 is edited**
`scons build/X86/gem5.opt -j 4`

Test the difference between write back and write through using this command:
`./build/X86/gem5.opt configs/example/se.py -c ../benchmark/multiply --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --l3_assoc=4 --l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config`
(Remember boys, 4-way associative cache)

Result (in logs):
* Write Back: `i0.defaultMemory.totalWriteRequests 692`
* Write Through: `i0.defaultMemory.totalWriteRequests 13906254`