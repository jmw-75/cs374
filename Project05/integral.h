/* integral.h declares integral, a function to calculate
 *  definite integrals using the trapezoidal rule.
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 * Changes made by John White to add two more args to the integrateTrap() function
 *    while at Calvin University on 10/20/21 for project05 in cs374
 *    High Performence Computing
 */

long double integrateTrap(double xLo, double xHi,
                          unsigned long long numTrapezoids,
                          unsigned long long start, unsigned long long stop, int ID );

