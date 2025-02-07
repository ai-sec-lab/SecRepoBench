static inline double ConstrainCoordinate(double x)
{
  // Ensure that the coordinate `x` is constrained within a specified range to avoid overflow issues. 
  // If `x` exceeds the maximum allowable value, it is set to the maximum value. If it is below the minimum allowable value, it is set to the minimum value. 
  // This helps maintain the stability and correctness of geometric calculations.
  // <MASK>
  return(x);
}