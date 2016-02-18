/**********************************************************************                                                                                                 
 *                                                                                                                                                                      
 * PA CWK3
 * Ewan Leaver s0800696
 * Main Simulator File
 *                                                                                                                                                                      
 **********************************************************************/

#include <cstdlib>
#include <iostream> // std::cout
#include <fstream>
#include <sstream>
#include <vector>

#include <iomanip> // For cout alignment
#include <string>  // std::string, std::stoi

#include "proc.h"

#define verbose 1

using namespace std;

ifstream trace;

// Global inputs
int lineCount;      // Global, No. lines in cach
int lineSize;       // Global, Size of cache lines
int writeBufSize;   // Global, Size of the write buffer
int retireAtN;      // Global, Retire-at-N
int TSO;

static vector<Line> mem;   // Records global MSI data

static Processor P0;
static Processor P1;
static Processor P2;
static Processor P3;

int maxUsedLine = -1; // Used for calculating some global stats, as mem has no clear size limit

Processor* currProc;

// Write function was moved out of the readTrace() function to simplify the structure somewhat.
// The Read function remains within readTrace()
void write(string proc, int addr, bool stalled) {

    int lineNo;
    int offset;
    
    int procInt;
    
    lineNo = addr/lineSize; // Equivalent line number within memory
    offset = addr%lineSize; // word offset within line
    
    // This simpilicifies some later loops and constructs
    if (proc == "P0") {
        currProc = &P0; // Pointer to the actual processor
        procInt = 0;    // References the processor as an int, useful for arrays
    } else if (proc == "P1") {
        currProc = &P1;
        procInt = 1;
    } else if (proc == "P2") {
        currProc = &P2;
        procInt = 2;
    } else if (proc == "P3") {
        currProc = &P3;
        procInt = 3;
    } else {
        return;
    }
    
    int temp;
    
    if (stalled) {
        // Latencies only accumulated if stalled (or at end of trace file)
        (*currProc).latency += 2; // Cache Access - Always Occurs
    }
    
    if (mem[lineNo].state <= 0) { // Line is shared, unused, or used and invalidated
        
        if (mem[lineNo].state == 0) {
            // Currently SHARED -> invalidate in all other processors, then claim the line
            if (stalled) {
                (*currProc).latency += 220;
            }
            
            if (proc == "P0") {
                if (mem[lineNo].shared[1]) {
                    P1.setState(addr,-1);
                    mem[lineNo].shared[1] = 0;
                }
                if (mem[lineNo].shared[2]) {
                    P2.setState(addr,-1);
                    mem[lineNo].shared[2] = 0;
                }
                if (mem[lineNo].shared[3]) {
                    P3.setState(addr,-1);
                    mem[lineNo].shared[3] = 0;
                }
                mem[lineNo].shared[0] = 0;
            } else if (proc == "P1") {
                if (mem[lineNo].shared[0]) {
                    P0.setState(addr,-1);
                    mem[lineNo].shared[0] = 0;
                }
                if (mem[lineNo].shared[2]) {
                    P2.setState(addr,-1);
                    mem[lineNo].shared[2] = 0;
                }
                if (mem[lineNo].shared[3]) {
                    P3.setState(addr,-1);
                    mem[lineNo].shared[3] = 0;
                }
                mem[lineNo].shared[1] = 0;
            } else if (proc == "P2") {
                if (mem[lineNo].shared[0]) {
                    P0.setState(addr,-1);
                    mem[lineNo].shared[0] = 0;
                }
                if (mem[lineNo].shared[1]) {
                    P1.setState(addr,-1);
                    mem[lineNo].shared[1] = 0;
                }
                if (mem[lineNo].shared[3]) {
                    P3.setState(addr,-1);
                    mem[lineNo].shared[3] = 0;
                }
                mem[lineNo].shared[2] = 0;
            } else if (proc == "P3") {
                if (mem[lineNo].shared[0]) {
                    P0.setState(addr,-1);
                    mem[lineNo].shared[0] = 0;
                }
                if (mem[lineNo].shared[1]) {
                    P1.setState(addr,-1);
                    mem[lineNo].shared[1] = 0;
                }
                if (mem[lineNo].shared[2]) {
                    P2.setState(addr,-1);
                    mem[lineNo].shared[2] = 0;
                }
                mem[lineNo].shared[3] = 0;
            }
        } else {
            // Not in any cache, need to access mem
            if (stalled) {
                (*currProc).latency += 220; // Bus Latency + Mem Access
            }
        }
        
        mem[lineNo].state = 1; // Set line as MODIFIED
        
        if (proc == "P0") {
            mem[lineNo].modified = 0;
            P0.store(addr);
        } else if (proc == "P1") {
            mem[lineNo].modified = 1;
            P1.store(addr);
        } else if (proc == "P2") {
            mem[lineNo].modified = 2;
            P2.store(addr);
        } else if (proc == "P3") {
            mem[lineNo].modified = 3;
            P3.store(addr);
        }
        
    } else if (mem[lineNo].state == 1) {
        // Already MODIFIED. If owned by another processor, need to gain control of the line.
        if (mem[lineNo].modified == procInt) {
            // Safe - just perform write.
            if (stalled) {
                (*currProc).latency += 200; // Bus Latency + Mem Access
            }
            (*currProc).store(addr);
            
        } else {
            // Owned by another processor - first need to invalidate, and then claim the line
            if (stalled) {
                (*currProc).latency = (*currProc).latency + 220; // bus + mem latency
            }
            
            if (mem[lineNo].modified == 0) {
                P0.setState(addr,-1);
            } else if (mem[lineNo].modified == 1) {
                P1.setState(addr,-1);
            } else if (mem[lineNo].modified == 2) {
                P2.setState(addr,-1);
            } else if (mem[lineNo].modified == 3) {
                P3.setState(addr,-1);
            }
            
            mem[lineNo].modified = procInt;
            (*currProc).store(addr);
            
        }
    }
}

void readTrace() {
    
    string proc;
    string accType;
    string addrStr;
    int addr;
    int lineNo;
    int offset;
    
    bool foundBuf = false;
    int procInt;
    
    // read each line of the file
    while (!trace.eof()) {
        proc = "VOID";
        
        // Read each trace line
        trace >> proc;
        trace >> accType;
        trace >> addrStr;
        addr = atoi(addrStr.c_str()); // Convert address value to int
        
        lineNo = addr/lineSize; // Equivalent line number within memory
        offset = addr%lineSize; // word offset within line
        
        if (lineNo > maxUsedLine) {
            maxUsedLine = lineNo; // Update to find largest used line number
        }
        
        if (mem[lineNo].accState == -1) {
            // If line is unused so far
            mem[lineNo].accState = 0;           // Make into private line
            mem[lineNo].privateOwner = proc;    // Claim the line
        } else if (mem[lineNo].accState == 0) {
            // Line is private
            if (mem[lineNo].privateOwner != proc) {
                // If line doesn't belong to requesting processor
                if (accType == "R") {
                    mem[lineNo].accState = 1; // Change to Shared Read-Only
                } else {
                    mem[lineNo].accState = 2; // Change to Shared Read-Write, if Write operation
                }
                mem[lineNo].privateOwner = "NULL"; // Line is no longer a Private line
            } // else nothing - processor already is private owner
        }
        
        mem[lineNo].noAccesses++;
        
        // Check if processor has accessed this address before.
        // If not, add itself to the list of accessers for the given address (stored within the line object)
        bool found = false;
        
        for (int i = 0; i < mem[lineNo].accesses[offset].size(); i++) {
            if (mem[lineNo].accesses[offset][i] == proc) {
                found = true;
            }
        }
        
        if (!found) {
            mem[lineNo].accesses[offset].push_back(proc);
        } else {
            // Address's accesses already contains proc, do nothing
        }
        
        // This broken on DICE, replaced with the above code.
        /*if(std::find(mem[lineNo].accesses[offset].begin(), mem[lineNo].accesses[offset].end(), proc) != mem[lineNo].accesses[offset].end()) {
            // Address's accesses already contains proc, do nothing
        } else {
            mem[lineNo].accesses[offset].push_back(proc);
        }*/
        
        if (proc == "P0") {
            currProc = &P0;
            procInt = 0;
        } else if (proc == "P1") {
            currProc = &P1;
            procInt = 1;
        } else if (proc == "P2") {
            currProc = &P2;
            procInt = 2;
        } else if (proc == "P3") {
            currProc = &P3;
            procInt = 3;
        } else {
            break;
        }
        
        if (accType == "R") {
            
           foundBuf = false;
            
            for (int i = 0; i < (*currProc).writeBuffer.size(); i++) {
                // Check all elements in the local writeBuffer.
                if ((*currProc).writeBuffer[i] == addr) {
                    foundBuf = true;
                    // To maintain the hit rate...
                    (*currProc).readHits++;
                    (*currProc).readAccesses++;
                    (*currProc).totalHits++;
                    (*currProc).totalAccesses++;
                    (*currProc).bypassCount++;
                    break;
                }
            }

            if (foundBuf) {
                (*currProc).latency++; // Read By-Pass. Latency = 1 cycle
                if (verbose == 3) {
                    cout << proc << "   R   Addr: " << setw(6) << addr << "  ***** READ BYPASS *****                            Latency:   " << (*currProc).latency << "\n";
                }
                
            } else {
                
                if (mem[lineNo].state == 1) {
                    // Line is MODIFIED -> make SHARED
                    
                    if (mem[lineNo].modified == procInt) {
                        // Line modifed by proc, DO NOT need bus transaction
                        (*currProc).latency += 2; // Local Cache Access
                    } else {
                        (*currProc).latency += 22; // Cache Access + Bus transaction;
                    }
                    
                    if (mem[lineNo].modified == 0) {
                        P0.setState(addr, 0); // Make the modifier's local cache line SHARED too
                        mem[lineNo].shared[0] = true;
                    } else if (mem[lineNo].modified == 1) {
                        P1.setState(addr, 0);
                        mem[lineNo].shared[1] = true;
                    } else if (mem[lineNo].modified == 2) {
                        P2.setState(addr, 0);
                        mem[lineNo].shared[2] = true;
                    } else if (mem[lineNo].modified == 3) {
                        P3.setState(addr, 0);
                        mem[lineNo].shared[3] = true;
                    }
                    mem[lineNo].modified = -1; // Line no longer has a modifying processing
                }
                
                if (mem[lineNo].state == 0) {
                    // Already shared
                    if (mem[lineNo].shared[procInt]) {
                        // Shared by current processor, no bus transaction required
                        (*currProc).latency += 2; // Local Cache Access
                    } else {
                        (*currProc).latency += 22; // bus transaction;
                    }
                    
                } else if (mem[lineNo].state < 0) {
                    // need to read from memory
                    (*currProc).latency += 222;
                }
                
                mem[lineNo].state = 0; // Line -> SHARED
                
                // Set the
                if (proc == "P0") {
                    mem[lineNo].shared[0] = true;
                    P0.load(addr);
                } else if (proc == "P1") {
                    mem[lineNo].shared[1] = true;
                    P1.load(addr);
                } else if (proc == "P2") {
                    mem[lineNo].shared[2] = true;
                    P2.load(addr);
                } else if (proc == "P3") {
                    mem[lineNo].shared[3] = true;
                    P3.load(addr);
                }
            }
            
        } else if (accType == "W") {
            if (TSO) {
                if (verbose == 3) {
                    cout << "PUSHING ADDR " << addr << " ONTO WRITEBUFFER FOR PROC " << proc << "\n";
                }
                (*currProc).writeBuffer.push_back(addr);
            } else {
                // If SC, just perform the write. Marked as 'stalled' to ensure latencies are accumulated.
                write(proc, addr, 1);
            }
        }
        if (TSO) {
            if ((*currProc).writeBuffer.size() == writeBufSize) {
                
                // STALL AND FLUSH
                if (verbose == 3) {
                    cout << "***     FLUSHING WRITEBUFFER FOR " << proc << "\n";
                }
                // Remove all elements from the buffer and process in turn.
                while ((*currProc).writeBuffer.size() > 0) {
                    write(proc, (*currProc).writeBuffer[0], true); // true indicates stalling
                    (*currProc).writeBuffer.erase((*currProc).writeBuffer.begin()); // Remove the first element
                }
                
                (*currProc).writing = false;
                
            } else if ((*currProc).writeBuffer.size() >= retireAtN) {
                
                if ((*currProc).writing) {
                    if (((*currProc).writeStart + (*currProc).writeDuration) < (*currProc).latency) {
                        // Actual local latency has caught up with the scheduled completion of the write
                        // - perform the write
                        if (verbose == 3) {
                            cout << "***     COMPLETED WRITE OF ADDR " << (*currProc).writeBuffer[0] << " FOR " << proc << ". Latency: " << (*currProc).latency << "\n";
                        }
                        write(proc, (*currProc).writeBuffer[0], false);
                        (*currProc).writeBuffer.erase((*currProc).writeBuffer.begin()); // Remove the first element
                        (*currProc).writing = false;
                        
                        if ((*currProc).writeBuffer.size() >= retireAtN) {
                            (*currProc).writing = true;
                            
                            (*currProc).writeStart = (*currProc).latency;
                            
                            if (mem[lineNo].state <= 0) {
                                // Currently shared or invalid - need to access cache, bus and mem
                                (*currProc).writeDuration = 222;
                            } else {
                                // Modified
                                if (mem[lineNo].modified == procInt) {
                                    // Owned by processor, no need to make bus transaction
                                    (*currProc).writeDuration = 202;
                                } else {
                                    // Need to access cache, bus and mem
                                    (*currProc).writeDuration = 222;
                                }
                            }
                            if (verbose == 3) {
                                cout << "***     STARTING WRITE OF ADDR " << (*currProc).writeBuffer[0] << " FOR " << proc << ".    Current Latency: " << (*currProc).latency << ",    Completed by: " << ((*currProc).writeStart + (*currProc).writeDuration) << "\n";
                            }
                            
                        }
                        
                    }
                } else {
                    (*currProc).writing = true;
                    
                    (*currProc).writeStart = (*currProc).latency;
                    
                    if (mem[lineNo].state <= 0) {
                        // Currently shared or invalid - need to access cache, bus and mem
                        (*currProc).writeDuration = 222;
                    } else {
                        // Modified
                        if (mem[lineNo].modified == procInt) {
                            // Owned by processor, no need to make bus transaction
                            (*currProc).writeDuration = 202;
                        } else {
                            // Need to access cache, bus and mem
                            (*currProc).writeDuration = 222;
                        }
                    }
                    if (verbose == 3) {
                        cout << "***     STARTING WRITE OF ADDR " << (*currProc).writeBuffer[0] << " FOR " << proc << ".    Current Latency: " << (*currProc).latency << ",    Completed by: " << ((*currProc).writeStart + (*currProc).writeDuration) <<"\n";
                    }
                }
            } else if ((*currProc).writing) {
                // Incase writing, but writebuffer size is smaller than retireAtN
                
                if (((*currProc).writeStart + (*currProc).writeDuration) < (*currProc).latency) {
                    // Actual local latency has caught up with the scheduled completion of the write
                    // - perform the write
                    if (verbose == 3) {
                        cout << "***     COMPLETED WRITE OF ADDR " << (*currProc).writeBuffer[0] << " FOR " << proc << ". Latency: " << (*currProc).latency << "\n";
                    }
                    
                    write(proc, (*currProc).writeBuffer[0], false);
                    (*currProc).writeBuffer.erase((*currProc).writeBuffer.begin()); // Remove the first element
                    (*currProc).writing = false;
                }
            }
            
            if (verbose == 2) {
                cout << "    Line state: " << setw(2) << mem[lineNo].state << "\n";
                cout << "    Processors sharing the line (from P0 to P3, 1 indicates shared): ";
                for (int i = 0; i < 4; i++) {
                    if (mem[lineNo].shared[i]) {
                        cout << "[1] ";
                    } else {
                        cout << "[0] ";
                    }
                }
                cout << "\n";
                if (mem[lineNo].state == 1) {
                    cout << "    Processor modifying line: P" << mem[lineNo].modified << "\n";
                }
                cout << "\n";
            }
        }
    }

    if (P0.writing) {
        // Incase still writing
        P0.latency = (P0.writeStart + P0.writeDuration);
        cout << "***     COMPLETED WRITE OF ADDR " << P0.writeBuffer[0] << " FOR P0. Latency: " << P0.latency << "\n";
        
        write("P0", P0.writeBuffer[0], false);
        P0.writeBuffer.erase(P0.writeBuffer.begin()); // Remove the first element
        P0.writing = false;
    }
    if (P1.writing) {
        // Incase still writing
        P1.latency = (P1.writeStart + P1.writeDuration);
        cout << "***     COMPLETED WRITE OF ADDR " << P1.writeBuffer[0] << " FOR P1. Latency: " << P1.latency << "\n";
        
        write("P1", P1.writeBuffer[0], false);
        P1.writeBuffer.erase(P1.writeBuffer.begin()); // Remove the first element
        P1.writing = false;
    }
    if (P2.writing) {
        // Incase still writing
        P2.latency = (P2.writeStart + P2.writeDuration);
        cout << "***     COMPLETED WRITE OF ADDR " << P2.writeBuffer[0] << " FOR P2. Latency: " << P2.latency << "\n";
        
        write("P2", P2.writeBuffer[0], false);
        P2.writeBuffer.erase(P2.writeBuffer.begin()); // Remove the first element
        P2.writing = false;
    }
    if (P3.writing) {
        // Incase still writing
        P3.latency = (P3.writeStart + P3.writeDuration);
        cout << "***     COMPLETED WRITE OF ADDR " << P3.writeBuffer[0] << " FOR P3. Latency: " << P3.latency << "\n";
        
        write("P3", P3.writeBuffer[0], false);
        P3.writeBuffer.erase(P3.writeBuffer.begin()); // Remove the first element
        P3.writing = false;
    }
    
    trace.close();
    
}

void clearBuffers() {
    
    while (P0.writeBuffer.size() > 0) {
        write("P0", P0.writeBuffer[0], true); // true indicates stalling
        P0.writeBuffer.erase(P0.writeBuffer.begin()); // Remove the first element
    }
           
    while (P1.writeBuffer.size() > 0) {
        write("P1", P1.writeBuffer[0], true); // true indicates stalling
        P1.writeBuffer.erase(P1.writeBuffer.begin()); // Remove the first element
    }
           
    while (P2.writeBuffer.size() > 0) {
        write("P2", P2.writeBuffer[0], true); // true indicates stalling
        P2.writeBuffer.erase(P2.writeBuffer.begin()); // Remove the first element
    }
        
    while (P3.writeBuffer.size() > 0) {
        write("P3", P3.writeBuffer[0], true); // true indicates stalling
        P3.writeBuffer.erase(P3.writeBuffer.begin()); // Remove the first element
    }
}

int main(int argc, char **argv) {
    
    /*if (argc > 1)  {
        //trig.loadFile(argv[1]);
    }
    else {
        cerr << argv[0] << " <filename> <lineSize> <number_lines>" << endl;
        exit(1);
    }*/

    trace.open(argv[1]); // open a file
    if (!trace.is_open()) {
        cerr << "Error.\n";
        exit(1); // exit if file not found
    }

    
    cout << "\n************************************************************************************************************\n";
    cout << "**                                        Starting Cache Simulator                                        **\n";
    cout << "************************************************************************************************************\n\n";
    
    cout << "Trace file: " << argv[1] << "\n\n";
    
    std::stringstream in1(argv[2]);
    in1 >> lineCount;
    std::stringstream in2(argv[3]);
    in2 >> lineSize;
    std::stringstream in3(argv[4]);
    in3 >> writeBufSize;
    std::stringstream in4(argv[5]);
    in4 >> retireAtN;
    std::stringstream in5(argv[6]);
    in5 >> TSO;
    
    // -1 = invalid, 0 = shared, 1 = modified
    mem.resize(20000, 4); 
    
    P0.init("P0",lineCount,lineSize);
    P1.init("P1",lineCount,lineSize);
    P2.init("P2",lineCount,lineSize);
    P3.init("P3",lineCount,lineSize);
    
    readTrace();
    
    if (TSO == 1) {
        clearBuffers();
    }
    
    // Print out stats
    
    if (verbose == 1) {
        P0.stats();
        P1.stats();
        P2.stats();
        P3.stats();
        
        cout << " -----------------------------------------------------------------------------------------------------------\n\n";
        // Global statistics
        
        float privateCount = 0;
        float sharedReadOnlyCount = 0;
        float sharedReadWriteCount = 0;
        float totalAccesses = 0;
        
        for (int i = 0; i <= maxUsedLine; i++) {
            if (mem[i].accState == 0) {
                // Private Line
                privateCount = privateCount + mem[i].noAccesses;
            } else if (mem[i].accState == 1) {
                // Shared Read-Only Line
                sharedReadOnlyCount = sharedReadOnlyCount + mem[i].noAccesses;
            } else if (mem[i].accState == 2) {
                // Shared Read-Write Line
                sharedReadWriteCount = sharedReadWriteCount + mem[i].noAccesses;
            }
        }
        
        totalAccesses = privateCount + sharedReadOnlyCount + sharedReadWriteCount;
        
        float a1procCount = 0; // accessed by one processor
        float a2procCount = 0; // accessed by two processors
        float a3procCount = 0; // accessed by three or more processors
        float memAddrCount = 0; // total number of accessed mem locations
        
        for (int i = 0; i <= maxUsedLine; i++) {
            if (mem[i].noAccesses > 0) {
                // If the line has actually been accessed...
                for (int j = 0; j < lineSize; j++) {
                    if (mem[i].accesses[j].size() == 1) {
                        a1procCount++;
                    } else if (mem[i].accesses[j].size() == 2) {
                        a2procCount++;
                    } else if (mem[i].accesses[j].size() > 2) {
                        a3procCount++;
                    }
                }
            }
        }
        
        memAddrCount = a1procCount + a2procCount + a3procCount;
        
        cout << "    *** GLOBAL STATISTICS ***\n\n";
        cout << "    Private ratio:            " << setw(8) << (privateCount/totalAccesses) << "\n";
        cout << "    Shared Read-Only ratio:   " << setw(8) << (sharedReadOnlyCount/totalAccesses) << "\n";
        cout << "    Shared Read-Write ratio:  " << setw(8) << (sharedReadWriteCount/totalAccesses) << "\n\n";
        
        cout << "    Memory locations accessed by 1 processor:             " << setw(8) << (a1procCount/memAddrCount) << "\n";
        cout << "    Memory locations accessed by 2 processors:            " << setw(8) << (a2procCount/memAddrCount) << "\n";
        cout << "    Memory locations accessed by more than 2 processors:  " << setw(8) << (a3procCount/memAddrCount) << "\n";
    }
    
    cout << "\n";
    
    return 0;
}