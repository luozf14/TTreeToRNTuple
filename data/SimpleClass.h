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
        void SetVecVecFloat(std::vector<std::vector<float> >);
        int GetInt(){return fInt;};
        float GetFloat(){return fFloat;};
        std::vector<double> GetVecDouble(){return fVecDouble;};
        std::vector<std::vector<float> > GetVecVecFloat(){return fVecVecFloat;};
    private:
        int fInt;
        float fFloat;
        std::vector<double> fVecDouble;
        std::vector<std::vector<float> >fVecVecFloat;
};

#endif