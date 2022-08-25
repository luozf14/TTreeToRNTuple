#include "SimpleClass.h"
void SimpleClass::SetInt(int i)
{
    fInt = i;
}

void SimpleClass::SetFloat(float f)
{
    fFloat = f;
}

void SimpleClass::SetVecDouble(std::vector<double> d)
{
    fVecDouble = d;
}

void SimpleClass::SetVecVecFloat(std::vector<std::vector<float> > ff)
{
    fVecVecFloat = ff;
}