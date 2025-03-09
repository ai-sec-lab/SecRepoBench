static inline double ConstrainCoordinate(double coordinate)
{
  if (coordinate < (double) -(SSIZE_MAX-512))
    return((double) -(SSIZE_MAX-512));
  if (coordinate > (double) (SSIZE_MAX-512))
    return((double) (SSIZE_MAX-512));
  return(coordinate);
}