#ifndef SimpleClass_H
#define SimpleClass_H
#include<vector>

class SimpleClass
{
    public:
        SimpleClass(){};
    
        void SetInt(int);
        void SetFloat(float);
        void SetVecDouble(std::vector<double>);
        int GetInt(){return fInt;};
        float GetFloat(){return fFloat;};
        std::vector<double> GetDouble(){return fVecDouble;};
        
    private:
        int fInt;
        float fFloat;
        std::vector<double> fVecDouble;
};

#endif