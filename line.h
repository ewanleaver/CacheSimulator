/**********************************************************************                                                                                                 
 *                                                                                                                                                                      
 * PA CWK2
 * Ewan Leaver s0800696
 *                                                                                                                                                                      
 **********************************************************************/

#include <vector>
#include <string>  // std::string, std::stoi

class Line {
    
    private:
    
    public:
        int size; // Line size in words
        std::vector<std::vector<std::string> > accesses; //store accesses to individual addresses
    
        int state; // Available states: -2 = unused, -1 = invalid, 0 = shared, 1 = modified

        Line(int lineSize);
    
        void setState(int s);
        int getState();
    
        bool shared [4]; // Flag is set to true if corresponding processor is sharing the line
        int modified;    // Values: -1 = invalid, 0 = P0, 1 = P1, 2 = P2, 3 = P3

        int accState; // 0 = private, 1 = shared read-only, 2 = shared read-write
        std::string privateOwner; // If line is private, which class does it belong to
        float noAccesses;
};