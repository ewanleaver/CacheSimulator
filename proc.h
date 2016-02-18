/**********************************************************************                                                                                                 
 *                                                                                                                                                                      
 * PA CWK2
 * Ewan Leaver s0800696
 *                                                                                                                                                                      
 **********************************************************************/

#include <vector>
#include <string>  // std::string, std::stoi
#include "line.h"

class Processor {
    
    private:
        std::string name;
    
    public:
    
        int lineCount;
        int lineSize;
        std::vector<std::vector<double> > cache;
        std::vector<Line> msiData;
        std::vector<double> writeBuffer;
    
        float readHits;
        float readMisses;
        float readAccesses;
        float readRate;
    
        float writeHits;
        float writeMisses;
        float writeAccesses;
        float writeRate;
    
        float totalHits;
        float totalMisses;
        float totalAccesses;
        float totalRate;
    
        float coherenceMisses;
        float coherenceRate;
    
        int invalidations;
    
        double latency;
    
        int bypassCount;
    
        bool writing;
    
        int writeStart;
        int writeDuration;
    
        // Methods
        int getState(int);
        void setState(int, int);
        void load(int);
        void store(int);
        void stats();
        void init(std::string,int,int);
    
};