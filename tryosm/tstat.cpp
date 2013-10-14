#include "tstat.h"
//#include "Defines.h"
#include "stdlib.h"

int compareAsDouble (const void * a, const void * b)
{
  if ( *(double*)a <  *(double*)b ) return -1;
  if ( *(double*)a >  *(double*)b ) return 1;
  return 0;
}


void TStatistic::incopy(const TStatistic &src)
{
  values = src.values;
  sum = src.sum;
  min = src.min;
  max = src.max;
  disp = src.disp;
  dispActual = src.dispActual;
}

double TStatistic::square(double x)
{
  return (x)*(x);
}

double TStatistic::count()
{
  return (values.size() > 0 ? values.size() : 1.0);
}

TStatistic::TStatistic()
{
  values.clear();
  sum = 0.0;
  disp = 0.0;
  dispActual = true;
}

vector <double> TStatistic::getSorted()
{
  vector <double> res = values;
  qsort((void*)&res[0], res.size(), sizeof(double), compareAsDouble);
//  res.resize(values.size());
//  for(size_t i = 0; i < res.size() - 1; i++)
//  {
//    size_t jmin = i;
//    for(size_t j = i + 1; j < res.size(); j++)
//    {
//        jmin = ((res[jmin] > res[j]) ? j : jmin);
//    }
//    if (jmin != i)
//    {
//      double d = res[i];
//      res[i] = res[jmin];
//      res[jmin] = d;
//    }
//  }
//  double d = res[16];
  
  return res;
}

vector <double> TStatistic::getZones(unsigned int n)
{
  vector <double> res, sorted;
  sorted = getSorted();
  if (sorted.size() <= 0) return res;
  res.resize(n + 1);
  int j = 0;
  for (unsigned int i = 0; i < sorted.size(); i += sorted.size() / n)
  {
//    if ((i % n) == 0)
//    {
//    LOG("sorted["<<i<<"] = "<<sorted[i]);
      res[j++] = sorted[i];
//    }
  }
//  LOG("sorted[" << int(sorted.size() - 1) << "] = " << (double)sorted[sorted.size() - 1]);
  res[res.size() - 1] = sorted[sorted.size() - 1];
  return res;
}

double TStatistic::Median(double proportion)
{
  if (values.size()==0) return 0.0;
  vector <double> tmp = getSorted();
  return tmp[int(tmp.size() * proportion)];

//  if (tmp.size()%2 == 0)
//  {
//    return (values[tmp.size()/2] + values[tmp.size()/2-1]) * 0.5;
//  }
//  else
//  {
//    return values[tmp.size()/2];
//  }
}

TStatistic::TStatistic(const TStatistic &src)
{
  incopy(src);
}



TStatistic & TStatistic::operator = (const TStatistic & src)
{
  incopy(src);
  return *this;
}

TStatistic & TStatistic::operator << (double x)
{
  push(x);
  return *this;
}

TStatistic & TStatistic::operator << (vector <double> x)
{
  for(size_t i = 0; i < x.size(); i++) push(x[i]);
  return *this;
}

TStatistic & TStatistic::operator << (const TStatistic &src)
{
  for(size_t i = 0; i < src.values.size(); i++) push(src.values[i]);
  return *this;
}

void TStatistic::push(double x)
{
  min = ((values.size()==0) || (min > x) ? x : min);
  max = ((values.size()==0) || (max < x) ? x : max);
  values.push_back(x);
  sum += x;
  dispActual = false;
}

double TStatistic::Maximum()
{
  return max;
}

double TStatistic::Minimum()
{
  return min;
}

double TStatistic::Average()
{
  return (double)sum/count();
}

double TStatistic::ExpectedValue()
{
  return Average();
}

double TStatistic::Sum()
{
  return sum;
}

double TStatistic::SumOfSquaredDeviations(double medium)
{
  disp = 0;
  for(size_t i = 0; i < values.size(); i++)
  {
    disp += square(values[i] - medium);
  }

  return disp/count();
}

double TStatistic::Dispersion()
{
  if (dispActual) return disp;

  dispActual = true;
  return SumOfSquaredDeviations(Average());
}

double TStatistic::StdDeviation(double value)
{
  return sqrt(SumOfSquaredDeviations(value));
}

double TStatistic::StdDeviation()
{
  return sqrt(Dispersion());
}

size_t TStatistic::Count()
{
  return values.size();
}

double TStatistic::operator [] (size_t i)
{
  return values[i];
}
