/**********************************************************************                                                                                                 
 *                                                                                                                                                                      
 * PA CWK2
 * Ewan Leaver s0800696
 * Processor Class File
 *                                                                                                                                                                      
 **********************************************************************/

#include "proc.h"

#include <cstdlib>
#include <iostream> // std::cout
#include <fstream>
#include <sstream>
#include <vector>
//#include <thread>
#include <iomanip> // For cout alignment


#define verbose 0

using namespace std;

int Processor::getState(int address) {
    
    int index = (address/lineSize)%lineCount; // Direct Mapped
    return msiData[index].state;
    
}

void Processor::setState(int address, int s) {
    
    int index = (address/lineSize)%lineCount; // Direct Mapped
    
    msiData[index].state = s;
    
    if (s == -1) {
        // If line is being invalidated, increment
        invalidations++;
    }
}

void Processor::load(int address) {
    // Load from memory
    
    int offset = address%lineSize;            // Word within the line
    int index = (address/lineSize)%lineCount; // Direct Mapped, which line within cache
    int tag = (address/(lineCount*lineSize));
    
    totalAccesses++;
    readAccesses++;
    
    string res = "BLANK";
        
    // If line is used but invalid, counts as a coherence miss.
    if (msiData[index].state == -1) {
        coherenceMisses++;
    }
    
    if (msiData[index].state < 1) {
        // Only set to SHARED if not already MODIFIED
        msiData[index].state = 0;
    }
    
    if (cache[index][offset] != address) {
        for (int i = 0; i < lineSize; i++) {
            // Populates the cache line with the occupying memory addresses
            cache[index][i] = index*lineSize + tag*lineCount*lineSize + i;
            //cout << P1cache[block][i] << " ";
        }
        totalMisses++;
        readMisses++;
        res = "miss";
    } else {
        totalHits++;
        readHits++;
        res = "hit";
    }
    
    if (verbose == 1) {
        cout << name << "   R   Addr: " << setw(6) << address << "  Tag: " << setw(4) << tag << "  Index: " << setw(4) << index << "  Offset: " << setw(4) << offset << "  Res:  " << res << "\n";
    } else if (verbose == 2) {
        cout << name << "   R   Addr: " << setw(6) << address << "  Tag: " << setw(4) << tag << "  Index: " << setw(4) << index << "  Offset: " << setw(4) << offset << "  Res:  " << res << "   Latency:   " << latency << "\n";
    }
}

void Processor::store(int address) {
    // Load from memory
    
    int offset = address%lineSize;            // Word offset within line
    int index = (address/lineSize)%lineCount; // Direct Mapped
    int tag = (address/(lineCount*lineSize));
    
    totalAccesses++;
    writeAccesses++;
    
    // If used but invalid, counts as a coherence miss.
    if (msiData[index].state == -1) {
        coherenceMisses++;
    }
    
    string res = "BLANK";
    
    if (cache[index][offset] != address) {
        for (int i = 0; i < lineSize; i++) {
            // Populates the cache line with the occupying memory addresses
            cache[index][i] = index*lineSize + tag*lineCount*lineSize + i;
        }
        totalMisses++;
        writeMisses++;
        res = "miss";
    } else {
        if (msiData[index].state == 0) {
            // Write to a shared line results in a miss
            totalMisses++;
            writeMisses++;
            res = "miss";
        } else {
            totalHits++;
            writeHits++;
            res = "hit";
        }
    }
    
    msiData[index].state = 1; // Will always become MODIFIED
    
    if (verbose == true) {
        cout << name << "   W   Addr: " << setw(6) << address << "  Tag: " << setw(4) << tag << "  Index: " << setw(4) << index << "  Offset: " << setw(4) << offset << "  Res:  " << res << "\n";
    } else if (verbose == 2) {
        cout << name << "   W   Addr: " << setw(6) << address << "  Tag: " << setw(4) << tag << "  Index: " << setw(4) << index << "  Offset: " << setw(4) << offset << "  Res:  " << res << "   Latency:   " << latency << "\n";
    }
}

void Processor::stats(){
    
    cout << " -----------------------------------------------------------------------------------------------------------\n";

    totalRate = totalMisses/totalAccesses;
    readRate = readMisses/readAccesses;
    writeRate = writeMisses/writeAccesses;
    
    coherenceRate = coherenceMisses/totalMisses;
    
    cout << " " << name << " - totalAccesses:  " << setw(8) << totalAccesses << "    totalHits:  " << setw(8) << totalHits
         << "  ||  Hit rate:       " << setw(8) << (1-totalRate) << "    Miss rate:  " << setw(8) << totalRate << "\n\n";
    
    cout << "      readAccesses:   " << setw(8) << readAccesses << "    readHits:   " << setw(8) << readHits
         << "  ||  Hit rate:       " << setw(8) << (1-readRate) << "    Miss rate:  " << setw(8) << readRate << "\n\n";
    
    cout << "      writeAccesses:  " << setw(8) << writeAccesses << "    writeHits:  " << setw(8) << writeHits
         << "  ||  Hit rate:       " << setw(8) << (1-writeRate) << "    Miss rate:  " << setw(8) << writeRate << "\n\n";
    
    cout << "      Total misses:   " << setw(8) << totalMisses << "    Coherence Misses: " << setw(8) << coherenceMisses
         << "      Coherence miss rate:  " << setw(8) << coherenceRate << "\n";
    cout << "      Invalidations:  " << setw(8) << invalidations  << "      Latency:  " << setw(20) << latency << "        Read ByPasses: " << setw(8) << bypassCount << "\n";
}

// Initialisation of processor
void Processor::init(string n, int count, int size) {
    
    name = n;
    
    lineCount = count;
    lineSize = size;
    latency = 0;
    
    writing = false;
    
    cache.resize(lineCount, vector<double>(lineSize , -1));
    msiData.resize(lineCount, 4);
    
}