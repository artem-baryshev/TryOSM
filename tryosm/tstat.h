#ifndef TSTAT_H
#define TSTAT_H

#include <vector>
#include <math.h>
#include <ostream>

using namespace std;

class TStatistic
{
  vector <double> values;
  double sum, min, max, disp;
  bool dispActual;
  void incopy(const TStatistic &src);
  double count();
  void push(double x);
public:
  double square(double x);
  TStatistic();
  TStatistic(const TStatistic &src);
  TStatistic & operator = (const TStatistic & src);
  double Maximum();
  double Minimum();
  double Average();
  double ExpectedValue();
  double Median(double proportion = 0.5);
  double Sum();
  double SumOfSquaredDeviations(double medium);
  double Dispersion();
  double StdDeviation(double value);
  double StdDeviation();
  size_t Count();
  void clear() {values.clear(); sum = 0.0; dispActual = false;}
  double operator [] (size_t i);
  TStatistic & operator << (double x);
  TStatistic & operator << (vector <double> x);
  TStatistic & operator << (const TStatistic &src);
  vector <double> getSorted();

  vector <double> getZones(unsigned int n);

  friend ostream & operator << (ostream &os, TStatistic &src)
  {
    os << "{ ";
    for(size_t i = 0; i < src.Count(); i++)
        os << src[i] << " ";
    os << "}\nCount = " << src.Count()
       << "\nSum = " << src.Sum()
       << "\nAvg = " << src.Average()
       << "\nStdDev = " << src.StdDeviation()
          ;
    return os;
  }
};



#endif // TSTAT_H
