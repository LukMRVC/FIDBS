//
// Created by lukas on 25.02.22.
//

#ifndef HEAPTABLE_UTILS_H
#define HEAPTABLE_UTILS_H
double GetThroughput(int opsCount, double period, int unit);

double GetThroughput(int opsCount, double  period, int unit) {
    return (((double )opsCount / (double )unit)) / period;
}
#endif //HEAPTABLE_UTILS_H
