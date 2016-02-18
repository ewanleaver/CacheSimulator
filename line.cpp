/**********************************************************************                                                                                                 
 *                                                                                                                                                                      
 * PA CWK2
 * Ewan Leaver s0800696
 * Line Class File
 *                                                                                                                                                                      
 **********************************************************************/

#include "line.h"

using namespace std;

Line::Line (int lineSize) {
    
    // Initialisation of line
    
    size = lineSize;        // Initialise line size
    accesses.resize(size);  // Array to record which processors have accessed each mem address within the line
    
    // Available states: -2 = unused, -1 = invalid, 0 = shared, 1 = modified
    state = -2;
    // Values for 'modified': -1 = invalid, 0 = P0, 1 = P1, 2 = P2, 3 = P3
    modified = -1;
    accState = -1;  // 0 = private, 1 = shared read-only, 2 = shared read-write
    noAccesses = 0; // Number of times line has been accessed
}

void Line::setState(int s) {
    state = s;
}

int Line::getState() {
    return state;
}